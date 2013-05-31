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

#include "Profile/ProfileKeys.hpp"
#include "Language/Language.hpp"
#include "Dialogs/Dialogs.h"
#include "Dialogs/Waypoint.hpp"
#include "Dialogs/FilePickAndDownload.hpp"
#include "Form/Button.hpp"
#include "Form/DataField/Listener.hpp"
#include "Form/DataField/FileReader.hpp"
#include "LocalPath.hpp"
#include "Protection.hpp"
#include "ConfigPanel.hpp"
#include "SiteConfigPanel.hpp"
#include "Form/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "Language/Language.hpp"
#include "Util/StaticString.hpp"
#include "Util/ConvertString.hpp"
#include "Interface.hpp"
#include "Repository/AvailableFile.hpp"

#include <string>

enum ControlIndex {
  DataPath,
  MapFile,
  WaypointFile,
  AdditionalWaypointFile,
  WatchedWaypointFile,
  AirspaceFile,
  AdditionalAirspaceFile,
  AirfieldFile
};

class SiteConfigPanel : public RowFormWidget, DataFieldListener {
private:
  WndButton *buttonWaypoints;
  StaticString<50> download_name;

public:
  SiteConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()), buttonWaypoints(0),
     download_name(N_("Download from internet")){}

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);
  virtual void Show(const PixelRect &rc);
  virtual void Hide();

private:
  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df);
};



void
SiteConfigPanel::OnModified(DataField &df)
{
  AvailableFile file_filter;
  file_filter.Clear();
  DataFieldFileReader* dff = (DataFieldFileReader*)&df;
  if (IsDataField(MapFile, df))
    file_filter.type = AvailableFile::Type::MAP;
  else if (IsDataField(WaypointFile, df))
    file_filter.type = AvailableFile::Type::WAYPOINT;
  else if (IsDataField(AirspaceFile, df))
    file_filter.type = AvailableFile::Type::AIRSPACE;
  else
    return;

  StaticString<255> label;
  label.clear();

  if (dff->GetAsDisplayString() != nullptr)
    label = dff->GetAsDisplayString();
  if (label == dff->GetScanInternetLabel()) {

    AvailableFile result = ShowFilePickAndDownload(file_filter);
    if (result.IsValid()) {
      ACPToWideConverter base(result.GetName());
      if (!base.IsValid())
        return;
      StaticString<255> temp_name(base);
      StaticString<255> buffer;
      LocalPath(buffer.buffer(), base);
      dff->AddFile(base, buffer.c_str());
      if (buffer == dff->GetScanInternetLabel())
        dff->SetAsInteger(0);
      else
        dff->Lookup(buffer.c_str());
    }
  }
}

void
SiteConfigPanel::Show(const PixelRect &rc)
{
  buttonWaypoints->SetVisible(CommonInterface::SetUISettings().dialog.expert);
  RowFormWidget::Show(rc);
}

void
SiteConfigPanel::Hide()
{
  buttonWaypoints->SetVisible(false);
  RowFormWidget::Hide();
}

static void
OnWaypoints(gcc_unused WndButton &button)
{
  dlgConfigWaypointsShowModal();
}

void
SiteConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  buttonWaypoints = ((WndButton *)ConfigPanel::GetForm().FindByName(_T("cmdWaypoints")));
  assert (buttonWaypoints);
  buttonWaypoints->SetOnClickNotify(OnWaypoints);

  WndProperty *wp = Add(_T(""), 0, true);
  wp->SetText(GetPrimaryDataPath());
  wp->SetEnabled(false);

  wp = AddFileReader(_("Map database"),
                     _("The name of the file (.xcm) containing terrain, topography, and optionally "
                         "waypoints, their details and airspaces.  Available at www.xcsoar.org/download/maps"),
                     ProfileKeys::MapFile, _T("*.xcm\0*.lkm\0"), true, this);
  DataFieldFileReader *dff = (DataFieldFileReader *)(wp->GetDataField());
  dff->EnableInternetDownload();

  wp = AddFileReader(_("Waypoints"),
                    _("Primary waypoints file.  Supported file types are Cambridge/WinPilot files (.dat), "
                        "Zander files (.wpz) or SeeYou files (.cup).  Available at http://soaringweb.org/TP"),
                    ProfileKeys::WaypointFile, _T("*.dat\0*.xcw\0*.cup\0*.wpz\0*.wpt\0"),
                    true, this);
  dff = (DataFieldFileReader *)(wp->GetDataField());
  dff->EnableInternetDownload();

  AddFileReader(_("More waypoints"),
                _("Secondary waypoints file.  This may be used to add waypoints for a competition."),
                ProfileKeys::AdditionalWaypointFile, _T("*.dat\0*.xcw\0*.cup\0*.wpz\0*.wpt\0"));
  SetExpertRow(AdditionalWaypointFile);

  AddFileReader(_("Watched waypoints"),
                _("Waypoint file containing special waypoints for which additional computations like "
                    "calculation of arrival height in map display always takes place. Useful for "
                    "waypoints like known reliable thermal sources (e.g. powerplants) or mountain passes."),
                ProfileKeys::WatchedWaypointFile, _T("*.dat\0*.xcw\0*.cup\0*.wpz\0*.wpt\0"));
  SetExpertRow(WatchedWaypointFile);

  wp = AddFileReader(_("Airspaces"), _("The file name of the primary airspace file.  Available at http://soaringweb.org/TP"),
                     ProfileKeys::AirspaceFile, _T("*.txt\0*.air\0*.sua\0"),
                     true, this);
  dff = (DataFieldFileReader *)(wp->GetDataField());
  dff->EnableInternetDownload();


  AddFileReader(_("More airspaces"), _("The file name of the secondary airspace file."),
                ProfileKeys::AdditionalAirspaceFile, _T("*.txt\0*.air\0*.sua\0"));
  SetExpertRow(AdditionalAirspaceFile);

  AddFileReader(_("Waypoint details"),
                _("The file may contain extracts from enroute supplements or other contributed "
                    "information about individual waypoints and airfields."),
                ProfileKeys::AirfieldFile, _T("*.txt\0"));
  SetExpertRow(AirfieldFile);
}

bool
SiteConfigPanel::Save(bool &_changed, bool &_require_restart)
{
  bool changed = false, require_restart = false;

  MapFileChanged = SaveValueFileReader(MapFile, ProfileKeys::MapFile);

  // WaypointFileChanged has already a meaningful value
  WaypointFileChanged |= SaveValueFileReader(WaypointFile, ProfileKeys::WaypointFile);
  WaypointFileChanged |= SaveValueFileReader(AdditionalWaypointFile, ProfileKeys::AdditionalWaypointFile);
  WaypointFileChanged |= SaveValueFileReader(WatchedWaypointFile, ProfileKeys::WatchedWaypointFile);

  AirspaceFileChanged = SaveValueFileReader(AirspaceFile, ProfileKeys::AirspaceFile);
  AirspaceFileChanged |= SaveValueFileReader(AdditionalAirspaceFile, ProfileKeys::AdditionalAirspaceFile);

  AirfieldFileChanged = SaveValueFileReader(AirfieldFile, ProfileKeys::AirfieldFile);


  changed = WaypointFileChanged || AirfieldFileChanged || MapFileChanged;

  _changed |= changed;
  _require_restart |= require_restart;

  return true;
}

Widget *
CreateSiteConfigPanel()
{
  return new SiteConfigPanel();
}
