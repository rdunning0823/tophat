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

#include "GlideComputerTask.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Terrain/RasterTerrain.hpp"
#include "NMEA/Aircraft.hpp"
#include "NMEA/MoreData.hpp"
#include "NMEA/Derived.hpp"
#include "ComputerSettings.hpp"

#include <algorithm>

using std::max;

// JMW TODO: abstract up to higher layer so a base copy of this won't
// call any event

GlideComputerTask::GlideComputerTask(ProtectedTaskManager &_task,
                                     const Airspaces &airspace_database)
  :task(_task),
   route(airspace_database),
   contest(trace.GetFull(), trace.GetSprint())
{
  task.SetRoutePlanner(&route.GetRoutePlanner());
}

void
GlideComputerTask::ResetFlight(const bool full)
{
  task.Reset();
  route.ResetFlight();
  trace.Reset();
  contest.Reset();

  last_flying = false;
}

void
GlideComputerTask::ProcessBasicTask(const MoreData &basic,
                                    const MoreData &last_basic,
                                    DerivedInfo &calculated,
                                    const DerivedInfo &last_calculated,
                                    const ComputerSettings &settings_computer,
                                    const Waypoints &waypoints, bool force)
{
  trace.Update(settings_computer, basic, calculated);

  ProtectedTaskManager::ExclusiveLease _task(task);

  _task->SetTaskBehaviour(settings_computer.task);

  if (force || (basic.HasTimeAdvancedSince(last_basic) &&
                basic.location_available)) {
    const AircraftState current_as = ToAircraftState(basic, calculated);
    const AircraftState last_as = ToAircraftState(last_basic, last_calculated);

    _task->Update(current_as, last_as, waypoints);

    const fixed fallback_mc = calculated.last_thermal.IsDefined() &&
      positive(calculated.last_thermal_average_smooth)
      ? calculated.last_thermal_average_smooth
      : fixed_zero;
    if (_task->UpdateAutoMC(current_as, fallback_mc))
      calculated.ProvideAutoMacCready(basic.clock,
                                      _task->GetGlidePolar().GetMC());
  }

  calculated.task_stats = _task->GetStats();
  calculated.common_stats = _task->GetCommonStats();
  calculated.glide_polar_safety = _task->GetSafetyPolar();
}

void
GlideComputerTask::ProcessMoreTask(const MoreData &basic,
                                   DerivedInfo &calculated,
                                   const DerivedInfo &last_calculated,
                                   const ComputerSettings &settings_computer)
{
  const GlidePolar &glide_polar = settings_computer.polar.glide_polar_task;
  const GlidePolar &safety_polar = calculated.glide_polar_safety;

  route.ProcessRoute(basic, calculated, last_calculated,
                     settings_computer.task.glide,
                     settings_computer.task.route_planner,
                     glide_polar, safety_polar);

  if (settings_computer.features.block_stf_enabled)
    calculated.V_stf = calculated.common_stats.V_block;
  else
    calculated.V_stf = calculated.common_stats.V_dolphin;

  if (calculated.task_stats.current_leg.vector_remaining.IsValid()) {
    const GeoVector &v = calculated.task_stats.current_leg.vector_remaining;
    calculated.auto_zoom_distance = v.distance;
  }
}

gcc_pure
static TracePoint
Predicted(const TaskBehaviour &settings,
          const MoreData &basic,
          const ElementStat &current_leg)
{
  if (!basic.time_available || !basic.gps_altitude_available ||
      !settings.predict_contest ||
      !current_leg.location_remaining.IsValid() ||
      !current_leg.solution_remaining.IsDefined())
    return TracePoint::Invalid();

  /* predict that the next task point will be reached, using the
     calculated remaining time and the minimum arrival altitude */
  return TracePoint(current_leg.location_remaining,
                    unsigned(basic.time + current_leg.time_remaining),
                    current_leg.solution_remaining.min_arrival_altitude,
                    fixed_zero, 0);
}

void
GlideComputerTask::ProcessIdle(const MoreData &basic, DerivedInfo &calculated,
                               const ComputerSettings &settings_computer,
                               bool exhaustive)
{
  contest.SetPredicted(Predicted(settings_computer.task, basic,
                                 calculated.task_stats.current_leg));

  if (exhaustive)
    contest.SolveExhaustive(settings_computer, calculated);
  else
    contest.Solve(settings_computer, calculated);

  const AircraftState as = ToAircraftState(basic, calculated);

  ProtectedTaskManager::ExclusiveLease _task(task);
  _task->UpdateIdle(as);
}

void 
GlideComputerTask::ProcessAutoTask(const NMEAInfo &basic,
                                   const DerivedInfo &calculated)
{
  if (!calculated.flight.flying) {
    /* not flying (yet) */
    last_flying = false;
    return;
  }

  if (last_flying)
    /* still flying, not a takeoff */
    return;

  last_flying = true;

  if (calculated.altitude_agl_valid &&
      calculated.altitude_agl > fixed(500))
    return;

  ProtectedTaskManager::ExclusiveLease _task(task);
  _task->TakeoffAutotask(calculated.flight.takeoff_location,
                         calculated.terrain_altitude);
}

void 
GlideComputerTask::SetTerrain(const RasterTerrain* _terrain) {
  route.set_terrain(_terrain);
}
