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

#ifndef XCSOAR_ANDROID_NOOK_HPP
#define XCSOAR_ANDROID_NOOK_HPP

#include "Language/Language.hpp"

namespace Nook {

/**
 * Controls the Nook's charge level so it gently charges or
 * discharges the battery.
 * Does this by keeping battery level within a range around the
 * battery percent on startup
 * The difficulty is that if we set the charge rate too high,
 * the Nook's USB and IOIO connections appear less reliable
 * Strives to keep it the battery level under 100% but near fully charged
 * while issuing as few commands as possible to the chip
 */
class BatteryController
{
  /**
   * the rate we told the Nook we want to charge (mA)
   */
  unsigned last_charge_rate;

  /**
   * last value of charge_percent read from device
   */
  unsigned last_charge_percent;

  /**
   * upper limit of range where we want to maintain the battery level
   */
  unsigned upper_battery_threshhold;

  bool initialised;

public:

  BatteryController()
    :last_charge_rate(0), initialised(false) {};

  /**
   * @param value. percent remaining in battery
   */
  void ProcessChargeRate(unsigned value);

  /**
   * @param value. percent remaining in battery
   */
  void Initialise(unsigned value);

  bool IsInitialised() {
    return initialised;
  }

  /**
   * true if we most recently told the Nook to Charge at 500mA or higher
   * (does not detect actual Nook hardware setting)
   */
  bool IsCharging() {
    return last_charge_rate > 100;
  }

protected:
  unsigned GetUpperChargeThreshhold() {
    return upper_battery_threshhold;
  }

  unsigned GetLowerChargeThreshhold() {
      return upper_battery_threshhold - 2;
  }

  unsigned GetLowerFastChargeThreshhold() {
      return upper_battery_threshhold - 20;
  }

  /**
   * Set charge rate to 1.5A
   */
  void SetFastCharging();

  void SetCharging();

  void SetDischarging();

  /**
  * Sets IsCharging status based on whether actual voltage of battery is
  * increasing or decreasing.
  * i.e. if user changes charge rate using external utility
  */
  void DetectIfActuallyCharging(unsigned value);
};

  /**
   * initialize USB mode in Nook (must be rooted with USB Kernel)
   */
  void InitUsb();


  /**
   * initialize the internal TTL port on the Nook's mother board as
   * /dev/ttyS1 (must be rooted with USB Kernel)
   * Access to this port requires soldering to the Nook's mother board
   */
  void InitInternalUsb();

  /**
   * Enter FastMode to eliminate full refresh of screen
   * requires Nook kernel rooted to support FastMode
   *
   * @return false if the operation has failed, e.g. because the
   * kernel does not support the mode
   */
  bool EnterFastMode();

  /**
   * Exit FastMode to restore full (slow) refresh of screen
   * requires Nook kernel rooted to support FastMode
   */
  void ExitFastMode();

  /**
   * Set Nook regulator's charge rate to 1.5A.
   */
  void SetCharge1500();

  /**
   * Set Nook regulator's charge rate to 500mA.
   */
  void SetCharge500();

  /**
   * Set Nook regulator's charge rate to 100mA.
   */
  void SetCharge100();


#define GetNookUsbHostDriverName() "Nook ST USB Host"

  const char* GetUsbHostDriverHelp();
  const char* GetUsbHostDriverPath();

}

#endif
