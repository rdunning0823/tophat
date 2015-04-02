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

#include "Widgets/TaskNavDataCache.hpp"
#include "Language/Language.hpp"
#include "Interface.hpp"
#include "Util/StaticString.hpp"
#include "OS/Clock.hpp"
#include "Components.hpp"
#include "Math/fixed.hpp"
#include "GlideSolvers/MacCready.hpp"
#include "GlideSolvers/GlideState.hpp"
#include "Task/Ordered/OrderedTask.hpp"
#include "Task/Ordered/Points/OrderedTaskPoint.hpp"

#include <stdio.h>
#include <stdlib.h>

TaskNavDataCache::TaskNavDataCache(const ComputerSettings &_settings,
                                   const NMEAInfo &_basic,
                                   const TaskStats &_task_stats)
  :settings(_settings), basic(_basic),
   ordered_task_size(0),
   task_manager_time_stamp(0) {}

void
TaskNavDataCache::UpdateOrderedTask(const OrderedTask &ordered_task,
                                    TaskType _mode,
                                    TaskFactoryType _task_factory_type,
                                    unsigned _active_task_point_index,
                                    int _task_manager_time_stamp)
{
  SetDirty();

  task_manager_time_stamp = _task_manager_time_stamp;
  mode = _mode;
  task_factory_type = _task_factory_type;
  ordered_task_size = ordered_task.TaskSize();
  active_task_point_index = _active_task_point_index;
  SetActiveWaypoint(ordered_task.GetActiveTaskPoint());

  if (mode == TaskType::ORDERED) {
    for (unsigned i = 0; i < ordered_task_size; i++)
      SetTaskPoint(ordered_task, i);

    UpdateTransitions(ordered_task);

    for (unsigned i = 0; i < ordered_task_size; i++)
      CalcTaskPoint(i);
  } else {
    CalcActivePoint();
  }
}

void
TaskNavDataCache::UpdateTransitions(const OrderedTask &ordered_task)
{
  assert(ordered_task_size == ordered_task.TaskSize());

  for (unsigned i = 0; i < ordered_task_size; i++) {
    const OrderedTaskPoint &tp = ordered_task.GetTaskPoint(i);
    tps[i].has_entered = tp.HasEntered();
    tps[i].has_exited = tp.HasExited();
  }
  transition_time_stamp = NowStamp();
}

void
TaskNavDataCache::UpdateTargets(const OrderedTask &ordered_task)
{
  assert(ordered_task_size == ordered_task.TaskSize());
  if (GetTaskFactoryType() != TaskFactoryType::AAT)
    return;

  for (unsigned i = 0; i < ordered_task_size; i++) {
    const OrderedTaskPoint &tp = ordered_task.GetTaskPoint(i);
    tps[i].target = &tp.GetLocationRemaining();
  }
}

TaskNavDataCache::tp_info &
TaskNavDataCache::GetPoint(unsigned i)
{
  assert(i < MAX_TURNPOINTS);

  if (mode == TaskType::ORDERED && ordered_task_size > 0)
    return CalcTaskPoint(i);

  return CalcActivePoint();
}

void
TaskNavDataCache::SetDirty()
{
  for (unsigned i = 0; i < MAX_TURNPOINTS; i++)
    tps[i].Invalidate();
  active_tp.Invalidate();
}

TaskNavDataCache::tp_info &
TaskNavDataCache::CalcActivePoint()
{
  if (active_tp.waypoint == nullptr) {
    active_tp.Invalidate();
    return active_tp;
  }

  if (active_tp.IsCurrent())
    return active_tp;

  assert(active_tp.waypoint != nullptr);
  assert(active_tp.target == nullptr);
  return CalcPoint(active_tp, *active_tp.waypoint);
}

