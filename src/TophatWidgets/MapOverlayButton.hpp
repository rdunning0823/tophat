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
#include "Look/InfoBoxLook.hpp"
#include "Util/tstring.hpp"
#include "Renderer/ButtonRenderer.hpp"

#include <tchar.h>

struct IconLook;
class ContainerWindow;
class Bitmap;
class ActionListener;
struct PixelRect;

class MapOverlayButton : public Button {

protected:
  /** optional text displayed after main text as a subscript */
  tstring subscript_text;

  /** optional text displayed under main text */
  tstring line_two_text;

  const IconLook &icon_look;
  const ButtonLook &button_look;
  const InfoBoxLook &infobox_look;
  /**
   * the bitmap displayed in the button
   */
  const Bitmap *bmp;

  ButtonFrameRenderer frame_renderer;

public:
  /**
   * size from 2 (tiny) to 6 (huge) of map overlay buttons
   */
  static unsigned GetScale();

  /**
   * returns standard button height.  Not scaled by GetScale()
   */
  static unsigned GetStandardButtonHeight();

public:

  MapOverlayButton(ContainerWindow &parent, const ButtonLook &_button_look,
                   const IconLook &_icon_look,
                   const InfoBoxLook &_infobox_look,
                    const Bitmap *_bmp,
                    const PixelRect &rc,
                    WindowStyle style,
                    ActionListener& listener, int id)
  :Button(parent, _button_look, _T(""), rc, style, listener, id),
   icon_look(_icon_look), button_look(_button_look),
   infobox_look(_infobox_look),
   bmp(_bmp),
   frame_renderer(_button_look) {}

  /**
   * The OnPaint event is called when the button needs to be drawn
   * (derived from PaintWindow)
   */
  virtual void OnPaint(Canvas &canvas);

  /**
   * updates subscript text of button
   */
  void SetSubscripText(const TCHAR * _text) {
    subscript_text = _text;
  }

  /**
   * updates line two text of button
   */
  void SetLineTwoText(const TCHAR * _text) {
    line_two_text = _text;
  }

  const Font &GetLargeFont() const
  {
    return infobox_look.value_font;
  }

  const Font &GetMediumFont() const
  {
    return infobox_look.title_font;
  }
};


#endif
