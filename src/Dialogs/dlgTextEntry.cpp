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

#include "Dialogs/TextEntry.hpp"
#include "Dialogs/XML.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/DialogSettings.hpp"
#include "Screen/Key.h"
#include "Form/Form.hpp"
#include "Form/Draw.hpp"
#include "Form/Button.hpp"
#include "Compatibility/string.h"
#include "MapSettings.hpp"
#include "Asset.hpp"
#include "Util/StringUtil.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Util/Macros.hpp"

#include <algorithm>

using std::min;

static WndForm *wf = NULL;
static WndOwnerDrawFrame *wGrid = NULL;

#define MAX_TEXTENTRY 40
static unsigned int cursor = 0;
static int lettercursor = 0;
static int max_width = MAX_TEXTENTRY;

static TCHAR edittext[MAX_TEXTENTRY];

static TCHAR EntryLetters[] = _T(" ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890.-");

#define MAXENTRYLETTERS (ARRAY_SIZE(EntryLetters) - 1)

static void
OnCloseClicked(gcc_unused WndButton &button)
{
  wf->SetModalResult(mrOK);
}

static void
OnTextPaint(gcc_unused WndOwnerDrawFrame *Sender, Canvas &canvas)
{
  const PixelRect rc = Sender->GetClientRect();

  canvas.Clear(Color(0x40, 0x40, 0x00));

  // Do the actual painting of the text
  const DialogLook &look = UIGlobals::GetDialogLook();
  canvas.Select(*look.text_font);

  PixelSize tsize = canvas.CalcTextSize(edittext);
  PixelSize tsizec = canvas.CalcTextSize(edittext, cursor);
  PixelSize tsizea = canvas.CalcTextSize(edittext, cursor + 1);

  RasterPoint p[5];
  p[0].x = 10;
  p[0].y = (rc.bottom - rc.top - tsize.cy - 5) / 2;

  p[2].x = p[0].x + tsizec.cx;
  p[2].y = p[0].y + tsize.cy + 5;

  p[3].x = p[0].x + tsizea.cx;
  p[3].y = p[0].y + tsize.cy + 5;

  p[1].x = p[2].x;
  p[1].y = p[2].y - 2;

  p[4].x = p[3].x;
  p[4].y = p[3].y - 2;

  canvas.SelectWhitePen();
  canvas.DrawPolyline(p + 1, 4);

  canvas.SetBackgroundTransparent();
  canvas.SetTextColor(COLOR_WHITE);
  canvas.text(p[0].x, p[0].y, edittext);
}

static void
UpdateCursor()
{
  if (lettercursor >= (int)MAXENTRYLETTERS)
    lettercursor = 0;

  if (lettercursor < 0)
    lettercursor = MAXENTRYLETTERS - 1;

  edittext[cursor] = EntryLetters[lettercursor];

  if (wGrid != NULL)
    wGrid->Invalidate();
}

static void
MoveCursor()
{
  if (cursor >= _tcslen(edittext))
    edittext[cursor + 1] = 0;

  for (lettercursor = 0; lettercursor < (int)MAXENTRYLETTERS; lettercursor++) {
    if (edittext[cursor] == EntryLetters[lettercursor])
      break;
  }

  if (lettercursor == MAXENTRYLETTERS) {
    lettercursor = 0;
    edittext[cursor] = EntryLetters[lettercursor];
  }

  if (edittext[cursor] == 0)
    lettercursor = 0;

  UpdateCursor();
}

static bool
FormKeyDown(gcc_unused WndForm &Sender, unsigned key_code)
{
  switch (key_code) {
  case VK_UP:
  case VK_LEFT:
    if ((key_code == VK_LEFT) ^ IsAltair()) {
      if (cursor < 1)
        return true; // min width

      cursor--;
      MoveCursor();
      return true;
    } else {
      lettercursor--;
      UpdateCursor();
      return true;
    }

  case VK_DOWN:
  case VK_RIGHT:
    if ((key_code == VK_RIGHT) ^ IsAltair()) {
      if ((int)cursor >= (max_width - 2))
        return true; // max width

      cursor++;
      MoveCursor();
      return true;
    } else {
      lettercursor++;
      UpdateCursor();
      return true;
    }
  case VK_RETURN:
    wf->SetModalResult(mrOK);
    return true;

  default:
    return false;
  }
}

static void
OnLeftClicked(gcc_unused WndButton &button)
{
  FormKeyDown(*wf, IsAltair()? VK_UP : VK_LEFT);
}

static void
OnRightClicked(gcc_unused WndButton &button)
{
  FormKeyDown(*wf, IsAltair()? VK_DOWN : VK_RIGHT);
}

static void
OnUpClicked(gcc_unused WndButton &button)
{
  FormKeyDown(*wf, !IsAltair()? VK_UP : VK_LEFT);
}

static void
OnDownClicked(gcc_unused WndButton &button)
{
  FormKeyDown(*wf, !IsAltair()? VK_DOWN : VK_RIGHT);
}

static constexpr CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(OnTextPaint),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnLeftClicked),
  DeclareCallBackEntry(OnRightClicked),
  DeclareCallBackEntry(OnUpClicked),
  DeclareCallBackEntry(OnDownClicked),
  DeclareCallBackEntry(NULL)
};

static void
dlgTextEntryHighscoreType(SingleWindow &parent, TCHAR *text, int width,
                          const TCHAR* caption)
{
  wf = NULL;
  wGrid = NULL;

  if (width == 0)
    width = MAX_TEXTENTRY;

  max_width = min(MAX_TEXTENTRY, width);

  wf = LoadDialog(CallBackTable, parent, _T("IDR_XML_TEXTENTRY"));
  if (!wf)
    return;

  if (caption)
    wf->SetCaption(caption);

  wGrid = (WndOwnerDrawFrame*)wf->FindByName(_T("frmGrid"));

  cursor = 0;
  edittext[0] = 0;
  edittext[1] = 0;
  if (!StringIsEmpty(text)) {
    _tcsupr(text);
    CopyString(edittext, text, max_width);
  }
  MoveCursor();

  wf->SetKeyDownNotify(FormKeyDown);

  if (wf->ShowModal() == mrOK) {
    CopyString(text, edittext, max_width);

    // strip trailing spaces
    int len = _tcslen(text) - 1;
    while ((len > 0) && (text[len] == _T(' '))) {
      text[len] = 0;
      len--;
    }
  }

  delete wf;
}

bool
dlgTextEntryShowModal(SingleWindow &parent, TCHAR *text, int width,
                      const TCHAR* caption, AllowedCharactersCallback_t accb)
{
  switch (UIGlobals::GetDialogSettings().text_input_style) {
  case DialogSettings::TextInputStyle::Default:
  case DialogSettings::TextInputStyle::Keyboard:
    if (HasPointer())
      return dlgTextEntryKeyboardShowModal(parent, text, width, caption, accb);
    else {
      dlgTextEntryHighscoreType(parent, text, width, caption);
      return true;
    }

  case DialogSettings::TextInputStyle::HighScore:
    dlgTextEntryHighscoreType(parent, text, width, caption);
    return true;
  }

  return false;
}
