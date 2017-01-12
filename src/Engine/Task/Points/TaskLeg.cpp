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

#include "TaskLeg.hpp"
#include "Task/Ordered/Points/OrderedTaskPoint.hpp"

#include <assert.h>
#include <algorithm>

inline const OrderedTaskPoint *
TaskLeg::GetOrigin() const
{
  return destination.GetPrevious();
}

inline const OrderedTaskPoint *
TaskLeg::GetNext() const
{
  return destination.GetNext();
}

inline OrderedTaskPoint *
TaskLeg::GetNext()
{
  return destination.GetNext();
}

inline GeoVector
TaskLeg::GetPlannedVector() const
{
  if (!GetOrigin()) {
    return GeoVector::Zero();
  } else {
    return memo_planned.calc(GetOrigin()->GetLocationRemaining(),
                             destination.GetLocationRemaining());
  }
}

inline GeoVector
TaskLeg::GetRemainingVector(const GeoPoint &ref) const
{
  switch (destination.GetActiveState()) {
  case OrderedTaskPoint::AFTER_ACTIVE:
    // this leg totally included
    return GetPlannedVector();

  case OrderedTaskPoint::CURRENT_ACTIVE: {
    // this leg partially included

    if (!ref.IsValid())
      /* if we don't have a GPS fix yet, we fall back to the "planned"
         vector unless this task leg has already been achieved */
      return destination.HasEntered()
        ? GeoVector::Zero()
        : GetPlannedVector();

    return memo_remaining.calc(ref, destination.GetLocationRemaining());
  }

  case OrderedTaskPoint::BEFORE_ACTIVE:
    // this leg not included
    return GeoVector::Zero();
  }

  gcc_unreachable();
  assert(false);
  return GeoVector::Invalid();
}

inline GeoVector
TaskLeg::GetTravelledVector(const GeoPoint &ref) const
{
  switch (destination.GetActiveState()) {
  case OrderedTaskPoint::BEFORE_ACTIVE:
    if (!GetOrigin())
      return GeoVector::Zero();

    // this leg totally included
    return memo_travelled.calc(GetOrigin()->GetLocationTravelled(),
                               destination.GetLocationTravelled());

  case OrderedTaskPoint::CURRENT_ACTIVE:
    // this leg partially included
    if (!GetOrigin())
      return GeoVector(fixed(0), 
                       ref.IsValid()
                       ? ref.Bearing(destination.GetLocationRemaining())
                       : Angle::Zero());

    if (destination.HasEntered())
      return memo_travelled.calc(GetOrigin()->GetLocationTravelled(),
                                 destination.GetLocationTravelled());
    else if (!ref.IsValid())
      return GeoVector::Zero();
    else {
      // not in cylinder (or has entered and left)
      // this provides vector with bearing to center (which is NOT bearing to plane)
      // and Dist from last travelled to center minus dist from glider to center.
      fixed dist = GetOrigin()->GetLocationTravelled().Distance(destination.GetLocation())
          - ref.Distance(destination.GetLocation());

      return GeoVector(
          std::max(fixed(0), dist),
          GetOrigin()->GetLocationTravelled().Bearing(destination.GetLocation()));
    }
  case OrderedTaskPoint::AFTER_ACTIVE:
    if (!GetOrigin())
      return GeoVector::Zero();

    // this leg may be partially included
    if (GetOrigin()->HasEntered())
      return memo_travelled.calc(GetOrigin()->GetLocationTravelled(),
                                 ref.IsValid()
                                 ? ref
                                 : destination.GetLocationTravelled());
    return GeoVector::Zero();
  }

  gcc_unreachable();
  assert(false);
  return GeoVector::Invalid();
}

GeoVector
TaskLeg::GetScoredLegVectorLandout(const GeoPoint &ref) const
{
  assert(!destination.HasEntered());
  assert(ref.IsValid());

  fixed dist = GetScoredLegDistanceLandout(ref);

  return GeoVector(
      std::max(fixed(0), dist),
      GetOrigin()->GetLocationScored().Bearing(ref));
}

