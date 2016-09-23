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

#include "GeoVectorMemento.hpp"
#include <assert.h>

GeoVector 
GeoVectorMemento::calc(const GeoPoint& _origin,
                       const GeoPoint& _destination,
                       const fixed _distance_adjustment) const
{
  if (!value.IsValid() ||
      _origin != origin ||
      _destination != destination ||
      _distance_adjustment != distance_adjustment) {
    assert(!negative(distance_adjustment));
    origin = _origin;
    destination = _destination;
    distance_adjustment = _distance_adjustment;
    if (positive(distance_adjustment)) {
      fixed t =  distance_adjustment / origin.Distance(destination);
      if (t > fixed(1))
        t = fixed(1);

      value = GeoVector(origin, origin.Interpolate(destination, fixed(1) - t));
    } else {
      value = GeoVector(origin, destination);
    }

  }
  return value;
}
