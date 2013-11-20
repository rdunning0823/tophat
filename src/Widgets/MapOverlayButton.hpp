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

#ifndef XCSOAR_MAP_OVERLAY_BUTTON_SIZE_HPP
#define XCSOAR_MAP_OVERLAY_BUTTON_SIZE_HPP

#include "Form/Button.hpp"
#include "Look/ButtonLook.hpp"

#include <tchar.h>

struct IconLook;
class ContainerWindow;
class Bitmap;
class ActionListener;
struct PixelRect;

/**
 * a shared properties of all the map overlay buttons
 */

namespace MapOverlayButton {

  /**
   * size from 2 (tiny) to 6 (huge) of map overlay buttons
   */
  unsigned GetScale();

  /**
   * returns standard button height.  Not scaled by GetScale()
   */
  unsigned GetStandardButtonHeight();

}

class OverlayButton : public WndButton {
protected:
  const IconLook &icon_look;
  const ButtonLook &button_look;
  /**
   * the bitmap displayed in the button
   */
  const Bitmap *bmp;

public:

  OverlayButton(ContainerWindow &parent, const ButtonLook &_button_look,
                 const IconLook &_icon_look,
                 const Bitmap *_bmp, const PixelRect &rc,
                 ButtonWindowStyle style,
                 ActionListener& listener, int id)
  :WndButton(parent, _button_look, _T(""), rc, style, listener, id),
   icon_look(_icon_look), button_look(_button_look), bmp(_bmp) {}

  /**
   * The OnPaint event is called when the button needs to be drawn
   * (derived from PaintWindow)
   */
  virtual void OnPaint(Canvas &canvas);

  /**
   * handles on mouse move, and if dragged off button face, cancels drag
   * This allows background object to accept capture at this time
   */
  virtual bool OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys);
};


#endif
