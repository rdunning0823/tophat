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

#include "WaypointListRenderer.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "Look/DialogLook.hpp"
#include "Look/Look.hpp"
#include "Renderer/WaypointIconRenderer.hpp"
#include "Renderer/NextArrowRenderer.hpp"
#include "NMEA/Info.hpp"
#include "Engine/Waypoint/Waypoint.hpp"
#include "Geo/GeoVector.hpp"
#include "Formatter/UserUnits.hpp"
#include "Formatter/AngleFormatter.hpp"
#include "Language/Language.hpp"
#include "Util/StaticString.hxx"
#include "Util/Macros.hpp"
#include "UIGlobals.hpp"
#include "Interface.hpp"

#include <cstdio>


typedef StaticString<256u> Buffer;

static void
FormatWaypointDetails(Buffer &buffer, const Waypoint &waypoint, fixed arrival_altitude)
{
  TCHAR alt[16];
  FormatUserAltitude(arrival_altitude, alt, 16);
  buffer = alt;
}

UPixelScalar
WaypointListRenderer::GetHeight(const DialogLook &look)
{
  return look.list.font->GetHeight() + Layout::Scale(6)
    + look.text_font.GetHeight();
}

/**
 * Used by Alternates screen.  Draws data in columns
 */
void
WaypointListRenderer::Draw3(Canvas &canvas, const PixelRect rc,
                            const Waypoint &waypoint, fixed distance,
                            fixed arrival_altitude,
                            const DialogLook &dialog_look,
                            const WaypointLook &look,
                            const WaypointRendererSettings &settings,
                            unsigned col_2_width)
{
  const unsigned padding = Layout::GetTextPadding();
  const PixelScalar line_height = rc.bottom - rc.top;

  const Font &name_font = *dialog_look.list.font_bold;
  const Font &text_font = dialog_look.text_font;

  PixelScalar middle = rc.GetSize().cy > (int)name_font.GetHeight() ?
      rc.top + (rc.GetSize().cy - name_font.GetHeight()) / 2 : rc.top;

  PixelRect rc_name = rc;
  rc_name.left = rc.left + line_height + padding;
  rc_name.right = rc_name.left + rc_name.GetSize().cx / 2 - padding;
  PixelRect rc_info = rc;
  rc_info.left = rc_name.right + padding;

  // Use small font for details
  canvas.Select(text_font);

  // Draw distance and arrival altitude
  TCHAR dist[20], alt[20];
  FormatUserDistance(distance, dist, true, 0);
  FormatRelativeUserAltitude(arrival_altitude, alt, true);

  canvas.DrawClippedText(rc_info.left, middle, rc_info, dist);
  canvas.DrawClippedText(rc_info.left + col_2_width, middle, rc_info, alt);

  // Draw waypoint name
  canvas.Select(name_font);
  canvas.DrawClippedText(rc_name.left, middle, rc_name, waypoint.name.c_str());

  // Draw icon
  const RasterPoint pt(rc.left + line_height / 2,
                       rc.top + line_height / 2);

  WaypointIconRenderer::Reachability reachable =
      positive(arrival_altitude) ?
      WaypointIconRenderer::ReachableTerrain : WaypointIconRenderer::Unreachable;

  WaypointIconRenderer wir(settings, look, canvas);
  wir.Draw(waypoint, pt, reachable);
}

static void
DrawVectorArrow(Canvas &canvas, const PixelRect rc, const GeoVector *vector)
{
  const NMEAInfo &basic = CommonInterface::Basic();

  if (!vector->IsValid() || !basic.track_available)
    return;

  Angle bd = vector->bearing - basic.track;

  NextArrowRenderer renderer(UIGlobals::GetLook().wind_arrow_info_box);
  renderer.DrawArrow(canvas, rc, bd);
}

/**
 * Used by MapItemList
 */
