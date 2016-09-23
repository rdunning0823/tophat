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

#include "OnlineContest.hpp"
#include "Base.hpp"
#include "Form/Panel.hpp"
#include "Form/Frame.hpp"
#include "UIGlobals.hpp"
#include "Computer/GlideComputer.hpp"
#include "Components.hpp"
#include "Blackboard/FullBlackboard.hpp"
#include "Interface.hpp"
#include "Screen/PaintWindow.hpp"
#include "Renderer/FlightStatisticsRenderer.hpp"
#include "Look/Look.hpp"
#include "Util/StaticString.hxx"
#include "Screen/Color.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Timer.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/SingleWindow.hpp"

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
    virtual void OnPaint(Canvas &canvas) override;
  };

    PixelRect graph_rc;
    PixelRect info_rc;

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
   * This timer updates the data
   */
  WindowTimer dialog_timer;


  /**
   * render the chart periodically
   */
  virtual bool OnTimer(WindowTimer &timer) override;

public:
  OnlineContestPanel(unsigned id)
    :BaseAccessPanel(id), dialog_timer(*this) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Unprepare() override;
  /* Move must discard rc and use GetMainWindow()'s ClientRect */
  virtual void Move(const PixelRect &rc) override;
  void CalculateLayout(const PixelRect &rc);
  void Refresh();
};

bool
OnlineContestPanel::OnTimer(WindowTimer &timer)
{
  if (timer == dialog_timer) {
    Refresh();
    return true;
  } else
    return BaseAccessPanel::OnTimer(timer);
}

void
OnlineContestPanel::Refresh()
{
  StaticString<1000> temp_string;
  const FullBlackboard &blackboard = CommonInterface::Full();
  const DerivedInfo &calculated = blackboard.Calculated();
  const ComputerSettings &settings = CommonInterface::GetComputerSettings();

  FlightStatisticsRenderer::CaptionOLC(temp_string.buffer(),
                                       settings.contest, calculated,
                                       true);
  info_frame->SetCaption(temp_string);
  online_contest_chart->Invalidate();
}

void
OnlineContestPanel::Move(const PixelRect &rc_unused)
{
  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();

  BaseAccessPanel::Move(rc);
  CalculateLayout(rc);

  online_contest_chart->Move(graph_rc);
  info_frame->Move(info_rc);
}

void
OnlineContestPanel::CalculateLayout(const PixelRect &rc)
{
  graph_rc = content_rc;
  info_rc = content_rc;

  graph_rc.bottom -= Layout::Scale(50);
  info_rc.top = graph_rc.bottom + 1;
}

void
OnlineContestPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);
  CalculateLayout(rc);

  WindowStyle style;

  WindowStyle style_frame;
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  info_frame = new WndFrame(GetClientAreaWindow(), dialog_look,
                            info_rc, style_frame);

  const Look &look = UIGlobals::GetLook();
  online_contest_chart =
    new OnlineContestChart(GetClientAreaWindow(),
                           graph_rc.left, graph_rc.top,
                           (UPixelScalar)(graph_rc.right - graph_rc.left),
                           (UPixelScalar)(graph_rc.bottom - graph_rc.top),
                           style, look);

  dialog_timer.Schedule(1000);
  online_contest_chart->Move(graph_rc);
  Refresh();
}

void
OnlineContestPanel::Unprepare()
{
  dialog_timer.Cancel();
  delete(info_frame);
  delete(online_contest_chart);
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
  const MoreData &basic = CommonInterface::Basic();
  const DerivedInfo &calculated = CommonInterface::Calculated();
  const ComputerSettings &settings = CommonInterface::GetComputerSettings();

  canvas.Clear(COLOR_WHITE);

  const FlightStatisticsRenderer fs(look.chart, look.map);

  fs.RenderOLC(canvas, GetClientRect(), basic,
               settings, blackboard.GetMapSettings(),
               calculated.contest_stats,
               glide_computer->GetTraceComputer(),
               glide_computer->GetRetrospective());
}

OnlineContestPanel::OnlineContestChart::OnlineContestChart(
  ContainerWindow &parent,
  PixelScalar x,
  PixelScalar y,
  UPixelScalar width,
  UPixelScalar height,
  WindowStyle style,
  const Look& _look)
  :look(_look)
{
  PixelRect rc (x, y, x + width, y + width);
  Create(parent, rc, style);
}
