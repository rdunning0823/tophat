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

#ifndef XCSOAR_BUTTON_RENDERER_HPP
#define XCSOAR_BUTTON_RENDERER_HPP

#include "Compiler.h"

struct PixelRect;
struct ButtonLook;
class Canvas;

class ButtonFrameRenderer {
  const ButtonLook &look;
  /* draw rounded corners */
  bool rounded;

public:
  explicit ButtonFrameRenderer(const ButtonLook &_look):look(_look),
#if defined(USE_GDI)
  rounded(false) {}
#else
  rounded(true) {}
#endif

  const ButtonLook &GetLook() const {
    return look;
  }

  void SetRounded(bool _rounded) {
    rounded = _rounded;
  }

  gcc_const
  static unsigned GetMargin();

  /**
   * @param force_transparent_background: will draw dransparent background when
   * not pressed, regardless of settings in look
   */
  void DrawButton(Canvas &canvas, PixelRect rc,
                  bool focused, bool pressed,
                  bool force_transparent_background = false) const;

  gcc_pure
  PixelRect GetDrawingRect(PixelRect rc, bool pressed) const;
};

class ButtonRenderer {
public:
  virtual ~ButtonRenderer() {}

  virtual void SetRounded(bool rounded) {};

  gcc_pure
  virtual unsigned GetMinimumButtonWidth() const;

  virtual void DrawButton(Canvas &canvas, const PixelRect &rc,
                          bool enabled, bool focused, bool pressed,
                          bool force_transparent_background) const = 0;
};

#endif
