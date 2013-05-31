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

#include "InfoBoxes/Content/Team.hpp"
#include "InfoBoxes/Panel/TeamCode.hpp"
#include "InfoBoxes/Data.hpp"
#include "Interface.hpp"
#include "Formatter/Units.hpp"
#include "Language/Language.hpp"
#include "Util/Macros.hpp"

#include <tchar.h>
#include <stdio.h>

static constexpr InfoBoxContentTeamCode::PanelContent panels[] = {
  InfoBoxContentTeamCode::PanelContent (
    N_("Edit"),
    LoadTeamCodePanel),
};

const InfoBoxContentTeamCode::DialogContent InfoBoxContentTeamCode::dlgContent = {
  ARRAY_SIZE(panels), &panels[0], false,
};

const InfoBoxContentTeamCode::DialogContent*
InfoBoxContentTeamCode::GetDialogContent() {
  return &dlgContent;
}

void
InfoBoxContentTeamCode::Update(InfoBoxData &data)
{
  const TeamCodeSettings &settings =
    CommonInterface::GetComputerSettings().team_code;
  const TeamInfo &teamcode_info = XCSoarInterface::Calculated();

  if (!settings.team_code_reference_waypoint) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValue(XCSoarInterface::Calculated().own_teammate_code.GetCode());

  // Set Comment
  if (teamcode_info.flarm_teammate_code_available) {
    data.SetComment(teamcode_info.flarm_teammate_code.GetCode());
    data.SetCommentColor(teamcode_info.flarm_teammate_code_current ? 2 : 1);
  } else if (settings.team_code_valid) {
    data.SetComment(settings.team_code.GetCode());
    data.SetCommentColor(0);
  }
  else
    data.SetCommentInvalid();
}

void
InfoBoxContentTeamBearing::Update(InfoBoxData &data)
{
  const TeamCodeSettings &settings =
    CommonInterface::GetComputerSettings().team_code;
  const TrafficList &flarm = XCSoarInterface::Basic().flarm.traffic;
  const TeamInfo &teamcode_info = CommonInterface::Calculated();

  if (teamcode_info.teammate_available) {
    // Set Value
    data.SetValue(teamcode_info.teammate_vector.bearing);
  }
  else
    data.SetValueInvalid();

  // Set Comment
  if (!settings.team_flarm_id.IsDefined())
    data.SetCommentInvalid();
  else if (!settings.team_flarm_callsign.empty())
    data.SetComment(settings.team_flarm_callsign.c_str());
  else
    data.SetComment(_T("???"));

  if (flarm.FindTraffic(settings.team_flarm_id) != NULL)
    data.SetCommentColor(2);
  else
    data.SetCommentColor(1);
}

void
InfoBoxContentTeamBearingDiff::Update(InfoBoxData &data)
{
  const TeamCodeSettings &settings =
    CommonInterface::GetComputerSettings().team_code;
  const NMEAInfo &basic = XCSoarInterface::Basic();
  const TrafficList &flarm = basic.flarm.traffic;
  const TeamInfo &teamcode_info = CommonInterface::Calculated();

  if (teamcode_info.teammate_available && basic.track_available) {
    // Set Value
    Angle Value = teamcode_info.teammate_vector.bearing - basic.track;
    data.SetValueFromBearingDifference(Value);
  } else
    data.SetValueInvalid();

  // Set Comment
  if (!settings.team_flarm_id.IsDefined())
    data.SetCommentInvalid();
  else if (!StringIsEmpty(settings.team_flarm_callsign))
    data.SetComment(settings.team_flarm_callsign);
  else
    data.SetComment(_T("???"));

  if (flarm.FindTraffic(settings.team_flarm_id) != NULL)
    data.SetCommentColor(2);
  else
    data.SetCommentColor(1);
}

void
InfoBoxContentTeamDistance::Update(InfoBoxData &data)
{
  const TeamCodeSettings &settings =
    CommonInterface::GetComputerSettings().team_code;
  const TeamInfo &teamcode_info = CommonInterface::Calculated();

  // Set Value
  if (teamcode_info.teammate_available)
    data.SetValueFromDistance(teamcode_info.teammate_vector.distance);
  else
    data.SetValueInvalid();

  // Set Comment
  if (!settings.team_flarm_id.IsDefined())
    data.SetCommentInvalid();
  else if (!StringIsEmpty(settings.team_flarm_callsign))
    data.SetComment(settings.team_flarm_callsign);
  else
    data.SetComment(_T("???"));

  data.SetCommentColor(teamcode_info.flarm_teammate_code_current ? 2 : 1);
}
