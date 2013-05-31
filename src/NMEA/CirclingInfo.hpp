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

#ifndef XCSOAR_CIRCLING_INFO_HPP
#define XCSOAR_CIRCLING_INFO_HPP

#include "Util/TypeTraits.hpp"
#include "Geo/GeoPoint.hpp"
#include "Math/fixed.hpp"

/** Enumeration for cruise/circling mode detection */
enum class CirclingMode: uint8_t {
  /** Established cruise mode */
  CRUISE = 0,
  /** In cruise, pending transition to climb */
  POSSIBLE_CLIMB,
  /** Established climb mode */
  CLIMB,
  /** In climb, pending transition to cruise */
  POSSIBLE_CRUISE,
};

/** Data for tracking of climb/cruise mode and transition points */
struct CirclingInfo
{
  /** Turn rate based on track */
  fixed turn_rate;

  /** Turn rate based on heading (including wind) */
  fixed turn_rate_heading;

  /** Turn rate after low pass filter */
  fixed turn_rate_smoothed;

  /** StartLocation of the current/last climb */
  GeoPoint climb_start_location;
  /** StartAltitude of the current/last climb */
  fixed climb_start_altitude;
  /** StartTime of the current/last climb */
  fixed climb_start_time;

  /** StartLocation of the current/last cruise */
  GeoPoint cruise_start_location;
  /** StartAltitude of the current/last cruise */
  fixed cruise_start_altitude;
  /** StartTime of the current/last cruise */
  fixed cruise_start_time;

  /** Start/End time of the turn (used for flight mode determination) */
  fixed turn_start_time;
  /** Start/End location of the turn (used for flight mode determination) */
  GeoPoint turn_start_location;
  /** Start/End altitude of the turn (used for flight mode determination) */
  fixed turn_start_altitude;
  /** Start/End energy height of the turn (used for flight mode determination) */
  fixed turn_start_energy_height;

  /** Current TurnMode (Cruise, Climb or somewhere between) */
  CirclingMode turn_mode;

  /**
   * True if the turn rate is above the threshold for circling.
   */
  bool turning;

  /** True if in circling mode, False otherwise */
  bool circling;

  /** Circling/Cruise ratio in percent */
  fixed circling_percentage;

  /** Time spent in cruise mode */
  fixed time_cruise;
  /** Time spent in circling mode */
  fixed time_climb;

  /** Minimum altitude since start of task */
  fixed min_altitude;

  /** Maximum height gain (from MinAltitude) during task */
  fixed max_height_gain;

  /** Total height climbed during task */
  fixed total_height_gain;

  void Clear();

  bool TurningLeft() const {
    return negative(turn_rate_smoothed);
  }
};

static_assert(is_trivial<CirclingInfo>::value, "type is not trivial");

#endif
