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

#include "MapItemListRenderer.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "MapWindow/Items/MapItem.hpp"
#include "Look/DialogLook.hpp"
#include "Look/MapLook.hpp"
#include "Renderer/AircraftRenderer.hpp"
#include "Renderer/AirspaceListRenderer.hpp"
#include "Renderer/WaypointListRenderer.hpp"
#include "Engine/Waypoint/Waypoint.hpp"
#include "Formatter/UserUnits.hpp"
#include "Formatter/UserGeoPointFormatter.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Formatter/LocalTimeFormatter.hpp"
#include "Formatter/AngleFormatter.hpp"
#include "Dialogs/Task/dlgTaskHelpers.hpp"
#include "Renderer/OZPreviewRenderer.hpp"
#include "Language/Language.hpp"
#include "Util/StringUtil.hpp"
#include "Util/Macros.hpp"
#include "Util/StaticString.hxx"
#include "Terrain/RasterBuffer.hpp"
#include "MapSettings.hpp"
#include "Math/Screen.hpp"
#include "Look/TrafficLook.hpp"
#include "Look/FinalGlideBarLook.hpp"
#include "Renderer/TrafficRenderer.hpp"
#include "FLARM/FlarmDetails.hpp"
#include "FLARM/FlarmNetRecord.hpp"
#include "Weather/Features.hpp"
#include "FLARM/List.hpp"
#include "Time/RoughTime.hpp"
#include "Time/BrokenDateTime.hpp"
#include "Task/Points/TaskPoint.hpp"
#include "Renderer/TwoTextRowsRenderer.hpp"
#include "Interface.hpp"

#ifdef HAVE_NOAA
#include "Renderer/NOAAListRenderer.hpp"
#endif

#include <cstdio>

namespace MapItemListRenderer
{
  void Draw(Canvas &canvas, const PixelRect rc, const LocationMapItem &item,
            const DialogLook &dialog_look);

  void Draw(Canvas &canvas, const PixelRect rc,
            const ArrivalAltitudeMapItem &item,
            const DialogLook &dialog_look, const FinalGlideBarLook &look);

  void Draw(Canvas &canvas, const PixelRect rc, const SelfMapItem &item,
            const DialogLook &dialog_look,
            const AircraftLook &look, const MapSettings &settings);

  void Draw(Canvas &canvas, const PixelRect rc, const AirspaceMapItem &item,
            const DialogLook &dialog_look, const AirspaceLook &look,
            const AirspaceRendererSettings &renderer_settings);

  void Draw(Canvas &canvas, const PixelRect rc, const WaypointMapItem &item,
            const DialogLook &dialog_look, const WaypointLook &look,
            const WaypointRendererSettings &renderer_settings);

#ifdef HAVE_NOAA
  void Draw(Canvas &canvas, const PixelRect rc, const WeatherStationMapItem &item,
            const DialogLook &dialog_look, const NOAALook &look);
#endif

  void Draw(Canvas &canvas, const PixelRect rc, const TaskOZMapItem &item,
            const DialogLook &dialog_look,
            const TaskLook &look, const AirspaceLook &airspace_look,
            const AirspaceRendererSettings &airspace_settings);

  void Draw(Canvas &canvas, const PixelRect rc, const TrafficMapItem &item,
            const DialogLook &dialog_look, const TrafficLook &traffic_look,
            const TrafficList *traffic_list);

  void Draw(Canvas &canvas, const PixelRect rc, const ThermalMapItem &item,
            RoughTimeDelta utc_offset,
            const DialogLook &dialog_look, const MapLook &look);
}

unsigned
MapItemListRenderer::CalculateLayout(const DialogLook &dialog_look,
                                     TwoTextRowsRenderer &row_renderer)
{
  return row_renderer.CalculateLayout(*dialog_look.list.font_bold,
                                      dialog_look.text_font);
}

/**
 * unknown item type
 */