inline GeoVector
TaskLeg::GetScoredVector(const GeoPoint &ref) const
{
  switch (destination.GetActiveState()) {
  case OrderedTaskPoint::BEFORE_ACTIVE:
    if (!GetOrigin()) {
      return GeoVector::Zero();
    }

    // this leg totally included
    return memo_travelled.calc(GetOrigin()->GetLocationScored(),
                               destination.GetLocationScored(),
                               GetOrigin()->ScoreAdjustment() +
                                   destination.ScoreAdjustment());

  case OrderedTaskPoint::CURRENT_ACTIVE:
    // this leg partially included
    if (!GetOrigin()) {
      return GeoVector(fixed(0),
                       ref.IsValid()
                       ? ref.Bearing(destination.GetLocationRemaining())
                       : Angle::Zero());
    }

    if (destination.HasEntered()) {
      return memo_travelled.calc(GetOrigin()->GetLocationScored(),
                                 destination.GetLocationScored(),
                                 GetOrigin()->ScoreAdjustment() +
                                     destination.ScoreAdjustment());
    }
    else if (!ref.IsValid())
      return GeoVector::Zero();
    else {
      // not in cylinder (or has entered and left)
      // this provides vector with bearing to center (which is NOT bearing to plane)
      // and Dist from last travelled to center minus dist from glider to center.
      return GetScoredLegVectorLandout(ref);
    }
  case OrderedTaskPoint::AFTER_ACTIVE:
    if (!GetOrigin()) {
      return GeoVector::Zero();
    }

    // this leg may be partially included
    if (GetOrigin()->HasEntered()) {
      if (ref.IsValid()) {
        return memo_travelled.calc(GetOrigin()->GetLocationScored(),
                                   ref,
                                   GetOrigin()->ScoreAdjustment() +
                                       destination.ScoreAdjustment());
      } else {
        return memo_travelled.calc(GetOrigin()->GetLocationScored(),
                                   destination.GetLocationScored(),
                                   GetOrigin()->ScoreAdjustment() +
                                       destination.ScoreAdjustment());
      }
    }
    if (GetOrigin()->HasEntered()) {
      if (destination.HasEntered()) {
        return memo_travelled.calc(GetOrigin()->GetLocationScored(),
                                   destination.GetLocationScored(),
                                   GetOrigin()->ScoreAdjustment() +
                                       destination.ScoreAdjustment());
      } else if (ref.IsValid()) {
        return GetScoredLegVectorLandout(ref);
      }
    }
    return GeoVector::Zero();
  }
  gcc_unreachable();
  assert(false);
  return GeoVector::Invalid();
}

fixed
TaskLeg::GetScoredLegDistanceLandout(const GeoPoint &ref) const
{
  assert(!destination.HasEntered());
  assert(ref.IsValid());

  fixed distance_new = landout_distance.GetDistance(ref);

  return std::max(fixed(0),
                  distance_new - GetOrigin()->ScoreAdjustment());
}

