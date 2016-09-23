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

#ifndef XCSOAR_DIALOGS_DLGSETUPQUICK_HPP
#define XCSOAR_DIALOGS_DLGSETUPQUICK_HPP

#include "Device/Descriptor.hpp"
#include "Device/Port/State.hpp"
#include "NMEA/Info.hpp"

struct Flags {
  bool duplicate:1;
  bool open:1, error:1;
  bool alive:1, location:1, gps:1, baro:1, airspeed:1, vario:1, traffic:1;
  bool debug:1;

  void Set(const DeviceConfig &config, const DeviceDescriptor &device,
           const NMEAInfo &basic) {
    /* if a DeviceDescriptor is "unconfigured" but its DeviceConfig
       contains a valid configuration, then it got disabled by
       DeviceConfigOverlaps(), i.e. it's duplicate */
    duplicate = !config.IsDisabled() && !device.IsConfigured();

    switch (device.GetState()) {
    case PortState::READY:
      open = true;
      error = false;
      break;

    case PortState::FAILED:
      open = false;
      error = true;
      break;

    case PortState::LIMBO:
      open = false;
      error = false;
      break;
    }

    alive = basic.alive;
    location = basic.location_available;
    gps = basic.gps.fix_quality_available;
    baro = basic.baro_altitude_available ||
      basic.pressure_altitude_available ||
      basic.static_pressure_available;
    airspeed = basic.airspeed_available;
    vario = basic.total_energy_vario_available;
    traffic = basic.flarm.IsDetected();
    debug = device.IsDumpEnabled();
  }
};

union Item {
private:
  Flags flags;
  uint16_t i;

  static_assert(sizeof(flags) <= sizeof(i), "wrong size");

public:
  void Clear() {
    i = 0;
  }

  void Set(const DeviceConfig &config, const DeviceDescriptor &device,
           const NMEAInfo &basic) {
    i = 0;
    flags.Set(config, device, basic);
  }

  bool operator==(const Item &other) const {
    return i == other.i;
  }

  bool operator!=(const Item &other) const {
    return i != other.i;
  }

  const Flags &operator->() const {
    return flags;
  }

  const Flags &operator*() const {
    return flags;
  }
};

#endif /* DLGSETUPQUICK_HPP_ */
