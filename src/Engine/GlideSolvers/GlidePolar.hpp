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
#ifndef GLIDEPOLAR_HPP
#define GLIDEPOLAR_HPP

struct GlideState;
struct GlideResult;
struct AircraftState;

#include "PolarCoefficients.hpp"
#include "Math/fixed.hpp"
#include "Util/TypeTraits.hpp"
#include "Compiler.h"

class Angle;
struct PolarInfo;
struct SpeedVector;

/**
 * Class implementing basic glide polar performance model
 * 
 * Implements aircraft-specific glide performance, including
 * bugs/ballast, MacCready setting and cruise efficiency.
 *
 * Cruise efficiency is the ratio of actual cruise speed to 
 * target to the classical MacCready speed.
 * Cruise efficiency is stored in this class for convenience,
 * it is used in MacCready class.
 * 
 * The MacCready class uses this GlidePolar data to calculate
 * specific GlideSolutions. 
 *
 * This uses a parabolic glide polar:
 * \f[ w = a.V^2+b.V+c \f]
 * Where \f$ w, V\f$ are in m/s
 *
 */
class GlidePolar
{
  /** MacCready ring setting (m/s) */
  fixed mc;
  /** Inverse of MC setting (s/m) */
  fixed inv_mc;

  /** Clean ratio (1=clean, 0=100% bugs) */
  fixed bugs;
  /** Ballast ratio (litres) */
  fixed ballast;
  /** Cruise efficiency */
  fixed cruise_efficiency;

  /** Best lift to drag ratio */
  fixed bestLD;
  /** Speed for best L/D (m/s) */
  fixed VbestLD;
  /** Sink rate at best L/D (m/s, positive down) */
  fixed SbestLD;

  /** Maximum cruise speed (m/s) */
  fixed Vmax;
  /** Sink rate at maximum cruise speed (m/s, positive down) */
  fixed Smax;

  /** Speed for minimum sink (m/s) */
  fixed Vmin;
  /** Minimum sink rate (m/s, positive down) */
  fixed Smin;

  /** coefficients of glide polar empty/clean */
  PolarCoefficients ideal_polar;
  /** coefficients of glide polar at bug/ballast */
  PolarCoefficients polar;

  /** Ratio of mass of ballast to glider empty weight */
  fixed ballast_ratio;
  /** Reference mass of polar, kg */
  fixed reference_mass;
  /** Dry/unballasted mass of glider, kg */
  fixed dry_mass;
  /** Reference wing area, m^2 */
  fixed wing_area;

  friend class GlidePolarTest;

public:
  /**
   * Constructs an uninitialized object.
   */
  GlidePolar() = default;

  /**
   * Constructor.  Performs search for best LD at instantiation
   *
   * @param _mc MacCready value at construction
   * @param _bugs Bugs (clean) ratio (default clean)
   * @param _ballast Ballast ratio (default empty)
   */
  GlidePolar(const fixed _mc, const fixed _bugs = fixed_one,
      const fixed _ballast = fixed_zero);

  /**
   * Constructs a GlidePolar object that is invalid.
   */
  gcc_const
  static GlidePolar Invalid() {
    GlidePolar gp(fixed_zero);
    gp.SetInvalid();
    return gp;
  }

  /**
   * Mark this polar as "invalid", but retain the settings (MacCready,
   * bugs, ballast, cruise efficiency).
   */
  void SetInvalid() {
    ideal_polar.SetInvalid();
    polar.SetInvalid();
    Update();
  }

  /**
   * Perform basic checks on the validity of the object.
   */
  bool IsValid() const {
    return Vmin < Vmax;
  }

  /**
   * Accesses minimum sink rate
   *
   * @return Sink rate (m/s, positive down)
   */
  gcc_pure
  fixed
  GetSMin() const
  {
    assert(IsValid());
    return Smin;
  }

  /**
   * Accesses airspeed for minimum sink rate
   *
   * @return Speed (m/s)
   */
  gcc_pure
  fixed
  GetVMin() const
  {
    assert(IsValid());
    return Vmin;
  }

  /**
   * Accesses maximum airspeed
   *
   * @todo this should be set by ComputerSettings SafetySpeed
   *
   * @return Speed (m/s)
   */
  gcc_pure
  fixed
  GetVMax() const
  {
    assert(IsValid());
    return Vmax;
  }

  void
  SetVMax(fixed _v_max, bool update = true)
  {
    Vmax = _v_max;

    if (update) {
      UpdateSMax();
      UpdateSMin();
    }
  }

  /**
   * Accesses sink rate at max airspeed
   *
   * @return Sink rate (m/s, positive down)
   */
  gcc_pure
  fixed
  GetSMax() const
  {
    assert(IsValid());

    return Smax;
  }

