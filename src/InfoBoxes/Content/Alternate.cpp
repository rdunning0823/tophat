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

#include "InfoBoxes/Content/Alternate.hpp"
#include "InfoBoxes/Panel/Panel.hpp"
#include "InfoBoxes/Panel/AlternateFullScreen.hpp"
#include "InfoBoxes/Data.hpp"
#include "InfoBoxes/InfoBoxTitleLocale.hpp"
#include "Util/StringFormat.hpp"
#include "Interface.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Engine/Util/Gradient.hpp"
#include "Engine/Task/Unordered/AlternateList.hpp"
#include "Dialogs/Task/TaskDialogs.hpp"
#include "Language/Language.hpp"

#include <stdio.h>
#include <tchar.h>


static constexpr
InfoBoxPanel alternate_infobox_panels[] = {
  { N_("Alternates"), LoadAlternatesPanelFullScreen },
  { nullptr, nullptr }
};

void
InfoBoxContentAlternateName::Update(InfoBoxData &data)
{
  if (protected_task_manager == NULL) {
    data.SetInvalid();
    return;
  }

  ProtectedTaskManager::Lease lease(*protected_task_manager);
  const AlternateList &alternates = lease->GetAlternates();

  const AlternatePoint *alternate;
  if (!alternates.empty()) {
    if (index >= alternates.size())
      index = alternates.size() - 1;

    alternate = &alternates[index];
  } else {
    alternate = NULL;
  }

  // Get localised custom title and Format it
  InfoBoxContentAlternateGR::FormatAlternateTitle(data, index, _T("Altn %d"), _T("Altn %d"));

  if (alternate == NULL || !CommonInterface::Basic().track_available) {
    data.SetInvalid();
    return;
  }

  data.SetComment(alternate->waypoint.name.c_str());

  // Set Value
  Angle Value = alternate->solution.vector.bearing -
    CommonInterface::Basic().track;

  data.SetValueFromBearingDifference(Value);

  // Set Color (blue/black)
  data.SetValueColor(alternate->solution.IsFinalGlide() ? 2 : 0);
}

bool
InfoBoxContentAlternateName::HandleKey(const InfoBoxKeyCodes keycode)
{
  switch (keycode) {
  case ibkLeft:
  case ibkUp:
    if (index > 0)
      index--;
    break;
  case ibkRight:
  case ibkDown:
    index++;
    break;
  }

  return true;
}

const InfoBoxPanel *
InfoBoxContentAlternateName::GetDialogContent()
{
  return alternate_infobox_panels;
}

void
InfoBoxContentAlternateGR::Update(InfoBoxData &data)
{
  const MoreData &basic = CommonInterface::Basic();
  if (!basic.NavAltitudeAvailable()) {
    data.SetInvalid();
    return;
  }

  if (protected_task_manager == NULL) {
    data.SetInvalid();
    return;
  }

  ProtectedTaskManager::Lease lease(*protected_task_manager);
  const AlternateList &alternates = lease->GetAlternates();

  const AlternatePoint *alternate;
  if (!alternates.empty()) {
    if (index >= alternates.size())
      index = alternates.size() - 1;

    alternate = &alternates[index];
  } else {
    alternate = NULL;
  }

  // Get localised custom title and Format it
  FormatAlternateTitle(data, index, _T("Altn%d GR"), _T("Altn %d GR"));

  if (alternate == NULL) {
    data.SetInvalid();
    return;
  }

  data.SetComment(alternate->waypoint.name.c_str());

  fixed gradient = ::CalculateGradient(alternate->waypoint, alternate->solution.vector.distance,
                                       basic, CommonInterface::GetComputerSettings().task.GRSafetyHeight());
  if (negative(gradient)) {
    data.SetValueColor(0);
    data.SetValue(_T("+++"));
    return;
  }
  if (::GradientValid(gradient)) {
    data.SetValueFromGlideRatio(gradient);
  } else {
    data.SetInvalid();
  }

  // Set Color (blue/black)
  data.SetValueColor(alternate->solution.IsFinalGlide() ? 2 : 0);
}

/**
 * Format title.
 * Replace default string with locale string if available
 *
 * @param data
 * @param index
 * @param locale_key The key to use to lookup the localised custom title
 * @param default_title name The default title to use if no custom title is found
 */
void
InfoBoxContentAlternateGR::FormatAlternateTitle(InfoBoxData &data, unsigned index, const TCHAR* locale_key, const TCHAR* default_title)
{
  // Get localised custom key
  StaticString<20> locale_key_indexed;
  locale_key_indexed.Format(locale_key, index+1);

  // Use localised title if any, fall back to default otherwise
  const TCHAR* title_locale = InfoBoxTitleLocale::GetLocale(locale_key_indexed); // Get localised custom title
  if (title_locale!=nullptr)
    data.FormatTitle(title_locale,index + 1);
  else
    data.FormatTitle(default_title, index + 1);
}

bool
InfoBoxContentAlternateGR::HandleKey(const InfoBoxKeyCodes keycode)
{
  switch (keycode) {
  case ibkLeft:
  case ibkUp:
    if (index > 0)
      index--;
    break;
  case ibkRight:
  case ibkDown:
    index++;
    break;
  }

  return true;
}

const InfoBoxPanel *
InfoBoxContentAlternateGR::GetDialogContent()
{
  return alternate_infobox_panels;
}
