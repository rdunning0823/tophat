/*
Copyright_License {

  Top Hat Soaring Glide Computer - http://www.tophatsoaring.org/
  Copyright (C) 2000-2016 The Top Hat Soaring Project
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

#include "Nook.hpp"
#include "OS/FileUtil.hpp"
#include "OS/Sleep.h"

#include <stdlib.h>
#include <assert.h>

static char cmd_host[] = "su -c 'echo host > /sys/devices/platform/musb_hdrc/mode'";
static char cmd_usb_rw[] = "su -c 'chmod 666 /dev/ttyUSB0'";
static char cmd_internal_usb_rw[] = "su -c 'chmod 666 /dev/ttyS1'";

static char cmd_set_charge_1500[] = "su -c 'echo 1500000 > /sys/class/regulator/regulator.5/device/force_current'";
static char cmd_set_charge_500[] = "su -c 'echo 500000 > /sys/class/regulator/regulator.5/device/force_current'";
static char cmd_set_charge_100[] = "su -c 'echo 100000 > /sys/class/regulator/regulator.5/device/force_current'";

bool
Nook::EnterFastMode()
{
  File::WriteExisting("/sys/class/graphics/fb0/epd_refresh", "0");
  Sleep(1000);
  File::WriteExisting("/sys/class/graphics/fb0/epd_refresh", "1");
  return File::WriteExisting("/sys/class/graphics/fb0/fmode", "1");
}

void
Nook::ExitFastMode()
{
  File::WriteExisting("/sys/class/graphics/fb0/fmode", "0");
  File::WriteExisting("/sys/class/graphics/fb0/epd_refresh", "0");
  Sleep(500);
  File::WriteExisting("/sys/class/graphics/fb0/epd_refresh", "1");
}

void
Nook::InitUsb()
{
  system(cmd_host);
  Sleep(500);

  system(cmd_host);
  Sleep(500);

  system(cmd_usb_rw);
}

void
Nook::InitInternalUsb()
{
  system(cmd_internal_usb_rw);
}

void
Nook::SetCharge1500()
{
  system(cmd_set_charge_1500);
}

void
Nook::SetCharge500()
{
  system(cmd_set_charge_500);
}

void
Nook::SetCharge100()
{
  system(cmd_set_charge_100);
}

void
Nook::BatteryController::Initialise(unsigned value) {
  assert(!initialised);

  initialised = true;
  upper_battery_threshhold = 100;
  last_charge_percent = value;
  if (value >= GetUpperChargeThreshhold())
    SetDischarging();
  else
    SetCharging();
}

void
Nook::BatteryController::DetectIfActuallyCharging(unsigned value)
{
  if (value > last_charge_percent && (last_charge_rate == 100))
    last_charge_rate = 500;
  else if (value < last_charge_percent)
    last_charge_rate = 100;
}

void
Nook::BatteryController::ProcessChargeRate(unsigned value)
{
  assert(initialised);

  DetectIfActuallyCharging(value);

  if (value >= GetUpperChargeThreshhold())
    SetDischarging();
  else if (value <= GetLowerFastChargeThreshhold())
    SetFastCharging();
  else
    SetCharging();

  last_charge_percent = value;
}

void
Nook::BatteryController::SetFastCharging()
{
  if (last_charge_rate != 1500) {
    last_charge_rate = 1500;
    Nook::SetCharge1500();
  }
}

void
Nook::BatteryController::SetCharging()
{
  if (last_charge_rate != 500) {
    last_charge_rate = 500;
    Nook::SetCharge500();
  }
}

void
Nook::BatteryController::SetDischarging()
{
  if(last_charge_rate != 100) {
    last_charge_rate = 100;
    Nook::SetCharge100();
  }
}

const char*
Nook::GetUsbHostDriverHelp()
{
  return "Sets Nook to USB Host mode.  This disables IOIO until Nook is REBOOTED!";
}

const char*
Nook::GetUsbHostDriverPath()
{
  return "/dev/ttyUSB0";
}

