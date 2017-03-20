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

#include "InfoBoxes/Content/Altitude.hpp"
#include "Factory.hpp"
#include "InfoBoxes/Data.hpp"
#include "InfoBoxes/Panel/Panel.hpp"
#include "InfoBoxes/Panel/AltitudeSimulator.hpp"
#include "InfoBoxes/Panel/BarometricPressure.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "Units/Units.hpp"
#include "Interface.hpp"
#include "Components.hpp"
#include "Engine/Waypoint/Waypoint.hpp"
#include "Engine/Waypoint/Waypoints.hpp"
#include "Language/Language.hpp"
#include "Blackboard/DeviceBlackboard.hpp"
#include "Components.hpp"
#include "Simulator.hpp"

#include <tchar.h>
#include <stdio.h>

/*
 * Subpart callback function pointers
 */

#ifdef __clang__
/* gcc gives "redeclaration differs in 'constexpr'" */
constexpr
#endif
const InfoBoxPanel agl_altitude_infobox_panels[] = {
  { N_("Simulator"), LoadAglAltitudeSimulatorPanel },
  { nullptr, nullptr }
};

#ifdef __clang__
/* gcc gives "redeclaration differs in 'constexpr'" */
constexpr
#endif
const InfoBoxPanel gps_altitude_infobox_panels[] = {
  { N_("Simulator"), LoadGpsAltitudeSimulatorPanel },
  { nullptr, nullptr }
};

#ifdef __clang__
/* gcc gives "redeclaration differs in 'constexpr'" */
constexpr
#endif
const InfoBoxPanel barometric_altitude_infobox_panels[] = {
  { N_("Simulator"), LoadBarometricPressurePanel },
  { nullptr, nullptr }
};

const InfoBoxPanel *
InfoBoxContentAltitudeGPS::GetDialogContent() {
  return gps_altitude_infobox_panels;
}

const InfoBoxPanel *
InfoBoxContentAltitudeAGL::GetDialogContent() {
  return agl_altitude_infobox_panels;
}

void
UpdateInfoBoxAltitudeNav(InfoBoxData &data)
{
  const MoreData &basic = CommonInterface::Basic();

  if (!basic.NavAltitudeAvailable()) {
    data.SetInvalid();

    if (basic.pressure_altitude_available)
      data.SetComment(_("no QNH"));

    return;
  }

  const ComputerSettings &settings_computer = CommonInterface::GetComputerSettings();
  const InfoBoxSettings &settings_info_boxes = CommonInterface::GetUISettings().info_boxes;

  if (basic.baro_altitude_available &&
      settings_computer.features.nav_baro_altitude_enabled)
    data.SetTitle(InfoBoxFactory::GetCaption(InfoBoxFactory::e_H_Baro));
  else
    data.SetTitle(InfoBoxFactory::GetCaption(InfoBoxFactory::e_HeightGPS));

  data.SetValueFromAltitude(basic.nav_altitude);
  if (settings_info_boxes.show_alternative_altitude_units)
    data.SetCommentFromAlternateAltitude(basic.nav_altitude);
  else
    data.SetCommentInvalid();
}

void
InfoBoxContentAltitudeGPS::Update(InfoBoxData &data)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const InfoBoxSettings &settings_info_boxes = CommonInterface::GetUISettings().info_boxes;

  if (!basic.gps_altitude_available) {
    data.SetInvalid();
    return;
  }

  data.SetValueFromAltitude(basic.gps_altitude);
  if (settings_info_boxes.show_alternative_altitude_units)
    data.SetCommentFromAlternateAltitude(basic.gps_altitude);
  else
    data.SetCommentInvalid();
}

static void
ChangeAltitude(const fixed step)
{
  const NMEAInfo &basic = CommonInterface::Basic();

  device_blackboard->SetAltitude(basic.gps_altitude +
                                 (fixed)Units::ToSysAltitude(step));
}

