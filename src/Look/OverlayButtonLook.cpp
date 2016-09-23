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

#include "OverlayButtonLook.hpp"
#include "FontDescription.hpp"
#include "Screen/Layout.hpp"


#ifdef HAVE_TEXT_CACHE
#include "Screen/Custom/Cache.hpp"
#endif

#include <algorithm>

void
OverlayButtonLook::Initialise(unsigned font_scale_overlay_button)
{
  ReinitialiseLayout(font_scale_overlay_button);
}

void
OverlayButtonLook::ReinitialiseLayout(unsigned font_scale_overlay_button)
{

  const FontDescription large_font_d((std::min(Layout::FontScale(24),
                                             (Layout::min_screen_pixels) / 12) *
      font_scale_overlay_button) / 100, true);

  const FontDescription small_font_d((std::min(Layout::FontScale(12),
                                             (Layout::min_screen_pixels) / 20) *
      font_scale_overlay_button) / 100);

  large_font.Load(large_font_d);
  small_font.Load(small_font_d);

  scaled_button_width = (large_font_d.GetHeight() * 5) / 2;

#ifdef HAVE_TEXT_CACHE
  TextCache::Flush();
#endif
}
