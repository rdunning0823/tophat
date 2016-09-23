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

#ifndef XCSOAR_ZOOM_IN_BUTTON_WIDGET_HPP
#define XCSOAR_ZOOM_IN_BUTTON_WIDGET_HPP

#include "TophatWidgets/OverlayButtonWidget.hpp"
#include "TophatWidgets/ScreensButtonWidget.hpp"
#include "Form/ActionListener.hpp"
#include "Projection/MapWindowProjection.hpp"
#include "Look/ButtonLook.hpp"

struct IconLook;
class ContainerWindow;
struct PixelRect;

/**
 * a class that is a widget that draws the a zoom in button
 */
class ZoomInButtonWidget : public OverlayButtonWidget {
protected:
  /**
   * pointer to the screen button widget so we can adjust
   * our position
   */
  ScreensButtonWidget *s_but;

public:
  ZoomInButtonWidget(ScreensButtonWidget *_s_but)
    :OverlayButtonWidget(), s_but(_s_but) {
    assert(s_but != nullptr);
  }

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) final;
  virtual void Move(const PixelRect &rc) final;

  /**
   * How much height does this widget use at the bottom of the map screen
   */
  virtual UPixelScalar HeightFromBottomLeft() final;
  /**
   * Shows or hides the widgets based on these parameters
   * @rc. the rc of the map
   * @is_panning.  is the map in panning mode
   * @is_main_window_widget. is the mainWindow's widget non-NULL
   * @is_map. is the map non-NULL
   */
  virtual void UpdateVisibility(const PixelRect &rc, bool is_panning,
                                bool is_main_window_widget, bool is_map,
                                bool is_top_widget) final;

  /**
   * The OnAction is derived from ActionListener
   * Toggles between the more/less menus (MapDisplay1, MapDisplay2)
   * and hiding the menu
   */
  virtual void OnAction(int id) final;
private:

};

#endif
