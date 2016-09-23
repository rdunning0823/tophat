/*
Copyright_License {

  Top Hat Soaring Glide Computer - http://www.tophatsoaring.org/
  Copyright (C) 2000-2016 The Top Hat Soaring Project
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
#include "Dialogs/Waypoint/WaypointDialogs.hpp"
#include "Dialogs/FilePickAndDownload.hpp"
#include "Form/Button.hpp"
#include "Form/DataField/Listener.hpp"
#include "Form/DataField/File.hpp"
#include "Form/ActionListener.hpp"
#include "LocalPath.hpp"
#include "UtilsSettings.hpp"
#include "ConfigPanel.hpp"
#include "SiteConfigPanel.hpp"
#include "Widget/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "Waypoint/Patterns.hpp"
#include "Language/Language.hpp"
#include "Util/StaticString.hxx"
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
  AirfieldFile,
  EditWaypoints,
};

class SiteConfigPanel final : public RowFormWidget, DataFieldListener,
    public ActionListener {
private:
  StaticString<50> download_name;
  /**
   * is this a stand-alone widget, or displayed in the dlgConfiguration menu?
   */
  bool stand_alone;

public:
  SiteConfigPanel()
  :RowFormWidget(UIGlobals::GetDialogLook()),
   download_name(N_("Download from internet")) {}

  SiteConfigPanel(bool _stand_alone)
    :RowFormWidget(UIGlobals::GetDialogLook()),
     download_name(N_("Download from internet")), stand_alone(_stand_alone){}

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;

private:
  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df) override;

  /* from ActionListener */
  virtual void OnAction(int id) override;
};

void
SiteConfigPanel::OnAction(int id)
{
  if (id == EditWaypoints)
    dlgConfigWaypointsShowModal();
}

void
SiteConfigPanel::OnModified(DataField &df)
{
  AvailableFile file_filter;
  file_filter.Clear();
  FileDataField* dff = (FileDataField*)&df;
  if (IsDataField(MapFile, df))
    file_filter.type = FileType::MAP;
  else if (IsDataField(WaypointFile, df))
    file_filter.type = FileType::WAYPOINT;
  else if (IsDataField(AirspaceFile, df))
    file_filter.type = FileType::AIRSPACE;
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
SiteConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  WndProperty *wp = Add(_T(""), 0, true);
  wp->SetText(GetPrimaryDataPath());
  wp->SetEnabled(false);

  wp = AddFile(_("Map database"),
               _("The name of the file (.xcm) containing terrain, topography, and optionally "
                   "waypoints, their details and airspaces."),
               ProfileKeys::MapFile, _T("*.xcm\0*.lkm\0"), this);
  FileDataField *dff = (FileDataField *)(wp->GetDataField());
  dff->EnableInternetDownload();


  wp = AddFile(_("Waypoints"),
               _("Primary waypoints file.  Supported file types are Cambridge/WinPilot files (.dat), "
                   "Zander files (.wpz) or SeeYou files (.cup).  Available at http://soaringweb.org/TP"),
               ProfileKeys::WaypointFile, WAYPOINT_FILE_PATTERNS,
               this);
  dff = (FileDataField *)(wp->GetDataField());
  dff->EnableInternetDownload();

  AddFile(_("More waypoints"),
          _("Secondary waypoints file.  This may be used to add waypoints for a competition."),
          ProfileKeys::AdditionalWaypointFile, WAYPOINT_FILE_PATTERNS);
  SetExpertRow(AdditionalWaypointFile);

  AddFile(_("Watched waypoints"),
          _("Waypoint file containing special waypoints for which additional computations like "
              "calculation of arrival height in map display always takes place. Useful for "
              "waypoints like known reliable thermal sources (e.g. powerplants) or mountain passes."),
          ProfileKeys::WatchedWaypointFile, WAYPOINT_FILE_PATTERNS);
  SetExpertRow(WatchedWaypointFile);

  wp = AddFile(_("Airspaces"), _("The file name of the primary airspace file.  Available at http://soaringweb.org/TP"),
               ProfileKeys::AirspaceFile, _T("*.txt\0*.air\0*.sua\0"),
               this);
  dff = (FileDataField *)(wp->GetDataField());
  dff->EnableInternetDownload();


  AddFile(_("More airspaces"), _("The file name of the secondary airspace file."),
          ProfileKeys::AdditionalAirspaceFile, _T("*.txt\0*.air\0*.sua\0"));
  SetExpertRow(AdditionalAirspaceFile);

  AddFile(_("Waypoint details"),
          _("The file may contain extracts from enroute supplements or other contributed "
              "information about individual waypoints and airfields."),
          ProfileKeys::AirfieldFile, _T("*.txt\0"));
  SetExpertRow(AirfieldFile);

  if (stand_alone)
    AddButton(_("Edit waypoints"), *this, EditWaypoints);
}

bool
SiteConfigPanel::Save(bool &_changed)
{
  bool changed = false;

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

  return true;
}

Widget *
CreateSiteConfigPanel(bool stand_alone)
{
  return new SiteConfigPanel(stand_alone);
}


Widget *
CreateSiteConfigPanel()
{
  return CreateSiteConfigPanel(false);
}
