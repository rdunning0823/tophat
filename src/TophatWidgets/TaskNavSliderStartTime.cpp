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

#include "TophatWidgets/TaskNavSliderStartTime.hpp"
#include "Language/Language.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Engine/Task/Stats/TaskStats.hpp"
#include "Engine/Task/Stats/CommonStats.hpp"
#include "Interface.hpp"

#include <tchar.h>

void
SliderStartTime::SetTypeTextFor2MinuteCount(TypeBuffer &type_buffer,
                                            TypeBuffer &type_buffer_short) const
{
  if (negative(fixed(GetTimeUnderStart()))) {
    type_buffer = _("Above Start");
    type_buffer_short = _("Above");
  }
  else if (GetTimeUnderStart() < 120) {
    TCHAR value[32];
    FormatSignedTimeMMSSCompact(value, 120 - GetTimeUnderStart());
    type_buffer.Format(_T("%s: %s"), _("2Minutes"), value);
    type_buffer_short.Format(_T("%s: %s"), _("2Min"), value);
  } else {
    type_buffer.Format(_T("%s: %s"), _("2Minutes"), _("OK"));
    type_buffer_short.Format(_T("%s: %s"), _("2Min"), _("OK"));
  }
}

int
SliderStartTime::GetTimeUnderStart() const
{
  const CommonStats &common_stats = CommonInterface::Calculated().common_stats;
  const TaskStats &task_stats = CommonInterface::Calculated().ordered_task_stats;

  if (!task_stats.task_valid
      || !common_stats.is_under_start_max_height
      || !ShowTwoMinutes()) {
    return -1;
  }

  return (int)(CommonInterface::Basic().time -
      common_stats.time_transition_below_max_start_height);
}

void
SliderStartTime::Init(bool _is_ordered, unsigned _idx,
                      bool _is_glider_close_to_start_cylinder,
                      bool _task_settings_show_2_minutes,
                      bool _flying, bool _is_USA, bool _max_height)
{
  is_ordered = _is_ordered;
  idx = _idx;
  start_cylinder_proximity = _is_glider_close_to_start_cylinder;
  settings_show_two_minute_start = _task_settings_show_2_minutes;
  flying = _flying;
  is_USA = _is_USA;
  max_height = _max_height;
}

bool
SliderStartTime::ShowTwoMinutes() const
{
  return is_ordered &&
      (idx < 1 || (idx == 1 && start_cylinder_proximity)) &&
      settings_show_two_minute_start && flying && is_USA && max_height > 0;
}


