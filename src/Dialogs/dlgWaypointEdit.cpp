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

#include "Dialogs/Waypoint.hpp"
#include "Dialogs/TextEntry.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/XML.hpp"
#include "Dialogs/Message.hpp"
#include "Form/Form.hpp"
#include "Form/Util.hpp"
#include "Form/Button.hpp"
#include "Form/Edit.hpp"
#include "Form/DataField/Enum.hpp"
#include "Units/Units.hpp"
#include "Screen/Layout.hpp"
#include "Terrain/RasterTerrain.hpp"
#include "Math/FastMath.h"
#include "UIGlobals.hpp"
#include "Components.hpp"
#include "Waypoint/Waypoint.hpp"
#include "Util/StringUtil.hpp"
#include "Compiler.h"
#include "Sizes.h"
#include "Interface.hpp"
#include "Language/Language.hpp"

#include <stdio.h>

static WndForm *wf = NULL;
static Waypoint *global_wpt = NULL;

static WndButton *buttonName = NULL;
static WndButton *buttonComment = NULL;

static void
UpdateButtons()
{
  StaticString<64> text;

  text.Format(_T("%s: %s"), _("Name"),
              global_wpt->name.empty()
              ? _("(blank)") : global_wpt->name.c_str());
  buttonName->SetCaption(text);

  text.Format(_T("%s: %s"), _("Comment"),
              global_wpt->comment.empty()
              ? _("(blank)") : global_wpt->comment.c_str());
  buttonComment->SetCaption(text);
}

static void
OnNameClicked(gcc_unused WndButton &button)
{
  StaticString<NAME_SIZE + 1> buff(global_wpt->name.c_str());
  if (!TextEntryDialog(*(SingleWindow *)button.GetRootOwner(), buff))
    return;

  global_wpt->name = buff;

  UpdateButtons();
}

static void
OnCommentClicked(gcc_unused WndButton &button)
{
  StaticString<51> buff(global_wpt->comment.c_str());
  if (!TextEntryDialog(*(SingleWindow *)button.GetRootOwner(), buff))
    return;

  global_wpt->comment = buff;

  UpdateButtons();
}

static void
SetUnits()
{
  WndProperty* wp;
  switch (CommonInterface::GetUISettings().coordinate_format) {
  case CoordinateFormat::DDMMSS: // ("DDMMSS");
  case CoordinateFormat::DDMMSS_SS: // ("DDMMSS.ss");
    wp = (WndProperty*)wf->FindByName(_T("prpLongitudeDDDD"));
    assert(wp != NULL);
    wp->Hide();

    wp = (WndProperty*)wf->FindByName(_T("prpLatitudeDDDD"));
    assert(wp != NULL);
    wp->Hide();

    wp = (WndProperty*)wf->FindByName(_T("prpLongitudemmm"));
    assert(wp != NULL);
    wp->Hide();

    wp = (WndProperty*)wf->FindByName(_T("prpLatitudemmm"));
    assert(wp != NULL);
    wp->Hide();

    break;

  case CoordinateFormat::DDMM_MMM: // ("DDMM.mmm");
    wp = (WndProperty*)wf->FindByName(_T("prpLongitudeDDDD"));
    assert(wp != NULL);
    wp->Hide();

    wp = (WndProperty*)wf->FindByName(_T("prpLatitudeDDDD"));
    assert(wp != NULL);
    wp->Hide();

    wp = (WndProperty*)wf->FindByName(_T("prpLongitudeS"));
    assert(wp != NULL);
    wp->Hide();

    wp = (WndProperty*)wf->FindByName(_T("prpLatitudeS"));
    assert(wp != NULL);
    wp->Hide();

    break;

  case CoordinateFormat::DD_DDDD: // ("DD.dddd");
    wp = (WndProperty*)wf->FindByName(_T("prpLongitudeM"));
    assert(wp != NULL);
    wp->Hide();

    wp = (WndProperty*)wf->FindByName(_T("prpLatitudeM"));
    assert(wp != NULL);
    wp->Hide();

    wp = (WndProperty*)wf->FindByName(_T("prpLongitudeS"));
    assert(wp != NULL);
    wp->Hide();

    wp = (WndProperty*)wf->FindByName(_T("prpLatitudeS"));
    assert(wp != NULL);
    wp->Hide();

    wp = (WndProperty*)wf->FindByName(_T("prpLongitudemmm"));
    // hide this field for DD.dddd format
    assert(wp != NULL);
    wp->Hide();

    wp = (WndProperty*)wf->FindByName(_T("prpLatitudemmm"));
    assert(wp != NULL);
    wp->Hide();

    break;
  case CoordinateFormat::UTM:
    break;
  }
}

