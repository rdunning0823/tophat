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

#ifndef TASK_SAVE_FILE_HPP
#define TASK_SAVE_FILE_HPP

#include <tchar.h>

class OrderedTask;

bool
SaveTask(const TCHAR *path, const OrderedTask &task);

/**
 * Saves the task if a transition occurred or periodically if we're in the sector
 * Don't save every second while in sector to reduce probability of
 * write-in-progress when crashes
 *
 * @transitioned. did a transition just occur
 * @in_sector. are we in a sector
 */
bool
SaveTaskState(bool transitioned, bool in_sector, const OrderedTask &task);

void RemoveTaskState();

#endif
