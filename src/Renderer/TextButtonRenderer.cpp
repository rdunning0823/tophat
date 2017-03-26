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

#include "TextButtonRenderer.hpp"
#include "Screen/Color.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Pen.hpp"
#include "Screen/Layout.hpp"
#include "Look/ButtonLook.hpp"
#include "Asset.hpp"
#ifdef _WIN32_WCE
#include "Util/StringAPI.hxx"
#endif

#include <winuser.h>

PixelSize
TextButtonRenderer::GetCaptionSize(Canvas &canvas, PixelRect rc,
                                   const TCHAR *text) const
{
  canvas.Select(GetFont());
  PixelSize sz = text_renderer.GetSize(canvas, rc, text);
#ifdef _WIN32_WCE
  if (StringLength(text) == 0)
    sz.cx = 0;
#endif
  return sz;
}

void
TextButtonRenderer::DrawCaption(Canvas &canvas, const PixelRect &rc,
                                bool enabled, bool focused, bool pressed,
                                bool outlined_text) const
{
  DrawCaption(canvas, GetCaption(), rc, enabled, focused, pressed,
              outlined_text);
}

void
TextButtonRenderer::DrawCaption(Canvas &canvas,
                                StaticString<96>::const_pointer _caption,
                                const PixelRect &rc, bool enabled,
                                bool focused, bool pressed,
                                bool outlined_text) const
{
  const ButtonLook &look = GetLook();

  canvas.SetBackgroundTransparent();
  if (!enabled)
    canvas.SetTextColor(look.disabled.color);
  else if (focused)
    canvas.SetTextColor(look.focused.foreground_color);
  else
    canvas.SetTextColor(look.standard.foreground_color);

  canvas.Select(GetFont());
  text_renderer.Draw(canvas, rc, _caption, outlined_text);
}

const Font&
TextButtonRenderer::GetFont() const
{
  return use_large_font ? GetLook().font_large : *GetLook().font;
}

unsigned
TextButtonRenderer::GetMinimumButtonWidth() const
{
  return 2 * (frame_renderer.GetMargin() + Layout::GetTextPadding())
    + GetFont().TextSize(caption.c_str()).cx;
}

void
TextButtonRenderer::DrawButton(Canvas &canvas, const PixelRect &rc,
                               bool enabled, bool focused, bool pressed,
                               bool force_transparent_background) const
{
  frame_renderer.DrawButton(canvas, rc, focused, pressed,
                            force_transparent_background);

  if (!caption.empty())
    DrawCaption(canvas, frame_renderer.GetDrawingRect(rc, pressed),
                enabled, focused, pressed, force_transparent_background);
}

