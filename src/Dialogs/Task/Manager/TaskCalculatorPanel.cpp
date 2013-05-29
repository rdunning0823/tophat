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
#include "Interface.hpp"
#include "ActionInterface.hpp"
#include "Units/Units.hpp"
#include "Formatter/UserUnits.hpp"
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
  MC,
  AAT_ESTIMATED,
};

/** XXX this hack is needed because the form callbacks don't get a
    context pointer - please refactor! */
static TaskCalculatorPanel *instance;

void
TaskCalculatorPanel::Refresh()
{

  const auto &calculated = CommonInterface::Calculated();
  const TaskStats &task_stats = calculated.ordered_task_stats;
  const CommonStats &common_stats = calculated.common_stats;

  SetRowVisible(AAT_TIME, common_stats.ordered_has_targets);
  if (common_stats.ordered_has_targets) {
    StaticString<5> sign;
    StaticString<32> both;
    if (!positive(common_stats.aat_time_remaining))
      sign = _T("-");
    else
      sign = _T("");

    unsigned seconds = abs((int)common_stats.aat_time_remaining % 60);
    unsigned minutes = ((int)common_stats.aat_time_remaining - seconds)
        / 60;
    both.Format(_T("%s%u min %u sec"), sign.c_str(), minutes, seconds);
    SetText(AAT_TIME, both.c_str());
  }

  SetRowVisible(AAT_ESTIMATED, common_stats.ordered_has_targets);
  if (common_stats.ordered_has_targets) {
    StaticString<5> sign;
    StaticString<32> both;
    if (!positive(common_stats.aat_time_remaining
        + task_stats.GetEstimatedTotalTime()))
      sign = _T("-");
    else
      sign = _T("");

    unsigned seconds = abs((int)(task_stats.GetEstimatedTotalTime()) % 60);
    unsigned minutes = ((int)(task_stats.GetEstimatedTotalTime()) - seconds)
        / 60;
    both.Format(_T("%s%u min"), sign.c_str(), minutes);
    SetText(AAT_ESTIMATED, both.c_str());

  }
  fixed rPlanned = task_stats.total.solution_remaining.IsDefined()
    ? task_stats.total.solution_remaining.vector.distance
    : fixed(0);

  if (positive(rPlanned))
    LoadValue(DISTANCE, rPlanned, UnitGroup::DISTANCE);
  else
    ClearValue(DISTANCE);

  LoadValue(MC, CommonInterface::GetComputerSettings().polar.glide_polar_task.GetMC(),
            UnitGroup::VERTICAL_SPEED);

  if (task_stats.total.travelled.IsDefined())
    LoadValue(SPEED_ACHIEVED, task_stats.total.travelled.GetSpeed(),
              UnitGroup::TASK_SPEED);
  else
    ClearValue(SPEED_ACHIEVED);
}

void
TaskCalculatorPanel::OnModified(DataField &df)
{
  if (IsDataField(MC, df)) {
    const DataFieldFloat &dff = (const DataFieldFloat &)df;
    fixed mc = Units::ToSysVSpeed(dff.GetAsFixed());
    ActionInterface::SetManualMacCready(mc);
    Refresh();
  }
}

void
TaskCalculatorPanel::OnSpecial(DataField &df)
{
  if (IsDataField(MC, df)) {
    const DerivedInfo &calculated = CommonInterface::Calculated();
    if (positive(calculated.time_climb)) {
      fixed mc = calculated.total_height_gain / calculated.time_climb;
      DataFieldFloat &dff = (DataFieldFloat &)df;
      dff.Set(Units::ToUserVSpeed(mc));
      ActionInterface::SetManualMacCready(mc);
      Refresh();
    }
  }
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

  AddFloat(_("Set MacCready"),
           _("Adjusts MC value used in the calculator.  "
             "Use this to determine the effect on estimated task time due to changes in conditions.  "
             "This value will not affect the main computer's setting if the dialog is exited with the Cancel button."),
           _T("%.1f %s"), _T("%.1f"),
           fixed(0), Units::ToUserVSpeed(fixed(5)),
           GetUserVerticalSpeedStep(), false, fixed(0),
           this);
  DataFieldFloat &mc_df = (DataFieldFloat &)GetDataField(MC);
  mc_df.SetFormat(GetUserVerticalSpeedFormat(false, false));

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
