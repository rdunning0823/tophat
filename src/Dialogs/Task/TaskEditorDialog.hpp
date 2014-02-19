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

#ifndef XCSOAR_TASK_EDITOR_US_HPP
#define XCSOAR_TASK_EDITOR_US_HPP

class SingleWindow;
class OrderedTask;

enum TaskEditorReturn {
  /**
   * the task has been modified
   */
  TASK_MODIFIED,
  /**
   * the task was not modified
   */
  TASK_NOT_MODIFIED,
  /**
   * the calling program should revert the task
   */
  TASK_REVERT,
};

/**
 * shows dlg task point with limitations to US task rules
 * @return TASK_REVERT if the task should be reverted by the caller, else
 * TASK_MODIFIED if task was modified by editor or TASK_NOT_MODIFIED
 */
TaskEditorReturn
dlgTaskEditorShowModal(OrderedTask** task, const unsigned index);

#endif
