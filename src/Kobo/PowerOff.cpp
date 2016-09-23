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

#include "Dialogs/DialogSettings.hpp"
#include "../test/src/Fonts.hpp"
#include "Hardware/RotateDisplay.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Custom/TopCanvas.hpp"
#include "Screen/FreeType/Init.hpp"
#include "Screen/Debug.hpp"
#include "Screen/Layout.hpp"
#include "Renderer/FlightListRenderer.hpp"
#include "FlightInfo.hpp"
#include "Logger/FlightParser.hpp"
#include "IO/FileLineReader.hpp"
#include "Resources.hpp"
#include "Version.hpp"

#include <algorithm>
#include <stdio.h>

/*
 * Draws logo and footer
 * @return remaining rectangle unused
 */
static PixelRect
DrawBanner(Canvas &canvas, PixelRect &rc)
{
  const unsigned padding = 10;
  const unsigned left_margin = 28;
  const unsigned top_logo_margin = 46;

  const Bitmap logo(IDB_LOGO);
  const unsigned banner_height = logo.GetHeight();
  PixelRect rc_remaining = rc;

  /* draw the XCSoar logo */
  int x = rc.left + left_margin;
  canvas.Copy(x, rc.top + top_logo_margin,
              logo.GetWidth(), logo.GetHeight(),
              logo, 0, 0);

  x += logo.GetWidth() + 8;

  canvas.SetTextColor(COLOR_BLACK);
  canvas.SetBackgroundTransparent();

  /* draw the XCSoar banner text with a larger font */
  Font logo_font;
  logo_font.LoadFile("/opt/tophat/share/fonts/VeraBd.ttf",
                     (canvas.GetSize().cx * 40)/ 600);
  canvas.Select(logo_font);
  const unsigned name_y = rc.top + top_logo_margin
    + (banner_height - logo_font.GetHeight()) / 2;

  const TCHAR *const name1 = _T("Top Hat Soaring");
  canvas.DrawText(x, name_y, name1);
  x += canvas.CalcTextWidth(name1);

  /* some more text */
  x = rc.left + left_margin;
  const TCHAR *const website = _T("www.tophatsoaring.org");
  canvas.Select(bold_font);
  unsigned website_top = rc.top + banner_height + padding + top_logo_margin;
  canvas.DrawText(x, website_top, website);
  rc_remaining.top = website_top + bold_font.GetHeight();

  /* Version at bottom right */
  const unsigned line_height = canvas.CalcTextSize(TopHat_Version).cy;
  rc_remaining.left = rc.left + left_margin;
  rc_remaining.bottom = rc.bottom - 2.5 * line_height;

  int y = rc_remaining.bottom + padding;
  x = rc.right - canvas.CalcTextSize(TopHat_Version).cx - padding;
  canvas.DrawText(x, y, TopHat_Version);
  const TCHAR *const version = _T("version ");
  x -= canvas.CalcTextSize(version).cx;
  canvas.DrawText(x, y, version);

  /* power off message */
  const TCHAR *const comment = _T("powered off");
  y += padding + line_height;
  canvas.DrawText(rc.right - canvas.CalcTextWidth(comment) - padding,
                  y, comment);

  rc.top += banner_height + 8;

  return rc_remaining;
}

static void
DrawFlights(Canvas &canvas, const PixelRect &rc)
{
  FileLineReaderA file("/mnt/onboard/XCSoarData/flights.log");
  if (file.error())
    return;

  FlightListRenderer renderer(normal_font, bold_font);

  FlightParser parser(file);
  FlightInfo flight;
  while (parser.Read(flight))
    renderer.AddFlight(flight);

  renderer.Draw(canvas, rc);
}

static void
Draw(Canvas &canvas)
{
  PixelRect rc = canvas.GetRect();
  rc.Grow(-16);

  PixelRect rc_remaining = DrawBanner(canvas, rc);
  DrawFlights(canvas, rc_remaining);
}

int main(int argc, char **argv)
{
  /* enable FreeType anti-aliasing, because we don't use dithering in
     this program */
  ScreenInitialized();
  FreeType::mono = false;

  FreeType::Initialise();

  Font::Initialise();

  {
    TopCanvas screen;
    screen.Create(PixelSize{100, 100}, true, false);

    Canvas canvas = screen.Lock();
    if (canvas.IsDefined()) {
      /* all black first, to eliminate E-ink ghost images */
      canvas.Clear(COLOR_BLACK);
      screen.Flip();
      screen.Wait();

      /* disable dithering, render with 16 shades of gray, to make the
         (static) display more pretty */
      screen.SetEnableDither(false);

      /* draw the pictuer */
      canvas.ClearWhite();
      InitialiseFonts();
      Draw(canvas);

      /* finish */
      screen.Unlock();
      screen.Flip();
      screen.Wait();
    }
  }

  /* now we can power off the Kobo; the picture remains on the
     screen */
  execl("/sbin/poweroff", "poweroff", nullptr);
  return 0;
}
