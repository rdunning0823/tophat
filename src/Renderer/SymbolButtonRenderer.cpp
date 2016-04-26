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

#include "SymbolButtonRenderer.hpp"
#include "SymbolRenderer.hpp"
#include "Screen/Canvas.hpp"
#include "Look/ButtonLook.hpp"
#include "Look/IconLook.hpp"
#include "Screen/Bitmap.hpp"
#include "Renderer/SymbolRenderer.hpp"
#include "Language/Language.hpp"
#include "Resources.hpp"
#include "Asset.hpp"
#include "Screen/Layout.hpp"
#include "UIGlobals.hpp"
#include "Formatter/HexColor.hpp"
#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Texture.hpp"
#include "Screen/OpenGL/Scope.hpp"
#include "Screen/OpenGL/Compatibility.hpp"
#endif
#include <algorithm>
#ifndef USE_GDI
#include <winuser.h>
#endif

unsigned
SymbolButtonRenderer::GetMinimumButtonWidth() const
{
  unsigned text_width = TextButtonRenderer::GetMinimumButtonWidth();
  unsigned icon_width = 0;
  const IconLook &icon_look = UIGlobals::GetIconLook();

  switch (prefix_icon) {
  case PrefixIcon::NONE:
    break;
  case PrefixIcon::HOME:
    icon_width += icon_look.icon_home.GetSize().cx + Layout::GetTextPadding();
    break;
  case PrefixIcon::CHECK_MARK:
    icon_width += icon_look.hBmpCheckMark.GetSize().cx + Layout::GetTextPadding();
    break;
  case PrefixIcon::SEARCH:
    icon_width += icon_look.hBmpSearch.GetSize().cx + Layout::GetTextPadding();
    break;
  case PrefixIcon::SEARCH_CHECKED:
    icon_width += icon_look.hBmpSearchChecked.GetSize().cx + Layout::GetTextPadding();
    break;
  }
  return icon_width + text_width;
}

static void
DrawIconOrBitmap(Canvas &canvas, PixelRect rc, const MaskedIcon &icon, bool pressed)
{
  icon.Draw(canvas, rc, pressed);
}

void
SymbolButtonRenderer::DrawIconAndText(Canvas &canvas, PixelRect rc,
                                      const TCHAR *text, bool enabled, bool
                                      focused, bool pressed) const
{
  const ButtonLook &look = GetLook();
  const IconLook &icon_look = UIGlobals::GetIconLook();

  const Font &font = *look.font;
  PixelSize sz_text = font.TextSize(text);
  UPixelScalar padding = Layout::GetTextPadding();

  PixelSize sz_icon;
  sz_icon.cx = sz_icon.cy = 0;
  PixelRect rc_caption = rc;
  PixelRect rc_icon = rc;
  const MaskedIcon *icon = nullptr;
  switch (prefix_icon) {
  case PrefixIcon::NONE:
    break;
  case PrefixIcon::HOME:
    icon = &icon_look.icon_home;
    sz_icon = icon->GetSize();
    break;
  case PrefixIcon::CHECK_MARK:
    icon = &icon_look.hBmpCheckMark;
    sz_icon = icon->GetSize();
    break;
  case PrefixIcon::SEARCH:
    icon = &icon_look.hBmpSearch;
    sz_icon = icon->GetSize();
    break;
  case PrefixIcon::SEARCH_CHECKED:
    icon = &icon_look.hBmpSearchChecked;
    sz_icon = icon->GetSize();
    break;
  }

  rc_icon.left = (rc.GetSize().cx - sz_icon.cx - sz_text.cx - padding) / 2;
  rc_icon.right = rc_icon.left + sz_icon.cx;

  rc_caption.left = rc_icon.right +
      (prefix_icon == PrefixIcon::NONE ? 0 : padding);
  rc_caption.right = rc_caption.left + sz_text.cx + padding;

  if (prefix_icon != PrefixIcon::NONE) {
    icon->Draw(canvas, rc_icon, focused);
  }

  DrawCaption(canvas, text, rc_caption, enabled, focused, pressed);
}

inline void
SymbolButtonRenderer::DrawSymbol(Canvas &canvas, PixelRect rc, bool enabled,
                                 bool focused, bool pressed) const
{
  const ButtonLook &look = GetLook();

  // If button has text on it
  if (caption.empty() && prefix_icon == PrefixIcon::NONE)
    return;

  canvas.SelectNullPen();
  if (!enabled)
    canvas.Select(look.disabled.brush);
  else if (focused)
    canvas.Select(look.focused.foreground_brush);
  else
    canvas.Select(look.standard.foreground_brush);

  const char ch = (char)caption[0u];
  RGB8Color color;

  // Draw arrow symbol instead of <
  if (caption == _T("<"))
    SymbolRenderer::DrawArrow(canvas, rc, SymbolRenderer::LEFT);

  // Draw arrow symbol instead of >
  else if (caption == _T(">"))
    SymbolRenderer::DrawArrow(canvas, rc, SymbolRenderer::RIGHT);

  // Draw arrow symbol instead of ^
  else if (ch == '^')
    SymbolRenderer::DrawArrow(canvas, rc, SymbolRenderer::UP);

  // Draw arrow symbol instead of v
  else if (ch == 'v')
    SymbolRenderer::DrawArrow(canvas, rc, SymbolRenderer::DOWN);

  // Draw symbols instead of + and -
  else if (ch == '+' || ch == '-')
    SymbolRenderer::DrawSign(canvas, rc, ch == '+');

  /* "play" buttons */
  else if (caption == _T("<<"))
    SymbolRenderer::DrawDoubleArrow(canvas, rc, SymbolRenderer::LEFT);
  else if (caption == _T(">>"))
    SymbolRenderer::DrawDoubleArrow(canvas, rc, SymbolRenderer::RIGHT);
  else if (caption == _T("||"))
    SymbolRenderer::DrawPause(canvas, rc);
  else if (caption == _T("[]"))
    SymbolRenderer::DrawStop(canvas, rc);

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

  //draw gear for set up icon
  } else if (caption == _("Setup")) {
    const IconLook &icon_look = UIGlobals::GetIconLook();
    const MaskedIcon &icon = icon_look.hBmpTabSettings;
    DrawIconOrBitmap(canvas, rc, icon, focused);

  } else if (caption == _("_X")) {
    const IconLook &icon_look = UIGlobals::GetIconLook();
    if (!icon_look.valid) {
      DrawCaption(canvas, _("OK"), rc, enabled, focused, pressed);
    } else {
      const MaskedIcon &bmp = icon_look.hBmpClose;
      DrawIconOrBitmap(canvas, rc, bmp, focused);
    }
  } else if (prefix_icon != PrefixIcon::NONE) {
    DrawIconAndText(canvas, rc, caption.c_str(),
                    enabled, focused, pressed);

  } else if (caption == _("More") || caption == _("Less")) {
    bool up = caption == _("Less");
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
  } else

    DrawCaption(canvas, GetCaption(), rc, enabled, focused, pressed);
}

void
SymbolButtonRenderer::DrawButton(Canvas &canvas, const PixelRect &rc,
                                 bool enabled,
                                 bool focused, bool pressed) const
{
  frame_renderer.DrawButton(canvas, rc, focused, pressed);

  if (!caption.empty() || prefix_icon != PrefixIcon::NONE)
    DrawSymbol(canvas, frame_renderer.GetDrawingRect(rc, pressed),
               enabled, focused, pressed);
}
