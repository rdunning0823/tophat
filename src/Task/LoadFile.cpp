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

#include "LoadFile.hpp"
#include "Deserialiser.hpp"
#include "XML/DataNodeXML.hpp"
#include "XML/Parser.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Util/StringUtil.hpp"
#include "Engine/Task/Factory/AbstractTaskFactory.hpp"
#include "Task/StateDeserialiser.hpp"
#include "LocalPath.hpp"
#include "OS/FileUtil.hpp"
#include "Simulator.hpp"
#include "LogFile.hpp"

#include <windef.h>

#include <memory>

OrderedTask *
LoadTask(const TCHAR *path, const TaskBehaviour &task_behaviour,
         const Waypoints *waypoints)
{
  // Load root node
  std::unique_ptr<XMLNode> xml_root(XML::ParseFile(path));
  if (!xml_root)
    return nullptr;

  const ConstDataNodeXML root(*xml_root);

  // Check if root node is a <Task> node
  if (!StringIsEqual(root.GetName(), _T("Task")))
    return nullptr;

  // Create a blank task
  OrderedTask *task = new OrderedTask(task_behaviour);

  // Read the task from the XML file
  LoadTask(*task, root, waypoints);

  task->GetFactory().MutateTPsToTaskType();
  task->ScanStartFinish();
  // Check if the task is valid
  if (!task->CheckTask()) {
    delete task;
    return nullptr;
  }

  // Return the parsed task
  return task;
}

bool
LoadTaskState(OrderedTask &task)
{
#ifdef NDEBUG
  if (is_simulator())
    return false;
#endif

  TCHAR path[MAX_PATH];
  LocalPath(path, _T("task_state"));

  if (!File::Exists(path))
    return false;

    // Load root node
  std::unique_ptr<XMLNode> xml_root(XML::ParseFile(path));
  if (!xml_root)
    return false;

  const ConstDataNodeXML root(*xml_root);
  LoadTaskState(task, root);

  LogFormat(_T("Loaded task_state file"));

  return true;
}
