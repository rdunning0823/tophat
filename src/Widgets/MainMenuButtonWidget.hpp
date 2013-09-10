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

#ifndef XCSOAR_MAIN_MENU_BUTTON_WIDGET_HPP
#define XCSOAR_MAIN_MENU_BUTTON_WIDGET_HPP

#include "Widgets/MapOverlayWidget.hpp"
#include "Form/Button.hpp"
#include "Form/ActionListener.hpp"
#include "Look/ButtonLook.hpp"

#include <tchar.h>

struct IconLook;
class ContainerWindow;
struct PixelRect;

class MainMenuButton : public WndButton {
protected:
  const IconLook &icon_look;
  const ButtonLook &button_look;

public:

  MainMenuButton(ContainerWindow &parent, const ButtonLook &_button_look,
                 const IconLook &_icon_look,
                 const TCHAR *caption, const PixelRect &rc,
                 ButtonWindowStyle style,
                 ActionListener& listener, int id)
  :WndButton(parent, _button_look, caption, rc, style, listener, id),
   icon_look(_icon_look), button_look(_button_look) {}

  /**
   * The OnPaint event is called when the button needs to be drawn
   * (derived from PaintWindow)
   */
  virtual void OnPaint(Canvas &canvas);

  /**
   * handles on mouse move, and if dragged off button face, cancels drag
   * This allows background object to accept capture at this time
   */
  virtual bool OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys);
};

class MainMenuButtonWidget : public MapOverlayWidget, protected ActionListener {
protected:
  /**
   * size of bitmap on which size of widget is based (unscaled)
   */
  PixelSize bitmap_size_raw;

  /**
   * a customized copy of button_look
   */
  ButtonLook white_look;

public:

  /**
   * Shows or hides the widgets based on these parameters
   * @rc. the rc of the map
   * @is_panning.  is the map in panning mode
   * @is_main_window_widget. is the mainWindow's widget non-NULL
   * @is_map. is the map non-NULL
   */
  virtual void UpdateVisibility(const PixelRect &rc, bool is_panning,
                                bool is_main_window_widget, bool is_map);
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();
  virtual void Show(const PixelRect &rc);
  virtual void Hide();
  virtual void Move(const PixelRect &rc);
  MainMenuButton& CreateButton(ContainerWindow &parent,
                               const ButtonLook &button_look,
                               const IconLook &icon_look,
                               const PixelRect &rc);


  /**
   * How much height does this widget use at the bottom right of the map screen
   */
  virtual UPixelScalar HeightFromBottomRight();

  /**
   * returns width of button
   */
  UPixelScalar GetWidth() const;

  /**
   * returns height of button
   */
  UPixelScalar GetHeight() const;

  /**
   * The OnAction is derived from ActionListener
   */
  virtual void OnAction(int id);
private:

};

#endif
