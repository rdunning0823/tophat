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

#include "FlatRay.hpp"

#include "Math/FastMath.h"

#define sgn(x) (x >= 0 ? 1 : -1)

int
FlatRay::Magnitude() const
{
  return ihypot(vector.longitude, vector.latitude);
}

/*
 * Checks whether two lines intersect or not
 * @see http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
 * adapted from line_line_intersection
 */
std::pair<int, int>
FlatRay::IntersectsRatio(const FlatRay &that) const
{
  std::pair<int, int> r;
  r.second = vector.CrossProduct(that.vector);
  if (r.second == 0)
    // lines are parallel
    return r;

  const FlatGeoPoint delta = that.point - point;
  r.first = delta.CrossProduct(that.vector);
  if ((sgn(r.first) * sgn(r.second) < 0) || (abs(r.first) > abs(r.second))) {
    // outside first line
    r.second = 0;
    return r;
  }

  const int ub = delta.CrossProduct(vector);
  if ((sgn(ub) * sgn(r.second) < 0) || (abs(ub) > abs(r.second))) {
    // outside second line
    r.second = 0;
    return r;
  }

  // inside both lines
  return r;
}

FlatGeoPoint
FlatRay::Parametric(const fixed t) const
{
  FlatGeoPoint p = point;
  p.longitude += iround(vector.longitude * t);
  p.latitude += iround(vector.latitude * t);
  return p;
}

fixed
FlatRay::Intersects(const FlatRay &that) const
{
  std::pair<int, int> r = IntersectsRatio(that);
  if (r.second == 0)
    return -fixed_one;
  return ((fixed)r.first) / r.second;
}

bool
FlatRay::IntersectsDistinct(const FlatRay& that) const
{
  std::pair<int, int> r = IntersectsRatio(that);
  return (r.second != 0) &&
         (sgn(r.second) * r.first > 0) &&
         (abs(r.first) < abs(r.second));
}

fixed
FlatRay::DistinctIntersection(const FlatRay& that) const
{
  std::pair<int, int> r = IntersectsRatio(that);
  if (r.second != 0 &&
      sgn(r.second) * r.first > 0 &&
      abs(r.first) < abs(r.second)) {
    return fixed(r.first) / r.second;
  }

  return fixed_minus_one;
}
