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

#ifndef XCSOAR_FLARM_STATUS_HPP
#define XCSOAR_FLARM_STATUS_HPP

#include "FLARM/Traffic.hpp"
#include "NMEA/Validity.hpp"

#include <type_traits>

/**
 * The FLARM operation status read from the PFLAU sentence.
 * BUG:: According to Flarm 7.0.3 dataport, values are:
 * 0 = no GPS reception 1 = 3d-fix on ground, i.e. not airborne 2 = 3d-fix when airborne
 */
struct FlarmStatus {
  enum class GPSStatus: uint8_t {
    NONE = 0,
    GPS_2D = 1,
    GPS_3D = 2,
  };

  /**
   * Save the prioirty alert info from the PFLAU record
   * These targets are not sync'd with  the traffic list because the IDs are different
   * when stealth mode is in use.
   */
  struct PriorityIntruder {

    /// PFLAU alarm type
    enum class AlarmTypePFLAU: uint16_t {
      NONE = 0x0,             // 0 = no aircraft within range or no-alarm traffic information
      AIRCRAFT = 0x2,         // 2 = aircraft alarm
      ALERT_ZONE = 0x3,       // 3 = obstacle/Alert Zone alarm (if data port version < 7, otherwise only obstacle alarms are indicated by <AlarmType> = 3)
      TRAFFIC_ADVISORY = 0x4, // 4 = traffic advisory (sent once each time an aircraft enters within distance 1.5 km and vertical distance 300 m from own ship)
      ZONE_SKYDIVER_DROP = 0x41,        // = Skydiver drop zone
      ZONE_AERODROME_TRAFFIC = 0x42,    // 0x42 = Aerodrome traffic zone
      ZONE_MILITARY_FIRING_AREA = 0x43, //0x43 = Military firing area
      ZONE_KITE_FLYING_ZONE = 0x44,     //44 = Kite flying zone
      ZONE_WINCH_LAUNCHING_AREA = 0x45, // 0x45 = Winch launching area
      ZONE_RC_FLYING_AREA = 0x46,       // 0x46 = RC flying area
      ZONE_UAS_FLYING_AREA = 0x47,      // 0x47 = UAS flying area
      ZONE_AEROBATIC_BOX = 0x48,        // 0x48 = Aerobatic box
      ZONE_GENERIC_DANGER_AREA = 0x7E,  // 0x7E = Generic danger area
      ZONE_PROHIBITED_AREA = 0x7F,      // 0x7F = Generic prohibited area
    };

    FlarmId id;

    /** type of alert in the PFLAU sentence */
    AlarmTypePFLAU alarm_type;

    /**
     * Relative bearing in degrees from true ground track to the intruder’s position.
     * Positive values are clockwise. 0° indicates that the object is exactly ahead.
     * Field is empty for non-directional targets or when no aircraft are within range.
     * For obstacle alarm and Alert Zone alarm, this field is 0.
     */
    fixed relative_bearing_degrees;
    bool relative_bearing_degrees_valid;

    /**
     * Decimal integer value. Range: from 0 to 2147483647.
     * Relative horizontal distance in meters to the target or obstacle.
     * For non-directional targets this value is estimated based on signal strength.
     * Field is empty when no aircraft are within range and no alarms are generated.
     * For Alert Zone, this field is 0.
     */
    fixed distance;
    bool distance_valid;

    fixed relative_altitude;
    bool relative_altitude_valid;

    void Reset() {
      distance_valid = false;
      relative_bearing_degrees_valid = false;
      relative_altitude_valid = false;
    }

    bool IsAircraftAlert() {
      return alarm_type == AlarmTypePFLAU::AIRCRAFT;
    }
  } priority_intruder;


  /** Number of received FLARM devices */
  unsigned short rx;
  /** Transmit status */
  bool tx;

  /** GPS status */
  GPSStatus gps;

  /** Alarm level of FLARM (0-3) */
  FlarmTraffic::AlarmType alarm_level;

  /** Is FLARM information available? */
  Validity available;

  void Clear() {
    available.Clear();
    priority_intruder.Reset();
  }

  void Complement(const FlarmStatus &add) {
    if (!available && add.available)
      *this = add;
  }

  void Expire(fixed clock) {
    available.Expire(clock, fixed(10));
  }
};

static_assert(std::is_trivial<FlarmStatus>::value, "type is not trivial");

#endif