  /**
   * Accesses best L/D speed
   *
   * @return Speed of best LD (m/s)
   */
  gcc_pure
  fixed
  GetVBestLD() const
  {
    assert(IsValid());

    return VbestLD;
  }

  /**
   * Accesses best L/D sink rate (positive down)
   *
   * @return Sink rate at best L/D (m/s)
   */
  gcc_pure
  fixed
  GetSBestLD() const
  {
    assert(IsValid());

    return SbestLD;
  }

  /**
   * Accesses best L/D ratio
   *
   * @return Best L/D ratio
   */
  gcc_pure
  fixed GetBestLD() const
  {
    assert(IsValid());

    return bestLD;
  }

  /**
   * Calculate the airspeed for the best glide ratio over ground,
   * considering the given head wind.
   */
  gcc_pure
  fixed GetBestGlideRatioSpeed(fixed head_wind) const;

  /**
   * Takeoff speed
   * @return Takeoff speed threshold (m/s)
   */
  gcc_pure
  fixed GetVTakeoff() const;

  /**
   * Set cruise efficiency value.  1.0 = perfect MacCready speed
   *
   * @param _ce The new cruise efficiency value
   */
  void
  SetCruiseEfficiency(const fixed _ce)
  {
    cruise_efficiency = _ce;
  }

  /**
   * Accessor for current cruise efficiency
   *
   * @return Cruise efficiency
   */
  gcc_pure
  fixed
  GetCruiseEfficiency() const
  {
    return cruise_efficiency;
  }

  /**
   * Set bugs value.
   *
   * @param clean The new bugs setting (clean ratio) (0-1]
   */
  void SetBugs(const fixed clean);

  /**
   * Retrieve bugs 
   * @return Cleanliness of glider (0-1]
   */
  gcc_pure
  fixed GetBugs() const {
    return bugs;
  }

  /**
   * Set ballast value.
   *
   * @param ratio The new ballast setting (proportion of possible ballast, [0-1]
   */
  void SetBallast(const fixed ratio);

  /**
   * Set ballast value in litres
   * @param litres The new ballast setting (l or kg)
   */
  void SetBallastLitres(const fixed litres);

  /**
   * Retrieve ballast 
   * @return Proportion of possible ballast [0-1]
   */
  gcc_pure
  fixed GetBallast() const {
    return ballast / (ballast_ratio * reference_mass);
  }

  /**
   * Retrieve if the glider is ballasted
   */
  bool HasBallast() const {
    return positive(ballast);
  }

  /**
   * Retrieve ballast in litres
   * @return Ballast (l or kg)
   */
  gcc_pure
  fixed GetBallastLitres() const;

  /**
   * Determine if glider carries ballast
   *
   * @return True if glider can carry ballast
   */
  gcc_pure
  bool IsBallastable() const;

  /**
   * Set MacCready value.  Internally this performs search
   * for best LD values corresponding to this setting.
   *
   * @param _mc The new MacCready ring setting (m/s)
   */
  void SetMC(const fixed _mc);

  /**
   * Accessor for MC setting
   *
   * @return The current MacCready ring setting (m/s)
   */
  gcc_pure
  fixed GetMC() const {
    return mc;
  }

  /**
   * Accessor for inverse of MC setting
   *
   * @return The inverse of current MacCready ring setting (s/m)
   */
  gcc_pure
  fixed GetInvMC() const {
    return inv_mc;
  }

  /**
   * Calculate all up weight
   *
   * @return Mass (kg) of aircraft including ballast
   */
  gcc_pure
  fixed GetTotalMass() const;

  /**
   * Calculate wing loading
   *
   * @return Wing loading (all up mass divided by reference area, kg/m^2)
   */
  gcc_pure
  fixed GetWingLoading() const;

  /**
   * Sink rate model (actual glide polar) function.
   *
   * @param V Speed at which sink rate is to be evaluated
   *
   * @return Sink rate (m/s, positive down)
   */
  gcc_pure
  fixed SinkRate(const fixed V) const;

  /**
   * Sink rate model (actual glide polar) function.
   *
   * Uses a parabolic load factor model to calculate additional sink rate
   * from loading:
   *
   * \f[ w(V,n) = w_0 + ({{V}\over{2 \Lambda}})[n^2-1]({{V_\Lambda}\over{V}})^2 \f]
   * Where:
   * - \f$n \f$ is the load factor
   * - \f$\Lambda \f$ is the best L/D ratio
   * - \f$V_\Lambda \f$ is the speed for best L/D
   *
   * @param V Speed at which sink rate is to be evaluated
   * @param n Load factor
   *
   * @return Sink rate (m/s, positive down)
   */
  gcc_pure
  fixed SinkRate(const fixed V, const fixed n) const;