void
WaypointListRenderer::Draw(Canvas &canvas, const PixelRect rc,
                           const Waypoint &waypoint, const GeoVector *vector,
                           const DialogLook &dialog_look,
                           const WaypointLook &look,
                           const WaypointRendererSettings &settings)
{
  bool draw_vector_arrow = true;
  const unsigned padding = Layout::GetTextPadding();
  const PixelScalar line_height = rc.bottom - rc.top;

  const Font &name_font = *dialog_look.list.font_bold;
  const Font &text_font = dialog_look.text_font;

  Buffer buffer;

  // Y-Coordinate of the first row
  PixelScalar top1 = rc.top;
  // Y-Coordinate of the second row
  PixelScalar top2 = rc.top + name_font.GetHeight() + Layout::FastScale(4);
  // Y-Coordinate of the only row where there is only one row
  PixelScalar top_middle = rc.top + (rc.GetSize().cy - name_font.GetHeight()) / 2;

  // Use small font for details
  canvas.Select(text_font);

  // Draw leg distance
  UPixelScalar leg_info_width = 0;
  if (vector) {
    unsigned arrow_width = 0;
    if (draw_vector_arrow) {
      PixelRect rc_arrow = rc;
      rc_arrow.top += rc.GetSize().cy / 4;
      rc_arrow.bottom -= rc.GetSize().cy / 4;
      rc_arrow.right = rc.right - padding;
      arrow_width = rc_arrow.GetSize().cy;
      rc_arrow.left = rc_arrow.right - arrow_width;

      DrawVectorArrow(canvas, rc_arrow, vector);
      arrow_width += padding * 2;
    } else
      arrow_width = padding;

    FormatUserDistanceSmart(vector->distance, buffer.buffer(), true);
    UPixelScalar width = leg_info_width = canvas.CalcTextWidth(buffer.c_str());
    canvas.DrawText(rc.right - arrow_width - width,
                    top_middle,
                    buffer.c_str());

    if (false) {
      // Draw leg bearing
      FormatBearing(buffer.buffer(), buffer.CAPACITY, vector->bearing);
      width = canvas.CalcTextWidth(buffer.c_str());
      canvas.DrawText(rc.right - arrow_width - width, top2,
                      buffer.c_str());

      if (width > leg_info_width)
        leg_info_width = width;
    }
    leg_info_width += padding;
  }
  top1 = top2 = top_middle;


  // Draw details line
  if (vector == nullptr) {
    FormatWaypointDetails(buffer, waypoint, waypoint.elevation);
    PixelSize sz_details = text_font.TextSize(buffer.c_str());

    PixelScalar details_left = rc.right - sz_details.cx - padding - leg_info_width;
    canvas.DrawClippedText(details_left, top2, sz_details.cx,
                           buffer.c_str());
  }

  // Draw waypoint name
  canvas.Select(name_font);
  canvas.DrawClippedText(rc.left + line_height, top1,
                         rc.left + (rc.right + rc.left) / 2,
                         waypoint.name.c_str());

  // Draw icon
  RasterPoint pt = { (PixelScalar)(rc.left + line_height / 2),
                     (PixelScalar)(rc.top + line_height / 2) };
  WaypointIconRenderer wir(settings, look, canvas);
  wir.Draw(waypoint, pt);
}

/**
 * Used by main waypoint list
 */
void
WaypointListRenderer::Draw2(Canvas &canvas, const PixelRect rc,
                            const Waypoint &waypoint, const GeoVector *vector,
                            fixed arrival_altitude,
                            const DialogLook &dialog_look,
                            const WaypointLook &look,
                            const WaypointRendererSettings &settings,
                            unsigned col_1_width,
                            unsigned col_2_width,
                            unsigned col_3_width)
{
  const unsigned padding = Layout::GetTextPadding();
  const PixelScalar line_height = rc.bottom - rc.top;

  const Font &name_font = *dialog_look.list.font_bold;
  const Font &text_font = dialog_look.text_font;

  PixelScalar middle = rc.GetSize().cy > (int)name_font.GetHeight() ?
      rc.top + (rc.GetSize().cy - name_font.GetHeight()) / 2 : rc.top;

  PixelRect rc_elevation = rc;
  rc_elevation.left = rc.left + col_1_width;
  rc_elevation.right = rc_elevation.left + col_2_width - padding;

  Buffer buffer;

  // Y-Coordinate of the second row
  PixelScalar top2 = rc.top + name_font.GetHeight() + Layout::FastScale(4);

  // Use small font for details
  canvas.Select(text_font);

  // Draw leg distance
  UPixelScalar leg_info_width = 0;
  if (vector) {
    FormatUserDistance(vector->distance, buffer.buffer(), true, 0);
    UPixelScalar width = leg_info_width = canvas.CalcTextWidth(buffer.c_str());
    canvas.DrawText(rc.right - padding - width,
                    rc.top + padding +
                    (name_font.GetHeight() - text_font.GetHeight()) / 2,
                    buffer.c_str());

    // Draw leg bearing
    FormatBearing(buffer.buffer(), buffer.CAPACITY, vector->bearing);
    width = canvas.CalcTextWidth(buffer.c_str());
    canvas.DrawText(rc.right - padding - width, top2,
                    buffer.c_str());

    if (width > leg_info_width)
      leg_info_width = width;

    leg_info_width += padding;
  }

  // Draw elevation (& possibly freq)
  FormatWaypointDetails(buffer, waypoint, arrival_altitude);
  canvas.DrawClippedText(rc_elevation.left, middle, rc_elevation.GetSize().cx,
                         buffer.c_str());

  // Draw waypoint name
  PixelScalar left = rc.left + line_height;
  canvas.Select(name_font);
  canvas.DrawClippedText(left, middle,
                         col_1_width - line_height - padding,
                         waypoint.name.c_str());

  // Draw icon
  RasterPoint pt = { (PixelScalar)(rc.left + line_height / 2),
                     (PixelScalar)(rc.top + line_height / 2) };
  WaypointIconRenderer wir(settings, look, canvas);
  wir.Draw(waypoint, pt);
}
