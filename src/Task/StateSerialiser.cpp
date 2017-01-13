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

#include "StateSerialiser.hpp"
#include "PointStateDeserialiser.hpp"
#include "Task/Ordered/OrderedTask.hpp"
#include "Task/Ordered/Points/ASTPoint.hpp"
#include "Navigation/Aircraft.hpp"
#include "XML/DataNode.hpp"
#include "Compiler.h"
#include "Components.hpp"
#include "Blackboard/DeviceBlackboard.hpp"

#ifdef WIN32
#include "LogFile.hpp" // fixes weird crash
#endif

#include <memory>
#include <assert.h>

static void
Serialise(WritableDataNode &node, const GeoPoint &data)
{
  node.SetAttribute(_T("longitude"), data.longitude);
  node.SetAttribute(_T("latitude"), data.latitude);
}

static void
Serialise(WritableDataNode &node, const AircraftState &data)
{
  std::unique_ptr<WritableDataNode> child(node.AppendChild(_T("Location")));
  Serialise(*child, data.location);
  node.SetAttribute(_T("altitude"), data.altitude);
  node.SetAttribute(_T("time"), data.time);
  node.SetAttribute(_T("ground_speed"), data.ground_speed);
}


/** Start stats */
static void
Serialise(WritableDataNode &node,
          const StartStats &data)
{
  std::unique_ptr<WritableDataNode> child(node.AppendChild(_T("StartStats")));
  child->SetAttribute(_T("task_started"), data.task_started);
  child->SetAttribute(_T("time"), data.time);
  child->SetAttribute(_T("altitude"), data.altitude);
  child->SetAttribute(_T("ground_speed"), data.ground_speed);
}

/** Task point */
static void
Serialise(WritableDataNode &node,
          const OrderedTaskPoint &data)
{
  std::unique_ptr<WritableDataNode> child(node.AppendChild(_T("Point")));
  child->SetAttribute(_T("has_sampled"), data.HasSampled());
  child->SetAttribute(_T("has_exited"), data.HasExited());

  std::unique_ptr<WritableDataNode> wchild(child->AppendChild(_T("StateEntered")));
  Serialise(*wchild, data.GetEnteredState());

  if (data.HasSampled()) {
    std::unique_ptr<WritableDataNode> ochild(child->AppendChild(_T("LocationMin")));
    Serialise(*ochild, data.GetLocationMin());
  }
  if (data.HasSampled()) {
    std::unique_ptr<WritableDataNode> ochild(child->AppendChild(_T("LocationMaxAchieved")));
    Serialise(*ochild, data.GetLocationMaxAchieved());
  }

}

void
SaveTaskState(WritableDataNode &node, const OrderedTask &task)
{
  const BrokenDateTime &date_time = device_blackboard->Basic().date_time_utc;

  node.SetAttribute(_T("task_size"), task.TaskSize());
  node.SetAttribute(_T("active_turnpoint"), task.GetActiveIndex());
  if (date_time.IsDatePlausible())
    node.SetAttribute(_T("date_time"), (int)date_time.ToUnixTimeUTC());

  Serialise(node, task.GetStats().start);
  for (const OrderedTaskPoint &tp : task.GetPoints()) {
    Serialise(node, tp);
  }
#ifdef WIN32
  // crash in PC build fixed with this line and #indlude LogFile.hpp
  // introduced in fe9441c06a90c1b2e9c487eb91d90082675364e3
  LogFormat(_T("SaveTask(WritableDataNode &node, const OrderedTask &task) 4 HasOptionalStarts():%u"),
            task.HasOptionalStarts());
#endif
}
