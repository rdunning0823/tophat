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

#ifndef XCSOAR_DEVICE_SIMULATOR_HPP
#define XCSOAR_DEVICE_SIMULATOR_HPP

#include "Math/fixed.hpp"

struct NMEAInfo;
struct DerivedInfo;
class GlidePolar;

class Simulator {
public:
  fixed last_airspeed;
  fixed last_altitude;
  /**
   * if true, the sim process will skip the next glide speed calculation
   */
  bool skip_next_glide_speed_calc;

  void Init(NMEAInfo &basic);

  /**
   * Update the clock and a few important Validity attributes, as if
   * there had been a new GPS fix, without actually modifying the
   * values.  This is useful to force a calculation update after a
   * simulation parameter has been changed (e.g. altitude).
   */
  void Touch(NMEAInfo &basic);

  void Process(NMEAInfo &basic, const DerivedInfo &calculated);

  /**
   * updates altitude based on glide slope at TAS speed.
   */
  void UpdateGlideAltitude(NMEAInfo &basic, const GlidePolar &polar);

  /**
   * updates groundspeed of glide to keep IAS constant given decent rate of
   * glide
   */
  void UpdateGlideSpeed(NMEAInfo &basic, const DerivedInfo &calculated);


  /**
   * sets the groundspeed from TAS by subtracting
   * the wind
   * @return groundspeed
   */
  fixed CalcSpeedFromTAS(const NMEAInfo &basic, const DerivedInfo &calculated,
                         fixed true_air_speed);

  /**
   * Calculates groundspeed from ias / tas and altitude and wind
   * @return groundspeed
   */
  fixed CalcSpeedFromIAS(const NMEAInfo &basic,
                         const DerivedInfo &calculated, fixed ias);


};

#endif
