/* Copyright_License {

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
#ifndef TASK_SUMMARY_HPP
#define TASK_SUMMARY_HPP

#include "Util/TrivialArray.hpp"
#include "Util/TypeTraits.hpp"
#include "Math/fixed.hpp"

struct TaskSummaryPoint {
  /** Distance planned to this point from previous, (m) */
  fixed d_planned;
  /** Proportion of total distance of this point [0-1] */
  fixed p;
  /** Whether this task point has been achieved */
  bool achieved;
};

struct TaskSummary {
  /** Proportion of planned distance remaining, [0-1] */
  fixed p_remaining;
  /** Index of active taskpoint */
  unsigned active;

  typedef TrivialArray<TaskSummaryPoint, 16u> TaskSummaryPointVector;

  /** Vector of turnpoint data */
  TaskSummaryPointVector pts;

  void clear() {
    active = 0;
    p_remaining = fixed_one;
    pts.clear();
  }
  void append(const TaskSummaryPoint& tsp) {
    pts.push_back(tsp);
  }
  void update(const fixed &d_remaining, const fixed &d_planned) {
    if (!positive(d_planned))
      return;

    p_remaining = d_remaining/d_planned;
    fixed p = fixed_zero;
    for (auto &i : pts) {
      p += i.d_planned / d_planned;
      i.p = p;
    }
  }
};

static_assert(is_trivial<TaskSummary>::value, "type is not trivial");

#endif
