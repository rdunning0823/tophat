/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
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
};

TaskComputerConfigPanel::TaskComputerConfigPanel()
  :RowFormWidget(UIGlobals::GetDialogLook()) {}

void
TaskComputerConfigPanel::OnModified(DataField &df)
{
  if (IsDataField(TaskOptimizationMode, df)) {
    const DataFieldEnum &dfe = (const DataFieldEnum &)df;
    TaskBehaviour::TaskPlanningSpeedMode mode =
        (TaskBehaviour::TaskPlanningSpeedMode)dfe.GetValue();
    UpdateVisibility(mode);
  }
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
           UnitGroup::TASK_SPEED, task_behaviour.task_planning_speed_override);

  UpdateVisibility(task_behaviour.task_planning_speed_mode);
}

void
TaskComputerConfigPanel::UpdateVisibility(TaskBehaviour::TaskPlanningSpeedMode mode)
{
  SetRowVisible(TaskOptimizationSpeed,
                mode == TaskBehaviour::TaskPlanningSpeedMode::OverrideSpeed);
}

bool
TaskComputerConfigPanel::Save(bool &_changed)
{
  bool changed = false;

  ComputerSettings &settings_computer = CommonInterface::SetComputerSettings();
  TaskBehaviour &task_behaviour = settings_computer.task;

  changed |= SaveValueEnum(TaskOptimizationMode,
                           ProfileKeys::TaskPlanningSpeedMode,
                           task_behaviour.task_planning_speed_mode);

  changed |= SaveValue(TaskOptimizationSpeed, UnitGroup::TASK_SPEED,
                       ProfileKeys::TaskPlanningSpeedOverride,
                       task_behaviour.task_planning_speed_override);

  _changed |= changed;

  return true;
}

Widget *
CreateTaskComputerConfigPanel()
{
  return new TaskComputerConfigPanel();
}
