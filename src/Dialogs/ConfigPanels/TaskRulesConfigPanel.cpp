/*
Copyright_License {

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

#include "TaskRulesConfigPanel.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Form/DataField/Enum.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Form/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "Profile/Profile.hpp"
#include "Units/UnitsStore.hpp"

enum ControlIndex {
  StartMaxSpeedMargin,
  StartMaxHeightMargin,
  AATTimeMargin,
  ContestNationality,
  spacer_1,
  Contests,
  PREDICT_CONTEST,
};

class TaskRulesConfigPanel : public RowFormWidget {
public:
  TaskRulesConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);
};

void
TaskRulesConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const ComputerSettings &settings_computer = XCSoarInterface::GetComputerSettings();
  const TaskBehaviour &task_behaviour = settings_computer.task;

  RowFormWidget::Prepare(parent, rc);

  AddFloat(_("Start max. speed margin"),
           _("Maximum speed above maximum start speed to tolerate.  Set to 0 for no tolerance."),
           _T("%.0f %s"), _T("%.0f"), fixed(0), fixed(300), fixed(5), false, UnitGroup::HORIZONTAL_SPEED,
           task_behaviour.start_max_speed_margin);
  SetExpertRow(StartMaxSpeedMargin);

  AddFloat(_("Start max. height margin"),
           _("Maximum height above maximum start height to tolerate.  Set to 0 for no tolerance."),
           _T("%.0f %s"), _T("%.0f"), fixed(0), fixed(10000), fixed(50), false, UnitGroup::ALTITUDE,
           fixed(task_behaviour.start_max_height_margin));
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

  AddSpacer();
  SetExpertRow(spacer_1);

  const StaticEnumChoice contests_list[] = {
    { OLC_FAI, ContestToString(OLC_FAI),
      N_("Conforms to FAI triangle rules. Three turns and common start and finish. No leg less than 28% "
          "of total except for tasks longer than 500km: No leg less than 25% or larger than 45%.") },
    { OLC_Classic, ContestToString(OLC_Classic),
      N_("Up to seven points including start and finish, finish height must not be lower than "
          "start height less 1000 meters.") },
    { OLC_League, ContestToString(OLC_League),
      N_("The most recent contest with Sprint task rules.") },
    { OLC_Plus, ContestToString(OLC_Plus),
      N_("A combination of Classic and FAI rules. 30% of the FAI score are added to the Classic score.") },
    { OLC_XContest, ContestToString(OLC_XContest), _T("tbd.") },
    { OLC_DHVXC, ContestToString(OLC_DHVXC), _T("tbd.") },
    { OLC_SISAT, ContestToString(OLC_SISAT), _T("tbd.") },
    { OLC_NetCoupe, ContestToString(OLC_NetCoupe), N_("The FFVV NetCoupe \"libre\" competiton.") },
    { 0 }
  };
  AddEnum(_("On-Line Contest"),
      _("Select the rules used for calculating optimal points for the On-Line Contest. "
          "The implementation  conforms to the official release 2010, Sept.23."),
          contests_list, task_behaviour.contest);

  AddBoolean(_("Predict Contest"),
             _("If enabled, then the next task point is included in the "
               "score calculation, assuming that you will reach it."),
             task_behaviour.predict_contest);
  SetExpertRow(PREDICT_CONTEST);
}


bool
TaskRulesConfigPanel::Save(bool &_changed, bool &_require_restart)
{
  bool changed = false, require_restart = false;

  ComputerSettings &settings_computer = XCSoarInterface::SetComputerSettings();
  TaskBehaviour &task_behaviour = settings_computer.task;

  changed |= SaveValue(StartMaxSpeedMargin, UnitGroup::HORIZONTAL_SPEED, ProfileKeys::StartMaxSpeedMargin,
                       task_behaviour.start_max_speed_margin);

  changed |= SaveValue(StartMaxHeightMargin, UnitGroup::ALTITUDE, ProfileKeys::StartMaxHeightMargin,
                       task_behaviour.start_max_height_margin);

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

//    SaveValueEnum(UnitsSpeed, ProfileKeys::SpeedUnitsValue, config.speed_unit);
    config.wind_speed_unit = config.speed_unit; // Mapping the wind speed to the speed unit
    Profile::Set(ProfileKeys::SpeedUnitsValue,
                 (int)config.speed_unit);

//    SaveValueEnum(UnitsDistance, ProfileKeys::DistanceUnitsValue, config.distance_unit);
    Profile::Set(ProfileKeys::DistanceUnitsValue,
                 (int)config.distance_unit);

//    SaveValueEnum(UnitsLift, ProfileKeys::LiftUnitsValue, config.vertical_speed_unit);
    Profile::Set(ProfileKeys::LiftUnitsValue,
                 (int)config.vertical_speed_unit);

//    SaveValueEnum(UnitsAltitude, ProfileKeys::AltitudeUnitsValue, config.altitude_unit);
    Profile::Set(ProfileKeys::AltitudeUnitsValue,
                 (int)config.altitude_unit);

//    SaveValueEnum(UnitsTemperature, ProfileKeys::TemperatureUnitsValue, config.temperature_unit);
    Profile::Set(ProfileKeys::ContestNationality,
                 (int)task_behaviour.contest_nationality);

//    SaveValueEnum(UnitsTaskSpeed, ProfileKeys::TaskSpeedUnitsValue, config.task_speed_unit);
    Profile::Set(ProfileKeys::TaskSpeedUnitsValue,
                 (int)config.temperature_unit);

//    SaveValueEnum(UnitsPressure, ProfileKeys::PressureUnitsValue, config.pressure_unit);
    Profile::Set(ProfileKeys::PressureUnitsValue,
                 (int)config.pressure_unit);
  }
  changed |= nat_changed;

  unsigned aatmargin = task_behaviour.optimise_targets_margin;
  if (SaveValue(AATTimeMargin, aatmargin)) {
    task_behaviour.optimise_targets_margin = aatmargin;
    Profile::Set(ProfileKeys::AATTimeMargin, aatmargin);
    changed = true;
  }

  changed |= SaveValueEnum(Contests, ProfileKeys::OLCRules, task_behaviour.contest);
  changed |= SaveValueEnum(PREDICT_CONTEST, ProfileKeys::PredictContest,
                           task_behaviour.predict_contest);

  _changed |= changed;
  _require_restart |= require_restart;

  return true;
}

Widget *
CreateTaskRulesConfigPanel()
{
  return new TaskRulesConfigPanel();
}
