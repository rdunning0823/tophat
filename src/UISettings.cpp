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

#include "UISettings.hpp"

void
UISettings::SetDefaults()
{
  display.SetDefaults();

  menu_timeout = 8 * 4;

  show_waypoints_list_warning = true;

  waypoint_sort_direction = UISettings::WaypointSortDirection::NAME;

#ifndef GNAV
  scale = 100;
  font_scale_nav_bar_waypoint_name = 100;
  font_scale_nav_bar_distance = 100;
  font_scale_map_waypoint_name = 100;
  font_scale_map_place_name = 100;
  font_scale_infobox_title = 100;
  font_scale_infobox_comment = 100;
  font_scale_infobox_value = 100;
  font_scale_overlay_button = 100;
  font_scale_dialog = 100;
#endif

  enable_thermal_assistant_gauge = false;

  restart_gesture_help = true;
  clear_gesture_help = false;

#if !defined(ENABLE_OPENGL) & !defined(KOBO)
  // WIN32 does not support screens button on map
  screens_button_location = ScreensButtonLocation::MENU;
#else
  screens_button_location = ScreensButtonLocation::MAP;
#endif
  replay_dialog_visible = false;

  enable_airspace_warning_dialog = true;

  popup_message_position = PopupMessagePosition::CENTER;

  kobo_mini_sunblind = false;

  haptic_feedback = HapticFeedback::DEFAULT;

  format.SetDefaults();
  map.SetDefaults();
  info_boxes.SetDefaults();
  vario.SetDefaults();
  traffic.SetDefaults();
  pages.SetDefaults();
  dialog.SetDefaults();
  sound.SetDefaults();

  navbar_enable_gr = false;
  navbar_enable_tp_index = true;
  navbar_navigate_to_aat_target = false;
}
