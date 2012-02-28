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
 * This program creates a serial port and feeds an
 * NMEA data read from stdin to it.
 * It can be used to output to a physical serial port to which a device
 * running XCSoar is attached.
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
  if (argc != 3) {
    fprintf(stderr, "Usage: %s PORT BAUD < 'nmea file name'\n", argv[0]);
    return EXIT_FAILURE;
  }

  PathName port_name(argv[1]);
  int baud = atoi(argv[2]);

#ifdef HAVE_POSIX
  TTYPort *port = new TTYPort(port_name, baud, *(Port::Handler *)NULL);
#else
  SerialPort *port = new SerialPort(port_name, baud, *(Port::Handler *)NULL);
#endif
  if (!port->Open()) {
    delete port;
    fprintf(stderr, "Failed to open COM port\n");
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
    ssize_t nbytes = port->Write(line, length);
    if (nbytes < 0) {
      perror("Failed to write to port\n");
      delete port;
      return EXIT_FAILURE;
    }
  }

  delete port;
  return EXIT_SUCCESS;
}
