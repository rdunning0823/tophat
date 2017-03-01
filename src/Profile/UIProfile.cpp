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

#include "UIProfile.hpp"
#include "ProfileKeys.hpp"
#include "Map.hpp"
#include "MapProfile.hpp"
#include "InfoBoxConfig.hpp"
#include "PageProfile.hpp"
#include "UnitsConfig.hpp"
#include "UISettings.hpp"

namespace Profile {
  static void Load(const ProfileMap &map, DisplaySettings &settings);
  static void Load(const ProfileMap &map, FormatSettings &settings);
  static void Load(const ProfileMap &map, VarioSettings &settings);
  static void Load(const ProfileMap &map, TrafficSettings &settings);
  static void Load(const ProfileMap &map, DialogSettings &settings);
  static void Load(const ProfileMap &map, SoundSettings &settings);
  static void Load(const ProfileMap &map, VarioSoundSettings &settings);
};

void
Profile::Load(const ProfileMap &map, DisplaySettings &settings)
{
  map.Get(ProfileKeys::AutoBlank, settings.enable_auto_blank);
  map.GetEnum(ProfileKeys::MapOrientation, settings.orientation);
}

void
Profile::Load(const ProfileMap &map, FormatSettings &settings)
{
  map.GetEnum(ProfileKeys::LatLonUnits, settings.coordinate_format);
  LoadUnits(map, settings.units);
}

void
Profile::Load(const ProfileMap &map, VarioSettings &settings)
{
  map.Get(ProfileKeys::AppGaugeVarioSpeedToFly, settings.show_speed_to_fly);
  map.Get(ProfileKeys::AppGaugeVarioAvgText, settings.show_average);
  map.Get(ProfileKeys::AppGaugeVarioMc, settings.show_mc);
  map.Get(ProfileKeys::AppGaugeVarioBugs, settings.show_bugs);
  map.Get(ProfileKeys::AppGaugeVarioBallast, settings.show_ballast);
  map.Get(ProfileKeys::AppGaugeVarioGross, settings.show_gross);
  map.Get(ProfileKeys::AppAveNeedle, settings.show_average_needle);
}

void
Profile::Load(const ProfileMap &map, TrafficSettings &settings)
{
  map.Get(ProfileKeys::EnableFLARMGauge, settings.enable_gauge);
  map.Get(ProfileKeys::AutoCloseFlarmDialog, settings.auto_close_dialog);
  map.Get(ProfileKeys::FlarmAutoZoom, settings.auto_zoom);
  map.Get(ProfileKeys::FlarmNorthUp, settings.north_up);
  map.GetEnum(ProfileKeys::FlarmLocation, settings.gauge_location);
}

void
Profile::Load(const ProfileMap &map, DialogSettings &settings)
{
  map.GetEnum(ProfileKeys::AppTextInputStyle, settings.text_input_style);
  //  use default tab style
  //map.GetEnum(ProfileKeys::AppDialogTabStyle, settings.tab_style);
  map.Get(ProfileKeys::UserLevel, settings.expert);
}

void
Profile::Load(const ProfileMap &map, VarioSoundSettings &settings)
{
  map.Get(ProfileKeys::SoundAudioVario, settings.enabled);
  map.Get(ProfileKeys::SoundVolume, settings.volume);
  map.Get(ProfileKeys::VarioDeadBandEnabled, settings.dead_band_enabled);

  map.Get(ProfileKeys::VarioMinFrequency, settings.min_frequency);
  map.Get(ProfileKeys::VarioZeroFrequency, settings.zero_frequency);
  map.Get(ProfileKeys::VarioMaxFrequency, settings.max_frequency);

  map.Get(ProfileKeys::VarioMinPeriod, settings.min_period_ms);
  map.Get(ProfileKeys::VarioMaxPeriod, settings.max_period_ms);

  map.Get(ProfileKeys::VarioDeadBandMin, settings.min_dead);
  map.Get(ProfileKeys::VarioDeadBandMax, settings.max_dead);
}

