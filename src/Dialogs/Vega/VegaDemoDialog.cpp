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

#include "VegaDialogs.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/XML.hpp"
#include "UIGlobals.hpp"
#include "Units/Units.hpp"
#include "Device/device.hpp"
#include "Math/FastMath.h"
#include "Form/Form.hpp"
#include "Form/Button.hpp"
#include "Form/DataField/Boolean.hpp"
#include "Form/DataField/Float.hpp"
#include "PeriodClock.hpp"
#include "Form/Util.hpp"
#include "Operation/PopupOperationEnvironment.hpp"

static WndForm *wf;

static fixed VegaDemoW = fixed_zero;
static fixed VegaDemoV = fixed_zero;
static bool VegaDemoAudioClimb = true;

static void
OnCloseClicked(gcc_unused WndButton &Sender)
{
  wf->SetModalResult(mrOK);
}

static void
VegaWriteDemo()
{
  static PeriodClock last_time;
  if (!last_time.CheckUpdate(250))
    return;

  TCHAR dbuf[100];
  _stprintf(dbuf, _T("PDVDD,%d,%d"),
            iround(VegaDemoW * 10),
            iround(VegaDemoV * 10));

  PopupOperationEnvironment env;
  VarioWriteNMEA(dbuf, env);
}

static void
OnVegaDemoW(DataField *Sender, DataField::DataAccessMode Mode)
{
  DataFieldFloat &df = *(DataFieldFloat *)Sender;

  switch (Mode){
  case DataField::daChange:
    VegaDemoW = Units::ToSysVSpeed(df.GetAsFixed());
    VegaWriteDemo();
    break;

  case DataField::daSpecial:
    return;
  }
}

static void
OnVegaDemoV(DataField *Sender, DataField::DataAccessMode Mode)
{
  DataFieldFloat &df = *(DataFieldFloat *)Sender;

  switch (Mode){
  case DataField::daChange:
    VegaDemoV = Units::ToSysSpeed(df.GetAsFixed());
    VegaWriteDemo();
    break;

  case DataField::daSpecial:
    return;
  }
}

static void
OnVegaDemoAudioClimb(DataField *Sender, DataField::DataAccessMode Mode)
{
  switch (Mode){
  case DataField::daChange:
    VegaDemoAudioClimb = Sender->GetAsInteger() == 1;
    VegaWriteDemo();
    break;

  case DataField::daSpecial:
    return;
  }
}

static constexpr CallBackTableEntry CallBackTable[]={
  DeclareCallBackEntry(OnVegaDemoW),
  DeclareCallBackEntry(OnVegaDemoV),
  DeclareCallBackEntry(OnVegaDemoAudioClimb),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};

void
dlgVegaDemoShowModal()
{
  wf = LoadDialog(CallBackTable, UIGlobals::GetMainWindow(),
                  _T("IDR_XML_VEGADEMO"));

  if (!wf) return;

  PopupOperationEnvironment env;
  VarioWriteNMEA(_T("PDVSC,S,DemoMode,0"), env);
  VarioWriteNMEA(_T("PDVSC,S,DemoMode,3"), env);

  LoadFormProperty(*wf, _T("prpVegaDemoW"), UnitGroup::VERTICAL_SPEED, VegaDemoW);
  LoadFormProperty(*wf, _T("prpVegaDemoV"), UnitGroup::VERTICAL_SPEED, VegaDemoV);
  LoadFormProperty(*wf, _T("prpVegaDemoAudioClimb"), VegaDemoAudioClimb);

  wf->ShowModal();
  delete wf;

  // deactivate demo.
  VarioWriteNMEA(_T("PDVSC,S,DemoMode,0"), env);
}
