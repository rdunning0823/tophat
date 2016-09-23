/* Copyright_License {

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

#include "Gradient.hpp"
#include "Util/Clamp.hpp"

fixed
AngleToGradient(const fixed d)
{
  if (d != fixed(0)) {
    return Clamp(fixed(1) / d, fixed(-999), fixed(999));
  } else {
    return fixed(999);
  }
}

bool
GradientValid(const fixed d)
{
  return fabs(d) < fixed(999);
}

fixed
CalculateGradient(const Waypoint &destination, fixed distance,
                  const MoreData &basic, fixed safety_height) {

  if (!positive(distance) || !basic.NavAltitudeAvailable()) {
    return fixed(1000);
  }

  fixed target_altitude = destination.elevation + safety_height;
  fixed height = basic.nav_altitude - target_altitude;
  return ::AngleToGradient(height / distance);
}

gcc_const
fixed
CalculateGradient(const Waypoint &destination,
                  const MoreData &basic, fixed safety_height) {
  return ::CalculateGradient(destination, basic.location.Distance(destination.location),
                             basic, safety_height);
}
