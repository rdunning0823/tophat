/*
Copyright_License {

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

#include "FlarmGlue.hpp"
#include "Waypoint/Factory.hpp"
#include "Language/Language.hpp"
#include "Terrain/RasterTerrain.hpp"
#include "Task/ProtectedTaskManager.hpp"

void
FlarmWaypointGlue::AddTeammateTask(const GeoPoint &location,
                                   const RasterTerrain *terrain,
                                   ProtectedTaskManager *protected_task_manager,
                                   bool non_current_flarm_lock)
{
  assert(location.IsValid());

  const WaypointFactory factory(WaypointOrigin::NONE, terrain);
  Waypoint wp_teammate = factory.Create(location);
  if (!factory.FallbackElevation(wp_teammate))
    wp_teammate.elevation = fixed(0);

  wp_teammate.name = _("Teammate");
  if (non_current_flarm_lock) {
    wp_teammate.name.append(_T(" - "));
    wp_teammate.name.append(_("not current"));
  }
  wp_teammate.type = Waypoint::Type::TEAMMATE;

  assert(protected_task_manager != nullptr);
  {
    ProtectedTaskManager::ExclusiveLease _task(*protected_task_manager);
    _task->DoTeammate(wp_teammate);
  }
}

bool
FlarmWaypointGlue::ClearTeammateTask(ProtectedTaskManager *protected_task_manager)
{
  {
    // we could use common_stats.task to check this, but it might be delayed
    // by a clock tick since this is called by the main thread
    // and common_stats are updated in the calculation thread.
    ProtectedTaskManager::Lease _task(*protected_task_manager);
    if (_task->GetMode() != TaskType::TEAMMATE)
      return false;
  }
  {
    ProtectedTaskManager::ExclusiveLease _task(*protected_task_manager);
    _task->Resume();
  }
  return true;
}
