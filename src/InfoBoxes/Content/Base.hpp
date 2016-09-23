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

#ifndef XCSOAR_INFOBOX_CONTENT_HPP
#define XCSOAR_INFOBOX_CONTENT_HPP

#include "Compiler.h"

struct PixelRect;
struct InfoBoxData;
struct InfoBoxPanel;
class Canvas;

class InfoBoxContent
{
public:
  enum InfoBoxKeyCodes {
    ibkLeft = -2,
    ibkDown = -1,
    ibkUp = 1,
    ibkRight = 2
  };

  virtual ~InfoBoxContent();

  virtual void Update(InfoBoxData &data) = 0;
  virtual bool HandleKey(const InfoBoxKeyCodes keycode);

  virtual void OnCustomPaint(Canvas &canvas, const PixelRect &rc);

  gcc_pure
  virtual const InfoBoxPanel *GetDialogContent();

  /**
   * Tells consumer of content whether to display it
   * in a legacy tab format or not.
   */
  virtual bool ShowInTabLayout() const {
    return false;
  }
};

class InfoBoxContentNonTabbed : public InfoBoxContent {
  /**
   * Tells consumer of content to show in full screen format instead of tabbed
   */
  virtual bool ShowInTabLayout() const {
    return false;
  }

};

class InfoBoxContentTabbed : public InfoBoxContent {
  /**
   * Tells consumer of content to show in a tabbed layout (not full screen)
   */
  virtual bool ShowInTabLayout() const {
    return true;
  }

};

#endif
