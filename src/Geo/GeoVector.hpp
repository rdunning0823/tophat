/* Copyright_License {

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

#ifndef GEO_VECTOR_HPP
#define GEO_VECTOR_HPP

#include "Math/Angle.hpp"
#include "Util/TypeTraits.hpp"
#include "Compiler.h"

struct GeoPoint;

/**
 * A constant bearing vector in lat/lon coordinates.  
 * Should later be extended to handle
 * separately constant bearing and minimum-distance paths. 
 *
 */
struct GeoVector {
  /** Distance in meters */
  fixed distance;

  /** Bearing (true north) */
  Angle bearing;

  /** Empty non-initializing constructor */
  GeoVector() = default;

  /** Constructor given supplied distance/bearing */
  constexpr
  GeoVector(const fixed _distance, const Angle &_bearing)
    :distance(_distance), bearing(_bearing) {}

  /**
   * Dummy constructor given distance, 
   * used to allow GeoVector x=0 calls. 
   */
  GeoVector(const fixed _distance)
    :distance(_distance), bearing(Angle::Zero()) {}

  /**
   * Constructor given start and end location.  
   * Computes Distance/Bearing internally. 
   */
  GeoVector(const GeoPoint &source, const GeoPoint &target);

  /**
   * Create an invalid instance.
   */
  gcc_const
  static GeoVector Invalid() {
    return GeoVector(fixed_minus_one);
  }

  /**
   * Returns the end point of the geovector projected from the start point.  
   * Assumes constant bearing. 
   */
  gcc_pure
  GeoPoint EndPoint(const GeoPoint &source) const;

  /**
   * Returns the end point of the geovector projected from the start point.  
   * Assumes constand Bearing. 
   *
   * @param source start of vector
   * @return location of end point
   */
  GeoPoint MidPoint(const GeoPoint &source) const;

  /**
   * Minimum distance from a point on the vector to the reference
   *
   * @param source Start of vector
   * @param ref Point to test
   *
   * @return Distance (m)
   */
  gcc_pure
  fixed MinimumDistance(const GeoPoint &source, const GeoPoint &ref) const;

  constexpr
  inline bool IsValid() const {
    return !negative(distance);
  }

  void SetInvalid() {
    distance = fixed_minus_one;
  }
};

static_assert(is_trivial<GeoVector>::value, "type is not trivial");

#endif