void
MapItemListRenderer::Draw(Canvas &canvas, const PixelRect rc,
                          const LocationMapItem &item,
                          const DialogLook &dialog_look)
{
  const Font &name_font = *dialog_look.list.font_bold;
  const Font &text_font = dialog_look.text_font;

  const unsigned text_padding = Layout::GetTextPadding();
  int left = rc.left + text_padding;

  TCHAR info_buffer[256], distance_buffer[32], direction_buffer[32];
  if (item.vector.IsValid()) {
    FormatUserDistanceSmart(item.vector.distance, distance_buffer);
    FormatBearing(direction_buffer, ARRAY_SIZE(direction_buffer),
                  item.vector.bearing);
    _stprintf(info_buffer, _T("%s: %s, %s: %s"),
              _("Distance"), distance_buffer,
              _("Direction"), direction_buffer);
  } else {
    _stprintf(info_buffer, _T("%s: %s, %s: %s"),
              _("Distance"), _T("???"), _("Direction"), _T("???"));
  }

  canvas.Select(name_font);

  canvas.DrawClippedText(left, rc.top + text_padding, rc, info_buffer);


  TCHAR elevation_buffer[32];
  if (!RasterBuffer::IsSpecial(item.elevation)) {
    FormatUserAltitude(fixed(item.elevation), elevation_buffer, 32);
    _stprintf(info_buffer, _T("%s: %s"), _("Elevation"), elevation_buffer);
  } else {
    _stprintf(info_buffer, _T("%s: %s"), _("Elevation"), _T("???"));
  }

  canvas.Select(text_font);
  canvas.DrawClippedText(left,
                         rc.top + name_font.GetHeight() + 2 * text_padding,
                         rc, info_buffer);
}

/**
 * ARRIVAL_ALTITUDE
 */
void
MapItemListRenderer::Draw(Canvas &canvas, const PixelRect rc,
                          const ArrivalAltitudeMapItem &item,
                          const DialogLook &dialog_look,
                          const FinalGlideBarLook &look)
{
  const UPixelScalar line_height = rc.bottom - rc.top;

  bool elevation_available =
      !RasterBuffer::IsSpecial((short)item.elevation);

  bool reach_relevant = item.reach.IsReachRelevant();

  RoughAltitude arrival_altitude =
    item.reach.terrain_valid == ReachResult::Validity::VALID
    ? item.reach.terrain
    : item.reach.direct;
  if (elevation_available)
    arrival_altitude -= item.elevation;

  bool reachable =
    item.reach.terrain_valid != ReachResult::Validity::UNREACHABLE &&
    arrival_altitude.IsPositive();

  // Draw final glide arrow icon

  RasterPoint pt = { (PixelScalar)(rc.left + line_height / 2),
                     (PixelScalar)(rc.top + line_height / 2) };

  RasterPoint arrow[] = {
      { -7, -3 }, { 0, 4 }, { 7, -3 }
  };

  Angle arrow_angle = reachable ? Angle::HalfCircle() : Angle::Zero();
  PolygonRotateShift(arrow, ARRAY_SIZE(arrow), pt, arrow_angle, 100);

  if (reachable) {
    canvas.Select(look.brush_above);
    canvas.Select(look.pen_above);
  } else {
    canvas.Select(look.brush_below);
    canvas.Select(look.pen_below);
  }
  canvas.DrawPolygon(arrow, ARRAY_SIZE(arrow));


  const Font &name_font = *dialog_look.list.font_bold;
  const Font &text_font = dialog_look.text_font;

  const unsigned text_padding = Layout::GetTextPadding();
  int left = rc.left + line_height + text_padding;


  // Format title row
  TCHAR altitude_buffer[32];
  StaticString<256> buffer;
  buffer.clear();

  if (reach_relevant) {
    buffer.Format(_T("%s: "), _("around terrain"));

    if (elevation_available) {
      RoughAltitude relative_arrival_altitude =
          item.reach.terrain - item.elevation;

      FormatRelativeUserAltitude(fixed((short)relative_arrival_altitude),
                                 altitude_buffer, ARRAY_SIZE(altitude_buffer));

     buffer.AppendFormat(_T("%s %s, "), altitude_buffer, _("AGL"));
    }

    FormatUserAltitude(fixed(item.reach.terrain),
                       altitude_buffer, ARRAY_SIZE(altitude_buffer));

    buffer.AppendFormat(_T("%s %s, "), altitude_buffer, _("MSL"));
  } else if (elevation_available &&
             (int)item.reach.direct >= (int)item.elevation &&
             item.reach.terrain_valid == ReachResult::Validity::UNREACHABLE) {
    buffer.UnsafeFormat(_T("%s "), _("Unreachable due to terrain."));
  } else {
    buffer.clear();
  }

  buffer += _("Arrival Alt");

  // Draw title row
  canvas.Select(text_font);
  canvas.DrawClippedText(left, rc.top + text_padding, rc, buffer);

  // Format comment row

  buffer.clear();
  if (elevation_available) {
    RoughAltitude relative_arrival_altitude =
      item.reach.direct - item.elevation;

    FormatRelativeUserAltitude(fixed((short)relative_arrival_altitude),
                               altitude_buffer, ARRAY_SIZE(altitude_buffer));

    buffer.AppendFormat(_T("%s %s, "), altitude_buffer, _("AGL"));
  }

  FormatUserAltitude(fixed(item.reach.direct),
                     altitude_buffer, ARRAY_SIZE(altitude_buffer));

  buffer.AppendFormat(_T("%s %s"), altitude_buffer, _("MSL"));

  // Draw comment row

  canvas.Select(name_font);
  canvas.DrawClippedText(left,
                         rc.top + name_font.GetHeight() + 2 * text_padding,
                         rc, buffer);
}

