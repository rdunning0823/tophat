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

#include "Dialogs/Dialogs.h"
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/XML.hpp"
#include "Form/Form.hpp"
#include "Form/Button.hpp"
#include "Form/Tabbed.hpp"
#include "Form/Draw.hpp"
#include "Screen/EditWindow.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/Font.hpp"
#include "Screen/Fonts.hpp"
#include "Screen/Key.h"
#include "ResourceLoader.hpp"
#include "Version.hpp"
#include "Inflate.hpp"
#include "Util/ConvertString.hpp"
#include "resource.h"

#include <assert.h>

static WndForm *wf = NULL;
static TabbedControl *tab = NULL;

static void
OnNext(gcc_unused WndButton &button)
{
  tab->NextPage();
}

static void
OnPrev(gcc_unused WndButton &button)
{
  tab->PreviousPage();
}

gcc_pure
static EditWindow *
FindEditWindow()
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

  return (EditWindow *)wf->FindByName(name);
}

static bool
FormKeyDown(gcc_unused WndForm &Sender, unsigned key_code)
{
  switch (key_code) {
    EditWindow *edit;

  case VK_UP:
    edit = FindEditWindow();
    if (edit != NULL) {
      edit->ScrollVertically(-3);
      return true;
    } else
      return false;

  case VK_DOWN:
    edit = FindEditWindow();
    if (edit != NULL) {
      edit->ScrollVertically(3);
      return true;
    } else
      return false;

  case VK_LEFT:
#ifdef GNAV
  case '6':
#endif
    tab->PreviousPage();
    return true;

  case VK_RIGHT:
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
OnClose(gcc_unused WndButton &button)
{
  wf->SetModalResult(mrOK);
}

static void
OnLogoPaint(gcc_unused WndOwnerDrawFrame *Sender, Canvas &canvas)
{
  const UPixelScalar width = canvas.get_width();
  PixelScalar x = Layout::FastScale(10), y = x;

  canvas.ClearWhite();

  Bitmap title(width > 360 ? IDB_TITLE_HD : IDB_TITLE);

  // Determine title image size
  PixelSize title_size = title.GetSize();

  // Draw 'XCSoar N.N' title
  canvas.copy(x, y, title_size.cx, title_size.cy, title, 0, 0);
  y += title_size.cy + Layout::FastScale(20);

  Font font;
  font.Set(Fonts::GetStandardFontFace(), Layout::FastScale(16));
  canvas.Select(font);
  canvas.SetTextColor(COLOR_BLACK);
  canvas.SetBackgroundTransparent();

  canvas.text(x, y, TopHat_ProductToken);
  y += Layout::FastScale(22);

  canvas.text(x, y, XCSoar_ProductTokenShort);
#ifdef GIT_COMMIT_ID
  y += Layout::FastScale(22);

  canvas.text(x, y, _T("git: "));
  canvas.text(x + Layout::FastScale(80), y, _T(GIT_COMMIT_ID));
#endif
  y += Layout::FastScale(37);

  canvas.text(x, y, _T("more information at"));
  y += Layout::FastScale(22);

  canvas.SetTextColor(COLOR_XCSOAR);
  canvas.text(x, y, _T("http://tophatsoaring.org"));
}

static void
LoadTextFromResource(const TCHAR* name, const TCHAR* control)
{
  ResourceLoader::Data data = ResourceLoader::Load(name, _T("TEXT"));
  assert(data.first != NULL);

  char *buffer = InflateToString(data.first, data.second);

  UTF8ToWideConverter text(buffer);
  if (text.IsValid())
    ((EditWindow *)wf->FindByName(control))->SetText(text);

  delete[] buffer;
}

static constexpr CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(OnClose),
  DeclareCallBackEntry(OnNext),
  DeclareCallBackEntry(OnPrev),
  DeclareCallBackEntry(OnLogoPaint),
  DeclareCallBackEntry(NULL)
};

void
dlgCreditsShowModal(SingleWindow &parent)
{
  wf = LoadDialog(CallBackTable, parent, Layout::landscape ?
                  _T("IDR_XML_CREDITS_L") : _T("IDR_XML_CREDITS"));
  assert(wf != NULL);

  tab = ((TabbedControl *)wf->FindByName(_T("tab")));
  assert(tab != NULL);

  wf->SetKeyDownNotify(FormKeyDown);

  LoadTextFromResource(_T("LICENSE"), _T("prpLicense"));
  LoadTextFromResource(_T("AUTHORS"), _T("prpAuthors"));

  wf->ShowModal();

  delete wf;
}
