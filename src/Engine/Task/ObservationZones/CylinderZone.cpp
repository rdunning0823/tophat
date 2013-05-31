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

#include "CylinderZone.hpp"
#include "Boundary.hpp"
#include "Geo/GeoVector.hpp"

fixed
CylinderZone::ScoreAdjustment() const
{
  return radius;
}

GeoPoint
CylinderZone::GetBoundaryParametric(fixed t) const
{
  return GeoVector(radius, Angle::Radians(t * fixed_two_pi)).
           EndPoint(GetReference());
}

OZBoundary
CylinderZone::GetBoundary() const
{
  OZBoundary boundary;

  const unsigned steps = 20;
  const Angle delta = Angle::FullCircle() / steps;

  GeoVector vector(GetRadius(), Angle::Zero());
  for (unsigned i = 0; i < steps; ++i, vector.bearing += delta)
    boundary.push_front(vector.EndPoint(GetReference()));

  return boundary;
}

bool
CylinderZone::Equals(const ObservationZonePoint &other) const
{
  const CylinderZone &z = (const CylinderZone &)other;

  return ObservationZonePoint::Equals(other) && GetRadius() == z.GetRadius();
}

GeoPoint
CylinderZone::GetRandomPointInSector(const fixed mag) const
{
  GeoPoint location;

  do {
    Angle dir = Angle::Degrees(fixed(rand() % 360));
    fixed dmag = max(min(radius, fixed(100.0)), radius * mag);
    fixed dis = fixed((0.1 + (rand() % 90) / 100.0)) * dmag;
    GeoVector vec(dis, dir);
    location = vec.EndPoint(GetReference());
  } while (!IsInSector(location));

  return location;
}
