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

#include "LogoView.hpp"
#include "Screen/Canvas.hpp"
#include "resource.h"
#include "Version.hpp"

LogoView::LogoView()
  :logo(IDB_SWIFT), big_logo(IDB_SWIFT_HD),
   title(IDB_TITLE), big_title(IDB_TITLE_HD)
{
#ifndef USE_GDI
  font.Set(_T("Droid Sans"), 18);
#endif
}

void
LogoView::draw(Canvas &canvas, const PixelRect &rc)
{
  const UPixelScalar width = rc.right - rc.left, height = rc.bottom - rc.top;

  enum {
    LANDSCAPE, PORTRAIT, SQUARE,
  } orientation;

  if (width == height)
    orientation = SQUARE;
  else if (width > height)
    orientation = LANDSCAPE;
  else
    orientation = PORTRAIT;

  // Load logo
  const Bitmap &bitmap_logo =
      (orientation == LANDSCAPE && width >= 510 && height >= 170) ||
      (orientation == PORTRAIT && width >= 330 && height >= 250) ||
      (orientation == SQUARE && width >= 210 && height >= 210) ?
      big_logo : logo;

  // Adjust the title to larger screens
  const Bitmap &bitmap_title =
      (orientation == LANDSCAPE && width >= 510 && height >= 170) ||
      (orientation == PORTRAIT && width >= 330 && height >= 250) ||
      (orientation == SQUARE && width >= 210 && height >= 210) ?
      big_title : title;

  // Determine logo size
  PixelSize logo_size = bitmap_logo.GetSize();

  // Determine title image size
  PixelSize title_size = bitmap_title.GetSize();

  PixelScalar logox, logoy, titlex, titley;

  // Determine logo and title positions
  switch (orientation) {
  case LANDSCAPE:
    logox = (width - (logo_size.cx + title_size.cy + title_size.cx)) / 2;
    logoy = (height - logo_size.cy) / 2;
    titlex = logox + logo_size.cx + title_size.cy;
    titley = (height - title_size.cy) / 2;
    break;
  case PORTRAIT:
    logox = (width - logo_size.cx) / 2;
    logoy = (height - (logo_size.cy + title_size.cy * 2)) / 2;
    titlex = (width - title_size.cx) / 2;
    titley = logoy + logo_size.cy + title_size.cy;
    break;
  case SQUARE:
    logox = (width - logo_size.cx) / 2;
    logoy = (height - logo_size.cy) / 2;
    break;
  }

  // Draw 'XCSoar N.N' title
  if (orientation != SQUARE)
    canvas.copy(titlex, titley, title_size.cx, title_size.cy, bitmap_title, 0, 0);

  // Draw XCSoar swift logo
  canvas.copy(logox, logoy, logo_size.cx, logo_size.cy, bitmap_logo, 0, 0);

  // Draw full XCSoar version number

#ifndef USE_GDI
  canvas.Select(font);
#endif

  const int text_height = canvas.CalcTextSize(TopHat_ProductToken).cy;
  canvas.SetTextColor(COLOR_BLACK);
  canvas.SetBackgroundTransparent();
  canvas.text(2, 2, TopHat_ProductToken);
  canvas.text(2, 4 + text_height, XCSoar_ProductToken);
}