/**
 * Your position
 */
void
MapItemListRenderer::Draw(Canvas &canvas, const PixelRect rc,
                          const SelfMapItem &item,
                          const DialogLook &dialog_look,
                          const AircraftLook &look,
                          const MapSettings &settings)
{
  const unsigned line_height = rc.bottom - rc.top;
  const unsigned text_padding = Layout::GetTextPadding();

  const Font &name_font = *dialog_look.list.font_bold;
  const Font &text_font = dialog_look.text_font;

  int left = rc.left + line_height + text_padding;
  canvas.Select(name_font);
  canvas.DrawClippedText(left, rc.top + text_padding, rc,
                         _("Your Position"));

  TCHAR buffer[128];
  FormatGeoPoint(item.location, buffer, 128);

  canvas.Select(text_font);
  canvas.DrawClippedText(left,
                         rc.top + name_font.GetHeight() + 2 * text_padding,
                         rc, buffer);

  RasterPoint pt = { (PixelScalar)(rc.left + line_height / 2),
                     (PixelScalar)(rc.top + line_height / 2) };
  AircraftRenderer::Draw(canvas, settings, look, item.bearing, pt);
}

/**
 * Airspace
 */
void
MapItemListRenderer::Draw(Canvas &canvas, const PixelRect rc,
                          const AirspaceMapItem &item,
                          const DialogLook &dialog_look,
                          const AirspaceLook &look,
                          const AirspaceRendererSettings &renderer_settings)
{
  TwoTextRowsRenderer row_renderer;
  MapItemListRenderer::CalculateLayout(dialog_look, row_renderer);

  AirspaceListRenderer::Draw(canvas, rc, *item.airspace, row_renderer, look,
                             renderer_settings);
}

/**
 * Waypoint
 */
