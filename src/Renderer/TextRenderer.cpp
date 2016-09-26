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

#include "TextRenderer.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "Screen/AnyCanvas.hpp"
#include "Renderer/TextInBox.hpp"
#include "Asset.hpp"

#include <winuser.h>

PixelSize
TextRenderer::GetSize(Canvas &canvas, PixelRect rc,
                        const TCHAR *text) const
{
  canvas.DrawFormattedText(&rc, text,
                           DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT);
  return rc.GetSize();
}

unsigned
TextRenderer::GetHeight(Canvas &canvas, PixelRect rc,
                        const TCHAR *text) const
{
  return GetSize(canvas, rc, text).cy;
  return rc.bottom - rc.top;
}

unsigned
TextRenderer::GetHeight(Canvas &canvas, unsigned width,
                        const TCHAR *text) const
{
  return GetHeight(canvas, PixelRect(0, 0, width, 0), text);
}

unsigned
TextRenderer::GetHeight(const Font &font, unsigned width,
                        const TCHAR *text) const
{
  AnyCanvas canvas;
  canvas.Select(font);
  return GetHeight(canvas, width, text);
}

void
TextRenderer::Draw(Canvas &canvas, const PixelRect &_rc,
                   const TCHAR *text, bool outlined_text) const
{
  PixelRect rc = _rc;

  unsigned format = DT_WORDBREAK | (center ? DT_CENTER : DT_LEFT);

#ifdef USE_GDI
  format |= DT_NOPREFIX | DT_NOCLIP;
  bool supports_transparency = false;

  if (vcenter) {
    canvas.DrawFormattedText(&rc, text, format | DT_CALCRECT);
    rc.right = _rc.right;

    int offset = _rc.bottom - rc.bottom;
    if (offset > 0)
      rc.top += offset / 2;
  }
#else
  bool supports_transparency = true;
  if (vcenter)
    format |= DT_VCENTER;
#endif

  if (outlined_text && supports_transparency) {
    canvas.SetTextColor(COLOR_WHITE);
    const int offset = canvas.GetFontHeight() / 12u;

    PixelRect rc_shadow = rc;
    rc_shadow.Offset(offset, 0);
    canvas.DrawFormattedText(&rc_shadow, text, format);

    rc_shadow = rc;
    rc_shadow.Offset(-1 * offset, 0);
    canvas.DrawFormattedText(&rc_shadow, text, format);

    rc_shadow = rc;
    rc_shadow.Offset(0, offset);
    canvas.DrawFormattedText(&rc_shadow, text, format);

    rc_shadow = rc;
    rc_shadow.Offset(0, -1 * offset);
    canvas.DrawFormattedText(&rc_shadow, text, format);


    canvas.SetTextColor(COLOR_BLACK);
    canvas.DrawFormattedText(&rc, text, format);

  } else {
    canvas.DrawFormattedText(&rc, text, format);
  }
}
