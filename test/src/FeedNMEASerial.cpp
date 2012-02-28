/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

/*
 * This program creates one or two serial ports: one for normal NMEA,
 * and optionally one for FLARM data.
 * The input is a single FLARM NMEA file from stdin.
 * It feeds the NMEA contents stripped of the Flarm records to the first
 * serial port.
 * It feeds the entire NMEA file to the optional second serial port.
 * It can be used to simulate two serial inputs to XCSoar, one for smart vario
 * data and one for Flarm data.
 */

#include "OS/PathName.hpp"



#ifdef HAVE_POSIX
#include "Device/Port/TTYPort.hpp"
#else
#include "Device/Port/SerialPort.hpp"
#endif

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  if (argc != 5 && argc != 3) {
    fprintf(stderr, "Usage: %s PORT BAUD [PORT_FLARM] [BAUD_FLARM] < 'nmea file name'\n", argv[0]);
    return EXIT_FAILURE;
  }

  const bool sync_flarm = argc == 5 ? true : false;

  PathName port_name(argv[1]);
  int baud = atoi(argv[2]);
  PathName port_name_flarm(sync_flarm ? argv[3] : "");
  int baud_flarm = sync_flarm ? atoi(argv[4]) : 0;


#ifdef HAVE_POSIX
  TTYPort *port = new TTYPort(port_name, baud, *(Port::Handler *)NULL);
  TTYPort *port_flarm = NULL;
  if (sync_flarm)
    port_flarm = new TTYPort(port_name_flarm, baud_flarm,
                             *(Port::Handler *)NULL);
#else
  SerialPort *port = new SerialPort(port_name, baud, *(Port::Handler *)NULL);
  SerialPort *port_flarm = NULL;
  if (sync_flarm)
    port_flarm = new SerialPort(port_name_flarm, baud_flarm,
                                *(Port::Handler *)NULL);
#endif
  if (!port->Open()) {
    delete port;
    fprintf(stderr, "Failed to open first COM port\n");
    return EXIT_FAILURE;
  }

  if (sync_flarm && !port_flarm->Open()) {
    delete port_flarm;
    fprintf(stderr, "Failed to open Flarm COM port\n");
    return EXIT_FAILURE;
  }

  char stamp[6] = "";

  char line[1024];
  while (fgets(line, sizeof(line), stdin) != NULL) {
    if (memcmp(line, "$GP", 3) == 0 &&
        (memcmp(line + 3, "GGA", 3) == 0 ||
         memcmp(line + 3, "RMC", 3) == 0) &&
        line[6] == ',' &&
        strncmp(stamp, line + 7, sizeof(stamp)) != 0) {
      /* the time stamp has changed - sleep for one second */
#ifdef HAVE_POSIX
      sleep(1);
#else
      Sleep(1000);
#endif
      strncpy(stamp, line + 7, sizeof(stamp));
    }

    size_t length = strlen(line);
    // only send non-flarm sentences to the main device
    if (memcmp(line, "$GP", 3) == 0 &&
        memcmp(line + 3, "FLAA", 4) != 0 &&
         memcmp(line + 3, "FLAU", 4) != 0) {
      ssize_t nbytes = port->Write(line, length);
      if (nbytes < 0) {
        perror("Failed to write to port\n");
        delete port;
        delete port_flarm;
        return EXIT_FAILURE;
      }
    }

    // send all sentences to the flarm port
    if (sync_flarm) {
      ssize_t nbytes_flarm = port_flarm->Write(line, length);
      if (nbytes_flarm < 0) {
        perror("Failed to write to Flarm port\n");
        delete port;
        delete port_flarm;
        return EXIT_FAILURE;
      }
    }
  }

  delete port;
  if (sync_flarm)
    delete port_flarm;
  return EXIT_SUCCESS;
}
