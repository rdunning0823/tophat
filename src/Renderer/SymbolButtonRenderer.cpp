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

#include "SymbolButtonRenderer.hpp"
#include "SymbolRenderer.hpp"
#include "Screen/Canvas.hpp"
#include "Look/ButtonLook.hpp"
#include "Look/IconLook.hpp"
#include "Look/MapLook.hpp"
#include "Screen/Bitmap.hpp"
#include "Renderer/SymbolRenderer.hpp"
#include "Language/Language.hpp"
#include "Resources.hpp"
#include "Asset.hpp"
#include "Screen/Layout.hpp"
#include "UIGlobals.hpp"
#include "Formatter/HexColor.hpp"
#include "Util/StringUtil.hpp"
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
  case PrefixIcon::SPEEDOMETER:
    icon_width += icon_look.hBmpSpeedometer.GetSize().cx + Layout::GetTextPadding();
    break;
  }
  return icon_width + text_width;
}

const MaskedIcon*
SymbolButtonRenderer::GetIcon(PrefixIcon prefix_icon) const
{
  const IconLook &icon_look = UIGlobals::GetIconLook();

  switch (prefix_icon) {
  case PrefixIcon::NONE:
    return nullptr;
    break;
  case PrefixIcon::HOME:
    return &icon_look.icon_home;
    break;
  case PrefixIcon::CHECK_MARK:
    return &icon_look.hBmpCheckMark;
    break;
  case PrefixIcon::SEARCH:
    return &icon_look.hBmpSearch;
    break;
  case PrefixIcon::SEARCH_CHECKED:
    return &icon_look.hBmpSearchChecked;
    break;
  case PrefixIcon::SPEEDOMETER:
    return &icon_look.hBmpSpeedometer;
    break;
  }
  return nullptr;
}

void
SymbolButtonRenderer::DrawIconAndText(Canvas &canvas, PixelRect rc,
                                      const TCHAR *text,
                                      const MaskedIcon *icon,
                                      bool enabled, bool
                                      focused, bool pressed,
                                      bool transparent_background_force) const
{
  UPixelScalar padding = Layout::GetTextPadding();
#ifdef USE_GDI
  bool supports_transparency = false;
#else
  bool supports_transparency = true;
#endif

  PixelSize sz_icon;
  sz_icon.cx = sz_icon.cy = 0;
  if (icon != nullptr)
    sz_icon = icon->GetSize();

  PixelRect rc_icon = rc;
  PixelRect rc_caption = rc;
  if (rc_caption.GetSize().cx > sz_icon.cx + 2 * (int)padding)
    rc_caption.right -= (sz_icon.cx + 2 * padding);

  PixelSize sz_text_one_line = GetCaptionSize(canvas, rc_caption, text);

  if (sz_text_one_line.cx + sz_icon.cx + (int)padding <= rc.GetSize().cx ) {
    rc_icon.left = (rc.GetSize().cx - sz_icon.cx - sz_text_one_line.cx - padding) / 2;
    rc_icon.right = rc_icon.left + sz_icon.cx;

    rc_caption.left = rc_icon.right +
        (icon == nullptr ? 0 : padding * 2);
    rc_caption.right = std::min((int)rc_caption.left + (int)sz_text_one_line.cx, (int)rc.right) + (int)padding;

  } else {
    // text and icon not fit on one line
    rc_icon.left = rc.left;
    rc_icon.right = rc_icon.left + sz_icon.cx;

    rc_caption.left = rc_icon.right +
        (icon == nullptr ? 0 : padding * 2);
    rc_caption.right = rc.right;
  }

  if (icon != nullptr) {
    if (!focused && transparent_background_force && supports_transparency) {
      // draw with outline

      const int offset = icon->GetSize().cx / 12u;

      PixelRect rc_shadow = rc_icon;
      rc_shadow.Offset(offset, 0);
      icon->Draw(canvas, rc_shadow, true);

      rc_shadow = rc_icon;
      rc_shadow.Offset(-1 * offset, 0);
      icon->Draw(canvas, rc_shadow, true);

      rc_shadow = rc_icon;
      rc_shadow.Offset(0, offset);
      icon->Draw(canvas, rc_shadow, true);

      rc_shadow = rc_icon;
      rc_shadow.Offset(0, -1 * offset);
      icon->Draw(canvas, rc_shadow, true);

      icon->Draw(canvas, rc_icon, false);
    } else {
      // draw once
      icon->Draw(canvas, rc_icon, focused);
    }
  }
  DrawCaption(canvas, text, rc_caption, enabled, focused, pressed, transparent_background_force);
}

