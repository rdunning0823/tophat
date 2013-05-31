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

/* this algorithm is obsolete, it has been replaced by WindEKF */

#ifndef WINDZIGZAG_HPP
#define WINDZIGZAG_HPP

#include "Geo/SpeedVector.hpp"
#include "Math/Angle.hpp"
#include "Math/fixed.hpp"

#include <list>

struct NMEAInfo;
struct DerivedInfo;

/**
 * This class provides a mechanism to estimate the wind speed and bearing
 * for an aircraft assumed to be flying level such that the airspeed vector
 * is in the horizontal plane, and has access to true airspeed and GPS speed
 * measurements.  No compass information is required by the algorithm.
 *
 * In order to resolve reasonable estimates, and in particular to
 * avoid an ambiguity regarding the hidden state variable of aircraft
 * heading, it is necessary that the aircraft performs bearing
 * adjustments during the sampling.  One way of achieving this is to
 * fly a zig-zag manoeuver; although the aircraft can perform any manoeuver in
 * which the bearing changes.
 *
 * However, due to the condition requiring the airspeed vector to be
 * predominantly in the horizontal plane, it is necessary to filter
 * out observations during high G-loading manoeuvers since for these
 * states the condition cannot be reliably assured.  Consequently,
 * this implementation uses a 'blackout' of a few seconds, in which no
 * samples are accepted, whenever the load magnitude deviates from one
 * by more than 0.3 g.
 *
 * The true airspeed measurements are assumed to be affected by noise
 * of zero mean; if the noise is not zero mean, then the resulting
 * wind estimate will be biased if the samples do not cover a large
 * variation of bearings.
 *
 * The algorithm also checks linearity of the estimated TAS against the
 * measured TAS.  The Pearson correlation coefficient, r (indicating linearity),
 * is used to reject solutions if r is low; secondly, the slope of the
 * TAS to estimated TAS is determined.  This could be used to provide a linear
 * calibration coefficient for the TAS (currently unused).
 *
 * This algorithm was developed by John Wharington.
 */
class WindZigZag {
public:
  struct ZZX {
    ZZX(const fixed mag, const Angle ang) {
      const auto sc = ang.SinCos();
      gps_east = sc.first * mag;
      gps_north = sc.second * mag;
    }
    fixed gps_north;
    fixed gps_east;
  };

  struct ZZY {
    ZZY(const fixed _tas): tas(_tas) {}
    const fixed tas;
  };

  struct ZZBeta {
    ZZBeta(fixed _north, fixed _east):north(_north),east(_east) {}
    const fixed north;
    const fixed east;
  };

  struct ZZObs {
    ZZObs(const unsigned _time, const fixed _speed, const Angle _track,
          const fixed _tas)
      :time(_time), x(_speed, _track), y(_tas), track(_track) {}

    const unsigned time;
    const ZZX x;
    const ZZY y;
    const Angle track;

    fixed Phi(const ZZBeta& beta) const {
      return SmallHypot(x.gps_north + beta.north, x.gps_east + beta.east);
    }

    fixed f(const ZZBeta& beta) const {
      return y.tas - Phi(beta);
    }

    fixed mag() const {
      return y.tas;
    }
  };

private:
  typedef std::list<ZZObs> ObsList;

  ObsList obs;
  unsigned time_blackout;

public:

  WindZigZag() {
    reset();
  }

  /**
   * Clear as if never used
   */
  void reset() {
    obs.clear();
    time_blackout = (unsigned)-1;
  }

  /**
   * Find optimal value
   */
  ZZBeta optimise(int &res_quality, const SpeedVector start,
                  const bool circling);

  /**
   * Operator used by newuoa optimiser to evaluate function to be
   * minimised
   */
  gcc_pure
  double operator()(int n, double* x) const {
    return (double)fmin(ZZBeta((fixed)x[0], (fixed)x[1]));
  }

protected:

  gcc_pure
  bool full() const;

  gcc_pure
  bool back_in_time(const unsigned time) const;

  /**
   * Determine whether the time and angle satisfy criterion for
   * adding it to the list
   */
  gcc_pure
  bool do_append(const unsigned time, const Angle &ang) const;

  /**
   * Set blackout timer
   */
  void blackout(const unsigned time);

  /**
   * Add observation (dropping out oldest if necessary), and reset
   * blackout timer.
   */
  void append(const ZZObs &o);

private:
  gcc_pure
  unsigned size() const {
    return obs.size();
  }

  /**
   * Remove items which have high relative error (leaving at least some items).
   * This is to remove outliers which are likely to be caused by manoeuvering.
   * On average, more outliers than useful values are likely to be removed
   * by this algorithm.
   */
  void prune_worst(const ZZBeta &beta);

  gcc_pure
  std::pair<fixed, fixed> correlation(const ZZBeta& beta) const;

  /**
   * Function to be optimised (minimised)
   * This is the sum of squared errors between TAS and estimated TAS
   */
  gcc_pure
  fixed fmin(const ZZBeta& beta) const;

  /**
   * Remove item which has worst error.
   */
  bool remove_worst(const ZZBeta& beta);

  /**
   * Find error
   */
  gcc_pure
  fixed relative_error_squared(const ZZBeta& beta) const;

  /**
   * Calculate quality of fit of wind estimate beta in
   * XCSoar's wind quality measure (0-5)
   */
  gcc_pure
  int quality(const ZZBeta& beta, const bool circling) const;

  /**
   * Find item in list with worst squared error
   */
  gcc_pure
  ObsList::iterator find_worst(const ZZBeta& beta);
};

class WindZigZagGlue: public WindZigZag
{
public:
  struct Result {
    SpeedVector wind;
    int quality;

    Result() {}
    Result(int _quality):quality(_quality) {}
  };

  Result Update(const NMEAInfo &basic, const DerivedInfo &derived);
};

#endif
