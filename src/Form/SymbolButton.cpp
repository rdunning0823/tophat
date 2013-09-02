/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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

#include "Form/SymbolButton.hpp"
#include "Formatter/HexColor.hpp"
#include "Look/DialogLook.hpp"
#include "Screen/Canvas.hpp"
#include "Look/IconLook.hpp"
#include "Screen/Bitmap.hpp"
#include "Renderer/SymbolRenderer.hpp"
#include "resource.h"
#include "Language/Language.hpp"
#include "Asset.hpp"
#include "Screen/Layout.hpp"

void
WndSymbolButton::OnPaint(Canvas &canvas)
{
  const ButtonLook &look = renderer.GetLook();

  const bool pressed = IsDown();
  const bool focused = HasCursorKeys() ? HasFocus() : pressed;

  PixelRect rc = canvas.GetRect();
  renderer.DrawButton(canvas, rc, focused, pressed, transparent);
  // If button has text on it
  const tstring caption = GetText();
  if (caption.empty())
    return;

  rc = renderer.GetDrawingRect(rc, pressed);

  canvas.SelectNullPen();
  if (!IsEnabled())
    canvas.Select(look.disabled.brush);
  else if (focused)
    canvas.Select(look.focused.foreground_brush);
  else
    canvas.Select(look.standard.foreground_brush);

  const char ch = (char)caption[0];

  RGB8Color color;

  // Draw arrow symbol instead of <
  if (ch == '<')
    SymbolRenderer::DrawArrow(canvas, rc, SymbolRenderer::LEFT);

  // Draw arrow symbol instead of >
  else if (ch == '>')
    SymbolRenderer::DrawArrow(canvas, rc, SymbolRenderer::RIGHT);

  // Draw arrow symbol instead of ^
  else if (ch == '^')
    SymbolRenderer::DrawArrow(canvas, rc, SymbolRenderer::UP);

  // Draw arrow symbol instead of v
  else if (ch == '^' || ch == 'v')
    SymbolRenderer::DrawArrow(canvas, rc, SymbolRenderer::DOWN);

  // Draw symbols instead of + and -
  else if (ch == '+' || ch == '-')
    SymbolRenderer::DrawSign(canvas, rc, ch == '+');

  // Draw Fly bitmap
  else if (caption == _T("Fly")) {
    Bitmap launcher1_bitmap(IDB_LAUNCHER1);
    launcher1_bitmap.EnableInterpolation();
    canvas.ClearWhite();
    if (pressed)
      canvas.StretchNot(launcher1_bitmap);
    else
      canvas.Stretch(launcher1_bitmap);
  }

  // Draw Simulator bitmap
  else if (caption == _T("Simulator")) {
    Bitmap launcher2_bitmap(IDB_LAUNCHER2);
    launcher2_bitmap.EnableInterpolation();
    canvas.ClearWhite();
    if (pressed)
      canvas.StretchNot(launcher2_bitmap);
    else
      canvas.Stretch(launcher2_bitmap);
  }

  else if (ParseHexColor(caption.c_str(), color)) {
    rc.Grow(-3);
    canvas.DrawFilledRectangle(rc, Color(color));

    //draw search icon
  } else if (caption == N_("Search") || caption == N_("SearchChecked")) {
    Bitmap bmp(caption == N_("Search") ?
        (Layout::scale == 1 ? IDB_SEARCH : IDB_SEARCH_HD) :
        (Layout::scale == 1 ? IDB_SEARCH_CHECKED : IDB_SEARCH_CHECKED_HD));

    const PixelSize bitmap_size = bmp.GetSize();
    const int offsetx = (rc.right - rc.left - bitmap_size.cx / 2) / 2;
    const int offsety = (rc.bottom - rc.top - bitmap_size.cy) / 2;
    if (IsDown())
      canvas.CopyNotOr(rc.left + offsetx,
                       rc.top + offsety,
                       bitmap_size.cx / 2,
                       bitmap_size.cy,
                       bmp,
                       bitmap_size.cx / 2, 0);
    else
      canvas.CopyAnd(rc.left + offsetx,
                      rc.top + offsety,
                      bitmap_size.cx / 2,
                      bitmap_size.cy,
                      bmp,
                      bitmap_size.cx / 2, 0);

  }

  //draw gear for set up icon
  else if (caption == N_("Setup")) {
    Bitmap bmp(Layout::scale == 1 ? IDB_SETTINGS : IDB_SETTINGS_HD);

    const PixelSize bitmap_size = bmp.GetSize();
    const int offsetx = (rc.right - rc.left - bitmap_size.cx / 2) / 2;
    const int offsety = (rc.bottom - rc.top - bitmap_size.cy) / 2;
    if (pressed)
      canvas.CopyNotOr(rc.left + offsetx,
                       rc.top + offsety,
                       bitmap_size.cx / 2,
                       bitmap_size.cy,
                       bmp,
                       bitmap_size.cx / 2, 0);
    else
      canvas.CopyAnd(rc.left + offsetx,
                      rc.top + offsety,
                      bitmap_size.cx / 2,
                      bitmap_size.cy,
                      bmp,
                      bitmap_size.cx / 2, 0);

  } else if (caption == N_("More") || caption == N_("Less")) {
    bool up = caption == N_("Less");
    // Draw arrow symbols instead of v and ^
    const Font &font = *look.font;
    canvas.Select(font);
    canvas.SetBackgroundTransparent();
    PixelSize text_size = font.TextSize(caption.c_str());

    UPixelScalar size = std::min(rc.right - rc.left, rc.bottom - rc.top) / 8;
    size = std::min(size, (UPixelScalar)(font.GetHeight() / 2));
    unsigned offset_x = (rc.right - rc.left -
        (text_size.cx + size * 2 + Layout::Scale(1))) / 2;

    unsigned left = rc.left + offset_x;

    RasterPoint Arrow[3];
    Arrow[0].x = left + size;
    Arrow[0].y = (rc.top + rc.bottom) / 2 +
                 (int)(up ? size : -size) * 2 / 3;
    Arrow[1].x = left;
    Arrow[1].y = (rc.top + rc.bottom) / 2 +
                 (up ? -size : size);
    Arrow[2].x = left - size;
    Arrow[2].y = (rc.top + rc.bottom) / 2 +
                 (int)(up ? size : -size) * 2 / 3;

    canvas.DrawText(left + size * 2 + Layout::Scale(1),
                    (rc.bottom - rc.top - text_size.cy) / 2 , caption.c_str());
    canvas.DrawTriangleFan(Arrow, 3);


    canvas.Select(Pen(Layout::Scale(1), COLOR_BLACK));
    canvas.SelectHollowBrush();
    canvas.DrawCircle(left, (rc.top + rc.bottom) / 2, (UPixelScalar)(size * 1.5));
  } else {
    WndButton::OnPaint(canvas);
  }
}
