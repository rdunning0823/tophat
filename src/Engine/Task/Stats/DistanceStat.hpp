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
#ifndef DISTANCE_STAT_HPP
#define DISTANCE_STAT_HPP

#include "Math/fixed.hpp"

#ifdef DO_PRINT
#include <iostream>
#endif

#include "Math/Filter.hpp"
#include "Math/AvFilter.hpp"
#include "Math/DiffFilter.hpp"

#include <assert.h>

/**
 * Simple distance statistics with derived values (speed, incremental speed)
 * Incremental speeds track the short-term variation of distance with time,
 * whereas the overall speed is defined by the distance divided by a time value.
 */
class DistanceStat
{
  friend class DistanceStatComputer;

protected:
  /** Distance (m) of metric */
  fixed distance;
  /** Speed (m/s) of metric */
  fixed speed;
  /** Incremental speed (m/s) of metric */
  fixed speed_incremental;

public:
  void Reset() {
    distance = fixed_minus_one;
    speed = fixed_zero;
    speed_incremental = fixed_zero;
  }

  bool IsDefined() const {
    return !negative(distance);
  }

  /**
   * Setter for distance value
   *
   * @param d Distance value (m)
   */
  void SetDistance(const fixed d) {
    distance = d;
  }

  /**
   * Accessor for distance value
   *
   * @return Distance value (m)
   */
  fixed GetDistance() const {
    assert(IsDefined());

    return distance;
  }

  /**
   * Accessor for speed
   *
   * @return Speed (m/s)
   */
  fixed GetSpeed() const {
    assert(IsDefined());

    return speed;
  }

  /**
   * Accessor for incremental speed (rate of change of
   * distance over dt, low-pass filtered)
   *
   * @return Speed incremental (m/s)
   */
  fixed GetSpeedIncremental() const {
    assert(IsDefined());

    return speed_incremental;
  }

#ifdef DO_PRINT
  friend std::ostream& operator<< (std::ostream& o, 
                                   const DistanceStat& ds);
#endif
};

/**
 * Computer class for DistanceStat.  It holds the incremental and
 * internal values, while DistanceStat has only the results.
 */
class DistanceStatComputer {
private:
  static const unsigned N_AV = 3;

  AvFilter<N_AV> av_dist;
  DiffFilter df;
  Filter v_lpf;
  bool is_positive; // ideally const but then non-copyable

public:
  /** Constructor; initialises all to zero */
  DistanceStatComputer(const bool is_positive=true);

  /**
   * Calculate bulk speed (distance/time), abstract base method
   *
   * @param es ElementStat (used for time access)
   */
  void CalcSpeed(DistanceStat &data, fixed time);

  /**
   * Calculate incremental speed from previous step.
   * Resets incremental speed to speed if dt=0
   *
   * @param dt Time step (s)
   */
  void CalcIncrementalSpeed(DistanceStat &data, const fixed dt);

private:
  void ResetIncrementalSpeed(DistanceStat &data);
};

#endif
