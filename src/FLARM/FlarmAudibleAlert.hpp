/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
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

#ifndef XCSOAR_FLARM_AUDIBLE_ALERT_HPP
#define XCSOAR_FLARM_AUDIBLE_ALERT_HPP

#include "Traffic.hpp"
#include "Blackboard/BlackboardListener.hpp"
#include "Time/PeriodClock.hpp"

struct MoreData;
struct FlarmStatus;
class LiveBlackboard;

/**
 * a class that generates audible alerts based on Flarm high priority
 * alert info
 */
class FlarmAudibleAlert : NullBlackboardListener {
public:
  enum TrafficAboveBelow {
    TRAFFIC_ABOVE,
    TRAFFIC_BELOW,
    TRAFFIC_LEVEL,
    TRAFFIC_UNKNOWN,
  } traffic_above_below;

private:
  LiveBlackboard &blackboard;

  /// 1-12, or 0 if non-directional or -1 invalid
  int traffic_oclock;

  /// recency of last audible alert
  PeriodClock last_alert_time;

  /// alarm level
  FlarmTraffic::AlarmType alarm_level;

public:
  FlarmAudibleAlert(LiveBlackboard &_blackboard);
  ~FlarmAudibleAlert();
  void Reset();

  /**
   *  Return true if type of alert is audible
   *  @param type.  Flarm alarm urgency type
   */

  bool IsAlertAudible(FlarmTraffic::AlarmType type);

  /// Plays the audible alarm
  void PlayAlarm();

private:
  void Update(const FlarmStatus& status);

  /**
   * Updates the audible alert information
   *
   * @param: location: location of traffic
   * @param: location_available
   */
  void Update(const MoreData &basic,
              FlarmTraffic::AlarmType new_alarm_level,
              GeoPoint location, bool location_available);

  /// From NullBlackboardListener
  void OnGPSUpdate(const MoreData &basic) override;
};

#endif
