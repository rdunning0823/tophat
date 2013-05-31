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

/*! @file
 * @brief Library for calculating Earth dimensions
 *
 * This library provides general functions for calculating dimensions
 * on the Earth with GPS coordinates.
 */

#ifndef XCSOAR_GEO_MATH_HPP
#define XCSOAR_GEO_MATH_HPP

#include "Math/fixed.hpp"
#include "Math/Angle.hpp"
#include "Compiler.h"

struct GeoPoint;

/**
 * Finds cross track error in meters and closest point P4 between P3
 * and desired track P1-P2.  Very slow function!
 */
fixed CrossTrackError(const GeoPoint loc1, const GeoPoint loc2,
                      const GeoPoint loc3, GeoPoint *loc4);

/**
 * Calculates projected distance from P3 along line P1-P2.
 */
gcc_const
fixed ProjectedDistance(const GeoPoint loc1, const GeoPoint loc2,
                        const GeoPoint loc3);

void DistanceBearing(const GeoPoint loc1, const GeoPoint loc2,
                     fixed *distance, Angle *bearing);

/**
 * Calculates the distance between two locations
 * @param loc1 Location 1
 * @param loc2 Location 2
 * @return The distance
 */
gcc_const
fixed Distance(const GeoPoint loc1, const GeoPoint loc2);

/**
 * Calculates the bearing between two locations
 * @param loc1 Location 1
 * @param loc2 Location 2
 * @return The bearing
 */
gcc_const
Angle Bearing(const GeoPoint loc1, const GeoPoint loc2);

/**
 * Finds the point along a distance dthis (m) between p1 and p2, which are
 * separated by dtotal.
 *
 * This is a slow function.  Adapted from The Aviation Formulary 1.42.
 */
gcc_const
GeoPoint IntermediatePoint(const GeoPoint loc1, const GeoPoint loc2,
                           const fixed dthis);

/**
 * Find the nearest great-circle middle point between the two.
 */
gcc_const
GeoPoint
Middle(GeoPoint a, GeoPoint b);

/** 
 * Calculate and add distances between point 1 and 2, and point 2 and 3.
 * 
 * @param loc1 Location 1
 * @param loc2 Location 2
 * @param loc3 Location 3
 * 
 * @return Distance 12 plus 23 (m)
 */
gcc_const
fixed DoubleDistance(const GeoPoint loc1, const GeoPoint loc2,
                     const GeoPoint loc3);

/**
 * Calculates the location (loc_out) you would have, after being at
 * a certain start location (loc) with a certain Bearing and going straight
 * forward for a certain Distance.
 * @param loc Current location
 * @param Bearing Current bearing
 * @param Distance Distance to predict
 * @param loc_out Future location
 */
gcc_const
GeoPoint FindLatitudeLongitude(const GeoPoint loc,
                               const Angle bearing, const fixed distance);

#endif
