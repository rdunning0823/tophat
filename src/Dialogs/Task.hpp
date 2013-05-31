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

#ifndef XCSOAR_DIALOGS_TASK_HPP
#define XCSOAR_DIALOGS_TASK_HPP

class SingleWindow;
class OrderedTask;
struct DialogLook;

void
dlgTaskManagerShowModal(SingleWindow &parent);

bool
dlgTaskPointShowModal(SingleWindow &parent, OrderedTask** task, const unsigned index);

/**
 * dialog with task properties needed for American task rules
 */
void
dlgTaskListUsShowModal(OrderedTask** task, bool &task_changed);

/**
 * dialog with task properties needed for American task rules
 */
void
dlgTaskPropertiesUsShowModal(const DialogLook &look, OrderedTask** task,
                             bool &task_changed);

bool
dlgTaskPointType(SingleWindow &parent, OrderedTask** task, const unsigned index);

bool
dlgTaskOptionalStarts(SingleWindow &parent, OrderedTask** task);

bool
dlgTaskPointNew(SingleWindow &parent, OrderedTask** task, const unsigned index);

/**
 * Shows map display zoomed to target point
 * with half dialog popup to manipulate point
 *
 * @param TargetPoint if -1 then goes to active target
 * else goes to TargetPoint by default
 */
void
dlgTargetShowModal(int TargetPoint = -1);

#endif
