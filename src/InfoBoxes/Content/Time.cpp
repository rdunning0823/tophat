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

#include "InfoBoxes/Content/Time.hpp"
#include "InfoBoxes/Panel/TimeInfoBox.hpp"
#include "InfoBoxes/Data.hpp"
#include "Interface.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Formatter/LocalTimeFormatter.hpp"
#include "Language/Language.hpp"
#include "InfoBoxes/Panel/Panel.hpp"

#include <tchar.h>

/*
 * Subpart callback function pointers
 */

static constexpr InfoBoxPanel panels[] = {
  { N_("Edit"), LoadTimeInfoBoxPanel },
  { nullptr, nullptr }
};

const InfoBoxPanel *
InfoBoxContentTime::GetDialogContent() {
  return panels;
}

/*
 * Subpart normal operations
 */

void
InfoBoxContentTimeLocal::Update(InfoBoxData &data)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const ComputerSettings &settings = CommonInterface::GetComputerSettings();

  if (!basic.time_available) {
    data.SetInvalid();
    return;
  }

  // Set Value
  FormatLocalTimeHHMM(data.value.buffer(), (int)basic.time,
                      settings.utc_offset);

  // Set Comment
  data.UnsafeFormatComment(_T("%02u"), basic.date_time_utc.second);
}

void
InfoBoxContentTimeUTC::Update(InfoBoxData &data)
{
  const NMEAInfo &basic = CommonInterface::Basic();

  if (!basic.time_available) {
    data.SetInvalid();
    return;
  }

  // Set Value
  const BrokenDateTime t = basic.date_time_utc;
  data.UnsafeFormatValue(_T("%02d:%02d"), t.hour, t.minute);

  // Set Comment
  data.UnsafeFormatComment(_T("%02d"), t.second);
}

void
UpdateInfoBoxTimeFlight(InfoBoxData &data)
{
  const FlyingState &flight = CommonInterface::Calculated().flight;

  if (!positive(flight.flight_time)) {
    data.SetInvalid();
    return;
  }

  // Set Value
  TCHAR value[32], comment[32];
  FormatTimeTwoLines(value, comment, (int)flight.flight_time);

  data.SetValue(value);
  data.SetComment(comment);
}
