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

#ifndef XCSOAR_FLYING_STATE_HPP
#define XCSOAR_FLYING_STATE_HPP

#include "Math/fixed.hpp"
#include "Util/TypeTraits.hpp"
#include "Geo/GeoPoint.hpp"

/**
 * Structure for flying state (takeoff/landing)
 */
struct FlyingState
{
  /** True if airborne, False otherwise */
  bool flying;
  /** Detects when glider is on ground for several seconds */
  bool on_ground;

  /** Time of flight */
  fixed flight_time;
  /** Time of takeoff */
  fixed takeoff_time;

  /**
   * The location of the aircraft when it took off.  This attribute is
   * only valid if #flying is true.
   */
  GeoPoint takeoff_location;

  /**
   * The time stamp when the aircraft released from towing.  This is
   * an estimate based on sink.  If the aircraft was never seen on
   * ground (i.e. XCSoar was switched on while flying), this value is
   * not too useful.  This is negative if the aircraft is assumed to
   * be still towing.
   */
  fixed release_time;

  /**
   * The location of the aircraft when it released from towing.
   * Always check GeoPoint::IsValid() before using this value.
   */
  GeoPoint release_location;

  /** Reset flying state as if never flown */
  void Reset();

  bool IsTowing() const {
    return flying && negative(release_time);
  }
};

static_assert(is_trivial<FlyingState>::value, "type is not trivial");

#endif
