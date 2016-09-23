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

#include "BatteryTimer.hpp"
#include "Hardware/Battery.hpp"
#include "UIActions.hpp"
#include "LogFile.hpp"
#include "Simulator.hpp"
#include "Language/Language.hpp"
#include "Message.hpp"
#ifdef ANDROID
#include "Asset.hpp"
#endif

void
BatteryTimer::Process()
{
#ifdef HAVE_BATTERY
  // TODO feature: Trigger a GCE (Glide Computer Event) when
  // switching to battery mode This can be used to warn users that
  // power has been lost and you are now on battery power - ie:
  // something else is wrong

  UpdateBatteryInfo();

  /* Battery status - simulator only - for safety of battery data
     note: Simulator only - more important to keep running in your plane
  */

  // JMW, maybe this should be active always...
  // we don't want the PDA to be completely depleted.

  if (Power::External::Status == Power::External::OFF) {
    if (is_simulator() && Power::Battery::RemainingPercentValid &&
        Power::Battery::RemainingPercent < BATTERY_EXIT) {
      LogFormat("Battery low exit...");
      // TODO feature: Warning message on battery shutdown
      UIActions::SignalShutdown(true);
    } else {
      if (Power::Battery::RemainingPercentValid &&
          Power::Battery::RemainingPercent < BATTERY_WARNING) {
        if (last_warning.CheckUpdate(BATTERY_REMINDER))
          // TODO feature: Show the user what the batt status is.
          Message::AddMessage(_("Battery low"));
      } else {
        last_warning.Reset();
      }
    }
  }
#ifdef ANDROID
  else {
    if (IsNookSimpleTouch()) {
      if (!nook_battery_controller.IsInitialised())
        nook_battery_controller.Initialise(Power::Battery::RemainingPercent);
      else
        nook_battery_controller.ProcessChargeRate(Power::Battery::RemainingPercent);
    }
  }
#endif
#endif /* HAVE_BATTERY */
}
