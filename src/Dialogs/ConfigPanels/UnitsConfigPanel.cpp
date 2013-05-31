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

#include "UnitsConfigPanel.hpp"
#include "UIGlobals.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Listener.hpp"
#include "Form/Form.hpp"
#include "Form/Frame.hpp"
#include "Units/Units.hpp"
#include "Units/UnitsStore.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Interface.hpp"
#include "Asset.hpp"
#include "Language/Language.hpp"
#include "Form/DataField/Base.hpp"
#include "Form/RowFormWidget.hpp"
#include "UIGlobals.hpp"

enum ControlIndex {
  UnitsPreset,
  spacer_1,
  UnitsSpeed,
  UnitsDistance,
  UnitsLift,
  UnitsAltitude,
  UnitsTemperature,
  UnitsTaskSpeed,
  UnitsPressure,
  spacer_2,
  UnitsLatLon
};

class UnitsConfigPanel
  : public RowFormWidget, DataFieldListener {
public:
  UnitsConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

  void PresetCheck();

  /* methods from Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);

private:
  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df);
};

void
UnitsConfigPanel::PresetCheck()
{
  UnitSetting current_dlg_set;
  current_dlg_set.speed_unit = (Unit)GetValueInteger((unsigned)UnitsSpeed);
  current_dlg_set.wind_speed_unit = current_dlg_set.speed_unit;
  current_dlg_set.distance_unit = (Unit)GetValueInteger((unsigned)UnitsDistance);
  current_dlg_set.vertical_speed_unit = (Unit)GetValueInteger((unsigned)UnitsLift);
  current_dlg_set.altitude_unit = (Unit)GetValueInteger((unsigned)UnitsAltitude);
  current_dlg_set.temperature_unit = (Unit)GetValueInteger((unsigned)UnitsTemperature);
  current_dlg_set.task_speed_unit = (Unit)GetValueInteger((unsigned)UnitsTaskSpeed);
  current_dlg_set.pressure_unit = (Unit)GetValueInteger((unsigned)UnitsPressure);

  LoadValueEnum(UnitsPreset, Units::Store::EqualsPresetUnits(current_dlg_set));
}

void
UnitsConfigPanel::OnModified(DataField &df)
{
  PresetCheck();
}

void
UnitsConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const UnitSetting &config = CommonInterface::GetUISettings().units;
  const CoordinateFormat coordinate_format =
      CommonInterface::GetUISettings().coordinate_format;

  RowFormWidget::Prepare(parent, rc);

  static const TCHAR * preset_help = N_("Load a set of units.");
  WndProperty *wp = AddEnum(_("Preset"), NULL);
  DataFieldEnum &df = *(DataFieldEnum *)wp->GetDataField();
  df.EnableItemHelp(true);

  df.addEnumText(_("Custom"), (unsigned)0, _("My individual set of units."));
  unsigned len = Units::Store::Count();
  for (unsigned i = 0; i < len; i++)
    df.addEnumText(Units::Store::GetName(i), i+1, preset_help);

  LoadValueEnum(UnitsPreset, Units::Store::EqualsPresetUnits(config));
  wp->GetDataField()->SetListener(this);
  wp->SetReadOnly(true);

  AddSpacer();
  SetExpertRow(spacer_1);

  static const TCHAR * units_speed_help = N_("Units used for airspeed and ground speed.  "
      "A separate unit is available for task speeds.");
  static const StaticEnumChoice  units_speed_list[] = {
    { (unsigned)Unit::STATUTE_MILES_PER_HOUR,  _T("mph"), units_speed_help },
    { (unsigned)Unit::KNOTS,                N_("knots"), units_speed_help },
    { (unsigned)Unit::KILOMETER_PER_HOUR,     _T("km/h"), units_speed_help },
    { 0 }
  };
  AddEnum(_("Aircraft/Wind speed"), NULL, units_speed_list,
          (unsigned int)config.speed_unit, this);
  SetExpertRow(UnitsSpeed);

  static const TCHAR *units_distance_help = _("Units used for horizontal distances e.g. "
      "range to waypoint, distance to go.");
  static const StaticEnumChoice  units_distance_list[] = {
    { (unsigned)Unit::STATUTE_MILES,  _T("sm"), units_distance_help },
    { (unsigned)Unit::NAUTICAL_MILES, _T("nm"), units_distance_help },
    { (unsigned)Unit::KILOMETER,     _T("km"), units_distance_help },
    { 0 }
  };
  AddEnum(_("Distance"), NULL, units_distance_list,
          (unsigned)config.distance_unit, this);
  SetExpertRow(UnitsDistance);

  static const TCHAR *units_lift_help = _("Units used for vertical speeds (variometer).");
  static const StaticEnumChoice  units_lift_list[] = {
    { (unsigned)Unit::KNOTS,          N_("knots"), units_lift_help },
    { (unsigned)Unit::METER_PER_SECOND, _T("m/s"), units_lift_help },
    { (unsigned)Unit::FEET_PER_MINUTE,  _T("ft/min"), units_lift_help },
    { 0 }
  };
  AddEnum(_("Lift"), NULL, units_lift_list,
          (unsigned)config.vertical_speed_unit, this);
  SetExpertRow(UnitsLift);

  static const TCHAR *units_altitude_help = _("Units used for altitude and heights.");
  static const StaticEnumChoice  units_altitude_list[] = {
    { (unsigned)Unit::FEET,  N_("foot"), units_altitude_help },
    { (unsigned)Unit::METER, N_("meter"), units_altitude_help },
    { 0 }
  };
  AddEnum(_("Altitude"), NULL, units_altitude_list,
          (unsigned)config.altitude_unit, this);
  SetExpertRow(UnitsAltitude);

  static const TCHAR *units_temperature_help = _("Units used for temperature.");
  static const StaticEnumChoice  units_temperature_list[] = {
    { (unsigned)Unit::DEGREES_CELCIUS,    _T(DEG "C"), units_temperature_help },
    { (unsigned)Unit::DEGREES_FAHRENHEIT, _T(DEG "F"), units_temperature_help },
    { 0 }
  };
  AddEnum(_("Temperature"), NULL, units_temperature_list,
          (unsigned)config.temperature_unit, this);
  SetExpertRow(UnitsTemperature);

  static const TCHAR *units_taskspeed_help = _("Units used for task speeds.");
  static const StaticEnumChoice  units_taskspeed_list[] = {
    { (unsigned)Unit::STATUTE_MILES_PER_HOUR,  _T("mph"), units_taskspeed_help },
    { (unsigned)Unit::KNOTS,                N_("knots"), units_taskspeed_help },
    { (unsigned)Unit::KILOMETER_PER_HOUR,     _T("km/h"), units_taskspeed_help },
    { 0 }
  };
  AddEnum(_("Task speed"), NULL, units_taskspeed_list,
          (unsigned)config.task_speed_unit, this);
  SetExpertRow(UnitsTaskSpeed);

  static const TCHAR *units_pressure_help = _("Units used for pressures.");
  static const StaticEnumChoice pressure_labels_list[] = {
    { (unsigned)Unit::HECTOPASCAL, _T("hPa"), units_pressure_help },
    { (unsigned)Unit::MILLIBAR,    _T("mb"), units_pressure_help },
    { (unsigned)Unit::INCH_MERCURY, _T("inHg"), units_pressure_help },
    { 0 }
  };
  AddEnum(_("Pressure"), NULL, pressure_labels_list,
          (unsigned)config.pressure_unit, this);
  SetExpertRow(UnitsPressure);

  AddSpacer();
  SetExpertRow(spacer_2);

  static const TCHAR *units_lat_lon_help = _("Units used for latitude and longitude.");
  static const StaticEnumChoice units_lat_lon_list[] = {
    { (unsigned)CoordinateFormat::DDMMSS, _T("DDMMSS"), units_lat_lon_help },
    { (unsigned)CoordinateFormat::DDMMSS_SS, _T("DDMMSS.ss"), units_lat_lon_help },
    { (unsigned)CoordinateFormat::DDMM_MMM, _T("DDMM.mmm"), units_lat_lon_help },
    { (unsigned)CoordinateFormat::DD_DDDD, _T("DD.dddd"), units_lat_lon_help },
    { (unsigned)CoordinateFormat::UTM, _T("UTM"), units_lat_lon_help },
    { 0 }
  };
  AddEnum(_("Lat./Lon."), NULL, units_lat_lon_list,
          (unsigned)coordinate_format);
  SetExpertRow(UnitsLatLon);
}

bool
UnitsConfigPanel::Save(bool &_changed, bool &_require_restart)
{
  bool changed = false, require_restart = false;

  UnitSetting &config = CommonInterface::SetUISettings().units;
  CoordinateFormat &coordinate_format =
      CommonInterface::SetUISettings().coordinate_format;

  /* the Units settings affect how other form values are read and translated
   * so changes to Units settings should be processed after all other form settings
   */
  changed |= SaveValueEnum(UnitsSpeed, ProfileKeys::SpeedUnitsValue, config.speed_unit);
  config.wind_speed_unit = config.speed_unit; // Mapping the wind speed to the speed unit

  changed |= SaveValueEnum(UnitsDistance, ProfileKeys::DistanceUnitsValue, config.distance_unit);

  changed |= SaveValueEnum(UnitsLift, ProfileKeys::LiftUnitsValue, config.vertical_speed_unit);

  changed |= SaveValueEnum(UnitsAltitude, ProfileKeys::AltitudeUnitsValue, config.altitude_unit);

  changed |= SaveValueEnum(UnitsTemperature, ProfileKeys::TemperatureUnitsValue, config.temperature_unit);

  changed |= SaveValueEnum(UnitsTaskSpeed, ProfileKeys::TaskSpeedUnitsValue, config.task_speed_unit);

  changed |= SaveValueEnum(UnitsPressure, ProfileKeys::PressureUnitsValue, config.pressure_unit);

  changed |= SaveValueEnum(UnitsLatLon, ProfileKeys::LatLonUnits, coordinate_format);

  _changed |= changed;
  _require_restart |= require_restart;

  return true;
}

Widget *
CreateUnitsConfigPanel()
{
  return new UnitsConfigPanel();
}