static void
SetValues()
{
  WndProperty* wp;
  bool sign;
  int dd,mm,ss;

  global_wpt->location.longitude.ToDMS(dd, mm, ss, sign);

  wp = (WndProperty*)wf->FindByName(_T("prpLongitudeSign"));
  assert(wp != NULL);

  DataFieldEnum* dfe;
  dfe = (DataFieldEnum*)wp->GetDataField();
  dfe->addEnumText((_T("W")));
  dfe->addEnumText((_T("E")));
  dfe->Set(sign);
  wp->RefreshDisplay();

  LoadFormProperty(*wf, _T("prpLongitudeD"), dd);

  switch (CommonInterface::GetUISettings().coordinate_format) {
  case CoordinateFormat::DDMMSS: // ("DDMMSS");
  case CoordinateFormat::DDMMSS_SS: // ("DDMMSS.ss");
    LoadFormProperty(*wf, _T("prpLongitudeM"), mm);
    LoadFormProperty(*wf, _T("prpLongitudeS"), ss);
    break;
  case CoordinateFormat::DDMM_MMM: // ("DDMM.mmm");
    LoadFormProperty(*wf, _T("prpLongitudeM"), mm);
    LoadFormProperty(*wf, _T("prpLongitudemmm"), 1000 * fixed(ss) / 60);
    break;
  case CoordinateFormat::DD_DDDD: // ("DD.dddd");
    LoadFormProperty(*wf, _T("prpLongitudeDDDD"),
                     10000 * (fixed)(mm + ss) / 3600);
    break;
  case CoordinateFormat::UTM:
    break;
  }

  global_wpt->location.latitude.ToDMS(dd, mm, ss, sign);

  LoadFormProperty(*wf, _T("prpLatitudeD"), dd);

  wp = (WndProperty*)wf->FindByName(_T("prpLatitudeSign"));
  assert(wp != NULL);
  dfe = (DataFieldEnum*)wp->GetDataField();
  dfe->addEnumText((_T("S")));
  dfe->addEnumText((_T("N")));
  dfe->Set(sign);
  wp->RefreshDisplay();

  wp = (WndProperty*)wf->FindByName(_T("prpLatitudeD"));
  assert(wp != NULL);
  wp->GetDataField()->SetAsInteger(dd);
  wp->RefreshDisplay();

  switch (CommonInterface::GetUISettings().coordinate_format) {
  case CoordinateFormat::DDMMSS: // ("DDMMSS");
  case CoordinateFormat::DDMMSS_SS: // ("DDMMSS.ss");
    LoadFormProperty(*wf, _T("prpLatitudeM"), mm);
    LoadFormProperty(*wf, _T("prpLatitudeS"), ss);
    break;
  case CoordinateFormat::DDMM_MMM: // ("DDMM.mmm");
    LoadFormProperty(*wf, _T("prpLatitudeM"), mm);
    LoadFormProperty(*wf, _T("prpLatitudemmm"), 1000 * fixed(ss) / 60);
    break;
  case CoordinateFormat::DD_DDDD: // ("DD.dddd");
    LoadFormProperty(*wf, _T("prpLatitudeDDDD"),
                     10000 * (fixed)(mm + ss) / 3600);
    break;
  case CoordinateFormat::UTM:
    break;
  }

  LoadFormProperty(*wf, _T("prpAltitude"), UnitGroup::ALTITUDE, global_wpt->elevation);

  wp = (WndProperty*)wf->FindByName(_T("prpFlags"));
  assert(wp != NULL);
  dfe = (DataFieldEnum*)wp->GetDataField();

  dfe->addEnumText(_T("Turnpoint"));
  dfe->addEnumText(_T("Airport"));
  dfe->addEnumText(_T("Landpoint"));

  if (global_wpt->IsAirport())
    dfe->Set(1);
  else if (global_wpt->IsLandable())
    dfe->Set(2);
  else
    dfe->Set(0);

  wp->RefreshDisplay();
}

