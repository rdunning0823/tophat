/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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
#include "Form/DataField/FileReader.hpp"
#include "LocalPath.hpp"
#include "UtilsSettings.hpp"
#include "ConfigPanel.hpp"
#include "SiteConfigPanel.hpp"
#include "Widget/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "Waypoint/Patterns.hpp"
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

class SiteConfigPanel final : public RowFormWidget, DataFieldListener {
private:
  WndButton *buttonWaypoints;
  StaticString<50> download_name;

public:
  SiteConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()), buttonWaypoints(0),
     download_name(N_("Download from internet")){}

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;
  virtual void Show(const PixelRect &rc) override;
  virtual void Hide() override;

private:
  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df) override;
};

void
SiteConfigPanel::OnModified(DataField &df)
{
  if (IsDataField(MapFile, df)) {
    const ComputerSettings &settings_computer = CommonInterface::GetComputerSettings();
    const TaskBehaviour &task_behaviour = settings_computer.task;

    DataFieldFileReader* dff = (DataFieldFileReader*)&df;
    StaticString<255> label;
    label.clear();
    if (dff->GetAsDisplayString() != nullptr)
      label = dff->GetAsDisplayString();
    if (label == dff->GetScanInternetLabel()) {
      AvailableFile file_filter;
      file_filter.type = AvailableFile::Type::MAP;
      StaticString<15> nationality;
      switch (task_behaviour.contest_nationality) {
      case UNKNOWN:
      case EUROPEAN:
        nationality.clear();
        break;
      case BRITISH:
        nationality = _T("br");
        break;
      case AUSTRALIAN:
        nationality = _T("au");
        break;
      case AMERICAN:
        nationality = _T("us");
        break;
      }

      WideToACPConverter base(nationality.c_str());
      file_filter.area = base;
      AvailableFile result = ShowFilePickAndDownload(file_filter);
      if (result.IsValid()) {
        ACPToWideConverter base(result.GetName());
        if (!base.IsValid())
          return;
        StaticString<255> temp_name(base);
        DataFieldFileReader *maps = (DataFieldFileReader*)&df;
        StaticString<255> buffer;
        LocalPath(buffer.buffer(), base);
        maps->AddFile(base, buffer.c_str());
        if (buffer == dff->GetScanInternetLabel())
          maps->SetAsInteger(0);
        else
          maps->Lookup(buffer.c_str());
      }
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

void
SiteConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  buttonWaypoints = ((WndButton *)ConfigPanel::GetForm().FindByName(_T("cmdWaypoints")));
  assert (buttonWaypoints);
  buttonWaypoints->SetOnClickNotify(dlgConfigWaypointsShowModal);

  WndProperty *wp = Add(_T(""), 0, true);
  wp->SetText(GetPrimaryDataPath());
  wp->SetEnabled(false);

  wp = AddFileReader(_("Map database"),
                     _("The name of the file (.xcm) containing terrain, topography, and optionally "
                         "waypoints, their details and airspaces."),
                     ProfileKeys::MapFile, _T("*.xcm\0*.lkm\0"), true, this);
  DataFieldFileReader *dff = (DataFieldFileReader *)(wp->GetDataField());
  dff->EnableInternetDownload();

  AddFileReader(_("Waypoints"),
                _("Primary waypoints file.  Supported file types are Cambridge/WinPilot files (.dat), "
                    "Zander files (.wpz) or SeeYou files (.cup)."),
                ProfileKeys::WaypointFile, WAYPOINT_FILE_PATTERNS);

  AddFileReader(_("More waypoints"),
                _("Secondary waypoints file.  This may be used to add waypoints for a competition."),
                ProfileKeys::AdditionalWaypointFile, WAYPOINT_FILE_PATTERNS);
  SetExpertRow(AdditionalWaypointFile);

  AddFileReader(_("Watched waypoints"),
                _("Waypoint file containing special waypoints for which additional computations like "
                    "calculation of arrival height in map display always takes place. Useful for "
                    "waypoints like known reliable thermal sources (e.g. powerplants) or mountain passes."),
                ProfileKeys::WatchedWaypointFile, WAYPOINT_FILE_PATTERNS);
  SetExpertRow(WatchedWaypointFile);

  AddFileReader(_("Airspaces"), _("The file name of the primary airspace file."),
                ProfileKeys::AirspaceFile, _T("*.txt\0*.air\0*.sua\0"));

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
CreateSiteConfigPanel()
{
  return new SiteConfigPanel();
}
