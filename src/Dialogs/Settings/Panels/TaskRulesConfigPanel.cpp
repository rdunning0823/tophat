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

#include "TaskRulesConfigPanel.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Form/DataField/Enum.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Widget/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "Profile/Profile.hpp"
#include "Units/UnitsStore.hpp"

enum ControlIndex {
  StartMaxSpeedMargin,
  StartMaxHeightMargin,
  AATTimeMargin,
  ContestNationality,
};

class TaskRulesConfigPanel final : public RowFormWidget {
public:
  TaskRulesConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;
};

void
TaskRulesConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const ComputerSettings &settings_computer = CommonInterface::GetComputerSettings();
  const TaskBehaviour &task_behaviour = settings_computer.task;

  RowFormWidget::Prepare(parent, rc);

  AddFloat(_("Start max. speed margin"),
           _("Maximum speed above maximum start speed to tolerate.  Set to 0 for no tolerance."),
           _T("%.0f %s"), _T("%.0f"), fixed(0), fixed(300), fixed(5), false, UnitGroup::HORIZONTAL_SPEED,
           task_behaviour.start_margins.max_speed_margin);
  SetExpertRow(StartMaxSpeedMargin);

  AddFloat(_("Start max. height margin"),
           _("Maximum height above maximum start height to tolerate.  Set to 0 for no tolerance."),
           _T("%.0f %s"), _T("%.0f"), fixed(0), fixed(10000), fixed(50), false, UnitGroup::ALTITUDE,
           fixed(task_behaviour.start_margins.max_height_margin));
  SetExpertRow(StartMaxHeightMargin);

  AddTime(_("Optimisation margin"),
          _("Safety margin for AAT task optimisation.  Optimisation "
            "seeks to complete the task at the minimum time plus this margin time."),
          0, 30 * 60, 60, (unsigned)task_behaviour.optimise_targets_margin);
  SetExpertRow(AATTimeMargin);

  WndProperty *wp = AddEnum(_("Nationality"), N_("If a specific nation is selected, then building a task is simplified to show only appropriate options.  Sets the appropriate system units also."));
  DataFieldEnum &df = *(DataFieldEnum *)wp->GetDataField();
  df.EnableItemHelp(true);

  unsigned len = Units::Store::Count();
  df.addEnumText(_("Unknown"), (unsigned)0, _("Unknown nationality"));
  for (unsigned i = 0; i < len; i++)
    df.addEnumText(Units::Store::GetName(i), i+1);

  LoadValueEnum(ContestNationality, (unsigned)task_behaviour.contest_nationality);
}


bool
TaskRulesConfigPanel::Save(bool &_changed)
{
  bool changed = false;

  ComputerSettings &settings_computer = CommonInterface::SetComputerSettings();
  TaskBehaviour &task_behaviour = settings_computer.task;

  changed |= SaveValue(StartMaxSpeedMargin, UnitGroup::HORIZONTAL_SPEED, ProfileKeys::StartMaxSpeedMargin,
                       task_behaviour.start_margins.max_speed_margin);

  changed |= SaveValue(StartMaxHeightMargin, UnitGroup::ALTITUDE, ProfileKeys::StartMaxHeightMargin,
                       task_behaviour.start_margins.max_height_margin);

  bool nat_changed = SaveValueEnum(ContestNationality, ProfileKeys::ContestNationality,
                                   task_behaviour.contest_nationality);
  if (nat_changed) {
    // use british (0th index) if unknown
    unsigned the_unit = (task_behaviour.contest_nationality > 0)
        ? task_behaviour.contest_nationality - 1 : 0;
    UnitSetting units = Units::Store::Read(the_unit);
    UnitSetting &config = CommonInterface::SetUISettings().units;
    config = units;

    /* the Units settings affect how other form values are read and translated
     * so changes to Units settings should be processed after all other form settings
     */
    Profile::Set(ProfileKeys::ContestNationality,
                 (int)task_behaviour.contest_nationality);

    config.wind_speed_unit = config.speed_unit; // Mapping the wind speed to the speed unit
    Profile::Set(ProfileKeys::SpeedUnitsValue,
                 (int)config.speed_unit);

    Profile::Set(ProfileKeys::DistanceUnitsValue,
                 (int)config.distance_unit);

    Profile::Set(ProfileKeys::LiftUnitsValue,
                 (int)config.vertical_speed_unit);

    Profile::Set(ProfileKeys::AltitudeUnitsValue,
                 (int)config.altitude_unit);

    Profile::Set(ProfileKeys::ContestNationality,
                 (int)task_behaviour.contest_nationality);

    Profile::Set(ProfileKeys::TaskSpeedUnitsValue,
                 (int)config.temperature_unit);

    Profile::Set(ProfileKeys::PressureUnitsValue,
                 (int)config.pressure_unit);
  }
  changed |= nat_changed;

  _changed |= changed;

  return true;
}

Widget *
CreateTaskRulesConfigPanel()
{
  return new TaskRulesConfigPanel();
}
