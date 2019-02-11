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

#include "LogoView.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "Look/FontDescription.hpp"
#include "Resources.hpp"
#include "Version.hpp"
#include "Language/Language.hpp"
#include "Math/fixed.hpp"
#ifdef KOBO
#include "Kobo/System.hpp"
#endif
#include <algorithm>

LogoView::LogoView()
  :logo(IDB_LOGO), big_logo(IDB_LOGO_HD),
   title(IDB_TITLE), big_title(IDB_TITLE_HD)
{
#ifndef USE_GDI
  font.Load(FontDescription(Layout::FontScale(12)));
#endif

  big_logo.EnableInterpolation();
  big_title.EnableInterpolation();
}

static int
Center(unsigned canvas_size, unsigned element_size)
{
  /* cast to int to force signed integer division, just in case the
     difference is negative */
  return int(canvas_size - element_size) / 2;
}

void
LogoView::draw(Canvas &canvas, const PixelRect &rc_outer)
{
#ifndef USE_GDI
  canvas.Select(font);
#endif

  const int text_height = canvas.CalcTextSize(TopHat_ProductToken).cy;
  canvas.SetTextColor(COLOR_BLACK);
  canvas.SetBackgroundTransparent();
  unsigned y = 2;
  canvas.DrawText(2, y, TopHat_ProductToken);
  y += 2 + text_height;
  canvas.DrawText(2, y, XCSoar_ProductTokenShort);
#ifndef NDEBUG
  y += 2 + text_height;
  canvas.DrawText(2, y, XCSoar_GitSuffix);
#endif
#ifdef KOBO
  WriteSystemInfo();
  if (IsKoboUsbHostKernel()) {
    y += 2 + text_height;
    canvas.DrawText(2, y, _T("USB host supported"));
  }
#endif
#ifdef NO_HORIZON
  y += 2 + text_height;
  canvas.DrawText(2, y, _("Horizon: disabled"));
#endif

#ifndef NDEBUG
  y += 2 + text_height;
  canvas.DrawText(2, y, _T("DEBUG"));
#endif

  PixelRect rc = rc_outer;
  rc.Grow(-1 * Layout::Scale(15), -1 * Layout::Scale(10));
  const unsigned logo_top = y + 2 + text_height;
  const unsigned width = rc.right - rc.left, height = rc.bottom - rc.top - logo_top;

  enum {
    LANDSCAPE, PORTRAIT, SQUARE,
  } orientation;

  if (width == height)
    orientation = SQUARE;
  else if (width > height)
    orientation = LANDSCAPE;
  else
    orientation = PORTRAIT;

  /* load bitmaps */
  const bool use_big =
    (orientation == LANDSCAPE && width >= 510 && height >= 170) ||
    (orientation == PORTRAIT && width >= 330 && height >= 250) ||
    (orientation == SQUARE && width >= 210 && height >= 210);
  const Bitmap &bitmap_logo = use_big ? big_logo : logo;
  const Bitmap &bitmap_title = use_big ? big_title : title;

  // Determine logo size
  PixelSize logo_size = bitmap_logo.GetSize();

  // Determine title image size
  PixelSize title_size = bitmap_title.GetSize();

  unsigned spacing = title_size.cy / 2;

  unsigned estimated_width, estimated_height;
  switch (orientation) {
  case LANDSCAPE:
    estimated_width = logo_size.cx + spacing + title_size.cx;
    estimated_height = logo_size.cy;
    break;

  case PORTRAIT:
    estimated_width = title_size.cx;
    estimated_height = logo_size.cy + spacing + title_size.cy;
    break;

  case SQUARE:
    estimated_width = logo_size.cx;
    estimated_height = logo_size.cy;
    break;
  }

  const fixed magnification =
    std::min(fixed(width) / fixed(estimated_width),
             fixed(height) / fixed(estimated_height));

  if (magnification > fixed(1)) {
    logo_size.cx = int(fixed(logo_size.cx) * magnification);
    logo_size.cy = int(fixed(logo_size.cy) * magnification);
    title_size.cx = int(fixed(title_size.cx) * magnification);
    title_size.cy = int(fixed(title_size.cy) * magnification);
    spacing = int(fixed(spacing) * magnification);
  }

  int logox, logoy, titlex, titley;

  // Determine logo and title positions
  switch (orientation) {
  case LANDSCAPE:
    logox = Center(width, logo_size.cx + spacing + title_size.cx) + rc.left;
    logoy = Center(height, logo_size.cy) + logo_top;
    titlex = logox + logo_size.cx + spacing;
    titley = Center(height, title_size.cy) + logo_top;
    break;
  case PORTRAIT:
  case SQUARE:
    logox = (width - logo_size.cx) / 2 + rc.left;
    logoy = (height - (logo_size.cy + title_size.cy * 2)) / 2  + logo_top;
    titlex = (width - title_size.cx) / 2 + rc.left;
    titley = logoy + logo_size.cy + title_size.cy;
    break;
  }

  // Draw 'XCSoar N.N' title
  if (orientation != SQUARE)
    canvas.Stretch(titlex, titley, title_size.cx, title_size.cy, bitmap_title);

  // Draw XCSoar swift logo
  canvas.Stretch(logox, logoy, logo_size.cx, logo_size.cy, bitmap_logo);

  // Draw full XCSoar version number
}
