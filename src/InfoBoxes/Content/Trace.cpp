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

#include "InfoBoxes/Content/Trace.hpp"
#include "InfoBoxes/InfoBoxWindow.hpp"
#include "InfoBoxes/Panel/Barogram.hpp"
#include "Renderer/BarographRenderer.hpp"
#include "Renderer/TraceHistoryRenderer.hpp"
#include "Renderer/ThermalBandRenderer.hpp"
#include "Renderer/TaskProgressRenderer.hpp"
#include "Formatter/UserUnits.hpp"
#include "Units/Units.hpp"
#include "Components.hpp"
#include "Interface.hpp"
#include "Blackboard/DeviceBlackboard.hpp"
#include "Screen/Layout.hpp"
#include "Protection.hpp"
#include "UIGlobals.hpp"
#include "Look/Look.hpp"
#include "Computer/GlideComputer.hpp"
#include "Dialogs/dlgInfoBoxAccess.hpp"
#include "Dialogs/dlgAnalysis.hpp"
#include "Util/Macros.hpp"
#include "Language/Language.hpp"

static PixelRect
get_spark_rect(const InfoBoxWindow &infobox)
{
  PixelRect rc = infobox.GetValueRect();
  rc.top += Layout::FastScale(2);
  rc.right -= Layout::FastScale(2);
  rc.left += Layout::FastScale(2);
  return rc;
}

void
InfoBoxContentSpark::do_paint(InfoBoxWindow &infobox, Canvas &canvas,
                              const TraceVariableHistory& var,
                              const bool center)
{
  if (var.empty())
    return;

  const Look &look = UIGlobals::GetLook();
  TraceHistoryRenderer renderer(look.trace_history, look.vario, look.chart);
  renderer.RenderVario(canvas, get_spark_rect(infobox), var, center,
                       CommonInterface::GetComputerSettings().polar.glide_polar_task.GetMC());
}

void
InfoBoxContentVarioSpark::OnCustomPaint(InfoBoxWindow &infobox, Canvas &canvas)
{
  do_paint(infobox, canvas, CommonInterface::Calculated().trace_history.BruttoVario);
}

void
InfoBoxContentNettoVarioSpark::OnCustomPaint(InfoBoxWindow &infobox, Canvas &canvas)
{
  do_paint(infobox, canvas, CommonInterface::Calculated().trace_history.NettoVario);
}

void
InfoBoxContentCirclingAverageSpark::OnCustomPaint(InfoBoxWindow &infobox, Canvas &canvas)
{
  do_paint(infobox, canvas, CommonInterface::Calculated().trace_history.CirclingAverage,
    false);
}

void
InfoBoxContentSpark::label_vspeed(InfoBoxData &data,
                                  const TraceVariableHistory& var)
{
  if (var.empty())
    return;

  TCHAR sTmp[32];
  FormatUserVerticalSpeed(var.last(), sTmp,
                          ARRAY_SIZE(sTmp));
  data.SetComment(sTmp);

  data.SetCustom();
}

void
InfoBoxContentVarioSpark::Update(InfoBoxData &data)
{
  label_vspeed(data, CommonInterface::Calculated().trace_history.BruttoVario);
}

void
InfoBoxContentNettoVarioSpark::Update(InfoBoxData &data)
{
  label_vspeed(data, CommonInterface::Calculated().trace_history.NettoVario);
}

void
InfoBoxContentCirclingAverageSpark::Update(InfoBoxData &data)
{
  label_vspeed(data, CommonInterface::Calculated().trace_history.CirclingAverage);
}


static constexpr InfoBoxContentBarogram::PanelContent panels[] = {
  InfoBoxContentBarogram::PanelContent (
    N_("Edit"),
    LoadBarogramPanel),
};

const InfoBoxContentBarogram::DialogContent InfoBoxContentBarogram::dlgContent = {
  ARRAY_SIZE(panels), &panels[0], false,
};

const InfoBoxContentBarogram::DialogContent*
InfoBoxContentBarogram::GetDialogContent() {
  return &dlgContent;
}

void
InfoBoxContentBarogram::Update(InfoBoxData &data)
{
  const MoreData &basic = CommonInterface::Basic();
  TCHAR sTmp[32];

  if (basic.NavAltitudeAvailable()) {
    FormatUserAltitude(basic.nav_altitude, sTmp,
                       ARRAY_SIZE(sTmp));
    data.SetComment(sTmp);
  } else
    data.SetCommentInvalid();

  data.SetCustom();
}

void
InfoBoxContentBarogram::OnCustomPaint(InfoBoxWindow &infobox, Canvas &canvas)
{
  const Look &look = UIGlobals::GetLook();
  RenderBarographSpark(canvas, get_spark_rect(infobox),
                       look.chart, look.cross_section,
                       infobox.GetLook().inverse,
                       glide_computer->GetFlightStats(),
                       XCSoarInterface::Basic(),
                       XCSoarInterface::Calculated(), protected_task_manager);
}


void
InfoBoxContentThermalBand::OnCustomPaint(InfoBoxWindow &infobox, Canvas &canvas)
{
  const Look &look = UIGlobals::GetLook();
  ThermalBandRenderer renderer(look.thermal_band, look.chart);
  renderer.DrawThermalBandSpark(CommonInterface::Basic(),
                                CommonInterface::Calculated(),
                                CommonInterface::GetComputerSettings(),
                                canvas,
                                infobox.GetValueAndCommentRect(),
                                XCSoarInterface::GetComputerSettings().task);
}

void
InfoBoxContentThermalBand::Update(InfoBoxData &data)
{
  data.SetCustom();
}


void
InfoBoxContentTaskProgress::OnCustomPaint(InfoBoxWindow &infobox, Canvas &canvas)
{
  const Look &look = UIGlobals::GetLook();
  TaskProgressRenderer renderer(look.map.task);
  renderer.Draw(CommonInterface::Calculated().
                common_stats.ordered_summary,
                canvas, infobox.GetValueAndCommentRect(),
                infobox.GetLook().inverse);
}

void
InfoBoxContentTaskProgress::Update(InfoBoxData &data)
{
  data.SetCustom();
}