inline fixed
TaskLeg::GetScoredDistance(const GeoPoint &ref) const
{
  if (!GetOrigin())
    return fixed(0);


  switch (destination.GetActiveState()) {
  case OrderedTaskPoint::BEFORE_ACTIVE:
    // this leg totally included
    if (destination.HasEntered()) {
      fixed leg_dist = std::max(fixed(0),
                                memo_scored.Distance(GetOrigin()->GetLocationScored(),
                                    destination.GetLocationScored())
                                    - GetOrigin()->ScoreAdjustment()
                                    - destination.ScoreAdjustment());
      return leg_dist;
    } else {
      // if we missed an OZ in the past, or just advance the task bar fwd
      // then
      if (ref.IsValid()) {
        fixed leg_dist = GetScoredLegDistanceLandout(ref);
        return leg_dist;
      } else {
        return fixed(0);
      }

    }

  case OrderedTaskPoint::CURRENT_ACTIVE:
    // this leg partially included
    if (destination.HasEntered()) {
      fixed leg_dist = std::max(fixed(0),
                                memo_scored.Distance(GetOrigin()->GetLocationScored(),
                                                     destination.GetLocationScored())
                                - GetOrigin()->ScoreAdjustment() - destination.ScoreAdjustment());
      return leg_dist;
    } else if (ref.IsValid()) {

      // not in cylinder (or has entered and left)
      // this provides vector with bearing to center (which is NOT bearing to plane)
      // and Dist from last travelled to center minus dist from glider to center.
      fixed leg_dist = GetScoredLegDistanceLandout(ref);
      return leg_dist;
    } else {
      return fixed(0);
    }

  case OrderedTaskPoint::AFTER_ACTIVE:

    // Include this leg, assume pilot has neglected to advance task;

    if (GetOrigin()->HasEntered()) {
      if (destination.HasEntered()) {
        fixed leg_dist = std::max(fixed(0),
                        memo_scored.Distance(GetOrigin()->GetLocationScored(),
                                             destination.GetLocationScored())
                        - GetOrigin()->ScoreAdjustment() - destination.ScoreAdjustment());
        return leg_dist;

      } else if (ref.IsValid()) {
        fixed leg_dist = GetScoredLegDistanceLandout(ref);
        return leg_dist;
      }
    }

    return fixed(0);
  }

  gcc_unreachable();
  assert(false);
  return fixed(0);
}

GeoVector
TaskLeg::GetNominalLegVector() const
{
  if (!GetOrigin()) {
    return GeoVector::Zero();
  } else {
    return memo_nominal.calc(GetOrigin()->GetLocation(),
                             destination.GetLocation());
  }
}

inline fixed
TaskLeg::GetMaximumLegDistance() const
{
  if (GetOrigin())
    return memo_max.Distance(GetOrigin()->GetLocationMax(),
                             destination.GetLocationMax());
  return fixed(0);
}

inline fixed
TaskLeg::GetMinimumLegDistance() const
{
  if (GetOrigin())
    return memo_min.Distance(GetOrigin()->GetLocationMin(),
                             destination.GetLocationMin());
  return fixed(0);
}

fixed 
TaskLeg::ScanDistanceTravelled(const GeoPoint &ref)
{
  vector_travelled = GetScoredVector(ref);

  return vector_travelled.distance +
    (GetNext() ? GetNext()->ScanDistanceTravelled(ref) : fixed(0));
}

fixed 
TaskLeg::ScanDistanceRemaining(const GeoPoint &ref)
{
  vector_remaining = GetRemainingVector(ref);
  return vector_remaining.distance +
    (GetNext() ? GetNext()->ScanDistanceRemaining(ref) : fixed(0));
}

fixed 
TaskLeg::ScanDistancePlanned()
{
  vector_planned = GetPlannedVector();
  return vector_planned.distance +
    (GetNext() ? GetNext()->ScanDistancePlanned() : fixed(0));
}

fixed 
TaskLeg::ScanDistanceMax() const
{
  return GetMaximumLegDistance() +
    (GetNext() ? GetNext()->ScanDistanceMax() : fixed(0));
}

fixed 
TaskLeg::ScanDistanceMin() const
{
  return GetMinimumLegDistance() +
    (GetNext() ? GetNext()->ScanDistanceMin() : fixed(0));
}

fixed 
TaskLeg::ScanDistanceNominal() const
{
  return GetNominalLegDistance() +
    (GetNext() ? GetNext()->ScanDistanceNominal() : fixed(0));
}

fixed 
TaskLeg::ScanDistanceScored(const GeoPoint &ref) const
{
  return GetScoredDistance(ref) +
    (GetNext() ? GetNext()->ScanDistanceScored(ref) : fixed(0));
}

bool
TaskLeg::ScanLandoutDistanceGeometry(bool force)
{
  return landout_distance.SetLandoutDistanceGeometry(force) &&
    (GetNext() ? GetNext()->ScanLandoutDistanceGeometry(force) : true);
}
