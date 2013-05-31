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
#include "Form/Edit.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Key.h"
#include "LocalPath.hpp"
#include "UIGlobals.hpp"
#include "Util/StringUtil.hpp"
#include "IO/DataFile.hpp"
#include "Language/Language.hpp"
#include "Compiler.h"

#include <assert.h>

#define XCSCHKLIST  "xcsoar-checklist.txt"

#define MAXTITLE 200
#define MAXDETAILS 5000

static int page = 0;
static WndForm *wf = NULL;
static WndProperty *wDetails = NULL;

#define MAXLINES 100
#define MAXLISTS 20
static unsigned nTextLines;
static int nLists = 0;
static TCHAR *ChecklistText[MAXTITLE];
static TCHAR *ChecklistTitle[MAXTITLE];

static void
NextPage(int Step)
{
  TCHAR buffer[80];

  page += Step;
  if (page >= nLists)
    page = 0;
  if (page < 0)
    page = nLists - 1;

  _tcscpy(buffer, _("Checklist"));

  if (ChecklistTitle[page] &&
      !StringIsEmpty(ChecklistTitle[page]) &&
      _tcslen(ChecklistTitle[page]) < 60) {
    _tcscat(buffer, _T(": "));
    _tcscat(buffer, ChecklistTitle[page]);
  }
  wf->SetCaption(buffer);

  wDetails->SetText(ChecklistText[page]);
}

static void
OnNextClicked(gcc_unused WndButton &button)
{
  NextPage(+1);
}

static void
OnPrevClicked(gcc_unused WndButton &button)
{
  NextPage(-1);
}

static void
OnCloseClicked(gcc_unused WndButton &button)
{
  wf->SetModalResult(mrOK);
}

static bool
FormKeyDown(gcc_unused WndForm &Sender, unsigned key_code)
{
  switch (key_code) {
  case VK_LEFT:
#ifdef GNAV
  case '6':
#endif
    ((WndButton *)wf->FindByName(_T("cmdPrev")))->SetFocus();
    NextPage(-1);
    return true;

  case VK_RIGHT:
#ifdef GNAV
  case '7':
#endif
    ((WndButton *)wf->FindByName(_T("cmdNext")))->SetFocus();
    NextPage(+1);
    return true;

  default:
    return false;
  }
}

static void
addChecklist(const TCHAR *name, const TCHAR *details)
{
  if (nLists >= MAXLISTS)
    return;

  ChecklistTitle[nLists] = _tcsdup(name);
  ChecklistText[nLists] = _tcsdup(details);
  nLists++;
}

static void
LoadChecklist()
{
  nLists = 0;

  free(ChecklistText[0]);
  ChecklistText[0] = NULL;

  free(ChecklistTitle[0]);
  ChecklistTitle[0] = NULL;

  TLineReader *reader = OpenDataTextFile(_T(XCSCHKLIST));
  if (reader == NULL) {
    addChecklist(_("No checklist loaded"),_("Create xcsoar-checklist.txt\n"));
    return;
  }

  TCHAR Details[MAXDETAILS];
  TCHAR Name[100];
  bool inDetails = false;
  int i;

  Details[0] = 0;
  Name[0] = 0;

  TCHAR *TempString;
  while ((TempString = reader->read()) != NULL) {
    // Look for start
    if (TempString[0] == '[') {
      if (inDetails) {
        addChecklist(Name, Details);
        Details[0] = 0;
        Name[0] = 0;
      }

      // extract name
      for (i = 1; i < MAXTITLE; i++) {
        if (TempString[i] == ']')
          break;

        Name[i - 1] = TempString[i];
      }
      Name[i - 1] = 0;

      inDetails = true;
    } else {
      // append text to details string
      _tcsncat(Details, TempString, MAXDETAILS - 2);
      _tcscat(Details, _T("\n"));
      // TODO code: check the string is not too long
    }
  }

  delete reader;

  if (inDetails) {
    addChecklist(Name, Details);
  }
}

static constexpr CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(OnNextClicked),
  DeclareCallBackEntry(OnPrevClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};

void
dlgChecklistShowModal()
{
  static bool first = true;
  if (first) {
    LoadChecklist();
    first = false;
  }

  wf = LoadDialog(CallBackTable, UIGlobals::GetMainWindow(),
                      Layout::landscape ?
                      _T("IDR_XML_CHECKLIST_L") : _T("IDR_XML_CHECKLIST"));
  if (!wf)
    return;

  nTextLines = 0;

  wf->SetKeyDownNotify(FormKeyDown);

  wDetails = (WndProperty*)wf->FindByName(_T("frmDetails"));
  assert(wDetails != NULL);

  page = 0;
  NextPage(0); // JMW just to turn proper pages on/off

  wf->ShowModal();

  delete wf;
}

