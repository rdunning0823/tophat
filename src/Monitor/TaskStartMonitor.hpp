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

#ifndef XCSOAR_TASK_START_MONITOR_HPP
#define XCSOAR_TASK_START_MONITOR_HPP

#include "Math/fixed.hpp"
#include "Util/StaticString.hxx"

#include <tchar.h>

struct StartStats;

typedef StaticString<128> message_string;

class TaskStartMonitor {
  fixed last_start_time;
  friend class TaskStartWidget;
  class TaskStartWidget *widget;

public:
  TaskStartMonitor():last_start_time(fixed(0)), widget(nullptr) {}

  void Reset() {
    last_start_time = fixed(0);
  }

  void Check();

  /**
   * updates the buffer with a message relevant to start stats
   * @param start: the StartStat relevant to the current start
   * @message: updated with text
   * @return rows of text in the message
   */
  unsigned GetMessage1(const StartStats &start, message_string &message);
};

#endif
