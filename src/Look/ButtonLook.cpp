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

#include "ButtonLook.hpp"
#include "FontDescription.hpp"
#include "Asset.hpp"


void
ButtonLook::StateLook::CreateBorder(Color light, Color dark)
{
  /* pen width 3 appears uneven on screens with low dpi */
  unsigned transparent_border_pen_width = (Layout::Scale(1) == 3) ? 2 : Layout::Scale(1);

  light_border_pen.Create(1, light);
  light_transparent_border_pen.Create(transparent_border_pen_width, light);
  light_border_brush.Create(light);
  dark_border_pen.Create(1, dark);
  dark_transparent_border_pen.Create(transparent_border_pen_width, dark);
  dark_border_brush.Create(dark);
}
void
ButtonLook::Initialise(const Font &_font)
{
  font = &_font;
  const FontDescription font_large_d(font->GetHeight() * 1.5);
  font_large.Load(font_large_d);
  background_transparent = false;

  standard.foreground_color = COLOR_BLACK;
  standard.foreground_brush.Create(standard.foreground_color);
  standard.background_color = IsDithered() ? COLOR_WHITE : COLOR_LIGHT_GRAY;
  if (IsDithered()) {
    standard.CreateBorder(COLOR_BLACK, COLOR_BLACK);
  } else if (!HasColors()) {
    standard.CreateBorder(LightColor(COLOR_DARK_GRAY), COLOR_BLACK);
  } else {
    standard.CreateBorder(LightColor(standard.background_color),
                          DarkColor(standard.background_color));
  }

  constexpr Color dimmed_dark = COLOR_DARK_GRAY;
  dimmed.foreground_color = dimmed_dark;
  dimmed.foreground_brush.Create(dimmed.foreground_color);
  dimmed.background_color = IsDithered() ? COLOR_WHITE : COLOR_LIGHT_GRAY;
  if (IsDithered()) {
    dimmed.CreateBorder(LightColor(COLOR_DARK_GRAY), dimmed_dark);
  } else if (!HasColors()) {
    dimmed.CreateBorder(LightColor(COLOR_DARK_GRAY), dimmed_dark);
  } else {
    dimmed.CreateBorder(LightColor(dimmed.background_color), dimmed_dark);
  }

  focused.foreground_color = COLOR_WHITE;
  focused.foreground_brush.Create(focused.foreground_color);
  focused.background_color = IsDithered() ? COLOR_BLACK : COLOR_XCSOAR_DARK;
  if (IsDithered()) {
    focused.CreateBorder(COLOR_WHITE, COLOR_WHITE);
  } else if (!HasColors()) {
    focused.CreateBorder(LightColor(COLOR_DARK_GRAY), COLOR_BLACK);
  } else {
    focused.CreateBorder(LightColor(focused.background_color),
                         DarkColor(focused.background_color));
  }

  disabled.color = COLOR_GRAY;
  disabled.brush.Create(disabled.color);
}
