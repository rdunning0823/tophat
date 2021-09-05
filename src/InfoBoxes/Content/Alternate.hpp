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

#ifndef XCSOAR_INFOBOX_CONTENT_ALTERNATE_HPP
#define XCSOAR_INFOBOX_CONTENT_ALTERNATE_HPP

#include "InfoBoxes/Content/Base.hpp"
#include <tchar.h>

class InfoBoxContentAlternateName : public InfoBoxContent
{
public:
  InfoBoxContentAlternateName(const unsigned _index)
  :index(_index) {}
  virtual void Update(InfoBoxData &data) override;
  virtual bool HandleKey(const InfoBoxKeyCodes keycode) override;
  virtual const InfoBoxPanel *GetDialogContent() override;

private:
  unsigned index;
};

class InfoBoxContentAlternateGR : public InfoBoxContent
{
public:
  InfoBoxContentAlternateGR(const unsigned _index)
  :index(_index) {}
  virtual void Update(InfoBoxData &data) override;
  virtual bool HandleKey(const InfoBoxKeyCodes keycode) override;
  virtual const InfoBoxPanel *GetDialogContent() override;
  static void FormatAlternateTitle(InfoBoxData &data, unsigned index, const TCHAR* locale_key, const TCHAR* default_title);

private:
  unsigned index;
};

#endif