void
TaskNavDataCache::SetTaskPoint(const OrderedTask &ordered_task, unsigned idx)
{
  const OrderedTaskPoint &tp = ordered_task.GetTaskPoint(idx);
  tps[idx].waypoint = &tp.GetWaypoint();
  tps[idx].target = &tp.GetLocationRemaining();
}

TaskNavDataCache::tp_info &
TaskNavDataCache::CalcTaskPoint(unsigned idx)
{
  assert(tps[idx].waypoint != nullptr);

  if (tps[idx].IsCurrent()) {
    return tps[idx];
  }

  CalcTarget(tps[idx], tps[idx].waypoint->elevation);
  return CalcPoint(tps[idx], *tps[idx].waypoint);
}

TaskNavDataCache::tp_info &
TaskNavDataCache::ValidateTarget(TaskNavDataCache::tp_info &tp_data,
                                 bool valid)
{
  tp_data.delta_bearing_remaining_valid = valid;
  tp_data.distance_remaining_valid = valid;

  return tp_data;
}

void
TaskNavDataCache::CalcTarget(TaskNavDataCache::tp_info &tp_data,
                             fixed target_altitude)
{
  assert(tp_data.target != nullptr);
  // New dist & bearing
  if (basic.location_available) {
    const GeoVector vector = basic.location.DistanceBearing(*tp_data.target);

    tp_data.distance_remaining = vector.distance;
    tp_data.delta_bearing_remaining = vector.bearing - basic.track;

    tp_data.altitude_difference_remaining_valid = false;
    const MoreData &more_data = CommonInterface::Basic();

    // altitude differential
    if (basic.location_available && more_data.NavAltitudeAvailable() &&
        settings.polar.glide_polar_task.IsValid()) {

      tp_data.altitude_difference_remaining =
          CalcAltitudeDifferential(*tp_data.target, target_altitude);
      tp_data.altitude_difference_remaining_valid = true;
    }
  }
  ValidateTarget(tp_data, basic.location_available);

  return;
}

fixed
TaskNavDataCache::CalcAltitudeDifferential(const GeoPoint &point,
                                           fixed point_elevation)
{
  const MoreData &more_data = CommonInterface::Basic();
  const DerivedInfo &calculated = CommonInterface::Calculated();

  assert(basic.location_available);
  assert(more_data.NavAltitudeAvailable());
  assert(settings.polar.glide_polar_task.IsValid());

  // altitude differential
  const GlideState glide_state(
    basic.location.DistanceBearing(point),
    point_elevation + settings.task.safety_height_arrival,
    more_data.nav_altitude,
    calculated.GetWindOrZero());

  const GlideResult &result =
    MacCready::Solve(settings.task.glide,
                     settings.polar.glide_polar_task,
                     glide_state);
  return result.pure_glide_altitude_difference;
}

TaskNavDataCache::tp_info &
TaskNavDataCache::CalcPoint(TaskNavDataCache::tp_info &tp_data,
                            const Waypoint &wp)
{
  const MoreData &more_data = CommonInterface::Basic();

  if (tp_data.IsCurrent()) {
    return tp_data;
  }

  tp_data.timestamp = NowStamp();

  // altitude differential
  tp_data.altitude_difference_valid = false;
  if (basic.location_available && more_data.NavAltitudeAvailable() &&
      settings.polar.glide_polar_task.IsValid()) {

    tp_data.altitude_difference = CalcAltitudeDifferential(wp.location, wp.elevation);
    tp_data.altitude_difference_valid = true;
  }

  // New dist & bearing
  if (basic.location_available) {
    const GeoVector vector = basic.location.DistanceBearing(wp.location);

    tp_data.distance = vector.distance;
    tp_data.distance_valid = true;

    tp_data.delta_bearing = vector.bearing - basic.track;
    tp_data.bearing_valid = true;
  } else {
    tp_data.bearing_valid = false;
    tp_data.distance_valid = false;
  }

  return tp_data;
}

unsigned
TaskNavDataCache::NowStamp()
{
  return MonotonicClockMS();
}
