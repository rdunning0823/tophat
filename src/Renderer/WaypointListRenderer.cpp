/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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
#include "Renderer/WaypointIconRenderer.hpp"
#include "Engine/Waypoint/Waypoint.hpp"
#include "Geo/GeoVector.hpp"
#include "Formatter/UserUnits.hpp"
#include "Formatter/AngleFormatter.hpp"
#include "Language/Language.hpp"
#include "Util/StaticString.hpp"
#include "Util/Macros.hpp"

#include <cstdio>

namespace WaypointListRenderer
{
/**
 * Used for main waypoint list renderer
 */
  void Draw2(Canvas &canvas, const PixelRect rc, const Waypoint &waypoint,
             const GeoVector *vector, fixed arrival_altitude,
             const DialogLook &dialog_look, const WaypointLook &look,
             const WaypointRendererSettings &settings, unsigned col_1_width,
             unsigned col_2_width, unsigned col_3_width);

  /**
   *  * Used by MapItemList
   */
  void Draw(Canvas &canvas, const PixelRect rc, const Waypoint &waypoint,
            const GeoVector *vector,
            const DialogLook &dialog_look, const WaypointLook &look,
            const WaypointRendererSettings &settings);

}

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
    + look.text_font->GetHeight();
}

/**
 * Used by the map items list
 */
void
WaypointListRenderer::Draw(Canvas &canvas, const PixelRect rc,
                           const Waypoint &waypoint,
                           const DialogLook &dialog_look,
                           const WaypointLook &look,
                           const WaypointRendererSettings &renderer_settings)
{
  Draw(canvas, rc, waypoint, NULL, dialog_look, look, renderer_settings);
}

/**
 * Calls Draw() that is used by main waypoint list
 */
void
WaypointListRenderer::Draw2(Canvas &canvas, const PixelRect rc,
                            const Waypoint &waypoint, const GeoVector &vector,
                            fixed arrival_altitude,
                            const DialogLook &dialog_look,
                            const WaypointLook &look,
                            const WaypointRendererSettings &settings,
                            unsigned col_1_width,
                            unsigned col_2_width,
                            unsigned col_3_width)
{
  Draw2(canvas, rc, waypoint, &vector, arrival_altitude, dialog_look, look, settings,
        col_1_width, col_2_width, col_3_width);
}

/**
 * Used by Alternates screen.  Draws data in columns
 */
void
WaypointListRenderer::Draw2(Canvas &canvas, const PixelRect rc,
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
  const Font &text_font = *dialog_look.text_font;

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
  StaticString<256> buffer;
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
  const unsigned padding = Layout::GetTextPadding();
  const PixelScalar line_height = rc.bottom - rc.top;

  const Font &name_font = *dialog_look.list.font_bold;
  const Font &text_font = *dialog_look.text_font;

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
    FormatUserDistanceSmart(vector->distance, buffer.buffer(), true);
    UPixelScalar width = leg_info_width = canvas.CalcTextWidth(buffer.c_str());
    canvas.DrawText(rc.right - padding - width,
                    rc.top + padding +
                    (name_font.GetHeight() - text_font.GetHeight()) / 2,
                    buffer.c_str());

    // Draw leg bearing
    FormatBearing(buffer.buffer(), buffer.MAX_SIZE, vector->bearing);
    width = canvas.CalcTextWidth(buffer.c_str());
    canvas.DrawText(rc.right - padding - width, top2,
                    buffer.c_str());

    if (width > leg_info_width)
      leg_info_width = width;

    leg_info_width += padding;
  } else {
    // draw everything on one row
    top1 = top2 = top_middle;
  }


  // Draw details line
  FormatWaypointDetails(buffer, waypoint, waypoint.elevation);
  PixelSize sz_details = text_font.TextSize(buffer.c_str());

  PixelScalar details_left = rc.right - sz_details.cx - padding - leg_info_width;
  canvas.DrawClippedText(details_left, top2, sz_details.cx,
                         buffer.c_str());

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
  const Font &text_font = *dialog_look.text_font;

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
    FormatBearing(buffer.buffer(), buffer.MAX_SIZE, vector->bearing);
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