  /**
   * Sink rate model adjusted by MC setting.  This is used
   * to accomodate speed ring (MC) settings in optimal glide
   * calculations.
   *
   * @param V Speed at which sink rate is to be evaluated
   *
   * @return Sink rate plus MC setting (m/s, positive down)
   */
  gcc_pure
  fixed MSinkRate(const fixed V) const;

  /**
   * Quickly determine whether a task is achievable without
   * climb, assuming favorable wind.  This can be used to quickly
   * pre-filter waypoints for arrival altitude before performing
   * expensive optimal glide solution searches.
   *
   * @param task The glide task for which to estimate a solution
   *
   * @return True if a glide solution is feasible (optimistically)
   */
  gcc_pure
  bool IsGlidePossible(const GlideState &task) const;

  /**
   * Calculate speed-to-fly according to MacCready dolphin theory
   * with ring setting at current MC value.
   *
   * @param state Aircraft state (taking TrueAirspeed and Vario)
   * @param solution Solution for which Vopt is desired
   * @param block_stf Whether to use block speed to fly or dolphin
   *
   * @return Speed to fly (true, m/s)
   */
  gcc_pure
  fixed SpeedToFly(const AircraftState &state, const GlideResult &solution,
      const bool block_stf) const;

  /**
   * Compute MacCready ring setting to adjust speeds to incorporate
   * risk as the aircraft gets low.
   *
   * @param height_fraction Ratio of height to climb ceiling
   * @param riskGamma Risk adjustment factor.  Lower gamma, MC is uniform
   * with height.  High gamma, MC scales almost uniformly with height
   *
   * @return MC value adjusted for risk (m/s)
   */
  gcc_pure
  fixed GetRiskMC(fixed height_fraction, const fixed riskGamma) const;

  /**
   * Find LD relative to ground for specified track bearing
   *
   * @param track the true aircraft ground direction
   * @param wind the wind vector
   * @return LD ratio (distance travelled per unit height loss)
   */
  gcc_pure
  fixed GetLDOverGround(Angle track, SpeedVector wind) const;

  /**
   * Find LD relative to ground for specified track bearing
   *
   * @param state Aircraft state (for wind)
   *
   * @return LD ratio (distance travelled per unit height loss)
   */
  gcc_pure
  fixed GetLDOverGround(const AircraftState &state) const;

  /**
   * Calculates the thermal value of next leg that is equivalent (gives the
   * same average speed) to the current MacCready setting.
   *
   * @param current_wind The head wind component on current leg
   * @param next_wind The head wind component on next leg
   *
   * @return Equivalent thermal strength. Normally a positive value, but in
   * some situations it can be negative.
   */
  gcc_pure
  fixed GetNextLegEqThermal(fixed current_wind, fixed next_wind) const;

  /** Returns the wing area in m^2 */
  fixed GetWingArea() const {
    return wing_area;
  }

  /** Sets the wing area in m^2 */
  void SetWingArea(fixed _wing_area) {
    wing_area = _wing_area;
  }

  /** Returns the reference mass in kg */
  fixed GetReferenceMass() const {
    return reference_mass;
  }

  /** Sets the reference mass in kg */
  void SetReferenceMass(fixed _reference_mass, bool update = true) {
    reference_mass = _reference_mass;

    if (update)
      Update();
  }

  /** Returns the dry mass in kg */
  fixed GetDryMass() const {
    return dry_mass;
  }

  /** Sets the dry mass in kg */
  void SetDryMass(fixed _dry_mass, bool update = true) {
    dry_mass = _dry_mass;

    if (update)
      Update();
  }

  /** Returns the ballast ratio */
  fixed GetBallastRatio() const {
    return ballast_ratio;
  }

  /** Sets the ballast ratio */
  void SetBallastRatio(fixed _ballast_ratio) {
    ballast_ratio = _ballast_ratio;
  }

  /** Returns the ideal polar coefficients */
  PolarCoefficients GetCoefficients() const {
    return ideal_polar;
  }

  /** Returns the real polar coefficients */
  PolarCoefficients GetRealCoefficients() const {
    return polar;
  }

  /** Sets the ideal polar coefficients */
  void SetCoefficients(PolarCoefficients coeff, bool update = true) {
    ideal_polar = coeff;

    if (update)
      Update();
  }

  /** Update glide polar coefficients and values depending on them */
  void Update();

private:
  /** Update sink rate at max. cruise speed */
  void UpdateSMax();

  /** Solve for best LD at current MC/bugs/ballast setting. */
  void UpdateBestLD();

  /** Solve for min sink rate at current bugs/ballast setting. */
  void UpdateSMin();
};

static_assert(is_trivial<GlidePolar>::value, "type is not trivial");

#endif
