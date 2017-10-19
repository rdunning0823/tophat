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

#ifndef TOPHAT_WAYPOINT_NAME_AND_ICON_WIDGET
#define TOPHAT_WAYPOINT_NAME_AND_ICON_WIDGET

#include "Widget/TwoWidgets.hpp"
#include "Screen/PaintWindow.hpp"
#include "Screen/Layout.hpp"
#include "Math/fixed.hpp"

class Waypoint;
class WaypointLook;
class DialogLook;
class WaypointRendererSettings;
class WaypointIconRendererWidget;
class ContainerWindow;
class WindowWidget;
class TextWidget;

/**
 * a class to render a waypoint icon in its own window
 */
class WaypointIconRendererWindow: public PaintWindow
{
  public:
    WaypointIconRendererWindow(ContainerWindow &parent, const PixelRect rc,
                      const Waypoint &_waypoint,
                      const WaypointLook &_waypoint_look,
                      const DialogLook &_dialog_look,
                      const WaypointRendererSettings &_wp_renderer_settings,
                      WindowStyle style)
     :waypoint(_waypoint),
      waypoint_look(_waypoint_look),
      dialog_look(_dialog_look),
      wp_renderer_settings(_wp_renderer_settings),
      altitude_difference(fixed(0)){
      Create(parent, rc, style);
    }
    void SetArrivalAltitude(fixed alt) {
      altitude_difference = alt;
    }

  protected:
    const Waypoint &waypoint;
    const WaypointLook &waypoint_look;
    const DialogLook &dialog_look;
    const WaypointRendererSettings &wp_renderer_settings;
    fixed altitude_difference;

    /**
     * draws the icon
     */
    virtual void OnPaint(Canvas &canvas) override;
};

/**
 * A widget class that displays the waypoint icon and name
 */
class WaypointNameAndIconWidget : public TwoWidgets
{
  const Waypoint &waypoint;
  const WaypointLook &waypoint_look;
  const DialogLook &dialog_look;
  const WaypointRendererSettings &wp_renderer_settings;

  PixelRect rc_icon;
  PixelRect rc_text;
  fixed arrival_altitude;
  /* pointer to the icon window */
  WaypointIconRendererWindow *icon_window;

public:
  WaypointNameAndIconWidget(const Waypoint &_waypoint,
                            const WaypointLook &_waypoint_look,
                            const DialogLook &_dialog_look,
                            const WaypointRendererSettings &_wp_renderer_settings)
    :TwoWidgets(false, false),
     waypoint(_waypoint),
     waypoint_look(_waypoint_look),
     dialog_look(_dialog_look),
     wp_renderer_settings(_wp_renderer_settings),
     arrival_altitude(fixed(0)){};

  void Initialise(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  void Show(const PixelRect &rc) override;
  virtual void Move(const PixelRect &rc) override;

  virtual PixelSize GetMinimumSize() const override {
    return PixelSize { 25u, Layout::GetMinimumControlHeight() / 2 };
  }
  virtual PixelSize GetMaximumSize() const override {
    return PixelSize { 25u, Layout::GetMaximumControlHeight() };
  }
  void CalculateLayout(const PixelRect &rc);

  WaypointIconRendererWidget& GetIconWidget() {
    return (WaypointIconRendererWidget&)GetFirst();
  }
  TextWidget& GetTextWidget() {
    return (TextWidget&)GetSecond();
  }
  /**
   * sets the arrival altitude so the icon can be drawn accordingly
   */
  void SetArrivalAltitude(fixed alt);
};

#endif
