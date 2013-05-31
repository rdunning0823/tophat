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

#ifndef XCSOAR_INFOBOX_CONTENT_ALTERNATE_HPP
#define XCSOAR_INFOBOX_CONTENT_ALTERNATE_HPP

#include "InfoBoxes/Content/Base.hpp"

class InfoBoxContentAlternate : public InfoBoxContent
{
protected:
  unsigned index;
  static const DialogContent dlgContent;

public:
  InfoBoxContentAlternate(const unsigned _index):
    index(_index) {}

  virtual const DialogContent *GetDialogContent();

  /**
   * sets the index of the Alternate for this IB and invalidates the IB data
   */
  virtual bool HandleQuickAccess(const TCHAR *misc);

  /**
   * @param misc
   * returns the index of this IB if misc is set to "index"
   */
  virtual unsigned GetQuickAccess() {
    return index;
  }
};

class InfoBoxContentAlternateName : public InfoBoxContentAlternate
{
public:
  InfoBoxContentAlternateName(const unsigned _index):
    InfoBoxContentAlternate(_index) {}
  virtual void Update(InfoBoxData &data);
};

class InfoBoxContentAlternateGR : public InfoBoxContentAlternate
{
public:
  InfoBoxContentAlternateGR(const unsigned _index):
    InfoBoxContentAlternate(_index) {}
  virtual void Update(InfoBoxData &data);
};

#endif
