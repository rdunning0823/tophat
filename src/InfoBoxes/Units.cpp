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

#include "Data.hpp"
#include "Units/Units.hpp"
#include "Formatter/UserUnits.hpp"

void
InfoBoxData::SetValueFromDistance(fixed new_value)
{

  FormatUserDistance(new_value, value.buffer(), false, 1);
  SetValueUnit(Units::GetUserDistanceUnit());
}

void
InfoBoxData::SetValueFromAltitude(fixed new_value)
{
  FormatUserAltitude(new_value, value.buffer(), false);
  SetValueUnit(Units::current.altitude_unit);
}

void
InfoBoxData::SetValueFromArrival(fixed new_value)
{
  FormatRelativeUserAltitude(new_value, value.buffer(), false);
  SetValueUnit(Units::current.altitude_unit);
}

void
InfoBoxData::SetValueFromSpeed(fixed new_value, bool precision)
{
  FormatUserSpeed(new_value, value.buffer(), false, precision);
  SetValueUnit(Units::current.speed_unit);
}

void
InfoBoxData::SetCommentFromDistance(fixed new_value)
{
  FormatUserDistance(new_value, comment.buffer(), true);
}

void
InfoBoxData::SetCommentFromAlternateAltitude(fixed new_value)
{
  FormatAlternateUserAltitude(new_value, comment.buffer());
}

void
InfoBoxData::SetCommentFromSpeed(fixed new_value, bool precision)
{
  FormatUserSpeed(new_value, comment.buffer(), true, precision);
}

void
InfoBoxData::SetCommentFromVerticalSpeed(fixed new_value, bool include_sign)
{
  FormatUserVerticalSpeed(new_value, comment.buffer(), true, include_sign);
}
