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

#ifndef MAT_CYLINDERZONE_HPP
#define MAT_CYLINDERZONE_HPP

#include "ObservationZonePoint.hpp"

#include <assert.h>

/**
 * Observation zone represented as a circular area with specified
 * radius is fixed at 1 sm
 */
class MatCylinderZone : public ObservationZonePoint
{
protected:
  /** radius (m) of OZ */
  fixed radius;

protected:
  MatCylinderZone(Shape _shape, const GeoPoint &loc)
    :ObservationZonePoint(_shape, loc), radius(fixed(1609.344)) {
    assert(positive(radius));
  }

  MatCylinderZone(const MatCylinderZone &other, const GeoPoint &reference)
    :ObservationZonePoint((const ObservationZonePoint &)other, reference),
     radius(fixed(1609.344)) {
  }

public:
  /**
   * Constructor.
   *
   * @param loc Location of center
   * @param _radius Radius (m) of cylinder
   *
   * @return Initialised object
   */
  MatCylinderZone(const GeoPoint &loc)
    :ObservationZonePoint(MAT_CYLINDER, loc), radius(fixed(1609.344)) {
  }

  /**
   * Set radius property
   * Noop
   *
   * @param new_radius Radius (m) of cylinder
   */
  virtual void SetRadius(fixed new_radius) {
    //TODO Remove this??
  }

  /**
   * Get radius property value
   *
   * @return Radius (m) of cylinder
   */
  fixed GetRadius() const {
    return radius;
  }

  /* virtual methods from class ObservationZone */
  virtual bool IsInSector(const GeoPoint &location) const {
    return DistanceTo(location) <= radius;
  }

  virtual bool TransitionConstraint(const GeoPoint &location,
                                    const GeoPoint &last_location) const {
    return true;
  }

  virtual GeoPoint GetBoundaryParametric(fixed t) const;
  virtual OZBoundary GetBoundary() const;
  virtual fixed ScoreAdjustment() const;

  /* virtual methods from class ObservationZonePoint */
  virtual bool Equals(const ObservationZonePoint &other) const;
  virtual GeoPoint GetRandomPointInSector(const fixed mag) const;

  virtual ObservationZonePoint *Clone(const GeoPoint &_reference) const {
    return new MatCylinderZone(*this, _reference);
  }
};

#endif
