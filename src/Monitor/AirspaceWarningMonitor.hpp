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

#ifndef XCSOAR_AIRSPACE_WARNING_MONITOR_HPP
#define XCSOAR_AIRSPACE_WARNING_MONITOR_HPP

#include "NMEA/Validity.hpp"
#include "Airspace/ProtectedAirspaceWarningManager.hpp"
#include "Time/BrokenDateTime.hpp"

/**
 * Check for new airspace warnings and show the airspace warning
 * dialog.
 */
class AirspaceWarningMonitor {
  friend class AirspaceWarningWidget;
  class AirspaceWarningWidget *widget;
  bool alarm_active;

  Validity last;
  BrokenDateTime last_alarm_time;

public:
  AirspaceWarningMonitor():widget(nullptr) {
    alarm_active = false;
    last_alarm_time = BrokenDateTime::NowUTC();
  }

  void Reset();
  void Check();
  bool isAlarmActive() const { return alarm_active; }
private:
  void HideWidget();
  void alarmSet();
  void alarmClear() { alarm_active = false; }
  unsigned timeToClosestAirspace(ProtectedAirspaceWarningManager &airspace_warnings);
  void replayAlarmSound(ProtectedAirspaceWarningManager &airspace_warnings);

  void Schedule() {
    last.Clear();
  }
};

#endif
