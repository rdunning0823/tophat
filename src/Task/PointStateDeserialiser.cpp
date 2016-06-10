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

#include "PointStateDeserialiser.hpp"
#include "Task/Ordered/OrderedTask.hpp"
#include "Task/Points/SampledTaskPoint.hpp"
#include "Task/Points/ScoredTaskPoint.hpp"
#include "Task/Ordered/Points/OrderedTaskPoint.hpp"
#include "Util/StringAPI.hxx"
#include "XML/DataNode.hpp"

#include "LogFile.hpp" //debug
#include "Formatter/UserGeoPointFormatter.hpp" //debug
#include "Util/Macros.hpp" //debug

bool
PointStateDeserialiser::UpdatePoint(OrderedTask &task, unsigned idx)
{

  TCHAR entered_loc[256];
  entered_loc[0] = '\0';
  if (positive(state_entered.time))
    FormatGeoPoint(state_entered.location, entered_loc, ARRAY_SIZE(entered_loc), _T(' '));

  TCHAR min_loc[256];
  FormatGeoPoint(location_min, min_loc, ARRAY_SIZE(min_loc), _T(' '));

  if (idx == 0) {
    LogDebug(_T("SerialisedTaskPointState::UpdatePoint idx:%u task.size:%u has_sampled:%u has_exited:%u entered:%s min:%s"),
             idx, task.TaskSize(), has_sampled, has_exited, entered_loc, min_loc);
  }
  if (idx >= task.TaskSize())
    return false;

  OrderedTaskPoint *p = task.task_points[idx];
  assert(p != nullptr);

  // ScoredTaskPoint friend access
  p->SetHasExited(has_exited);

  // ScoredTaskPoint public methods
  p->SetStateEntered(state_entered);
  //p->SetPast(idx <= 1); // ********** <<<<<<<<<<<<< DEBUG

  if (has_sampled) {
    // ScoredTaskPoint friend access
    if (idx == 1) {
      location_min.Dump(_T("SerialisedTaskPointState::UpdatePoint SetPointSearchMin 1:"));
    }

    p->SetSearchMin(SearchPoint(location_min,
                                task.task_projection));

    AircraftState state_location_min = state_entered;
    state_location_min.location = location_min;
    // ScoredTaskPoint friend access
    p->UpdateSampleNear(state_location_min, task.task_projection);

    /** a few fictional points nearby to create edges for hull scan algorithm
     * which are needed by the DijikstraMin algorithm
     */
    state_location_min.location.latitude += Angle::Degrees(0.0001);
    p->UpdateSampleNear(state_location_min, task.task_projection);
    state_location_min.location.longitude += Angle::Degrees(0.0001);
    p->UpdateSampleNear(state_location_min, task.task_projection);
    state_location_min.location.latitude -= Angle::Degrees(0.0001);
    p->UpdateSampleNear(state_location_min, task.task_projection);

    // add both points to the search vector
    p->UpdateSampleNear(state_entered, task.task_projection);
  }

  return true;
}

