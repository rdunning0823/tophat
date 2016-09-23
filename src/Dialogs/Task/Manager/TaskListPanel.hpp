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

#ifndef XCSOAR_TASK_LIST_PANEL_HPP
#define XCSOAR_TASK_LIST_PANEL_HPP

class Widget;
class TaskManagerDialog;
class OrderedTask;
class WndForm;

/**
 * @param dialog.  pointer to TaskManagerDialog if use in that environment
 * @param active_task
 * @param task_modified
 * @param parent_form.  pointer to parent form if used in that environment.
 *  either dialog or parent_form must be nullptr
 */
Widget *
CreateTaskListPanel(TaskManagerDialog *dialog,
                    OrderedTask **active_task, bool *task_modified,
                    WndForm *parent_form = nullptr);

/**
 * Mark the previously scanned list of tasks as "dirty".  Before the
 * list will be shown next time, a new list will be collected.  Call
 * this function after a task file has been created or deleted.
 */
void
DirtyTaskListPanel();

#endif
