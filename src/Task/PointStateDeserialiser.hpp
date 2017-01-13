/* Copyright_License {

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
#ifndef SERIALISED_TASK_POINT_STATE_HPP
#define SERIALISED_TASK_POINT_STATE_HPP

#include "Task/Points/Type.hpp"
#include "Navigation/Aircraft.hpp"

#include <tchar.h>

class ConstDataNode;
class Waypoints;
class OrderedTask;

/** a class to serialise and deserialise the state of a task point
 * This class relies on the friend declaration to update task properties in
 *   OrderedTask
 *   SampledTaskPoint
 *   ScoredTaskPoint
 */

class PointStateDeserialiser {
public:
  bool has_sampled;
  bool has_exited;
  AircraftState state_entered;
  GeoPoint location_min;
  GeoPoint location_max_achieved;
  void Reset() {
    has_sampled = false;
    has_exited = false;
    state_entered.Reset();
    location_min = GeoPoint::Invalid();
    location_max_achieved = GeoPoint::Invalid();
  }
  PointStateDeserialiser() {
    Reset();
  }

  /**
   * update the task point using (or abusing) the friend declaration
   */
  bool UpdatePoint(OrderedTask &task, unsigned idx);
};

#endif
