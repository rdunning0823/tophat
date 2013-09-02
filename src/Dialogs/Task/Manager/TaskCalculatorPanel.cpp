/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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

#include "TaskCalculatorPanel.hpp"
#include "Internal.hpp"
#include "../TaskDialogs.hpp"
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
  SPEED_ACHIEVED,
  DISTANCE,
  AAT_TIME,
  AAT_ESTIMATED,
};

/** XXX this hack is needed because the form callbacks don't get a
    context pointer - please refactor! */
static TaskCalculatorPanel *instance;

void
TaskCalculatorPanel::Refresh()
{
  const CommonStats &common_stats = XCSoarInterface::Calculated().common_stats;
  const TaskStats &task_stats = CommonInterface::Calculated().task_stats;
  StaticString<32> time;

  SetRowVisible(AAT_TIME, task_stats.has_targets);
  FormatTimespanSmart(time.buffer(), (int)common_stats.aat_time_remaining, 2);
  SetText(AAT_TIME, time.c_str());

  SetRowVisible(AAT_ESTIMATED, task_stats.has_targets);
  if (task_stats.has_targets) {
    FormatTimespanSmart(time.buffer(), (int)task_stats.total.time_planned, 2);
    SetText(AAT_ESTIMATED, time.c_str());
  }
  fixed rPlanned = task_stats.total.solution_remaining.IsDefined()
    ? task_stats.total.solution_remaining.vector.distance
    : fixed(0);

  if (positive(rPlanned))
    LoadValue(DISTANCE, rPlanned, UnitGroup::DISTANCE);
  else
    ClearValue(DISTANCE);

  if (task_stats.total.travelled.IsDefined())
    LoadValue(SPEED_ACHIEVED, task_stats.total.travelled.GetSpeed(),
              UnitGroup::TASK_SPEED);
  else
    ClearValue(SPEED_ACHIEVED);
}

void
TaskCalculatorPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  assert(protected_task_manager != NULL);

  instance = this;

  AddReadOnly(_("Achieved speed"), NULL, _T("%.0f %s"),
              UnitGroup::TASK_SPEED, fixed(0));

  AddReadOnly(_("Distance remaining"), NULL, _T("%.0f %s"),
              UnitGroup::DISTANCE, fixed(0));
  AddReadOnly(_("AAT Time remaining"), NULL, _T(""));

  AddReadOnly(_("Estimated total time"), NULL, _T(""));
}

void
TaskCalculatorPanel::Show(const PixelRect &rc)
{
  emc = CommonInterface::Calculated().task_stats.effective_mc;

  Refresh();

  CommonInterface::GetLiveBlackboard().AddListener(*this);

  RowFormWidget::Show(rc);
}

void
TaskCalculatorPanel::Hide()
{
  CommonInterface::GetLiveBlackboard().RemoveListener(*this);

  RowFormWidget::Hide();
}
