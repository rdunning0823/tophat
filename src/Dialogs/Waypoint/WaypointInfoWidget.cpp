/*
Copyright_License {

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

#include "WaypointInfoWidget.hpp"
#include "Engine/Waypoint/Waypoint.hpp"
#include "Engine/GlideSolvers/GlideState.hpp"
#include "Engine/GlideSolvers/MacCready.hpp"
#include "Engine/GlideSolvers/GlideResult.hpp"
#include "Engine/Util/Gradient.hpp"
#include "NMEA/MoreData.hpp"
#include "NMEA/Derived.hpp"
#include "Computer/Settings.hpp"
#include "Math/SunEphemeris.hpp"
#include "Util/StaticString.hxx"
#include "Util/Macros.hpp"
#include "Language/Language.hpp"
#include "Interface.hpp"
#include "Formatter/UserUnits.hpp"
#include "Formatter/AngleFormatter.hpp"
#include "Formatter/UserGeoPointFormatter.hpp"
#include "Formatter/GlideRatioFormatter.hpp"

static const TCHAR *
FormatGlideResult(TCHAR *buffer, size_t size,
                  const GlideResult &result, const GlideSettings &settings)
{
  switch (result.validity) {
  case GlideResult::Validity::OK:
    FormatRelativeUserAltitude(result.SelectAltitudeDifference(settings),
                               buffer, size);
    return buffer;

  case GlideResult::Validity::WIND_EXCESSIVE:
  case GlideResult::Validity::MACCREADY_INSUFFICIENT:
    return _("Too much wind");

  case GlideResult::Validity::NO_SOLUTION:
    return _("No solution");
  }

  return nullptr;
}

void
WaypointInfoWidget::AddGlideResult(const TCHAR *label,
                                   const GlideResult &result)
{
  const ComputerSettings &settings = CommonInterface::GetComputerSettings();

  TCHAR buffer[64];
  AddReadOnly(label, nullptr,
              FormatGlideResult(buffer, ARRAY_SIZE(buffer),
                                result, settings.task.glide));
}

gcc_const
static BrokenTime
BreakHourOfDay(fixed t)
{
  /* depending on the time zone, the SunEphemeris library may return a
     negative time of day; the following check catches this before we
     cast the value to "unsigned" */
  if (negative(t))
    t += fixed(24);

  unsigned i = uround(t * 3600);

  BrokenTime result;
  result.hour = i / 3600;
  i %= 3600;
  result.minute = i / 60;
  result.second = i % 60;
  return result;
}

void
WaypointInfoWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);

  const MoreData &basic = CommonInterface::Basic();
  const DerivedInfo &calculated = CommonInterface::Calculated();
  const ComputerSettings &settings = CommonInterface::GetComputerSettings();

  StaticString<64> buffer;

  AddReadOnly(_T(""), nullptr, waypoint.name.c_str());

  if (basic.location_available) {
    const GeoVector vector = basic.location.DistanceBearing(waypoint.location);

    FormatUserDistanceSmart(vector.distance, buffer.buffer());

    StaticString<64> bearing_buffer;
    FormatBearing(bearing_buffer.buffer(), bearing_buffer.CAPACITY,
                  vector.bearing);
    buffer.AppendFormat(_T(" / %s"), bearing_buffer.c_str());
    AddReadOnly(_("Distance"), nullptr, buffer);
  }

  if (basic.location_available && basic.NavAltitudeAvailable() &&
      settings.polar.glide_polar_task.IsValid()) {
    const GlideState glide_state(basic.location.DistanceBearing(waypoint.location),
                                 waypoint.elevation + settings.task.safety_height_arrival,
                                 basic.nav_altitude,
                                 calculated.GetWindOrZero());

    fixed mc = settings.polar.glide_polar_task.GetMC();
    StaticString<10>mc_value;
    FormatUserVerticalSpeed(mc, mc_value.buffer(), false, false);
    buffer.Format(_T("%s. %s %s"), _("Arrival alt"), _("MC"), mc_value.c_str());
    AddGlideResult(buffer.c_str(),
                   MacCready::Solve(settings.task.glide,
                                    settings.polar.glide_polar_task,
                                    glide_state));
  }

  if (basic.location_available && basic.NavAltitudeAvailable()) {
    StaticString<10> gr_text(_T("+++"));
    fixed gradient = ::CalculateGradient(waypoint, basic,
                                   CommonInterface::GetComputerSettings().task.GRSafetyHeight());

    if (positive(gradient) &&  GradientValid(gradient)) {
      ::FormatGlideRatio(gr_text.buffer(), gr_text.CAPACITY, gradient);
    }
    AddReadOnly(_("Glide ratio"), nullptr, gr_text.c_str());
  }

  AddSpacer();

  FormatUserAltitude(waypoint.elevation,
                     buffer.buffer(), buffer.CAPACITY);
  AddReadOnly(_("Elevation"), nullptr, buffer);

  if (FormatGeoPoint(waypoint.location,
                     buffer.buffer(), buffer.CAPACITY) != nullptr)
    AddReadOnly(_("Location"), nullptr, buffer);

  if (waypoint.radio_frequency.IsDefined() &&
      waypoint.radio_frequency.Format(buffer.buffer(),
                                      buffer.CAPACITY) != nullptr) {
    buffer += _T(" MHz");
    AddReadOnly(_("Radio frequency"), nullptr, buffer);
  }

  if (basic.time_available && basic.date_time_utc.IsDatePlausible()) {
    const SunEphemeris::Result sun =
      SunEphemeris::CalcSunTimes(waypoint.location, basic.date_time_utc,
                                 settings.utc_offset);

    const BrokenTime sunset = BreakHourOfDay(sun.time_of_sunset);

    buffer.UnsafeFormat(_T("%02u:%02u"),
                        sunset.hour, sunset.minute);
    AddReadOnly(_("Sunset"), nullptr, buffer);
  }

  if (waypoint.runway.IsDirectionDefined())
    buffer.UnsafeFormat(_T("%02u"), waypoint.runway.GetDirectionName());
  else
    buffer.clear();

  if (waypoint.runway.IsLengthDefined()) {
    if (!buffer.empty())
      buffer += _T("; ");

    TCHAR length_buffer[16];
    FormatSmallUserDistance(length_buffer,
                            fixed(waypoint.runway.GetLength()));
    buffer += length_buffer;
  }
  if (!buffer.empty())
    AddReadOnly(_("Runway"), nullptr, buffer);

  if (!waypoint.comment.empty())
    AddMultiLine(waypoint.comment.c_str());
}
