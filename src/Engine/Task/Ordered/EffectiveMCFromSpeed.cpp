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

#include "EffectiveMCFromSpeed.hpp"
#include "Task/TaskEvents.hpp"
#include "TaskAdvance.hpp"
#include "Points/OrderedTaskPoint.hpp"
#include "Points/StartPoint.hpp"
#include "Points/FinishPoint.hpp"
#include "Task/Solvers/TaskEffectiveMacCready.hpp"

#include "Waypoint/Waypoints.hpp"

#include "GlideSolvers/GlideState.hpp"
#include "GlideSolvers/MacCready.hpp"
#include "Util/Clamp.hpp"

void
EffectiveMCFromSpeed::DeleteDummyTask()
{
  task_points_dummy.clear();

  if (start != nullptr)
    delete start;
  if (tp1 != nullptr)
    delete tp1;

  start = nullptr;
  tp1 = nullptr;
}

bool
EffectiveMCFromSpeed::IsValid() const
{
  return start != nullptr && tp1 != nullptr;
}

void
EffectiveMCFromSpeed::CreateDummyTask(const StartPoint* start_point_real,
                                      const OrderedTaskPoint* task_point_real,
                                      const TaskBehaviour &task_behaviour,
                                      const OrderedTaskSettings &ordered_settings)
{
  DeleteDummyTask();

  start = (StartPoint*)start_point_real->Clone(task_behaviour,
                                               ordered_settings,
                                               &start_point_real->GetWaypoint());
  tp1 = (OrderedTaskPoint*)task_point_real->Clone(task_behaviour,
                                                  ordered_settings,
                                                  &task_point_real->GetWaypoint());
  task_points_dummy.push_back(start);
  task_points_dummy.push_back(tp1);
}

bool
EffectiveMCFromSpeed::Calc(const AircraftState &state,
                           const GlidePolar &glide_polar,
                           fixed speed_travelled,
                           const TaskBehaviour &task_behaviour,
                           const TaskProjection &task_projection,
                           fixed &val)
{
  if (!IsValid()) {
    val = glide_polar.GetMC();
    return false;
  }

  if (speed_travelled - last_speed < speed_threshold &&
      last_speed - speed_travelled < speed_threshold
      && positive(last_value)) {
    val = last_value;
    return true;
  }

  /** aircraft state at point of fake start */
  AircraftState start_state = state;
  /** aircraft state at point of fake progress */
  AircraftState aircraft_state = state;

  start_state.location = start->GetLocation();
  start_state.altitude = aircraft_state.altitude =
      start->GetWaypoint().elevation + fixed(10000); // level flight above terrain


  const fixed duration = fixed(10); // make super short so IntermediatePoint works
  const fixed distance = speed_travelled * duration;

  aircraft_state.location = start->GetLocation().IntermediatePoint(tp1->GetLocation(), distance);
  aircraft_state.time += duration;


  start->SetStateEntered(start_state);
  start->SetHasExitedOverride();
  start->SetNeighbours(nullptr, tp1);
  tp1->SetNeighbours(start, nullptr);

  start->ScanActive(*tp1);

  start->SetSearchMin(SearchPoint(start_state.location, task_projection));
  start->SetSearchMax(SearchPoint(start_state.location, task_projection));

  tp1->SetSearchMin(SearchPoint(aircraft_state.location, task_projection));
  tp1->SetSearchMax(SearchPoint(aircraft_state.location, task_projection));
  start->ScanDistanceTravelled(aircraft_state.location);

  {
    TaskEffectiveMacCready bce(task_points_dummy, 1, aircraft_state,
                               task_behaviour.glide, glide_polar);
    val = bce.search(glide_polar.GetMC());
  }
  last_speed = speed_travelled;
  last_value = val;

  return true;
}

