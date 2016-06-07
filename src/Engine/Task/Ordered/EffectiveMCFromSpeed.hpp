/* Copyright_License {

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

#ifndef EFFECTIVE_MC_FROM_SPEED_HPP
#define EFFECTIVE_MC_FROM_SPEED_HPP

#include "OrderedTask.hpp"

#include <vector>

class StartPoint;
class OrderedTaskPoint;
class AircraftState;

/**
 * A helper class that takes a speed as an input and calculates
 * the effective mc without concern for wind or other environmental factors
 */
class EffectiveMCFromSpeed
{
  typedef std::vector<OrderedTaskPoint*> DummyTaskPointVector;
  DummyTaskPointVector task_points_dummy;
  StartPoint *start;
  OrderedTaskPoint *tp1;
  fixed last_speed;
  fixed last_value;
  /** threshold for determining if speed has changed */
  fixed speed_threshold;

public:
  bool IsValid() const;

  EffectiveMCFromSpeed():
    start(nullptr), tp1(nullptr),
    last_speed(fixed(-1)), last_value(fixed(-1)), speed_threshold(fixed(.01))
  {}

  ~EffectiveMCFromSpeed() {
    DeleteDummyTask();
  }

  /** sets val to the effective MC based on the speed and polar
   * @param aircraft. the current aircraft state
   * @param glide_polar.  the polar used in the calculation
   * @parm speed_travelled.  the speed used in the effective MC calculation
   * @param val. set to the value of the effective mc.  Or the value of the
   * polar if the calculation fails
   *
   * @return.  True if the calculation was performed
   **/
  bool Calc(const AircraftState &state,
            const GlidePolar &glide_polar,
            fixed speed_travelled,
            const TaskBehaviour &task_behaviour,
            const TaskProjection &task_projection,
            fixed &val);

  /** create the necessary task items for the calculation */
  void CreateDummyTask(const StartPoint* start_point_real,
                       const OrderedTaskPoint* task_point_real,
                       const TaskBehaviour &task_behaviour,
                       const OrderedTaskSettings &ordered_settings);

  /** clean up the task items and invalidate it*/
  void DeleteDummyTask();
private:
};

#endif //EFFECTIVE_MC_FROM_SPEED_HPP
