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

#include "WaypointNameAndIconWidget.hpp"
#include "Widget/TextWidget.hpp"
#include "Widget/WindowWidget.hpp"
#include "Waypoint/Waypoint.hpp"
#include "Renderer/WaypointIconRenderer.hpp"
#include "Look/WaypointLook.hpp"
#include "Look/DialogLook.hpp"
#include "Look/MapLook.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Features.hpp"

void
WaypointIconRendererWindow::OnPaint(Canvas &canvas)
{
  if (HaveClipping())
    canvas.Clear(dialog_look.background_brush);

  WaypointIconRenderer wir(wp_renderer_settings, waypoint_look, canvas);
  WaypointIconRenderer::Reachability reachability =
      (altitude_difference > fixed(0)) ?
          WaypointIconRenderer::Reachability::ReachableTerrain :
          WaypointIconRenderer::Reachability::Unreachable;

  RasterPoint pt (canvas.GetHeight() / 2, canvas.GetHeight() / 2);
  wir.Draw(waypoint, pt, reachability, false);
}

/** a class to encapsulate window widget */
class WaypointIconRendererWidget : public WindowWidget
{
public:
  WaypointIconRendererWidget(Window *_window)
  :WindowWidget(_window) {}

  PixelSize GetMinimumSize() const override {
    return PixelSize { Layout::GetMinimumControlHeight() / 2, Layout::GetMinimumControlHeight() / 2 };
  }
  PixelSize GetMaximumSize() const override {
    return PixelSize { Layout::GetMinimumControlHeight(), Layout::GetMinimumControlHeight() };
  }

};

/** a class to encapsulate Textwidget */
class WaypointNameRendererWidget : public TextWidget
{
public:

  PixelSize GetMinimumSize() const override {
    return PixelSize { Layout::GetMinimumControlHeight() / 2, Layout::GetMinimumControlHeight() / 2 };
  }
  PixelSize GetMaximumSize() const override {
    return PixelSize { Layout::GetMinimumControlHeight(), Layout::GetMinimumControlHeight() };
  }
};

/** methods for WaypointNameAndIconWidget class */
void
WaypointNameAndIconWidget::CalculateLayout(const PixelRect &rc)
{
  rc_icon = rc_text = rc;
  rc_icon.right = rc_icon.left + Layout::GetMinimumControlHeight();
  rc_text.left = rc_icon.right;
}

void
WaypointNameAndIconWidget::Move(const PixelRect &rc)
{
  TwoWidgets::Move(rc);
  CalculateLayout(rc);
  GetIconWidget().Move(rc_icon);
  GetTextWidget().Move(rc_text);
}

void
WaypointNameAndIconWidget::Initialise(ContainerWindow &parent, const PixelRect &rc)
{
  WindowStyle style;
  style.Hide();
  icon_window = new WaypointIconRendererWindow(parent, rc, waypoint,
                                               waypoint_look,
                                               dialog_look,
                                               wp_renderer_settings, style);

  WaypointIconRendererWidget *renderer_widget = new WaypointIconRendererWidget(
      icon_window);

  WaypointNameRendererWidget *t = new WaypointNameRendererWidget();
  t->SetBold(true);
  t->SetVAlignCenter();
  TwoWidgets::Set(renderer_widget, t);
  TwoWidgets::Initialise(parent, rc);
}

void
WaypointNameAndIconWidget::Prepare(ContainerWindow &parent,
                                    const PixelRect &rc)
{
  TwoWidgets::Prepare(parent, rc);
  CalculateLayout(rc);
  GetTextWidget().SetText(waypoint.name.c_str());
}

void
WaypointNameAndIconWidget::Show(const PixelRect &rc)
{
  TwoWidgets::Show(rc);

  assert(icon_window != nullptr);
  icon_window->SetArrivalAltitude(arrival_altitude);
}

void
WaypointNameAndIconWidget::SetArrivalAltitude(fixed alt) {
  arrival_altitude = alt;
}

