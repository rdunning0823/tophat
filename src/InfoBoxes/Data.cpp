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
#include "Language/Language.hpp"

void
InfoBoxData::Clear()
{
  title.clear();
  SetInvalid();
}

void
InfoBoxData::SetNotConfigured()
{
  SetAllColors(0);
  SetValue(_T("N/A"));
  SetValueUnit(Unit::UNDEFINED);
  SetComment(_("Not configured"));
}

void
InfoBoxData::SetClickToConfigure()
{
  SetAllColors(0);
  SetValue(_T(""));
  SetValueUnit(Unit::UNDEFINED);
  SetComment(_("Click to configure"));
}

void
InfoBoxData::SetInvalid()
{
  SetAllColors(0);
  SetValueInvalid();
  SetValueUnit(Unit::UNDEFINED);
  SetCommentInvalid();
}

void
InfoBoxData::SetValueInvalid()
{
  SetValue(_T("---"));
  SetValueUnit(Unit::UNDEFINED);
}

void
InfoBoxData::SetTitle(const TCHAR *_title)
{
  title = _title;
  title.CropIncompleteUTF8();
}

void
InfoBoxData::SetValue(const TCHAR *_value)
{
  value = _value;
}

void
InfoBoxData::SetComment(const TCHAR *_comment)
{
  comment = _comment;
  comment.CropIncompleteUTF8();
}

void
InfoBoxData::SetAllColors(unsigned color)
{
  SetTitleColor(color);
  SetValueColor(color);
  SetCommentColor(color);
}

bool
InfoBoxData::CompareTitle(const InfoBoxData &other) const
{
  return title == other.title &&
    title_color == other.title_color;
}

bool
InfoBoxData::CompareValue(const InfoBoxData &other) const
{
  return value == other.value &&
    value_unit == other.value_unit &&
    value_color == other.value_color;
}

bool
InfoBoxData::CompareComment(const InfoBoxData &other) const
{
  return comment == other.comment &&
    comment_color == other.comment_color;
}
