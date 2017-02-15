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

#include "InfoBoxes/Content/Speed.hpp"
#include "Factory.hpp"
#include "InfoBoxes/Data.hpp"
#include "InfoBoxes/Panel/Panel.hpp"
#include "Interface.hpp"
#include "InfoBoxes/Panel/SpeedSimulator.hpp"


#include "Simulator.hpp"
#include "Blackboard/DeviceBlackboard.hpp"
#include "Components.hpp"
#include "Language/Language.hpp"
#include "Units/Units.hpp"


#ifdef __clang__
/* gcc gives "redeclaration differs in 'constexpr'" */
constexpr
#endif
const InfoBoxPanel ground_speed_simulator_infobox_panels[] = {
  { N_("Simulator"), LoadGroundSpeedSimulatorPanel },
  { nullptr, nullptr }
};

const InfoBoxPanel *
InfoBoxContentSpeedGround::GetDialogContent() {
  return ground_speed_simulator_infobox_panels;
}

void
InfoBoxContentSpeedGround::Update(InfoBoxData &data)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  if (!basic.ground_speed_available) {
    data.SetInvalid();
    return;
  }

  data.SetValueFromSpeed(basic.ground_speed, false);
}

#ifdef __clang__
/* gcc gives "redeclaration differs in 'constexpr'" */
constexpr
#endif
const InfoBoxPanel indicated_speed_simulator_infobox_panels[] = {
  { N_("Simulator"), LoadIndicatedSpeedSimulatorPanel },
  { nullptr, nullptr }
};

const InfoBoxPanel *
InfoBoxContentSpeedIndicated::GetDialogContent() {
  return indicated_speed_simulator_infobox_panels;
}

void
InfoBoxContentSpeedIndicated::Update(InfoBoxData &data)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  if (!basic.airspeed_available) {
    data.SetInvalid();
    return;
  }

  data.SetValueFromSpeed(basic.indicated_airspeed, false);
}


#ifdef __clang__
/* gcc gives "redeclaration differs in 'constexpr'" */
constexpr
#endif
const InfoBoxPanel true_speed_simulator_infobox_panels[] = {
  { N_("Simulator"), LoadTrueSpeedSimulatorPanel },
  { nullptr, nullptr }
};

const InfoBoxPanel *
InfoBoxContentSpeed::GetDialogContent() {
  return true_speed_simulator_infobox_panels;
}

void
InfoBoxContentSpeed::Update(InfoBoxData &data)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  if (!basic.airspeed_available) {
    data.SetInvalid();
    return;
  }

  data.SetValueFromSpeed(basic.true_airspeed, false);
}

void
UpdateInfoBoxSpeedMacCready(InfoBoxData &data)
{
  const CommonStats &common_stats = CommonInterface::Calculated().common_stats;
  data.SetValueFromSpeed(common_stats.V_block, false);
}

void
UpdateInfoBoxSpeedDolphin(InfoBoxData &data)
{
  // Set Value
  const DerivedInfo &calculated = CommonInterface::Calculated();
  data.SetValueFromSpeed(calculated.V_stf, false);

  // Set Comment
  if (CommonInterface::GetComputerSettings().features.block_stf_enabled)
    data.SetComment(_("BLOCK"));
  else
    data.SetComment(_("DOLPHIN"));

}
