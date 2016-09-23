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

#include "Dialogs/Dialogs.h"
#include "Dialogs/WidgetDialog.hpp"
#include "Widget/CreateWindowWidget.hpp"
#include "Widget/ArrowPagerWidget.hpp"
#include "Widget/LargeTextWidget.hpp"
#include "Look/FontDescription.hpp"
#include "Look/DialogLook.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/Font.hpp"
#include "Screen/Key.h"
#include "Version.hpp"
#include "Inflate.hpp"
#include "Util/ConvertString.hpp"
#include "Resources.hpp"
#include "UIGlobals.hpp"
#include "Language/Language.hpp"

#if defined(KOBO)
#include <sys/utsname.h>
#endif

class LogoPageWindow final : public PaintWindow {
protected:
  /** from class PaintWindow */
  virtual void OnPaint(Canvas &canvas) override;
};

void
LogoPageWindow::OnPaint(Canvas &canvas)
{
  const PixelRect rc = GetClientRect();
  const unsigned width = rc.right - rc.left;
  const unsigned height = rc.bottom - rc.top;
  int x = rc.left + Layout::FastScale(10);
  int y = rc.top + Layout::FastScale(10);

  canvas.ClearWhite();

  Bitmap title(width > 360 ? IDB_TITLE_HD : IDB_TITLE);

  // Determine title image size
  PixelSize title_size = title.GetSize();

  const unsigned magnification =
      std::max(1u,
               std::min((width - 16u) / unsigned(title_size.cx),
                        (height - 16u) / unsigned(title_size.cy) / 3));
  title_size.cx *= magnification;
  title_size.cy *= magnification;

  // Draw 'XCSoar N.N' title
  canvas.Stretch(x, y, title_size.cx, title_size.cy, title);
  y += title_size.cy + Layout::FastScale(20);

  Font font;
  font.Load(FontDescription(Layout::FontScale(16)));
  canvas.Select(font);
  canvas.SetTextColor(COLOR_BLACK);
  canvas.SetBackgroundTransparent();

  canvas.DrawText(x, y, TopHat_ProductToken);
  y += Layout::FastScale(22);

  canvas.DrawText(x, y, XCSoar_ProductTokenShort);
#if defined(GIT_COMMIT_ID)
  y += Layout::FastScale(22);

  canvas.DrawText(x, y, _T("git: "));
  canvas.DrawText(x + Layout::FastScale(80), y, _T(GIT_COMMIT_ID));
#endif
#if defined(KOBO)
  canvas.SetTextColor(COLOR_BLACK);
  y += Layout::FastScale(22);
  canvas.DrawText(x, y, _T("uImage: "));
  struct utsname buf;
  uname(&buf);
  canvas.DrawText(x + Layout::FastScale(70), y, _T(buf.release));
#endif

  y += Layout::FastScale(37);

  canvas.DrawText(x, y, _T("more information at"));
  y += Layout::FastScale(22);

  canvas.SetTextColor(COLOR_XCSOAR);
  canvas.DrawText(x, y, _T("http://www.tophatsoaring.org"));
}

static Window *
CreateLogoPage(ContainerWindow &parent, const PixelRect &rc,
               WindowStyle style)
{
  LogoPageWindow *window = new LogoPageWindow();
  window->Create(parent, rc, style);
  return window;
}

extern "C"
{
  extern const uint8_t COPYING_gz[];
  extern const size_t COPYING_gz_size;

  extern const uint8_t AUTHORS_gz[];
  extern const size_t AUTHORS_gz_size;
}

void
dlgCreditsShowModal(SingleWindow &parent)
{
  const DialogLook &look = UIGlobals::GetDialogLook();

  char *authors = InflateToString(AUTHORS_gz, AUTHORS_gz_size);
  const UTF8ToWideConverter authors2(authors);

  char *license = InflateToString(COPYING_gz, COPYING_gz_size);
  const UTF8ToWideConverter license2(license);

  WidgetDialog dialog(look);

  ArrowPagerWidget pager(dialog, look.button);
  pager.Add(new CreateWindowWidget(CreateLogoPage));
  pager.Add(new LargeTextWidget(look, authors2));
  pager.Add(new LargeTextWidget(look, license2));

  dialog.CreateFull(UIGlobals::GetMainWindow(), _("Credits"), &pager);
  dialog.ShowModal();
  dialog.StealWidget();

  delete[] authors;
  delete[] license;
}