void
MapItemListRenderer::Draw(Canvas &canvas, const PixelRect rc,
                          const WaypointMapItem &item,
                          const DialogLook &dialog_look,
                          const WaypointLook &look,
                          const WaypointRendererSettings &renderer_settings)
{
  GeoVector *v = nullptr;
  GeoVector vector;
  if (CommonInterface::Basic().location_available &&
      item.waypoint.location.IsValid()) {
    vector = CommonInterface::Basic().location.DistanceBearing(item.waypoint.location);
    v = &vector;
  }

  WaypointListRenderer::Draw(canvas, rc, item.waypoint, v,
                             dialog_look, look, renderer_settings);
}

#ifdef HAVE_NOAA
void
MapItemListRenderer::Draw(Canvas &canvas, const PixelRect rc,
                          const WeatherStationMapItem &item,
                          const DialogLook &dialog_look,
                          const NOAALook &look)
{
  TwoTextRowsRenderer row_renderer;
  MapItemListRenderer::CalculateLayout(dialog_look, row_renderer);

  const NOAAStore::Item &station = *item.station;
  NOAAListRenderer::Draw(canvas, rc, station, look, row_renderer);
}
#endif

/**
 * Thermal Source
 */
void
MapItemListRenderer::Draw(Canvas &canvas, const PixelRect rc,
                          const ThermalMapItem &item,
                          RoughTimeDelta utc_offset,
                          const DialogLook &dialog_look,
                          const MapLook &look)
{
  const unsigned line_height = rc.bottom - rc.top;
  const unsigned text_padding = Layout::GetTextPadding();

  const ThermalSource &thermal = item.thermal;

  const RasterPoint pt(rc.left + line_height / 2,
                       rc.top + line_height / 2);

  look.thermal_source_icon.Draw(canvas, pt);

  const Font &name_font = *dialog_look.list.font_bold;
  const Font &text_font = dialog_look.text_font;

  int left = rc.left + line_height + text_padding;

  canvas.Select(name_font);
  canvas.DrawClippedText(left, rc.top + text_padding,
                         rc, _("Thermal"));

  StaticString<256> buffer;
  TCHAR lift_buffer[32], time_buffer[32], timespan_buffer[32];
  FormatUserVerticalSpeed(thermal.lift_rate, lift_buffer, 32);
  FormatLocalTimeHHMM(time_buffer, (int)thermal.time, utc_offset);

  int timespan = BrokenDateTime::NowUTC().GetSecondOfDay() - (int)thermal.time;
  if (timespan < 0)
    timespan += 24 * 60 * 60;

  FormatTimespanSmart(timespan_buffer, timespan);

  buffer.Format(_T("%s: %s"), _("Avg. lift"), lift_buffer);
  buffer.append(_T(" - "));
  buffer.AppendFormat(_("left %s ago"), timespan_buffer);
  buffer.AppendFormat(_T(" (%s)"), time_buffer);
  canvas.Select(text_font);
  canvas.DrawClippedText(left,
                         rc.top + name_font.GetHeight() + 2 * text_padding,
                         rc, buffer);
}

/**
 *  Task OZ
 */