inline void
SymbolButtonRenderer::DrawSymbol(Canvas &canvas, PixelRect rc, bool enabled,
                                 bool focused, bool pressed,
                                 bool transparent_background_force) const
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
    const MaskedIcon *icon = &icon_look.hBmpTabSettings;
    DrawIconAndText(canvas, rc, _T(""), icon,
                    enabled, focused, pressed, transparent_background_force);
  } else if (caption == _("_MapLayers")) {
    const IconLook &icon_look = UIGlobals::GetIconLook();
    const MaskedIcon *icon = &icon_look.hBmpLayers;
    DrawIconAndText(canvas, rc, _T("Layers"), icon,
                    enabled, focused, pressed, transparent_background_force);
  } else if (caption == _("_SetupNavBar")) {
    const IconLook &icon_look = UIGlobals::GetIconLook();
    const MaskedIcon *icon = &icon_look.hBmpTabSettingsNavBar;
    DrawIconAndText(canvas, rc, _T(""), icon,
                    enabled, focused, pressed, transparent_background_force);

  } else if (caption == _("_NavBarToTarget")) {
    const IconLook &icon_look = UIGlobals::GetIconLook();
    const MaskedIcon *icon = &icon_look.target_icon;
    DrawIconAndText(canvas, rc, _("Nav to target"), icon,
                    enabled, focused, pressed, transparent_background_force);

  } else if (StringStartsWith(caption.c_str(), _T("_CheckMark"))) {
    StaticString <96>caption2(caption);
    ReplaceInString(caption2.buffer(), _T("_CheckMark"), _T(""),
                    caption2.length());
    const IconLook &icon_look = UIGlobals::GetIconLook();
    const MaskedIcon *icon = &icon_look.hBmpCheckMark;
    DrawIconAndText(canvas, rc, caption2, icon,
                    enabled, focused, pressed, transparent_background_force);

  } else if (caption == _("_NavBarToCenter")) {
    const IconLook &icon_look = UIGlobals::GetIconLook();
    const MaskedIcon *icon = &icon_look.task_turn_point_icon;
    DrawIconAndText(canvas, rc, _("Nav to center"), icon,
                    enabled, focused, pressed, transparent_background_force);

  } else if (caption == _("_X")) {
    const IconLook &icon_look = UIGlobals::GetIconLook();
    if (!icon_look.valid) {
      DrawCaption(canvas, _("OK"), rc, enabled, focused, pressed, transparent_background_force);
    } else {
      const MaskedIcon *icon = &icon_look.hBmpClose;
      DrawIconAndText(canvas, rc, _T(""), icon,
                      enabled, focused, pressed, transparent_background_force);
    }
  } else if (caption == _T("_Backspace")) {
    const IconLook &icon_look = UIGlobals::GetIconLook();
    const MaskedIcon *icon = &icon_look.icon_backspace;
    DrawIconAndText(canvas, rc, _(""), icon,
                    enabled, focused, pressed, transparent_background_force);
  } else if (caption == _("_TaskStats")) {
    const IconLook &icon_look = UIGlobals::GetIconLook();
    const MaskedIcon *icon = &icon_look.hBmpSpeedometer;
    DrawIconAndText(canvas, rc, _("Stats"), icon,
                    enabled, focused, pressed, transparent_background_force);

  } else if (caption == _("_EditTask")) {
    const IconLook &icon_look = UIGlobals::GetIconLook();
    const MaskedIcon &icon = icon_look.hBmpTabTask;
    DrawIconAndText(canvas, rc, _("Edit task"), &icon,
                    enabled, focused, pressed, transparent_background_force);

  } else if (prefix_icon != PrefixIcon::NONE) {
    const MaskedIcon *icon = GetIcon(prefix_icon);
    DrawIconAndText(canvas, rc, caption.c_str(), icon,
                    enabled, focused, pressed, transparent_background_force);

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

    DrawCaption(canvas, GetCaption(), rc, enabled, focused, pressed, transparent_background_force);
}

void
SymbolButtonRenderer::DrawButton(Canvas &canvas, const PixelRect &rc,
                                 bool enabled,
                                 bool focused, bool pressed,
                                 bool transparent_background_force) const
{
  frame_renderer.DrawButton(canvas, rc, focused, pressed, transparent_background_force);

  if (!caption.empty() || prefix_icon != PrefixIcon::NONE)
    DrawSymbol(canvas, frame_renderer.GetDrawingRect(rc, pressed),
               enabled, focused, pressed, transparent_background_force);
}
