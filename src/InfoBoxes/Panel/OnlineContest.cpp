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

#include "OnlineContest.hpp"
#include "Base.hpp"
#include "Form/Panel.hpp"
#include "Form/Frame.hpp"
#include "UIGlobals.hpp"
#include "Computer/GlideComputer.hpp"
#include "ComputerSettings.hpp"
#include "Components.hpp"
#include "Blackboard/FullBlackboard.hpp"
#include "Interface.hpp"
#include "Screen/PaintWindow.hpp"
#include "Renderer/FlightStatisticsRenderer.hpp"
#include "Look/Look.hpp"
#include "Util/StaticString.hpp"
#include "Screen/Color.hpp"
#include "Screen/Layout.hpp"

/**
 * Class to draw the chart of the OLC progress
 */
class OnlineContestPanel : public BaseAccessPanel {
    class OnlineContestChart: public PaintWindow
  {
  public:
    OnlineContestChart(ContainerWindow &parent,
                       PixelScalar x, PixelScalar y,
                       UPixelScalar Width, UPixelScalar Height,
                       WindowStyle style,
                       const Look&);
  protected:
    const Look& look;

    /**
     * draws the the OLC image
     */
    virtual void OnPaint(Canvas &canvas);
  };

protected:

  /**
   * draws the OLC chart on the widget
   */
  OnlineContestChart *online_contest_chart;

  /**
   * displays text statistics about the OLC
   */
  WndFrame *info_frame;

  /**
   * render the chart periodically
   */
  virtual bool OnTimer(WindowTimer &_timer);

public:
  OnlineContestPanel(unsigned id)
    :BaseAccessPanel(id) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();

  void Refresh();
};

bool
OnlineContestPanel::OnTimer(WindowTimer &_timer)
{
  if (_timer == timer) {
    Refresh();
    return true;
  } else
    return BaseAccessPanel::OnTimer(_timer);
}

void
OnlineContestPanel::Refresh()
{
  StaticString<1000> temp_string;
  const FullBlackboard &blackboard = CommonInterface::Full();
  const DerivedInfo &calculated = blackboard.Calculated();
  const ComputerSettings &settings_computer = blackboard.GetComputerSettings();

  FlightStatisticsRenderer::CaptionOLC(temp_string.buffer(),
                                       settings_computer.task, calculated,
                                       true);
  info_frame->SetCaption(temp_string);
  online_contest_chart->Invalidate();
}

void
OnlineContestPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);

  PixelRect graph_rc = content_rc;
  PixelRect info_rc = content_rc;
  graph_rc.bottom -= Layout::Scale(50);
  info_rc.top = graph_rc.bottom + 1;

  WindowStyle style;

  WindowStyle style_frame;
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  info_frame = new WndFrame(GetClientAreaWindow(), dialog_look,
                            info_rc.left, info_rc.top,
                            info_rc.right - info_rc.left,
                            info_rc.bottom - info_rc.top,
                            style_frame);

  const Look &look = UIGlobals::GetLook();
  online_contest_chart =
    new OnlineContestChart(GetClientAreaWindow(),
                           graph_rc.left, graph_rc.top,
                           (UPixelScalar)(graph_rc.right - graph_rc.left),
                           (UPixelScalar)(graph_rc.bottom - graph_rc.top),
                           style, look);
  timer.Schedule(1000);
  Refresh();
}

void
OnlineContestPanel::Unprepare()
{
  delete info_frame;
  delete online_contest_chart;
  BaseAccessPanel::Unprepare();
}

Widget *
LoadOnlineContestPanel(unsigned id)
{
  return new OnlineContestPanel(id);
}

void
OnlineContestPanel::OnlineContestChart::OnPaint(Canvas &canvas)
{
  assert(glide_computer != NULL);
  const FullBlackboard &blackboard = CommonInterface::Full();
  const MoreData &basic = blackboard.Basic();
  const DerivedInfo &calculated = blackboard.Calculated();
  const ComputerSettings &settings_computer = blackboard.GetComputerSettings();
  const MapSettings &settings_map = blackboard.GetMapSettings();

  canvas.Clear(COLOR_WHITE);

  const FlightStatisticsRenderer fs(look.chart, look.map);

  fs.RenderOLC(canvas, GetClientRect(), basic, calculated,
               settings_computer, settings_map,
               calculated.contest_stats,
               glide_computer->GetTraceComputer());
}

OnlineContestPanel::OnlineContestChart::OnlineContestChart(
  ContainerWindow &parent,
  PixelScalar X,
  PixelScalar Y,
  UPixelScalar Width,
  UPixelScalar Height,
  WindowStyle style,
  const Look& _look)
  :look(_look)
{
  set(parent, X, Y, Width, Height, style);
}
