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

#ifdef GNAV

#include "Form/Form.hpp"
#include "Form/Util.hpp"
#include "Form/Button.hpp"
#include "Units/Units.hpp"
#include "Math/FastMath.h"
#include "Form/DataField/Base.hpp"
#include "Form/DataField/Boolean.hpp"
#include "Compatibility/string.h"
#include "PeriodClock.hpp"
#include "Components.hpp"
#include "Hardware/AltairControl.hpp"
#include "UIGlobals.hpp"

static WndForm *wf=NULL;

static void
OnCloseClicked(gcc_unused WndButton &Sender)
{
	wf->SetModalResult(mrOK);
}

static bool EnableAutoBrightness = true;
static int BrightnessValue = 0;

static void UpdateValues() {
  static PeriodClock last_time;
  if (!last_time.CheckUpdate(200))
    return;

  if (EnableAutoBrightness) {
    altair_control.SetBacklight(-100);
  } else {
    altair_control.SetBacklight(BrightnessValue);
  }
}

static void OnAutoData(DataField *Sender, DataField::DataAccessMode Mode){
  DataFieldBoolean &df = *(DataFieldBoolean *)Sender;

  switch(Mode){
    case DataField::daChange:
      EnableAutoBrightness = df.GetAsBoolean();
      UpdateValues();
    break;

  case DataField::daSpecial:
    return;
  }
}


static void OnBrightnessData(DataField *Sender,
			     DataField::DataAccessMode Mode){
  switch(Mode){
    case DataField::daChange:
      BrightnessValue = Sender->GetAsInteger();
      UpdateValues();
    break;

  case DataField::daSpecial:
    return;
  }
}


static constexpr CallBackTableEntry CallBackTable[]={
  DeclareCallBackEntry(OnAutoData),
  DeclareCallBackEntry(OnBrightnessData),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};



void dlgBrightnessShowModal(){
  wf = LoadDialog(CallBackTable, UIGlobals::GetMainWindow(),
		      _T("IDR_XML_BRIGHTNESS"));
  if (wf == NULL)
    return;

  LoadFormProperty(*wf, _T("prpBrightness"), BrightnessValue);
  LoadFormProperty(*wf, _T("prpAuto"), EnableAutoBrightness);

  wf->ShowModal();

  UpdateValues();

  delete wf;
}

#else /* !GNAV */

#include "Dialogs/Message.hpp"

void
dlgBrightnessShowModal()
{
  /* XXX this is ugly, non-Altair platforms should not even see the
     according menu item; not translating this superfluous message */
  ShowMessageBox(_T("Only available on Altair"), _T("Brightness"),
              MB_OK|MB_ICONERROR);
}

#endif /* !GNAV */
