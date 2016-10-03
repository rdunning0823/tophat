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

#ifndef XCSOAR_WIND_SETTINGS_HPP
#define XCSOAR_WIND_SETTINGS_HPP

#include "Geo/SpeedVector.hpp"
#include "NMEA/Validity.hpp"

#include <type_traits>

// control of calculations, these only changed by user interface
// but are used read-only by calculations

enum UserWindSource {
  MANUAL_WIND = 0,
  INTERNAL_WIND,
  EXTERNAL_WIND_IF_AVAILABLE,
};

/**
 * Wind calculator settings
 */
struct WindSettings {

  /* what is the source of the wind being used for calculations right now */
  UserWindSource user_wind_source;

  /**
   * This is the manual wind set by the pilot. Validity is set when
   * changeing manual wind but does not expire.
   */
  SpeedVector manual_wind;
  Validity manual_wind_available;

  void SetDefaults();

  /**
   * are we using either external wind or EKF circling algorithm
   */
  bool IsAutoWindEnabled() const {
    return user_wind_source != UserWindSource::MANUAL_WIND;
  }

  bool UseExternalWindIfEnabled() const {
    return user_wind_source == UserWindSource::EXTERNAL_WIND_IF_AVAILABLE;
  }

  UserWindSource GetUserWindSource() const {
    return user_wind_source;
  }

  void SetUserWindSource(UserWindSource source) {
    user_wind_source = source;
  }
};

static_assert(std::is_trivial<WindSettings>::value, "type is not trivial");

#endif