void
Profile::Load(const ProfileMap &map, SoundSettings &settings)
{
  map.Get(ProfileKeys::SoundTask, settings.sound_task_enabled);
  map.Get(ProfileKeys::SoundModes, settings.sound_modes_enabled);
  map.Get(ProfileKeys::SoundDeadband, settings.sound_deadband);
  map.Get(ProfileKeys::SystemSoundVolume, settings.volume);

  Load(map, settings.vario);
}

static void CheckFontScale(unsigned &scale)
{
  if (scale < 40 || scale > 150)
    scale = 100;
}

void
Profile::Load(const ProfileMap &map, UISettings &settings)
{
  Load(map, settings.display);

  map.Get(ProfileKeys::ShowWaypointListWarning,
      settings.show_waypoints_list_warning);
  map.Get(ProfileKeys::StartupTipId, settings.last_startup_tip);

  map.GetEnum(ProfileKeys::WaypointSortDirection, settings.waypoint_sort_direction);
  // hard code to default
  //map.Get(ProfileKeys::MenuTimeout, settings.menu_timeout);

#ifndef GNAV
  // use default.  Fonts are controlled individually.
  //map.Get(ProfileKeys::UIScale, settings.scale);
  //CheckFontScale(settings.scale);

  map.Get(ProfileKeys::FontNavBarWaypointName, settings.font_scale_nav_bar_waypoint_name);
  map.Get(ProfileKeys::FontNavBarDistance, settings.font_scale_nav_bar_distance);
  map.Get(ProfileKeys::FontMapWaypointName, settings.font_scale_map_waypoint_name);
  map.Get(ProfileKeys::FontMapPlaceName, settings.font_scale_map_place_name);
  map.Get(ProfileKeys::FontInfoBoxTitle, settings.font_scale_infobox_title);
  map.Get(ProfileKeys::FontInfoBoxComment, settings.font_scale_infobox_comment);
  map.Get(ProfileKeys::FontInfoBoxValue, settings.font_scale_infobox_value);
  map.Get(ProfileKeys::FontOverlayButton, settings.font_scale_overlay_button);
  map.Get(ProfileKeys::FontDialog, settings.font_scale_dialog);
  CheckFontScale(settings.font_scale_nav_bar_waypoint_name);
  CheckFontScale(settings.font_scale_nav_bar_distance);
  CheckFontScale(settings.font_scale_map_waypoint_name);
  CheckFontScale(settings.font_scale_map_place_name);
  CheckFontScale(settings.font_scale_infobox_title);
  CheckFontScale(settings.font_scale_infobox_comment);
  CheckFontScale(settings.font_scale_infobox_value);
  CheckFontScale(settings.font_scale_overlay_button);
  CheckFontScale(settings.font_scale_dialog);
#endif

  map.Get(ProfileKeys::EnableTAGauge, settings.enable_thermal_assistant_gauge);

  map.Get(ProfileKeys::AirspaceWarningDialog, settings.enable_airspace_warning_dialog);

  //  hard code to default value
  map.GetEnum(ProfileKeys::AppStatusMessageAlignment, settings.popup_message_position);

  map.GetEnum(ProfileKeys::KoboMiniSunblind, settings.kobo_mini_sunblind);

  map.GetEnum(ProfileKeys::HapticFeedback, settings.haptic_feedback);

#if !defined(ENABLE_OPENGL) & !defined(KOBO)
  settings.screens_button_location = UISettings::ScreensButtonLocation::MENU;
#else
  map.GetEnum(ProfileKeys::ScreensButtonLocation, settings.screens_button_location);
#endif

  Load(map, settings.format);
  Load(map, settings.map);
  Load(map, settings.info_boxes);
  Load(map, settings.vario);
  Load(map, settings.traffic);
  Load(map, settings.pages);
  Load(map, settings.dialog);
  Load(map, settings.sound);

  map.Get(ProfileKeys::NavBarDisplayGR, settings.navbar_enable_gr);
  map.Get(ProfileKeys::NavBarDisplayTpIndex, settings.navbar_enable_tp_index);
  map.Get(ProfileKeys::NavBarNavigateToAATTarget, settings.navbar_navigate_to_aat_target);
}
