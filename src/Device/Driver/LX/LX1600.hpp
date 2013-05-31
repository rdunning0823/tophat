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

#ifndef XCSOAR_DEVICE_DRIVER_LX_LX1600_HPP
#define XCSOAR_DEVICE_DRIVER_LX_LX1600_HPP

#include "Device/Port/Port.hpp"
#include "Device/Internal.hpp"

/**
 * Code specific to LX Navigation varios (e.g. LX1600).
 */
namespace LX1600 {
  /**
   * Enable pass-through mode on the LX1600.  This command was provided
   * by Crtomir Rojnik (LX Navigation) in an email without further
   * explanation.  Tests have shown that this command can be sent at
   * either 4800 baud or the current vario baud rate.  Since both works
   * equally well, we don't bother to switch.
   */
  static bool
  ModeColibri(Port &port, OperationEnvironment &env)
  {
    return PortWriteNMEA(port, "PFLX0,COLIBRI", env);
  }

  /**
   * Cancel pass-through mode on the LX1600.  This command was provided
   * by Crtomir Rojnik (LX Navigation) in an email.  It must always be
   * sent at 4800 baud.  After this command has been sent, we switch
   * back to the "real" baud rate.
   */
  static bool
  ModeLX1600(Port &port, OperationEnvironment &env)
  {
    unsigned old_baud_rate = port.GetBaudrate();
    if (old_baud_rate == 4800)
      old_baud_rate = 0;
    else if (old_baud_rate != 0 && !port.SetBaudrate(4800))
      return false;

    const bool success = PortWriteNMEA(port, "PFLX0,LX1600", env);

    if (old_baud_rate != 0)
      port.SetBaudrate(old_baud_rate);

    return success;
  }

  static bool
  SetupNMEA(Port &port, OperationEnvironment &env)
  {
    /* This line initiates the Color Vario to send out LXWP2 and LXWP3
       LXWP0 once started, is repeated every second.  This is a copy
       of the initiation done in LK8000, realized by LX Navigation
       developers We have no documentation and so we do not know what
       this exactly means. */
    return PortWriteNMEA(port, "PFLX0,LXWP0,1,LXWP2,3,LXWP3,4", env);
  }
}

#endif