static void
GetValues()
{
  WndProperty* wp;
  bool sign = false;
  int dd = 0;
  // mm,ss are numerators (division) so don't want to lose decimals
  double num = 0, mm = 0, ss = 0;

  sign = GetFormValueInteger(*wf, _T("prpLongitudeSign")) == 1;
  dd = GetFormValueInteger(*wf, _T("prpLongitudeD"));

  switch (CommonInterface::GetUISettings().coordinate_format) {
  case CoordinateFormat::DDMMSS: // ("DDMMSS");
  case CoordinateFormat::DDMMSS_SS: // ("DDMMSS.ss");
    mm = GetFormValueInteger(*wf, _T("prpLongitudeM"));
    ss = GetFormValueInteger(*wf, _T("prpLongitudeS"));
    num = dd + mm / 60.0 + ss / 3600.0;
    break;
  case CoordinateFormat::DDMM_MMM: // ("DDMM.mmm");
    mm = GetFormValueInteger(*wf, _T("prpLongitudeM"));
    ss = GetFormValueInteger(*wf, _T("prpLongitudemmm"));
    num = dd + (mm + ss / 1000.0) / 60.0;
    break;
  case CoordinateFormat::DD_DDDD: // ("DD.dddd");
    mm = GetFormValueInteger(*wf, _T("prpLongitudeDDDD"));
    num = dd + mm / 10000;
    break;
  case CoordinateFormat::UTM:
    break;
  }

  if (!sign)
    num = -num;

  global_wpt->location.longitude = Angle::Degrees(fixed(num));

  sign = GetFormValueInteger(*wf, _T("prpLatitudeSign")) == 1;
  dd = GetFormValueInteger(*wf, _T("prpLatitudeD"));

  switch (CommonInterface::GetUISettings().coordinate_format) {
  case CoordinateFormat::DDMMSS: // ("DDMMSS");
  case CoordinateFormat::DDMMSS_SS: // ("DDMMSS.ss");
    mm = GetFormValueInteger(*wf, _T("prpLatitudeM"));
    ss = GetFormValueInteger(*wf, _T("prpLatitudeS"));
    num = dd + mm / 60.0 + ss / 3600.0;
    break;
  case CoordinateFormat::DDMM_MMM: // ("DDMM.mmm");
    mm = GetFormValueInteger(*wf, _T("prpLatitudeM"));
    ss = GetFormValueInteger(*wf, _T("prpLatitudemmm"));
    num = dd + (mm + ss / 1000.0) / 60.0;
    break;
  case CoordinateFormat::DD_DDDD: // ("DD.dddd");
    mm = GetFormValueInteger(*wf, _T("prpLatitudeDDDD"));
    num = dd + mm / 10000;
    break;
  case CoordinateFormat::UTM:
    break;
  }

  if (!sign)
    num = -num;

  global_wpt->location.latitude = Angle::Degrees(fixed(num));

  ss = GetFormValueInteger(*wf, _T("prpAltitude"));
  global_wpt->elevation = (ss == 0 && terrain != NULL)
    ? fixed(terrain->GetTerrainHeight(global_wpt->location))
    : Units::ToSysAltitude(fixed(ss));

  wp = (WndProperty*)wf->FindByName(_T("prpFlags"));
  assert(wp != NULL);
  switch(wp->GetDataField()->GetAsInteger()) {
  case 1:
    global_wpt->flags.turn_point = true;
    global_wpt->type = Waypoint::Type::AIRFIELD;
    break;
  case 2:
    global_wpt->type = Waypoint::Type::OUTLANDING;
    break;
  default:
    global_wpt->type = Waypoint::Type::NORMAL;
    global_wpt->flags.turn_point = true;
  };
}

static void
OnCloseClicked(gcc_unused WndButton &Sender)
{
  wf->SetModalResult(mrOK);
}

static constexpr CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnNameClicked),
  DeclareCallBackEntry(OnCommentClicked),
  DeclareCallBackEntry(NULL)
};

bool
dlgWaypointEditShowModal(Waypoint &way_point)
{
  global_wpt = &way_point;

  wf = LoadDialog(CallBackTable, UIGlobals::GetMainWindow(),
                  Layout::landscape ?
                  _T("IDR_XML_WAYPOINTEDIT_L") : _T("IDR_XML_WAYPOINTEDIT"));
  assert(wf != NULL);

  buttonName = ((WndButton *)wf->FindByName(_T("cmdName")));
  buttonComment = ((WndButton *)wf->FindByName(_T("cmdComment")));
  assert(buttonName != NULL);
  assert(buttonComment != NULL);

  UpdateButtons();

  SetUnits();

  SetValues();

  if (CommonInterface::GetUISettings().coordinate_format ==
      CoordinateFormat::UTM) {
    ShowMessageBox(
        _("Sorry, the waypoint editor is not yet available for the UTM coordinate format."),
        _("Waypoint Editor"), MB_OK);
    return false;
  }

  wf->SetModalResult(mrCancel);

  bool retval = false;
  if (wf->ShowModal() == mrOK) {
    GetValues();
    retval = true;
  }

  delete wf;
  return retval;
}
