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

#include "HelpDialog.hpp"
#include "Dialogs/XML.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "Form/Form.hpp"
#include "Form/Util.hpp"
#include "Screen/Layout.hpp"
#include "Units/Units.hpp"
#include "Language/Language.hpp"

class WndButton;

#include <stdio.h>

static WndForm *wf = NULL;

static void
OnCloseClicked(gcc_unused WndButton &Sender)
{
  wf->SetModalResult(mrOK);
}

static constexpr CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};

void
dlgHelpShowModal(SingleWindow &parent,
                 const TCHAR* Caption, const TCHAR* HelpText)
{
  if (!Caption || !HelpText)
    return;

  wf = LoadDialog(CallBackTable, parent,
                  Layout::landscape ? _T("IDR_XML_HELP_L"): _T("IDR_XML_HELP"));

  if (wf == NULL)
    return;

  StaticString<100> full_caption;
  full_caption.Format(_T("%s: %s"), _("Help"), Caption);
  wf->SetCaption(full_caption);

  SetFormValue(*wf, _T("prpHelpText"), HelpText);

  wf->ShowModal();

  delete wf;
}
