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

#ifndef XCSOAR_ZOOM_BUTTON_HPP
#define XCSOAR_ZOOM_BUTTON_HPP

#include "Form/Button.hpp"
#include <tchar.h>

struct ButtonLook;
struct IconLook;
class ContainerWindow;
struct PixelRect;

/**
 * a class that is a button that draws the Map Scale information
 */
class ZoomButton : public WndButton {
public:
  /**
   * if  true, paint zoom in, else zoom out
   */
  bool zoom_in;

  const IconLook &icon_look;
  const ButtonLook &button_look;

  ZoomButton(ContainerWindow &parent, const ButtonLook &_button_look,
             const IconLook &_icon_look,
             const PixelRect &rc,
             ButtonWindowStyle style,
             bool _zoom_in,
             ActionListener &listener, int id)
  :WndButton(parent, _button_look, _T(""), rc, style, listener, id),
  zoom_in(_zoom_in), icon_look(_icon_look), button_look(_button_look) {};

  /**
   * The OnPaint event is called when the button needs to be drawn
   * (derived from PaintWindow)
   */
  virtual void OnPaint(Canvas &canvas);

  /**
   * handles on mouse move, and if dragged off button face, cancels drag
   * This allows background object to accept capture at this time
   */
  bool OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys);
};

#endif
