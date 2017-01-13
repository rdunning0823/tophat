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

#include "StateDeserialiser.hpp"
#include "PointStateDeserialiser.hpp"
#include "Task/Ordered/OrderedTask.hpp"
#include "Util/StringAPI.hxx"
#include "XML/DataNode.hpp"
#include "Components.hpp"
#include "Blackboard/DeviceBlackboard.hpp"

#include <memory>

static void
Deserialise(GeoPoint &data, const ConstDataNode &node)
{
  node.GetAttribute(_T("longitude"), data.longitude);
  node.GetAttribute(_T("latitude"), data.latitude);
}

static void
Deserialise(AircraftState &data, const ConstDataNode &node)
{
  std::unique_ptr<ConstDataNode> loc_node(node.GetChildNamed(_T("Location")));
  if (loc_node != nullptr) {
    Deserialise(data.location, *loc_node);
  }

  node.GetAttribute(_T("altitude"), data.altitude);
  node.GetAttribute(_T("time"), data.time);
  node.GetAttribute(_T("ground_speed"), data.ground_speed);
}

static void
Deserialise(StartStats &data, const ConstDataNode &node)
{
  std::unique_ptr<ConstDataNode> start_stats_node(node.GetChildNamed(_T("StartStats")));
  if (start_stats_node != nullptr) {
    start_stats_node->GetAttribute(_T("task_started"), data.task_started);
    start_stats_node->GetAttribute(_T("time"), data.time);
    start_stats_node->GetAttribute(_T("altitude"), data.altitude);
    start_stats_node->GetAttribute(_T("ground_speed"), data.ground_speed);
  }
}

static bool
DeserialiseTaskpointState(PointStateDeserialiser &data,
                          const ConstDataNode &node)
{

  node.GetAttribute(_T("has_exited"), data.has_exited);
  node.GetAttribute(_T("has_sampled"), data.has_sampled);

  std::unique_ptr<ConstDataNode> state_entered(node.GetChildNamed(_T("StateEntered")));
  if (state_entered != nullptr)
    Deserialise(data.state_entered, *state_entered);

  if (data.has_sampled) {
    std::unique_ptr<ConstDataNode> loc_node(node.GetChildNamed(_T("LocationMin")));
    if (loc_node != nullptr)
      Deserialise(data.location_min, *loc_node);
  }
  if (data.has_sampled) {
    std::unique_ptr<ConstDataNode> loc_node(node.GetChildNamed(_T("LocationMaxAchieved")));
    if (loc_node != nullptr)
      Deserialise(data.location_max_achieved, *loc_node);
  }

  return true;
}

bool
LoadTaskState(OrderedTask &task, const ConstDataNode &node)
{
  int task_size;
  node.GetAttribute(_T("task_size"), task_size);
  if (task_size < 0)
    return false;
  if ((unsigned)task_size != task.TaskSize())
    return false;

  int active_turnpoint;
  node.GetAttribute(_T("active_turnpoint"), active_turnpoint);
  if (active_turnpoint >= 0)
    task.SetActiveTaskPoint((unsigned)active_turnpoint);

  Deserialise(task.SetStats().start, node);

  int time;
  node.GetAttribute(_T("date_time"), time);
#ifdef NDEBUG
  const BrokenDateTime &now = device_blackboard->Basic().date_time_utc;
  if (time > now.ToUnixTimeUTC() || now.ToUnixTimeUTC() - time > 3600) {
    return false;
  }
#endif

  const auto children = node.ListChildrenNamed(_T("Point"));
  unsigned idx = 0;
  for (const auto &i : children) {
    std::unique_ptr<ConstDataNode> point_node(i);
    PointStateDeserialiser task_point_state;
    task_point_state.Reset();
    DeserialiseTaskpointState(task_point_state, *point_node);
    task_point_state.UpdatePoint(task, idx++);
  }
  return true;
}