void
MapItemListRenderer::Draw(Canvas &canvas, const PixelRect rc,
                          const TaskOZMapItem &item,
                          const DialogLook &dialog_look,
                          const TaskLook &look, const AirspaceLook &airspace_look,
                          const AirspaceRendererSettings &airspace_settings)
{
  const unsigned line_height = rc.bottom - rc.top;
  const unsigned padding = Layout::GetTextPadding();

  const ObservationZonePoint &oz = *item.oz;
  const Waypoint &waypoint = item.waypoint;

  const Font &name_font = *dialog_look.list.font_bold;
  const Font &text_font = dialog_look.text_font;

  TCHAR buffer[256];

  // Y-Coordinate of the single row of text
  PixelScalar top_middle = rc.top + (rc.GetSize().cy - name_font.GetHeight()) / 2;


  // Use small font for details
  canvas.Select(text_font);

  // Draw details line
  OrderedTaskPointRadiusLabel(*item.oz, buffer);
  PixelScalar radius_text_width = text_font.TextSize(buffer).cx;
  PixelScalar radius_text_left = rc.right - radius_text_width - padding;
  if (!StringIsEmpty(buffer))
    canvas.DrawClippedText(radius_text_left, top_middle,
                           rc.right - radius_text_width, buffer);

  // Draw waypoint name
  canvas.Select(name_font);
  UPixelScalar left = rc.left + line_height + padding;
  OrderedTaskPointLabelMapAction(item.tp_type, waypoint.name.c_str(),
                                 item.index, buffer);
  canvas.DrawClippedText(left, top_middle,
                         rc.right - left - radius_text_width - 2 * padding, buffer);

  const RasterPoint pt(rc.left + line_height / 2,
                       rc.top + line_height / 2);
  PixelScalar radius = std::min(int(line_height / 2
                                            - 2 * padding),
                                Layout::FastScale(10));
  OZPreviewRenderer::Draw(canvas, oz, pt, radius, look,
                          airspace_settings, airspace_look,
                          item.tp_type == TaskPointType::AAT);
}

/**
 * Flarm Traffic
 */
void
MapItemListRenderer::Draw(Canvas &canvas, const PixelRect rc,
                          const TrafficMapItem &item,
                          const DialogLook &dialog_look,
                          const TrafficLook &traffic_look,
                          const TrafficList *traffic_list)
{
  const unsigned line_height = rc.bottom - rc.top;
  const unsigned text_padding = Layout::GetTextPadding();

  const FlarmTraffic *traffic = traffic_list == NULL ? NULL :
      traffic_list->FindTraffic(item.id);

  // Now render the text information
  const Font &name_font = *dialog_look.list.font_bold;
  const Font &text_font = dialog_look.text_font;
  int left = rc.left + line_height + text_padding;

  const FlarmNetRecord *record = FlarmDetails::LookupRecord(item.id);

  StaticString<256> title_string;
  if (record && !StringIsEmpty(record->pilot))
    title_string = record->pilot.c_str();
  else
    title_string = _("FLARM Traffic");

  // Append name to the title, if it exists
  const TCHAR *callsign = FlarmDetails::LookupCallsign(item.id);
  if (callsign != NULL && !StringIsEmpty(callsign)) {
    title_string.append(_T(", "));
    title_string.append(callsign);
  }

  canvas.Select(name_font);
  canvas.DrawClippedText(left, rc.top + text_padding,
                         rc, title_string);

  StaticString<256> info_string;
  if (record && !StringIsEmpty(record->plane_type))
    info_string = record->plane_type;
  else if (traffic != NULL)
    info_string = FlarmTraffic::GetTypeString(traffic->type);
  else
    info_string = _("Unknown");

  // Generate the line of info about the target, if it's available
  if (traffic != NULL) {
    if (traffic->altitude_available) {
      TCHAR tmp[15];
      FormatUserAltitude(traffic->altitude, tmp, 15);
      info_string.AppendFormat(_T(", %s: %s"), _("Altitude"), tmp);
    }
    if (traffic->climb_rate_avg30s_available) {
      TCHAR tmp[15];
      FormatUserVerticalSpeed(traffic->climb_rate_avg30s, tmp, 15);
      info_string.AppendFormat(_T(", %s: %s"), _("Vario"), tmp);
    }
  }
  canvas.Select(text_font);
  canvas.DrawClippedText(left,
                         rc.top + name_font.GetHeight() + 2 * text_padding,
                         rc, info_string);

  RasterPoint pt = { (PixelScalar)(rc.left + line_height / 2),
                     (PixelScalar)(rc.top + line_height / 2) };

  // Render the representation of the traffic icon
  if (traffic != NULL)
    TrafficRenderer::Draw(canvas, traffic_look, *traffic, traffic->track,
                          item.color, pt);
}

#ifdef HAVE_SKYLINES_TRACKING_HANDLER

