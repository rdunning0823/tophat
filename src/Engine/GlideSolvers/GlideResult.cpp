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

#include "GlideResult.hpp"
#include "GlideState.hpp"
#include "LogFile.hpp"

GlideResult::GlideResult(const GlideState &task, const fixed V)
  :head_wind(task.head_wind),
   v_opt(V),
#ifndef NDEBUG
   start_altitude(task.min_arrival_altitude + task.altitude_difference),
#endif
   min_arrival_altitude(task.min_arrival_altitude),
   vector(task.vector),
   pure_glide_min_arrival_altitude(task.min_arrival_altitude),
   pure_glide_altitude_difference(task.altitude_difference),
   altitude_difference(task.altitude_difference),
   effective_wind_speed(task.wind.norm),
   effective_wind_angle(task.effective_wind_angle),
   validity(Validity::NO_SOLUTION)
{
}

void
GlideResult::CalcDeferred()
{
  CalcCruiseBearing();
}

void
GlideResult::CalcCruiseBearing()
{
  if (!IsOk())
    return;

  cruise_track_bearing = vector.bearing;
  if (!positive(effective_wind_speed))
    return;

  const fixed sintheta = effective_wind_angle.sin();
  if (sintheta == fixed(0))
    return;

  cruise_track_bearing -=
    Angle::asin(sintheta * effective_wind_speed / v_opt).Half();
}

void
GlideResult::Add(const GlideResult &s2)
{
  if ((unsigned)s2.validity > (unsigned)validity)
    /* downgrade the validity */
    validity = s2.validity;

  if (!IsDefined())
    return;

  vector.distance += s2.vector.distance;

  if (!IsOk())
    /* the other attributes are not valid if validity is not OK or
       PARTIAL */
    return;

  if (s2.GetRequiredAltitudeWithDrift() < min_arrival_altitude) {
    /* must meet the safety height of the first leg */
    assert(s2.min_arrival_altitude < s2.GetArrivalAltitudeWithDrift(min_arrival_altitude));

    /* calculate a new minimum arrival height that considers the
       "mountain top" in the middle */
    min_arrival_altitude = s2.GetArrivalAltitudeWithDrift(min_arrival_altitude);
  } else {
    /* must meet the safety height of the second leg */

    /* apply the increased altitude requirement */
    altitude_difference -=
      s2.GetRequiredAltitudeWithDrift() - min_arrival_altitude;

    /* adopt the minimum height of the second leg */
    min_arrival_altitude = s2.min_arrival_altitude;
  }

  /* same as above, but for "pure glide" */

  if (s2.GetRequiredAltitude() < pure_glide_min_arrival_altitude) {
    /* must meet the safety height of the first leg */
    assert(s2.pure_glide_min_arrival_altitude <
           s2.GetArrivalAltitude(pure_glide_min_arrival_altitude));

    /* calculate a new minimum arrival height that considers the
       "mountain top" in the middle */
    pure_glide_min_arrival_altitude =
      s2.GetArrivalAltitude(pure_glide_min_arrival_altitude);
  } else {
    /* must meet the safety height of the second leg */

    /* apply the increased altitude requirement */
    pure_glide_altitude_difference -=
      s2.GetRequiredAltitude() - pure_glide_min_arrival_altitude;

    /* adopt the minimum height of the second leg */
    pure_glide_min_arrival_altitude = s2.pure_glide_min_arrival_altitude;
  }

  pure_glide_height += s2.pure_glide_height;
  time_elapsed += s2.time_elapsed;
  height_glide += s2.height_glide;
  height_climb += s2.height_climb;
  time_virtual += s2.time_virtual;
}

bool
GlideResult::IsFinalGlide() const
{
  return IsOk() && !negative(altitude_difference) && !positive(height_climb);
}

void
GlideResult::Reset()
{
  validity = Validity::NO_SOLUTION;
}

void
GlideResult::DumpGlideResult(const TCHAR *label) const
{
  LogDebug("GlideR(%s):        v_opt:%4.0f     start_alt:%4.0f  min_arrive_atl:%4.0f  altitude_difference:%4.0f           pure_glide_height:%4.0f v.dist:%4.0f  v.bearing:%4.0f",
           label, (double)v_opt, (double)start_altitude,
           (double)min_arrival_altitude, (double)altitude_difference, (double)pure_glide_height, (double)vector.distance,
           (double)vector.bearing.AbsoluteDegrees());

  LogDebug("          : height_climb:%4.0f  height_glide:%4.0f       t_elapsed:%4.0f            t_virtual:%4.0f    pure_glide_min_arriv_alt:%4.0f  valid:%i",
           (double)height_climb, (double)height_glide, (double)time_elapsed, (double)time_virtual, (double)pure_glide_min_arrival_altitude, (int)validity);
}

