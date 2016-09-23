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

#include "ElementStat.hpp"
#include "Navigation/Aircraft.hpp"
#include "LogFile.hpp"
#include <tchar.h>
#include <algorithm>

void
ElementStat::Reset()
{
  location_remaining = GeoPoint::Invalid();
  vector_remaining = GeoVector::Invalid();
  next_leg_vector = GeoVector::Invalid();

  time_started = fixed(-1);
  time_elapsed = time_remaining_now = time_remaining_start = time_planned = fixed(0);
  gradient = fixed(0);

  remaining_effective.Reset();
  remaining.Reset();
  planned.Reset();
  travelled.Reset();
  pirker.Reset();

  solution_planned.Reset();
  solution_travelled.Reset();
  solution_remaining.Reset();
  solution_mc0.Reset();
  solution_remaining_safety_mc.Reset();

  vario.Reset();
}

static void
DumpGeoVector(const GeoVector &g, const TCHAR* name)
{
  if (!g.IsValid()) {
    LogFormat(_T("  GeoVector: %s INVALID"), name);
    return;
  }
  LogFormat(_T("  GeoVector: %s dist:%i, bearing:%i"),
            name, (int)g.distance, (int)g.bearing.AsBearing().Degrees());
}

static void
DumpDistanceStat(const DistanceStat &d, const TCHAR* name)
{
  if (!d.IsDefined()) {
    LogFormat(_T("  DistanceStat: %s INVALID"), name);
    return;
  }
  LogFormat(_T("  DistanceStat: %s dist:%i, speed:%i speed_incremental:%i"),
            name, (int)d.GetDistance(), (int)d.GetSpeed(), (int)d.GetSpeedIncremental());
}

void
ElementStat::DumpStat() const
{
  DumpGeoVector(vector_remaining,       _T("     vector_remaining"));
  DumpGeoVector(next_leg_vector,        _T("      next_leg_vector"));
  DumpDistanceStat(remaining_effective, _T("  remaining_effective"));
  DumpDistanceStat(remaining,           _T("            remaining"));
  DumpDistanceStat(planned,             _T("              planned"));
  DumpDistanceStat(travelled,           _T("            travelled"));
  DumpDistanceStat(pirker,              _T("               pirker"));
}

void
ElementStat::SetTimes(const fixed until_start_s, const fixed ts,
                      const fixed time)
{
  time_started = ts;

  if (negative(time_started) || negative(time))
    /* not yet started */
    time_elapsed = fixed(0);
  else
    time_elapsed = std::max(time - fixed(ts), fixed(0));

  if (solution_remaining.IsOk()) {
    time_remaining_now = solution_remaining.time_elapsed;
    time_remaining_start = std::max(time_remaining_now - until_start_s,
                                    fixed(0));
    time_planned = time_elapsed + time_remaining_start;
  } else {
    time_remaining_now = time_remaining_start = time_planned = fixed(0);
  }
}
