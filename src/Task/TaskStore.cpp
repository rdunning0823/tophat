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

#include "Task/TaskStore.hpp"
#include "Task/TaskFile.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Components.hpp"
#include "OS/FileUtil.hpp"
#include "LocalPath.hpp"
#include "Language/Language.hpp"

#include <cstdio>
#include <algorithm>
#include <memory>

class TaskFileVisitor: public File::Visitor
{
private:
  TaskStore::ItemVector &store;

public:
  TaskFileVisitor(TaskStore::ItemVector &_store):
    store(_store) {}

  void Visit(const TCHAR *path, const TCHAR *base_name) {
    // Create a TaskFile instance to determine how many
    // tasks are inside of this task file
    std::unique_ptr<TaskFile> task_file(TaskFile::Create(path));
    if (!task_file)
      return;

    // Count the tasks in the task file
    unsigned count = task_file->Count();
    // For each task in the task file
    for (unsigned i = 0; i < count; i++) {
      // Copy base name of the file into task name
      StaticString<256> name(base_name);

      // If the task file holds more than one task
      const TCHAR *saved_name = task_file->GetName(i);
      if (saved_name != NULL) {
        name += _T(": ");
        name += saved_name;
      } else if (count > 1) {
        // .. append " - Task #[n]" suffix to the task name
        name.AppendFormat(_T(": %s #%d"), _("Task"), i + 1);
      }

      // Add the task to the TaskStore
      store.push_back(TaskStore::Item(path, name.empty() ? path : name, i));
    }
  }
};

void
TaskStore::Clear()
{
  // clear entries first
  store.erase(store.begin(), store.end());
}

void
TaskStore::Scan(bool extra)
{
  Clear();

  // scan files
  TaskFileVisitor tfv(store);
  VisitDataFiles(_T("*.tsk"), tfv);
  VisitDataFiles(_T("*.cup"), tfv);

  if (extra) {
    VisitDataFiles(_T("*.igc"), tfv);
  }

  std::sort(store.begin(), store.end());
}

TaskStore::Item::~Item()
{
  if (!filename.empty())
    delete task;
}

OrderedTask*
TaskStore::Item::GetTask(const TaskBehaviour &task_behaviour)
{
  if (task != NULL)
    return task;

  if (valid)
    task = TaskFile::GetTask(filename.c_str(), task_behaviour,
                             &way_points, task_index);

  if (task == NULL)
    valid = false;

  return task;
}

const TCHAR *
TaskStore::GetName(unsigned index) const
{
  return store[index].GetName();
}

const TCHAR *
TaskStore::GetPath(unsigned index) const
{
  return store[index].GetPath();
}

OrderedTask* 
TaskStore::GetTask(unsigned index, const TaskBehaviour &task_behaviour)
{
  return store[index].GetTask(task_behaviour);
}