/**
 * Calculate how many minutes have passed since #past_ms.
 */
gcc_const
static unsigned
SinceInMinutes(fixed now_s, uint32_t past_ms)
{
  const unsigned day_minutes = 24 * 60;
  unsigned now_minutes = uint32_t(now_s / 60) % day_minutes;
  unsigned past_minutes = (past_ms / 60000) % day_minutes;

  if (past_minutes >= 20 * 60 && now_minutes < 4 * 60)
    /* midnight rollover */
    now_minutes += day_minutes;

  if (past_minutes > now_minutes)
    return 0;

  return now_minutes - past_minutes;
}

#include "Interface.hpp"

static void
Draw(Canvas &canvas, const PixelRect rc,
     const SkyLinesTrafficMapItem &item,
     const DialogLook &dialog_look)
{
  const Font &name_font = *dialog_look.list.font_bold;
  const Font &text_font = dialog_look.text_font;

  const unsigned line_height = rc.bottom - rc.top;
  const unsigned text_padding = Layout::GetTextPadding();
  const int left = rc.left + line_height + text_padding;
  const int top = rc.top + text_padding;

  canvas.Select(name_font);
  canvas.DrawText(left, top, item.name);

  if (CommonInterface::Basic().time_available) {
    canvas.Select(text_font);

    StaticString<64> buffer;
    buffer.UnsafeFormat(_("%u minutes ago"),
                        SinceInMinutes(CommonInterface::Basic().time,
                                       item.time_of_day_ms));

    canvas.DrawText(left, rc.top + name_font.GetHeight() + 2 * text_padding,
                    buffer);
  }
}

#endif /* HAVE_SKYLINES_TRACKING_HANDLER */

void
MapItemListRenderer::Draw(Canvas &canvas, const PixelRect rc,
                          const MapItem &item,
                          const DialogLook &dialog_look, const MapLook &look,
                          const TrafficLook &traffic_look,
                          const FinalGlideBarLook &final_glide_look,
                          const MapSettings &settings,
                          RoughTimeDelta utc_offset,
                          const TrafficList *traffic_list)
{
  switch (item.type) {
  case MapItem::LOCATION:
    Draw(canvas, rc, (const LocationMapItem &)item, dialog_look);
    break;
  case MapItem::ARRIVAL_ALTITUDE:
    Draw(canvas, rc, (const ArrivalAltitudeMapItem &)item,
         dialog_look, final_glide_look);
    break;
  case MapItem::SELF:
    Draw(canvas, rc, (const SelfMapItem &)item,
         dialog_look, look.aircraft, settings);
    break;
  case MapItem::AIRSPACE:
    Draw(canvas, rc, (const AirspaceMapItem &)item,
         dialog_look, look.airspace,
         settings.airspace);
    break;
  case MapItem::WAYPOINT:
    Draw(canvas, rc, (const WaypointMapItem &)item,
         dialog_look, look.waypoint,
         settings.waypoint);
    break;
  case MapItem::TASK_OZ:
    Draw(canvas, rc, (const TaskOZMapItem &)item,
         dialog_look, look.task, look.airspace,
         settings.airspace);
    break;

#ifdef HAVE_NOAA
  case MapItem::WEATHER:
    Draw(canvas, rc, (const WeatherStationMapItem &)item, dialog_look, look.noaa);
    break;
#endif

  case MapItem::TRAFFIC:
    Draw(canvas, rc, (const TrafficMapItem &)item,
         dialog_look, traffic_look, traffic_list);
    break;

#ifdef HAVE_SKYLINES_TRACKING_HANDLER
  case MapItem::SKYLINES_TRAFFIC:
    ::Draw(canvas, rc, (const SkyLinesTrafficMapItem &)item, dialog_look);
    break;
#endif

  case MapItem::THERMAL:
    Draw(canvas, rc, (const ThermalMapItem &)item, utc_offset,
         dialog_look, look);
    break;
  }
}
