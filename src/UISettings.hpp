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

#ifndef XCSOAR_UI_SETTINGS_HPP
#define XCSOAR_UI_SETTINGS_HPP

#include "FormatSettings.hpp"
#include "MapSettings.hpp"
#include "InfoBoxes/InfoBoxSettings.hpp"
#include "Gauge/VarioSettings.hpp"
#include "Gauge/TrafficSettings.hpp"
#include "PageSettings.hpp"
#include "Dialogs/DialogSettings.hpp"
#include "DisplaySettings.hpp"
#include "Audio/Settings.hpp"

#include <type_traits>

#include <stdint.h>

/**
 * User interface settings.
 */
struct UISettings {
  DisplaySettings display;

  /** timeout in quarter seconds of menu button */
  unsigned menu_timeout;

#ifndef GNAV
  unsigned scale;
  unsigned font_scale_nav_bar_waypoint_name;
  unsigned font_scale_nav_bar_distance;
  unsigned font_scale_map_waypoint_name;
  unsigned font_scale_map_place_name;
  unsigned font_scale_infobox_title;
  unsigned font_scale_infobox_comment;
  unsigned font_scale_infobox_value;
  unsigned font_scale_overlay_button;
  unsigned font_scale_dialog;
#endif

  /** last startup tip display. 1 is first tip, 0 means never shown */
  unsigned last_startup_tip;

  /** if too many waypoints are in the list,
   * warn user when list is diaplsyed
   */
  bool show_waypoints_list_warning;

  enum WaypointSortDirection {
    NAME,
    DISTANCE,
    BEARING,
    ELEVATION,
    ARRIVAL_ALTITUDE,
  };

  /** Direction of sort in waypoint list */
  WaypointSortDirection waypoint_sort_direction;

  /** show the Screens map overlay button instead of the menu with Screens */
  enum class ScreensButtonLocation : uint8_t {
    MAP,
    MENU,
  } screens_button_location;

  /// is the replay dialog visible
  bool replay_dialog_visible;
  /**
   * if true, gesture zone help timer will be reset to draw next time
   */
  bool restart_gesture_help;

  /**
   * if true, gesture zone help timer will be cleared to not draw next time
   */
  bool clear_gesture_help;

  /** Show ThermalAssistant if circling */
  bool enable_thermal_assistant_gauge;

  /** Enable warning dialog */
  bool enable_airspace_warning_dialog;

  enum class PopupMessagePosition : uint8_t {
    CENTER,
    TOP_LEFT,
  } popup_message_position;

  /** is a kobo mini sunblind installed */
  bool kobo_mini_sunblind;

  /** Haptic feedback settings */
  enum class HapticFeedback : uint8_t {
    DEFAULT,
    OFF,
    ON,
  } haptic_feedback;

  FormatSettings format;
  MapSettings map;
  InfoBoxSettings info_boxes;
  VarioSettings vario;
  TrafficSettings traffic;
  PageSettings pages;
  DialogSettings dialog;
  SoundSettings sound;

  /** TopHat NavBar display settings */
  bool navbar_enable_gr;
  bool navbar_enable_tp_index;
  /** if true, navigates to target instead of center in AAT mode */
  bool navbar_navigate_to_aat_target;

  void SetDefaults();

  unsigned GetPercentScale() const {
#ifndef GNAV
    return scale;
#endif

    return 100;
  }
};

static_assert(std::is_trivial<UISettings>::value, "type is not trivial");

#endif
