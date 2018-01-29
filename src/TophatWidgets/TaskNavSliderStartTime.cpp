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

gcc_pure
static unsigned
SecondsUntil(unsigned now, RoughTime until)
{
  int d = until.GetMinuteOfDay() * 60 - now;
  if (d < 0)
    d += 24 * 60 * 60;
  return d;
}

bool
SliderStartTime::ShowStartCountDown() const
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const auto &calculated = CommonInterface::Calculated();
  const TaskStats &task_stats = calculated.ordered_task_stats;

  return InOrCloseToStart() &&
      basic.time_available &&
      task_stats.task_valid &&
      start_open_time_span.IsDefined();
}

void
SliderStartTime::SetTypeTextForStartCountDown(TypeBuffer &type_buffer,
                                              TypeBuffer &type_buffer_short) const
{
  const NMEAInfo &basic = CommonInterface::Basic();

  if (!ShowStartCountDown())
    return;

  const unsigned now_s(basic.time);
  const RoughTime now = RoughTime::FromSecondOfDayChecked(now_s);

  if (start_open_time_span.HasEnded(now)) {
    type_buffer = _("Closed");
    type_buffer_short = _("Closed");
  } else if (start_open_time_span.HasBegun(now)) {
    type_buffer = _("Open");
    type_buffer_short = _("Open");
  } else {
    unsigned seconds = SecondsUntil(now_s, start_open_time_span.GetStart());
    type_buffer.Format(_T("Wait %02u:%02u"), seconds / 60, seconds % 60);
    type_buffer_short.Format(_T("%02u:%02u"), seconds / 60, seconds % 60);
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
                      bool _flying, bool _is_USA, bool _max_height,
                      RoughTimeSpan _start_open_time_span)
{
  is_ordered = _is_ordered;
  idx = _idx;
  start_cylinder_proximity = _is_glider_close_to_start_cylinder;
  settings_show_two_minute_start = _task_settings_show_2_minutes;
  flying = _flying;
  is_USA = _is_USA;
  max_height = _max_height;
  start_open_time_span = _start_open_time_span;
}

bool
SliderStartTime::InOrCloseToStart() const
{
  return is_ordered &&
      (idx < 1 || (idx == 1 && start_cylinder_proximity));
}

bool
SliderStartTime::ShowTwoMinutes() const
{
  return InOrCloseToStart() &&
      settings_show_two_minute_start && flying && is_USA && max_height > 0;
}

void
SliderStartTime::GetTypeText(TypeBuffer &type_buffer,
                             TypeBuffer &type_buffer_short,
                             TaskType task_mode,
                             unsigned idx, unsigned task_size, bool is_start,
                             bool is_finish, bool is_aat, bool navigate_to_target,
                             bool enable_index) const
{
  // calculate but don't yet draw label "goto" abort, tp#
  bool different_short_buffer = false;
  type_buffer.clear();
  switch (task_mode) {
  case TaskType::ORDERED:
    if (task_size == 0)
      type_buffer = _("Go'n home:");

    else if (is_finish) {
      type_buffer = _("Finish");

    } else if (ShowTwoMinutes()) {
      SetTypeTextFor2MinuteCount(type_buffer, type_buffer_short);
      different_short_buffer = true;

    } else if (ShowStartCountDown()) {
      SetTypeTextForStartCountDown(type_buffer, type_buffer_short);

    } else if (is_start) {
        type_buffer = _("Start");
    } else if (is_aat && navigate_to_target)
      // append "Target" text to distance in center
      type_buffer.clear();
    else if (is_aat && enable_index)
      type_buffer.Format(_T("%s %u"), _("Center"), idx);
    else if (enable_index)
      type_buffer.Format(_T("%s %u"), _("TP"), idx);

    break;
  case TaskType::GOTO:
  case TaskType::TEAMMATE:
  case TaskType::ABORT:
    type_buffer = _("Goto:");
    break;

  case TaskType::NONE:
    type_buffer = _("Go'n home:");

    break;
  }
  if (!different_short_buffer)
    type_buffer_short = type_buffer;
}
