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

#include "SaveFile.hpp"
#include "Serialiser.hpp"
#include "StateSerialiser.hpp"
#include "XML/DataNodeXML.hpp"
#include "IO/TextWriter.hpp"
#include "OS/FileUtil.hpp"
#include "LocalPath.hpp"
#include "Simulator.hpp"
#include "Time/PeriodClock.hpp"

#include <windef.h>

bool
SaveTask(const TCHAR *path, const OrderedTask &task)
{
  XMLNode root_node = XMLNode::CreateRoot(_T("Task"));

  {
    WritableDataNodeXML root(root_node);
    SaveTask(root, task);
  }

  TextWriter writer(path);
  if (!writer.IsOpen())
    return false;

  root_node.Serialise(writer, true);
  return true;
}

void
RemoveTaskState()
{
  TCHAR path[MAX_PATH];
  LocalPath(path, _T("task_state"));
  if (File::Exists(path))
    File::Delete(path);
}

bool
SaveTaskState(bool transitioned, bool in_sector, const OrderedTask &task)
{
#ifdef NDEBUG
  if (is_simulator())
    return false;
#endif

  static PeriodClock clock;

  if (transitioned || (in_sector && clock.Check(10000))) {
    TCHAR path[MAX_PATH];
    LocalPath(path, _T("task_state"));

    XMLNode root_node = XMLNode::CreateRoot(_T("TaskState"));
    {
      WritableDataNodeXML root(root_node);
      SaveTaskState(root, task);
    }

    TextWriter writer(path);
    if (!writer.IsOpen())
      return false;

    root_node.Serialise(writer, true);
    clock.Update();
    return true;
  }
  return false;
}
