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

#ifndef XCSOAR_BASE_PANEL_HPP
#define XCSOAR_BASE_PANEL_HPP

#include "Screen/Point.hpp"
#include "Form/Widget.hpp"
#include "Form/Form.hpp"

class WndButton;
class WndFrame;


/**
 * This is a base class that can be inherited by InfoBox Panel Widgets.
 * The class creates the Header for the InfoBox Panel and the "Setup"
 * button and handles the events for the Setup button.
 * It also calculates the "content_rc"  (below the header) where derived
 * classes may display controls related to the InfoBox
 */
class BaseAccessPanel : public NullWidget, protected WndForm {
protected:
  unsigned id;
  WndButton *setup_button;
  WndFrame *header_text;

  /**
   * Follow widget API for calling.
   */
  virtual void Hide();

  /**
   * the rect of the usable windows space below the header
   * Derived classes must only display items within this rect
   */
  PixelRect content_rc;

  /**
   * Call this at the end of any derived class Show() operations
   * because this starts the dialog loop.
   */
  virtual void Show(const PixelRect &rc);

  /**
   * Called by a derived class at the beginning of its OnPaint
   */
  virtual void OnPaint(Canvas &canvas);

public:
  BaseAccessPanel(unsigned _id);

  /**
   * Called at the start of a derived class Prepare()
   * Draws the header text and the Setup button
   */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);

  /*
   * Called at the end of derived class Unprepare()
   * Unprepares the header and tells InfoBox window that the popup is closed.
   */
  virtual void Unprepare();

  gcc_pure PixelScalar GetHeaderHeight();

protected:

  /**
   * Called at the end of the derived class OnAction.
   */
  virtual void OnAction(int id);

  /**
   * Closes the dialog
   */
  void Close();
};


/**
 *  Base class that sizes the rectangles needed to layout four buttons
 *  and a frame so there are two "up" buttons and two "down" buttons
 *  above and below the frame.
 *  The user is responsible for creating the buttons and frame
 *  objects.  Layout looks like:
 *    +    +
 *     xxxx
 *    -    -
 */
class NumberButtonLayout {
protected:


  /**
   *  positioned above the frame, 50% of its width, and flush left
   */
  PixelRect big_plus_rc;

  /**
   *  positioned above the frame, 50% of its width, and flush right
   */
  PixelRect little_plus_rc;

  /**
   *  positioned below the frame, 50% of its width, and flush left
   */
  PixelRect big_minus_rc;

  /**
   *  positioned below the frame, 50% of its width, and flush right
   */
  PixelRect little_minus_rc;

  /**
   * the frame between the buttons that contains the value to be changed
   * when the buttons are pressed
   */
  PixelRect value_rc;

protected:
  /*
   * Sizes the rectangles for the layout of the buttons and the frame in the center
   * of the rc
   * @param parent_rc The rc of the parent window's usable area
   */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &parent_rc);
};
#endif
