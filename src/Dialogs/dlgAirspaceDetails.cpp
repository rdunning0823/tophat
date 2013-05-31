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

#include "Dialogs/Airspace.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/Message.hpp"
#include "Dialogs/XML.hpp"
#include "Form/Form.hpp"
#include "Form/Button.hpp"
#include "Form/Edit.hpp"
#include "Airspace/AbstractAirspace.hpp"
#include "Airspace/ProtectedAirspaceWarningManager.hpp"
#include "Formatter/AirspaceFormatter.hpp"
#include "Math/FastMath.h"
#include "Geo/GeoVector.hpp"
#include "Units/Units.hpp"
#include "UIGlobals.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"

#include "Compiler.h"

#include <assert.h>
#include <stdio.h>

static ProtectedAirspaceWarningManager *airspace_warnings;
static const AbstractAirspace* airspace;
static WndForm *wf = NULL;

static void
OnAcknowledgeClicked(gcc_unused WndButton &Sender)
{
  assert(airspace);

  if (airspace_warnings == NULL)
    return;

  bool acked = airspace_warnings->get_ack_day(*airspace);
  airspace_warnings->acknowledge_day(*airspace, !acked);

  wf->SetModalResult(mrOK);
}

static void
OnCloseClicked(gcc_unused WndButton &Sender)
{
  wf->SetModalResult(mrOK);
}

static constexpr CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(OnAcknowledgeClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};

static void
UpdateAckButton()
{
  assert(airspace);

  if (airspace_warnings == NULL)
    return;

  WndButton* ack = (WndButton*)wf->FindByName(_T("cmdAcknowledge"));
  assert(ack != NULL);
  ack->SetCaption(airspace_warnings->get_ack_day(*airspace) ?
                  _("Enable") : _("Ack Day"));
}

static void
SetValues()
{
  assert(airspace);

  WndProperty* wp;

  wp = (WndProperty*)wf->FindByName(_T("prpName"));
  assert(wp != NULL);
  wp->SetText(airspace->GetName());
  wp->RefreshDisplay();

  wp = (WndProperty*)wf->FindByName(_T("prpRadio"));
  assert(wp != NULL);
  wp->SetText(airspace->GetRadioText().c_str());
  wp->RefreshDisplay();

  wp = (WndProperty*)wf->FindByName(_T("prpType"));
  assert(wp != NULL);
  wp->SetText(AirspaceFormatter::GetClass(*airspace));
  wp->RefreshDisplay();

  wp = (WndProperty*)wf->FindByName(_T("prpTop"));
  assert(wp != NULL);
  wp->SetText(AirspaceFormatter::GetTop(*airspace).c_str());
  wp->RefreshDisplay();

  wp = (WndProperty*)wf->FindByName(_T("prpBase"));
  assert(wp != NULL);
  wp->SetText(AirspaceFormatter::GetBase(*airspace).c_str());
  wp->RefreshDisplay();

  if (airspace_warnings != NULL) {
    wp = (WndProperty*)wf->FindByName(_T("prpRange"));
    assert(wp != NULL);
    const GeoPoint &ac_loc = XCSoarInterface::Basic().location;
    const GeoPoint closest_loc =
      airspace->ClosestPoint(ac_loc, airspace_warnings->GetProjection());
    const GeoVector vec(ac_loc, closest_loc);
    StaticString<80> buf;
    buf.Format(_T("%d%s"), (int)Units::ToUserDistance(vec.distance),
               Units::GetDistanceName());
    wp->SetText(buf);
    wp->RefreshDisplay();
  }
}

void
dlgAirspaceDetails(const AbstractAirspace& the_airspace,
                   ProtectedAirspaceWarningManager *_airspace_warnings)
{
  if (wf)
    return;

  airspace = &the_airspace;
  airspace_warnings = _airspace_warnings;

  wf = LoadDialog(CallBackTable, UIGlobals::GetMainWindow(),
                  _T("IDR_XML_AIRSPACEDETAILS"));
  assert(wf != NULL);

  SetValues();
  UpdateAckButton();

  wf->ShowModal();

  delete wf;
  wf = NULL;
}
