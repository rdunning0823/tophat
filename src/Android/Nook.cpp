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

#include "Android/Nook.hpp"
#include <stdlib.h>

static char cmd_sleep[] = "sleep 0.5";
static char cmd_host[] = "su -c 'echo host > /sys/devices/platform/musb_hdrc/mode'";
static char cmd_usb_rw[] = "su -c 'chmod 666 /dev/ttyUSB0'";
//static char cmd_fast_mode_enter[] = "/data/scripts/fastmode1.sh";
//static char cmd_fast_mode_exit[] = "/data/scripts/fastmode0.sh";
static char cmd_fast_mode_enter[] = "echo 1 > /sys/class/graphics/fb0/fmode";
static char cmd_fast_mode_exit[] = "echo 0 > /sys/class/graphics/fb0/fmode";
static char cmd_epd_refresh_0[] = " echo 0 > /sys/class/graphics/fb0/epd_refresh";
static char cmd_epd_refresh_1[] = " echo 1 > /sys/class/graphics/fb0/epd_refresh";
static char cmd_set_charge_500[] = "su -c 'echo 500000 > /sys/class/regulator/regulator.5/device/force_current'";

void
Nook::EnterFastMode()
{
  system(cmd_epd_refresh_0);
  system(cmd_sleep);
  system(cmd_sleep);
  system(cmd_epd_refresh_1);
  system(cmd_fast_mode_enter);
}

void
Nook::ExitFastMode()
{
  system(cmd_fast_mode_exit);
  system(cmd_epd_refresh_0);
  system(cmd_sleep);
  system(cmd_epd_refresh_1);
}

void
Nook::InitUsb()
{
  system(cmd_host);
  system(cmd_sleep);

  system(cmd_host);
  system(cmd_sleep);

  system(cmd_usb_rw);
}

void
Nook::SetCharge500()
{
  system(cmd_set_charge_500);
}
