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

#ifndef XCSOAR_UI_SETTINGS_HPP
#define XCSOAR_UI_SETTINGS_HPP

#include "Units/Settings.hpp"
#include "MapSettings.hpp"
#include "InfoBoxes/InfoBoxSettings.hpp"
#include "Gauge/VarioSettings.hpp"
#include "Gauge/TrafficSettings.hpp"
#include "PageSettings.hpp"
#include "Dialogs/DialogSettings.hpp"
#include "Util/TypeTraits.hpp"
#include "Geo/CoordinateFormat.hpp"
#include "DisplaySettings.hpp"
#include "Audio/Settings.hpp"

/**
 * User interface settings.
 */
struct UISettings {
  DisplaySettings display;

  /** timeout in quarter seconds of menu button */
  unsigned menu_timeout;

  bool custom_fonts;

  /** last startup tip display. 1 is first tip, 0 means never shown */
  unsigned last_startup_tip;

  /** Show ThermalAssistant if circling */
  bool enable_thermal_assistant_gauge;

  enum StateMessageAlign_t {
    smAlignCenter = 0,
    smAlignTopLeft,
  } popup_message_position;

  /** Haptic feedback settings */
  enum HapticFeedback {
    Default,
    Off,
    On,
  } haptic_feedback;

  CoordinateFormat coordinate_format;

  UnitSetting units;
  MapSettings map;
  InfoBoxSettings info_boxes;
  VarioSettings vario;
  TrafficSettings traffic;
  PageSettings pages;
  DialogSettings dialog;
  SoundSettings sound;

  void SetDefaults();
};

static_assert(is_trivial<UISettings>::value, "type is not trivial");

#endif
