/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/XML.hpp"
#include "Form/Form.hpp"
#include "Form/Button.hpp"
#include "Form/Tabbed.hpp"
#include "Look/StandardFonts.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/LargeTextWindow.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/Font.hpp"
#include "Screen/Key.h"
#include "Version.hpp"
#include "Inflate.hpp"
#include "Util/ConvertString.hpp"
#include "Resources.hpp"
#include "Language/Language.hpp"

#include <assert.h>
#if defined(KOBO)
#include <sys/utsname.h>
#endif

static WndForm *wf = NULL;
static TabbedControl *tab = NULL;

static void
OnNext()
{
  tab->NextPage();
}

static void
OnPrev()
{
  tab->PreviousPage();
}

gcc_pure
static LargeTextWindow *
FindLargeTextWindow()
{
  const TCHAR *name;
  switch (tab->GetCurrentPage()) {
  case 1:
    name = _T("prpAuthors");
    break;

  case 2:
    name = _T("prpLicense");
    break;

  default:
    return NULL;
  }

  return (LargeTextWindow *)wf->FindByName(name);
}

static bool
FormKeyDown(unsigned key_code)
{
  switch (key_code) {
    LargeTextWindow *edit;

  case KEY_UP:
    edit = FindLargeTextWindow();
    if (edit != NULL) {
      edit->ScrollVertically(-3);
      return true;
    } else
      return false;

  case KEY_DOWN:
    edit = FindLargeTextWindow();
    if (edit != NULL) {
      edit->ScrollVertically(3);
      return true;
    } else
      return false;

  case KEY_LEFT:
#ifdef GNAV
  case '6':
#endif
    tab->PreviousPage();
    return true;

  case KEY_RIGHT:
#ifdef GNAV
  case '7':
#endif
    tab->NextPage();
    return true;

  default:
    return false;
  }
}

static void
OnLogoPaint(Canvas &canvas, const PixelRect &rc)
{
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
  font.Load(GetStandardFontFace(), Layout::FastScale(16));
  canvas.Select(font);
  canvas.SetTextColor(COLOR_BLACK);
  canvas.SetBackgroundTransparent();

  canvas.DrawText(x, y, TopHat_ProductToken);
  y += Layout::FastScale(22);

  canvas.DrawText(x, y, XCSoar_ProductTokenShort);
#ifdef GIT_COMMIT_ID
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
#ifdef NO_HORIZON
  y += Layout::FastScale(22);
  canvas.DrawText(x, y, _("Horizon: disabled"));
#endif

  y += Layout::FastScale(37);

  canvas.DrawText(x, y, _T("more information at"));
  y += Layout::FastScale(22);

  canvas.SetTextColor(COLOR_XCSOAR);
  canvas.DrawText(x, y, _T("http://www.tophatsoaring.org"));
}

static void
LoadTextFromResource(const void *data, size_t size, const TCHAR *control)
{
  char *buffer = InflateToString(data, size);

  UTF8ToWideConverter text(buffer);
  if (text.IsValid())
    ((LargeTextWindow *)wf->FindByName(control))->SetText(text);

  delete[] buffer;
}

static void
LoadTextFromResource(const uint8_t *start, const uint8_t *end,
                     const TCHAR *control)
{
  LoadTextFromResource(start, end - start, control);
}

static constexpr CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(OnNext),
  DeclareCallBackEntry(OnPrev),
  DeclareCallBackEntry(OnLogoPaint),
  DeclareCallBackEntry(NULL)
};

/* workaround note: we would prefer to use the "_size" symbol here,
   but it turns out that Android 4 relocates these symbols for some
   reason, therefore we use "end-start" instead */

extern const uint8_t license_start[] asm("_binary_COPYING_gz_start");
extern const uint8_t license_end[] asm("_binary_COPYING_gz_end");

extern const uint8_t authors_start[] asm("_binary_AUTHORS_gz_start");
extern const uint8_t authors_end[] asm("_binary_AUTHORS_gz_end");

void
dlgCreditsShowModal(SingleWindow &parent)
{
  wf = LoadDialog(CallBackTable, parent, Layout::landscape ?
                  _T("IDR_XML_CREDITS_L") : _T("IDR_XML_CREDITS"));
  assert(wf != NULL);

  tab = ((TabbedControl *)wf->FindByName(_T("tab")));
  assert(tab != NULL);

  wf->SetKeyDownFunction(FormKeyDown);

  LoadTextFromResource(license_start, license_end, _T("prpLicense"));
  LoadTextFromResource(authors_start, authors_end, _T("prpAuthors"));

  wf->ShowModal();

  delete wf;
}
