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

#include "Task/TaskFileXCSoar.hpp"
#include "Deserialiser.hpp"
#include "XML/DataNodeXML.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Util/StringUtil.hpp"

#include <memory>
#include <assert.h>

OrderedTask* 
TaskFileXCSoar::GetTask(const TaskBehaviour &task_behaviour,
                        const Waypoints *waypoints, unsigned index) const
{
  assert(index == 0);

  // Load root node
  std::unique_ptr<DataNode> root(DataNodeXML::Load(path));
  if (!root)
    return NULL;

  // Check if root node is a <Task> node
  if (!StringIsEqual(root->GetName(), _T("Task")))
    return NULL;

  // Create a blank task
  OrderedTask *task = new OrderedTask(task_behaviour);

  // Read the task from the XML file
  Deserialiser des(*root, waypoints);
  des.Deserialise(*task);

  // Check if the task is valid
  if (!task->CheckTask()) {
    delete task;
    return NULL;
  }

  // Return the parsed task
  return task;
}
