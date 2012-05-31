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

#include "Barogram.hpp"
#include "Base.hpp"
#include "Form/Frame.hpp"
#include "UIGlobals.hpp"
#include "Computer/GlideComputer.hpp"
#include "ComputerSettings.hpp"
#include "Components.hpp"
#include "Interface.hpp"
#include "Renderer/BarographRenderer.hpp"
#include "Look/Look.hpp"
#include "Util/StaticString.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Screen/Layout.hpp"

class BarogramPanel : public BaseAccessPanel {
  /**
   * a class to paint the barogram
   */
  class BarogramChart: public PaintWindow
  {
  public:
    BarogramChart(ContainerWindow &parent,
                  PixelScalar x, PixelScalar y,
                  UPixelScalar Width, UPixelScalar Height,
                  WindowStyle style,
                  const Look&_look);
  protected:
    const Look& look;

    /**
     * paints the barogram
     */
    virtual void OnPaint(Canvas &canvas);
  };

protected:
  WndFrame *info_frame;

  /**
  * control to render the barogram chart
  */
  BarogramChart *barogram_chart;

public:
  BarogramPanel(unsigned id)
    :BaseAccessPanel(id) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();

  void Refresh();

  /**
   * render the chart periodically
   */
  virtual bool OnTimer(WindowTimer &_timer);
};

bool
BarogramPanel::OnTimer(WindowTimer &_timer)
{
  if (_timer == timer) {
    Refresh();
    return true;
  } else
    return BaseAccessPanel::OnTimer(_timer);
}

void
BarogramPanel::Refresh()
{
  StaticString<1000> temp_string;

  BarographCaption(temp_string.buffer(), glide_computer->GetFlightStats(),
                   true);
  info_frame->SetCaption(temp_string.c_str());
  barogram_chart->Invalidate();
}

void
BarogramPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);

  PixelRect graph_rc = content_rc;
  PixelRect info_rc = content_rc;
  graph_rc.bottom -= Layout::Scale(40);
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
  barogram_chart =
      new BarogramChart(GetClientAreaWindow(),
                        graph_rc.left, graph_rc.top,
                        (UPixelScalar)(graph_rc.right - graph_rc.left),
                        (UPixelScalar)(graph_rc.bottom - graph_rc.top),
                        style_frame,
                        look);
  timer.Schedule(1000);
  Refresh();
}

void
BarogramPanel::Unprepare()
{
  delete info_frame;
  delete barogram_chart;
  BaseAccessPanel::Unprepare();
}

Widget *
LoadBarogramPanel(unsigned id)
{
  return new BarogramPanel(id);
}

BarogramPanel::BarogramChart::BarogramChart(ContainerWindow &parent,
                                            PixelScalar X, PixelScalar Y,
                                            UPixelScalar Width,
                                            UPixelScalar Height,
                                            WindowStyle style,
                                            const Look &_look)
  :look(_look)
{
  set(parent, X, Y, Width, Height, style);
}

void
BarogramPanel::BarogramChart::OnPaint(Canvas &canvas)
{
  assert(glide_computer != NULL);

  const FullBlackboard &blackboard = CommonInterface::Full();
  const MoreData &basic = blackboard.Basic();
  const DerivedInfo &calculated = blackboard.Calculated();

  canvas.Clear(COLOR_WHITE);
  canvas.SetTextColor(COLOR_BLACK);

  RenderBarograph(canvas, GetClientRect(), look.chart, look.cross_section,
                  glide_computer->GetFlightStats(),
                  basic, calculated, protected_task_manager);
}
