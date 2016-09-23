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

#include "TaskStatusPanel.hpp"
#include "Widget/TextWidget.hpp"
#include "Interface.hpp"
#include "ActionInterface.hpp"
#include "Units/Units.hpp"
#include "Formatter/UserUnits.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Form/Draw.hpp"
#include "Form/Button.hpp"
#include "Form/DataField/Float.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Components.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Icon.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Look/MapLook.hpp"
#include "Language/Language.hpp"
#include "Formatter/TimeFormatter.hpp"

enum Controls {
  SPEED,
  DISTANCE_DONE,
  DISTANCE_REMAINING,
  AAT_TIME,
  AAT_ESTIMATED,
  TIME_ELAPSED,
  TIME_REMAINING
};

/** XXX this hack is needed because the form callbacks don't get a
    context pointer - please refactor! */
static TaskStatusPanel *instance;

void
TaskStatusPanel::Refresh()
{
  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;
  const TaskStats &task_stats = CommonInterface::Calculated().task_stats;
  StaticString<32> time;


  if (task_stats.task_valid && positive(task_stats.distance_scored))
    LoadValue(SPEED, task_stats.GetScoredSpeed(),
              UnitGroup::TASK_SPEED);
  else
    ClearValue(SPEED);

  fixed distance = positive(task_stats.distance_scored)
    ? task_stats.distance_scored
    : fixed(0);
  if (positive(distance))
    LoadValue(DISTANCE_DONE, distance,
              UnitGroup::DISTANCE);
  else
    ClearValue(DISTANCE_DONE);

  fixed rPlanned = task_stats.total.solution_remaining.IsDefined()
    ? task_stats.total.solution_remaining.vector.distance
    : fixed(0);
  if (positive(rPlanned))
    LoadValue(DISTANCE_REMAINING, rPlanned, UnitGroup::DISTANCE);
  else
    ClearValue(DISTANCE_REMAINING);

  if (task_stats.has_targets) {
    FormatTimespanSmart(time.buffer(), (int)common_stats.aat_time_remaining, 2);
    SetText(AAT_TIME, time.c_str());

    FormatTimespanSmart(time.buffer(), (int)task_stats.total.time_planned, 2);
    SetText(AAT_ESTIMATED, time.c_str());

  } else {
    FormatTimespanSmart(time.buffer(), (int)task_stats.total.time_elapsed, 2);
    SetText(TIME_ELAPSED, time.c_str());

    FormatTimespanSmart(time.buffer(), (int)task_stats.total.time_remaining_now, 2);
    SetText(TIME_REMAINING, time.c_str());
  }
}

void
TaskStatusPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  assert(protected_task_manager != nullptr);

  const TaskStats &task_stats = CommonInterface::Calculated().task_stats;

  instance = this;

  AddReadOnly(_("Task speed"), NULL, _T("%.0f %s"),
              UnitGroup::TASK_SPEED, fixed(0));
  AddReadOnly(_("Distance done"), NULL, _T("%.0f %s"),
              UnitGroup::DISTANCE, fixed(0));
  AddReadOnly(_("Distance remaining"), NULL, _T("%.0f %s"),
              UnitGroup::DISTANCE, fixed(0));

  if (task_stats.has_targets) {
    if (task_stats.is_mat)
      AddReadOnly(_("MAT Time remaining"), NULL, _T(""));
    else
      AddReadOnly(_("AAT Time remaining"), NULL, _T(""));

    AddReadOnly(_("Estimated total time"), NULL, _T(""));
    AddDummy();
    AddDummy();
  } else {
    AddDummy();
    AddDummy();
    AddReadOnly(_("Time Elapsed"), NULL, _T(""));
    AddReadOnly(_("Time Remaining"), NULL, _T(""));
  }
}

void
TaskStatusPanel::Show(const PixelRect &rc)
{
  Refresh();

  CommonInterface::GetLiveBlackboard().AddListener(*this);

  RowFormWidget::Show(rc);
}

void
TaskStatusPanel::Hide()
{
  CommonInterface::GetLiveBlackboard().RemoveListener(*this);

  RowFormWidget::Hide();
}