bool
InfoBoxContentAltitudeGPS::HandleKey(const InfoBoxKeyCodes keycode)
{
  const NMEAInfo &basic = CommonInterface::Basic();

  if (!is_simulator())
    return false;
  if (!basic.gps.simulator)
    return false;

  const Angle a5 = Angle::Degrees(5);

  switch (keycode) {
  case ibkUp:
    ChangeAltitude(fixed(+100));
    return true;

  case ibkDown:
    ChangeAltitude(fixed(-100));
    return true;

  case ibkLeft:
    device_blackboard->SetTrack(
        basic.track - a5);
    return true;

  case ibkRight:
    device_blackboard->SetTrack(
        basic.track + a5);
    return true;
  }

  return false;
}

void
InfoBoxContentAltitudeAGL::Update(InfoBoxData &data)
{
  const DerivedInfo &calculated = CommonInterface::Calculated();
  const InfoBoxSettings &settings_info_boxes = CommonInterface::GetUISettings().info_boxes;
  if (!calculated.altitude_agl_valid) {
    data.SetInvalid();
    return;
  }

  data.SetValueFromAltitude(calculated.altitude_agl);
  if (settings_info_boxes.show_alternative_altitude_units)
    data.SetCommentFromAlternateAltitude(calculated.altitude_agl);
  else
    data.SetCommentInvalid();

  // Set Color (red/black)
  data.SetValueColor(calculated.altitude_agl <
      CommonInterface::GetComputerSettings().task.route_planner.safety_height_terrain ? 1 : 0);
}

void
UpdateInfoBoxAltitudeBaro(InfoBoxData &data)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const InfoBoxSettings &settings_info_boxes = CommonInterface::GetUISettings().info_boxes;

  if (!basic.baro_altitude_available) {
    data.SetInvalid();

    if (basic.pressure_altitude_available)
      data.SetComment(_("no QNH"));

    return;
  }

  data.SetValueFromAltitude(basic.baro_altitude);
  if (settings_info_boxes.show_alternative_altitude_units)
    data.SetCommentFromAlternateAltitude(basic.baro_altitude);
  else
    data.SetCommentInvalid();
}

void
UpdateInfoBoxAltitudeQFE(InfoBoxData &data)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const FlyingState &flight = CommonInterface::Calculated().flight;

  if (!basic.gps_altitude_available ||
      !flight.flying ||
      flight.takeoff_altitude < fixed(-999)) {
    data.SetInvalid();
    return;
  }

  fixed Value = basic.gps_altitude - flight.takeoff_altitude;

  data.SetValueFromAltitude(Value);
}

void
UpdateInfoBoxAltitudeFlightLevel(InfoBoxData &data)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const ComputerSettings &settings_computer =
    CommonInterface::GetComputerSettings();

  if (basic.pressure_altitude_available) {
    fixed Altitude = Units::ToUserUnit(basic.pressure_altitude, Unit::FEET);

    // Title color black
    data.SetTitleColor(0);

    // Set Value
    data.UnsafeFormatValue(_T("%03d"), iround(Altitude / 100));

    // Set Comment
    data.UnsafeFormatComment(_T("%dft"), iround(Altitude));

  } else if (basic.gps_altitude_available &&
             settings_computer.pressure_available) {
    // Take gps altitude as baro altitude. This is inaccurate but still fits our needs.
    const AtmosphericPressure &qnh = settings_computer.pressure;
    fixed Altitude = Units::ToUserUnit(qnh.QNHAltitudeToPressureAltitude(basic.gps_altitude), Unit::FEET);

    // Title color red
    data.SetTitleColor(1);

    // Set Value
    data.UnsafeFormatValue(_T("%03d"), iround(Altitude / 100));

    // Set Comment
    data.UnsafeFormatComment(_T("%dft"), iround(Altitude));

  } else if ((basic.baro_altitude_available || basic.gps_altitude_available) &&
             !settings_computer.pressure_available) {
    data.SetInvalid();
    data.SetComment(_("no QNH"));
  } else {
    data.SetInvalid();
  }
}
