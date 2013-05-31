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

#ifndef XCSOAR_ZOOM_IN_BUTTON_WIDGET_HPP
#define XCSOAR_ZOOM_IN_BUTTON_WIDGET_HPP

#include "Widgets/MapOverlayWidget.hpp"
#include "Widgets/ZoomButton.hpp"
#include "Form/ActionListener.hpp"
#include "Projection/MapWindowProjection.hpp"
#include "Look/DialogLook.hpp"

struct IconLook;
class ContainerWindow;
struct PixelRect;

/**
 * a class that is a widget that draws the a zoom in button
 */
class ZoomInButtonWidget : public MapOverlayWidget, protected ActionListener {
protected:
  /**
   * size of the button (unscaled)
   */
  PixelSize button_size_raw;

  /**
   * a customized copy of dialog_look
   */
  DialogLook white_look;



public:

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();
  virtual void Show(const PixelRect &rc);
  virtual void Hide();
  virtual void Move(const PixelRect &rc);

  /**
   * How much height does this widget use at the bottom of the map screen
   */
  virtual UPixelScalar HeightFromBottomLeft();

  /**
   * returns width of button
   */
  UPixelScalar GetWidth() const;

  /**
   * returns height of button
   */
  UPixelScalar GetHeight() const;

  ZoomButton& CreateButton(ContainerWindow &parent,
                           const DialogLook &dialog_look,
                           const IconLook &icon_look,
                           const PixelRect &rc_map);

  /**
   * Shows or hides the widgets based on these parameters
   * @rc. the rc of the map
   * @is_panning.  is the map in panning mode
   * @is_main_window_widget. is the mainWindow's widget non-NULL
   * @is_map. is the map non-NULL
   */
  virtual void UpdateVisibility(const PixelRect &rc, bool is_panning,
                                bool is_main_window_widget, bool is_map);

  /**
   * The OnAction is derived from ActionListener
   * Toggles between the more/less menus (MapDisplay1, MapDisplay2)
   * and hiding the menu
   */
  virtual void OnAction(int id);
private:

};

#endif
