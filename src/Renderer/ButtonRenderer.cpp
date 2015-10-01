/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
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

#include "ButtonRenderer.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "Look/ButtonLook.hpp"
#include "Util/Macros.hpp"

unsigned
ButtonFrameRenderer::GetMargin()
{
  return Layout::VptScale(2);
}

void
ButtonFrameRenderer::DrawButton(Canvas &canvas, PixelRect rc,
                                bool focused, bool pressed) const
{
  const ButtonLook::StateLook &_look = focused ? look.focused : look.standard;

  canvas.Select(pressed ? _look.light_border_pen :
      _look.dark_border_pen);
  rc.right -= 1;
  rc.top += 1;

  if (look.background_transparent && !pressed)
    canvas.SelectHollowBrush();
  else
    canvas.Select(Brush(_look.background_color));

  canvas.DrawRoundRectangle(rc.left, rc.top,
                            rc.right, rc.bottom - 1,
                            20,
                            20);
}

PixelRect
ButtonFrameRenderer::GetDrawingRect(PixelRect rc, bool pressed) const
{
  rc.Grow(-2);
  if (pressed)
    rc.Offset(1, 1);

  return rc;
}

unsigned
ButtonRenderer::GetMinimumButtonWidth() const
{
  return Layout::GetMaximumControlHeight();
}
