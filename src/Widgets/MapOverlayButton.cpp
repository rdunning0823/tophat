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

#include "MapOverlayButton.hpp"
#include "Look/GlobalFonts.hpp"
#include "Screen/Font.hpp"

#include "Form/SymbolButton.hpp"
#include "UIGlobals.hpp"
#include "Look/IconLook.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/Layout.hpp"
#include "Language/Language.hpp"
#include "Interface.hpp"
#include "MainWindow.hpp"
#include "Look/Look.hpp"
#include "Look/GlobalFonts.hpp"
#include "Screen/Canvas.hpp"
#include "Interface.hpp"
#include "Pan.hpp"
#include "Renderer/TextInBox.hpp"

unsigned
MapOverlayButton::GetScale()
{
  return 3;
}

unsigned
MapOverlayButton::GetClearBorderWidth()
{
  return 2 * Layout::Scale(2);
}

unsigned
MapOverlayButton::GetStandardButtonHeight()
{
  return Fonts::map_bold.GetHeight();
}

bool
MapOverlayButton::OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys)
{
  if (IsInside(x, y))
    return WndButton::OnMouseMove(x, y, keys);
  else
    OnCancelMode();
  return true;
}

void
MapOverlayButton::OnPaint(Canvas &canvas)
{
  PixelRect rc = {
    PixelScalar(0), PixelScalar(0), PixelScalar(canvas.GetWidth()),
    PixelScalar(canvas.GetHeight())
  };

  bool dimmed = CommonInterface::Calculated().flight.flying && !IsPanning();

  bool pressed = IsDown();
#ifdef ENABLE_OPENGL
  bool transparent = true;
#else
  bool transparent = false;
  if (IsKobo())
    transparent = true;
#endif
  //Todo fix the GDI rendering so it draws transparent correctly
  renderer.DrawButton(canvas, rc, HasFocus(), pressed, transparent, dimmed);
  rc = renderer.GetDrawingRect(rc, pressed);

  if (bmp != nullptr) {
    canvas.SelectNullPen();
    if (!IsEnabled())
      canvas.Select(button_look.disabled.brush);
    else
      canvas.Select(button_look.standard.foreground_brush);
    const PixelSize bitmap_size = bmp->GetSize();
    const int offsetx = (rc.right - rc.left - bitmap_size.cx / 2) / 2;
    const int offsety = (rc.bottom - rc.top - bitmap_size.cy) / 2;
    canvas.CopyAnd(rc.left + offsetx,
                    rc.top + offsety,
                    bitmap_size.cx / 2,
                    bitmap_size.cy,
                    *bmp,
                    bitmap_size.cx / 2, 0);
  } else {
    canvas.SetBackgroundTransparent();
    if (HasCursorKeys() ? (HasFocus() | pressed) : pressed)
      canvas.SetTextColor(button_look.focused.foreground_color);
    else
      canvas.SetTextColor(dimmed ? button_look.dimmed.foreground_color :
          button_look.standard.foreground_color);

    PixelSize sz;
    sz.cx = rc.right - rc.left;
    sz.cy = rc.bottom - rc.top;
    PixelSize sz_main = GetLargeFont().TextSize(GetText().c_str());
    PixelSize sz_subscript = GetMediumFont().TextSize(subscript_text.c_str());
    PixelSize sz_line_two = GetMediumFont().TextSize(line_two_text.c_str());
    sz_main.cx *= 0.85;
    sz_subscript.cx *= 0.85;

    PixelRect rc_main_text = rc;
    rc_main_text.left = rc.left + (sz.cx - sz_main.cx) / 2;
    rc_main_text.right = rc_main_text.left + sz_main.cx;
    rc_main_text.top = rc.top + (sz.cy - sz_main.cy) / 2;
    rc_main_text.bottom = rc_main_text.top + sz_main.cy;

    rc_main_text.Offset(-sz_subscript.cx / 2, 0);
    if (sz_line_two.cx > 0)
      rc_main_text.Offset(0, -sz_line_two.cy / 2);

    canvas.Select(GetLargeFont());

  #ifndef USE_GDI
    TextInBoxMode mode;
    mode.shape = LabelShape::OUTLINED;
    mode.align = TextInBoxMode::Alignment::LEFT;
    TextInBox(canvas, GetText().c_str(), rc_main_text.left, rc_main_text.top, mode,
              rc.GetSize().cx, rc.GetSize().cy);

    canvas.SetBackgroundOpaque();
    const ButtonLook::StateLook &_look =
        (HasFocus() || pressed) ? button_look.focused :
        (dimmed ? button_look.dimmed : button_look.standard);
    canvas.SetBackgroundColor(pressed ? _look.background_color : COLOR_WHITE);
    if (sz_subscript.cx > 0) {
      PixelRect rc_subscript = rc_main_text;
      rc_subscript.left = rc_main_text.right + Layout::Scale(1);
      rc_subscript.top = rc_main_text.bottom - sz_subscript.cy - sz_main.cy / 10;
      canvas.Select(GetMediumFont());
      canvas.DrawOpaqueText(rc_subscript.left, rc_subscript.top, rc_subscript,
                            subscript_text.c_str());
    }

    if (sz_line_two.cx > 0) {
      PixelRect rc_line_two = rc;
      rc_line_two.left = rc.left + (sz.cx - sz_line_two.cx) / 2;
      rc_line_two.top = rc_line_two.bottom - sz_line_two.cy;
      rc_line_two.right = rc_line_two.left + sz_line_two.cx;
      rc_line_two.Offset(sz_line_two.cx * 0.15, 0);
      canvas.Select(GetMediumFont());
      canvas.DrawOpaqueText(rc_line_two.left, rc_line_two.top, rc_line_two,
                            line_two_text.c_str());
    }

  #else
    unsigned style = DT_CENTER | DT_NOCLIP | DT_WORDBREAK;

    PixelRect text_rc = rc;
    canvas.DrawFormattedText(&text_rc,  GetText().c_str(), style | DT_CALCRECT);
    text_rc.right = rc.right;

    PixelScalar offset = rc.bottom - text_rc.bottom;
    if (offset > 0) {
      offset /= 2;
      text_rc.top += offset;
      text_rc.bottom += offset;
    }

    canvas.DrawFormattedText(&text_rc,  GetText().c_str(), style);
  #endif
  }
}
