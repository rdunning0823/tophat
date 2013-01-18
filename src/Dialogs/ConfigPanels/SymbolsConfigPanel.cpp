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

#include "SymbolsConfigPanel.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Base.hpp"
#include "Form/DataField/Listener.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Form/Form.hpp"
#include "Form/RowFormWidget.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "UIGlobals.hpp"
#include "MapSettings.hpp"

enum ControlIndex {
  DISPLAY_TRACK_BEARING,
  ENABLE_FLARM_MAP,
  TRAIL_LENGTH,
  TRAIL_DRIFT,
  TRAIL_TYPE,
  TRAIL_WIDTH,
  WIND_ARROW_STYLE
};

class SymbolsConfigPanel
  : public RowFormWidget, DataFieldListener {
public:
  SymbolsConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
  void ShowTrailControls(bool show);

  /* methods from Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);

private:
  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df);
};

void
SymbolsConfigPanel::ShowTrailControls(bool show)
{
  SetRowVisible(TRAIL_DRIFT, show);
  SetRowVisible(TRAIL_TYPE, show);
  SetRowVisible(TRAIL_WIDTH, show);
}

void
SymbolsConfigPanel::OnModified(DataField &df)
{
  if (IsDataField(TRAIL_LENGTH, df)) {
    TrailSettings::Length trail_length = (TrailSettings::Length)df.GetAsInteger();
    ShowTrailControls(trail_length != TrailSettings::Length::OFF);
  }
}

static const StaticEnumChoice  ground_track_mode_list[] = {
  { (unsigned)DisplayGroundTrack::OFF, N_("Off"), N_("Disable display of ground track line.") },
  { (unsigned)DisplayGroundTrack::ON, N_("On"), N_("Always display ground track line.") },
  { (unsigned)DisplayGroundTrack::AUTO, N_("Auto"), N_("Display ground track line if there is a significant difference to plane heading.") },
  { 0 }
};

const TCHAR *trail_length_help = N_("Determines whether and how long a snail trail is drawn behind the glider.");
static const StaticEnumChoice  trail_length_list[] = {
  { (unsigned)TrailSettings::Length::OFF, N_("Off"), trail_length_help },
  { (unsigned)TrailSettings::Length::LONG, N_("Long"), trail_length_help },
  { (unsigned)TrailSettings::Length::SHORT, N_("Short"), trail_length_help },
  { (unsigned)TrailSettings::Length::FULL, N_("Full"), trail_length_help },
  { 0 }
};

const TCHAR *trail_type_help = N_("Sets the type of the snail trail display.");
static const StaticEnumChoice  trail_type_list[] = {
  { (unsigned)TrailSettings::Type::VARIO_1, N_("Vario #1"), trail_type_help },
  { (unsigned)TrailSettings::Type::VARIO_1_DOTS, N_("Vario #1 (with dots)"), trail_type_help },
  { (unsigned)TrailSettings::Type::VARIO_2, N_("Vario #2"), trail_type_help },
  { (unsigned)TrailSettings::Type::VARIO_2_DOTS, N_("Vario #2 (with dots)"), trail_type_help },
  { (unsigned)TrailSettings::Type::ALTITUDE, N_("Altitude"), trail_type_help },
  { 0 }
};

static const StaticEnumChoice  wind_arrow_list[] = {
  { (unsigned)WindArrowStyle::NO_ARROW, N_("Off"), N_("No wind arrow is drawn.") },
  { (unsigned)WindArrowStyle::ARROW_HEAD, N_("Arrow head"), N_("Draws an arrow head only.") },
  { (unsigned)WindArrowStyle::FULL_ARROW, N_("Full arrow"), N_("Draws an arrow head with a dashed arrow line.") },
  { 0 }
};

void
SymbolsConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const MapSettings &settings_map = CommonInterface::GetMapSettings();

  AddEnum(_("Ground track"),
          _("Display the ground track as a grey line on the map."),
          ground_track_mode_list, (unsigned)settings_map.display_ground_track);

  AddBoolean(_("FLARM traffic"), _("This enables the display of FLARM traffic on the map window."),
             settings_map.show_flarm_on_map);

  AddEnum(_("Trail length"), NULL, trail_length_list,
          (unsigned)settings_map.trail.length, this);
  SetExpertRow(TRAIL_LENGTH);

  AddBoolean(_("Trail drift"),
             _("Determines whether the snail trail is drifted with the wind when displayed in "
                 "circling mode."),
             settings_map.trail.wind_drift_enabled);
  SetExpertRow(TRAIL_DRIFT);

  AddEnum(_("Trail type"), NULL, trail_type_list, (int)settings_map.trail.type);
  SetExpertRow(TRAIL_TYPE);

  AddBoolean(_("Trail scaled"),
             _("If set to ON the snail trail width is scaled according to the vario signal."),
             settings_map.trail.scaling_enabled);
  SetExpertRow(TRAIL_WIDTH);

  AddEnum(_("Wind arrow"), _("Determines the way the wind arrow is drawn on the map."),
          wind_arrow_list, (unsigned)settings_map.wind_arrow_style);
  SetExpertRow(WIND_ARROW_STYLE);

  ShowTrailControls(settings_map.trail.length != TrailSettings::Length::OFF);
}

bool
SymbolsConfigPanel::Save(bool &_changed, bool &_require_restart)
{
  bool changed = false, require_restart = false;

  MapSettings &settings_map = CommonInterface::SetMapSettings();

  changed |= SaveValueEnum(DISPLAY_TRACK_BEARING, ProfileKeys::DisplayTrackBearing,
                           settings_map.display_ground_track);

  changed |= SaveValue(ENABLE_FLARM_MAP, ProfileKeys::EnableFLARMMap,
                       settings_map.show_flarm_on_map);

  changed |= SaveValueEnum(TRAIL_LENGTH, ProfileKeys::SnailTrail, settings_map.trail.length);

  changed |= SaveValue(TRAIL_DRIFT, ProfileKeys::TrailDrift, settings_map.trail.wind_drift_enabled);

  changed |= SaveValueEnum(TRAIL_TYPE, ProfileKeys::SnailType, settings_map.trail.type);

  changed |= SaveValue(TRAIL_WIDTH, ProfileKeys::SnailWidthScale,
                       settings_map.trail.scaling_enabled);

    changed |= SaveValueEnum(WIND_ARROW_STYLE, ProfileKeys::WindArrowStyle, settings_map.wind_arrow_style);

  _changed |= changed;
  _require_restart |= require_restart;

  return true;
}

Widget *
CreateSymbolsConfigPanel()
{
  return new SymbolsConfigPanel();
}
