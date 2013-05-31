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
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/XML.hpp"
#include "Dialogs/Message.hpp"
#include "Form/Form.hpp"
#include "Form/Button.hpp"
#include "UIGlobals.hpp"
#include "Protection.hpp"
#include "Screen/Layout.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Profile/Profile.hpp"
#include "Components.hpp"
#include "Waypoint/Waypoints.hpp"
#include "Waypoint/WaypointGlue.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Compiler.h"

#include <assert.h>

static bool WaypointsNeedSave = false;
static WndForm *wf = NULL;

static void
OnCloseClicked(gcc_unused WndButton &button)
{
  wf->SetModalResult(mrOK);
}

static void
OnWaypointNewClicked(gcc_unused WndButton &Sender)
{
  if (!WaypointGlue::IsWritable()) {
    ShowMessageBox(_("Waypoints not editable"), _("Error"), MB_OK);
    return;
  }

  Waypoint edit_waypoint = way_points.Create(XCSoarInterface::Basic().location);
  edit_waypoint.elevation = XCSoarInterface::Calculated().terrain_valid
    ? XCSoarInterface::Calculated().terrain_altitude
    : XCSoarInterface::Basic().nav_altitude;

  if (dlgWaypointEditShowModal(edit_waypoint) &&
      edit_waypoint.name.size()) {
    WaypointsNeedSave = true;

    ScopeSuspendAllThreads suspend;
    way_points.Append(edit_waypoint);
    way_points.Optimise();
  }
}

static void
OnWaypointEditClicked(gcc_unused WndButton &Sender)
{
  if (!WaypointGlue::IsWritable()) {
    ShowMessageBox(_("Waypoints not editable"), _("Error"), MB_OK);
    return;
  }

  const Waypoint *way_point = ShowWaypointListDialog(UIGlobals::GetMainWindow(),
                                                XCSoarInterface::Basic().location);
  if (way_point) {
    Waypoint wp_copy = *way_point;
    if (dlgWaypointEditShowModal(wp_copy)) {
      WaypointsNeedSave = true;

      ScopeSuspendAllThreads suspend;
      way_points.Replace(*way_point, wp_copy);
      way_points.Optimise();
    }
  }
}

static void
SaveWaypoints()
{
  if (!WaypointGlue::SaveWaypoints(way_points))
    ShowMessageBox(_("Waypoints not editable"), _("Error"), MB_OK);
  else
    WaypointFileChanged = true;

  WaypointsNeedSave = false;
}

static void
OnWaypointSaveClicked(gcc_unused WndButton &Sender)
{
  SaveWaypoints();
}

static void
OnWaypointDeleteClicked(gcc_unused WndButton &Sender)
{
  if (!WaypointGlue::IsWritable()) {
    ShowMessageBox(_("Waypoints not editable"), _("Error"), MB_OK);
    return;
  }

#ifdef OLD_TASK
  int res;
  res = ShowWaypointListDialog(XCSoarInterface::Basic().location);
  if (res != -1){
    if(ShowMessageBox(way_points.get(res).name,
                   _("Delete Waypoint?"),
                   MB_YESNO|MB_ICONQUESTION) == IDYES) {
      Waypoint &waypoint = way_points.set(res);

      waypoint.FileNum = -1;
      waypoint.original_id = 0;
      WaypointsNeedSave = true;
    }
  }
#endif
}

static constexpr CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(OnWaypointNewClicked),
  DeclareCallBackEntry(OnWaypointDeleteClicked),
  DeclareCallBackEntry(OnWaypointEditClicked),
  DeclareCallBackEntry(OnWaypointSaveClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};

void
dlgConfigWaypointsShowModal()
{
  wf = LoadDialog(CallBackTable, UIGlobals::GetMainWindow(),
                  Layout::landscape ? _T("IDR_XML_CONFIG_WAYPOINTS_L") :
                                      _T("IDR_XML_CONFIG_WAYPOINTS"));
  assert(wf != NULL);

  WaypointsNeedSave = false;

  wf->ShowModal();

  if (WaypointsNeedSave &&
      ShowMessageBox(_("Save changes to waypoint file?"), _("Waypoints edited"),
                  MB_YESNO | MB_ICONQUESTION) == IDYES)
      SaveWaypoints();

  delete wf;
}
