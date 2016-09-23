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

#include "TaskComputerConfigPanel.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Boolean.hpp"
#include "Form/DataField/Listener.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Widget/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "Asset.hpp"


enum ControlIndex {
  TaskOptimizationMode,
  TaskOptimizationSpeed,
  Spacer1,
  Spacer2,
  EffectiveSpeed,
  EffectiveMC,
};

TaskComputerConfigPanel::TaskComputerConfigPanel()
  :RowFormWidget(UIGlobals::GetDialogLook()), changed(false) {}

void
TaskComputerConfigPanel::OnModified(DataField &df)
{
  changed = true;
  if (IsDataField(TaskOptimizationMode, df)) {
    const DataFieldEnum &dfe = (const DataFieldEnum &)df;
    TaskBehaviour::TaskPlanningSpeedMode mode =
        (TaskBehaviour::TaskPlanningSpeedMode)dfe.GetValue();
    UpdateVisibility(mode);
  }

  ComputerSettings &settings_computer = CommonInterface::SetComputerSettings();
  TaskBehaviour &task_behaviour = settings_computer.task;

  SaveValueEnum(TaskOptimizationMode,
                ProfileKeys::TaskPlanningSpeedMode,
                task_behaviour.task_planning_speed_mode);

  SaveValue(TaskOptimizationSpeed, UnitGroup::TASK_SPEED,
            ProfileKeys::TaskPlanningSpeedOverride,
            task_behaviour.task_planning_speed_override);
  UpdateValues();
}

void
TaskComputerConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{

  const ComputerSettings &settings_computer = CommonInterface::SetComputerSettings();
  const TaskBehaviour &task_behaviour = settings_computer.task;

  static constexpr StaticEnumChoice task_optimization_mode_list[] = {
    { (unsigned)TaskBehaviour::TaskPlanningSpeedMode::PastPerformanceSpeed, N_("Flown task speed"),
      N_("AAT/TAT task targets will be optimized based on the task speed flown.") },
    { (unsigned)TaskBehaviour::TaskPlanningSpeedMode::OverrideSpeed, N_("Manually enter task speed"),
      N_("AAT/TAT task targets will be optimized based on the task speed entered by the pilot.") },
      { (unsigned)TaskBehaviour::TaskPlanningSpeedMode::MacCreadyValue, N_("MacCready setting"),
        N_("AAT/TAT task targets will be optimized based on the speed predicted by the current MacCready value.") },
    { 0 }
  };

  AddEnum(_("Task optimization mode"),
          _("AAT/TAT targets can be optimized based on either the actual flown speed, or by a speed entered by the pilot."),
          task_optimization_mode_list,
          (unsigned)task_behaviour.task_planning_speed_mode,
          this);

  AddFloat(_("Target optimization speed"),
           _("AAT/TAT task targets will be optimized based on the speed entered by the pilot."),
           _T("%.0f %s"), _T("%.0f"),
           fixed(5), fixed(200), fixed(1), false,
           UnitGroup::TASK_SPEED, task_behaviour.task_planning_speed_override, this);


  AddReadOnly(_T(""), NULL, _T(""));
  SetRowVisible(Spacer1, false);
  AddSpacer();

  AddReadOnly(_("Effective task speed"), NULL, _T("%.0f %s"),
              UnitGroup::TASK_SPEED, fixed(0));
  AddReadOnly(_("Effective task MC"), NULL, _T("%.1f %s"),
              UnitGroup::VERTICAL_SPEED, fixed(0));

  UpdateVisibility(task_behaviour.task_planning_speed_mode);
  UpdateValues();
}

void
TaskComputerConfigPanel::Hide()
{
  Timer::Cancel();
  RowFormWidget::Hide();
}

void
TaskComputerConfigPanel::Show(const PixelRect &rc)
{
  RowFormWidget::Show(rc);
  Timer::Schedule(500);
}

void
TaskComputerConfigPanel::OnTimer()
{
  UpdateValues();
}

void
TaskComputerConfigPanel::UpdateVisibility(TaskBehaviour::TaskPlanningSpeedMode mode)
{
  SetRowVisible(TaskOptimizationSpeed,
                mode == TaskBehaviour::TaskPlanningSpeedMode::OverrideSpeed);
  SetRowVisible(EffectiveSpeed,
                mode != TaskBehaviour::TaskPlanningSpeedMode::OverrideSpeed);
}

void
TaskComputerConfigPanel::UpdateValues()
{
  const TaskStats &task_stats = CommonInterface::Calculated().task_stats;

  LoadValue(EffectiveSpeed, task_stats.task_mc_effective_speed,
            UnitGroup::TASK_SPEED);

  LoadValue(EffectiveMC, task_stats.task_mc,
            UnitGroup::VERTICAL_SPEED);
}

bool
TaskComputerConfigPanel::Save(bool &_changed)
{
  _changed |= changed;

  return true;
}

Widget *
CreateTaskComputerConfigPanel()
{
  return new TaskComputerConfigPanel();
}
