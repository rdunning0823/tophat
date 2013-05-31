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

#include "InfoBoxes/Content/Glide.hpp"
#include "Engine/Util/Gradient.hpp"
#include "Computer/GlideRatioCalculator.hpp"
#include "InfoBoxes/Data.hpp"
#include "InfoBoxes/Panel/GrAverage.hpp"
#include "Interface.hpp"
#include "Util/Macros.hpp"
#include "Language/Language.hpp"

#include <tchar.h>
#include <stdio.h>

void
InfoBoxContentGRInstant::Update(InfoBoxData &data)
{
  const fixed gr = XCSoarInterface::Calculated().gr;

  if (!::GradientValid(gr)) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValueFromGlideRatio(gr);
}

void
InfoBoxContentGRCruise::Update(InfoBoxData &data)
{
  const fixed cruise_gr = XCSoarInterface::Calculated().cruise_gr;

  if (!::GradientValid(cruise_gr)) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValueFromGlideRatio(cruise_gr);

  data.SetCommentFromDistance(XCSoarInterface::Basic().location.Distance(
      XCSoarInterface::Calculated().cruise_start_location));
}

static constexpr InfoBoxContentGRAvg::PanelContent panels[] = {
    InfoBoxContentGRAvg::PanelContent (
    N_("Set GR averager period"),
    LoadGrAveragePanel),
};

const InfoBoxContentGRAvg::DialogContent InfoBoxContentGRAvg::dlgContent = {
  ARRAY_SIZE(panels), &panels[0], false,
};

const InfoBoxContentGRAvg::DialogContent*
InfoBoxContentGRAvg::GetDialogContent() {
  return &dlgContent;
}

void
InfoBoxContentGRAvg::Update(InfoBoxData &data)
{
  const fixed average_gr = XCSoarInterface::Calculated().average_gr;

  if (average_gr == fixed_zero) {
    data.SetInvalid();
    return;
  }

  // Set Value
  if (average_gr < fixed_zero)
    data.SetValue(_T("^^^"));
  else if (!::GradientValid(average_gr))
    data.SetValue(_T("+++"));
  else
    data.SetValueFromGlideRatio(average_gr);
}

void
InfoBoxContentLDVario::Update(InfoBoxData &data)
{
  const fixed ld_vario = XCSoarInterface::Calculated().ld_vario;

  if (!::GradientValid(ld_vario) ||
      !XCSoarInterface::Basic().total_energy_vario_available ||
      !XCSoarInterface::Basic().airspeed_available) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValueFromGlideRatio(ld_vario);
}
