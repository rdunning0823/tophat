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

#ifndef XCSOAR_BASE_PANEL_HPP
#define XCSOAR_BASE_PANEL_HPP

#include "Screen/Point.hpp"
#include "Widget/Widget.hpp"
#include "Form/Form.hpp"
#include "Widget/ManagedWidget.hpp"
#include "Util/StaticString.hxx"

class Button;
class WndFrame;


/**
 * This is a base class that can be inherited by InfoBox Panel Widgets.
 * The class creates the Header for the InfoBox Panel and the "Setup"
 * button and handles the events for the Setup button.
 * It also calculates the "content_rc"  (below the header) where derived
 * classes may display controls related to the InfoBox
 */
class BaseAccessPanel : public NullWidget, public WndForm
{
protected:
  unsigned id;

  // only visible if infobox HasCustomContent() == true
  Button *help_button;

  Button *setup_button;
  Button *close_button;
  WndFrame *header_text;
  ManagedWidget managed_widget;

  /* rectangles for items on the base page */
  PixelRect base_rc, close_button_rc, setup_button_rc, frame_rc;
  PixelRect help_button_rc;

  /* caption text for infobox */
  StaticString<64> caption_text;

  /**
   * Follow widget API for calling.
   */
  virtual void Hide() override;

  /**
   * the rect of the usable windows space below the header
   * Derived classes must only display items within this rect
   */
  PixelRect content_rc;

  /**
   * Call this at the end of any derived class Show() operations
   * because this starts the dialog loop.
   */
  virtual void Show(const PixelRect &rc) override;

  /* this is a hack.  The derived class must discard rc and use GetMainWindow() */
  virtual void Move(const PixelRect &rc) override;

  /**
   * Called by a derived class at the beginning of its OnPaint
   */
  virtual void OnPaint(Canvas &canvas) override;

  /**
   * Caption for InfoBox dialog
   */
  virtual void SetCaption();

  /**
   * Display dialog popup showing help text
   * Only used when Infobox has a custom panel
   */
  virtual void ShowHelp();

  /** Does this infobox have custom content
   * or does it just display the helptext in the content area
   */
  virtual bool HasCustomContent() {
    return true;
  }

public:
  BaseAccessPanel(unsigned _id);

  BaseAccessPanel(unsigned _id, Widget *widget);

  /**
   * Called at the start of a derived class Prepare()
   * Draws the header text and the Setup button
   */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;

  virtual void Unprepare() override;
  /**
   * calculates the rectangles on the base class itself
   */
  void CalculateLayout(const PixelRect &rc);

  gcc_pure PixelScalar GetHeaderHeight();

protected:

  /**
   * Called at the end of the derived class OnAction.
   */
  virtual void OnAction(int id) override;

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
 *
 *  An alternative layout with only one "up" and one "dn" button is also provided:
 *      +
 *    xxxxx
 *      -
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


  /**
   *  positioned above the value frame, 100% of its width
   */
  PixelRect double_size_plus_rc;

  /**
   *  positioned below the value frame, 100% of its width
   */
  PixelRect double_size_minus_rc;

protected:
  /*
   * Sizes the rectangles for the layout of the buttons and the frame in the center
   * of the rc
   * @param parent_rc The rc of the parent window's usable area
   * @param value_height.  If > 0, defines min value height
   */
  void CalculateLayout(const PixelRect &parent_rc, unsigned min_value_height);
};

/**
 * Layout class that inherits NumberButtonLayout, and adds a sub_number to
 * the right of the value
 */
class NumberButtonSubNumberLayout : public NumberButtonLayout {
protected:
/**
 *  positioned flush right on screen, at same X as big value
 *  Layout looks like (where yy is the sub number).
 *    +    +
 *     xxxx  yy
 *    -    -
 */
  PixelRect sub_number_rc;
protected:
  /*
   * Sizes the rectangles for the layout
   * @param value_height.  If > 0, defines min value height
   */
  void CalculateLayout(const PixelRect &parent_rc, unsigned min_value_height);
};

/**
 * Layout class that inherits SubNumberButtonLayout, and splits the sub_number int
 * two vertically stacked numbers to the right of the value
 */
class NumberButton2SubNumberLayout : public NumberButtonSubNumberLayout {
protected:
/**
 *  positioned flush right on screen, at same X as big value
 *  Layout looks like (where yy and zz are the two sub number).
 *    +    +
 *     xxxx  yy
 *           zz
 *    -    -
 */
  PixelRect sub_number_top_rc;
  PixelRect sub_number_bottom_rc;
protected:
  /*
   * Sizes the rectangles for the layout
   * @param value_height.  If > 0, defines min value height
   * @param sub_number_height. height of each of the two sub numbers on the right
   */
  void CalculateLayout(const PixelRect &parent_rc, unsigned min_value_height,
                       unsigned sub_number_height);
};

/**
 *  Base class that sizes the rectangles needed to layout a
 *  list with two big 1/2 height buttons up/dn on the right,
 *  and one full width button below the list.
 *  To be used as an alternative to a Scrollbar for the list.
 *  These controls are flush with the borders of the parent rc
 */
class RatchetListLayout {
protected:
  /**
   * "Up" button located to the right of the list, 50% of the height of the
   * list, flush with the top
   */
  PixelRect ratchet_up_rc;

  /**
   * "Dn" button located to the right of the list, 50% of the height of the
   * list, flush with the bottom
   */
  PixelRect ratchet_down_rc;

  /**
   * The list is in the upper left of the area
   */
  PixelRect ratchet_list_rc;

protected:
  /*
   * Sizes the rectangles for the layout in the center
   * of the rc
   * @param parent_rc The rc of the parent window's usable area
   */
  void CalculateLayout(const PixelRect &rc);
};

/**
 * A layout with a widget area and three command buttons under it
 */
class ThreeCommandButtonWidgetLayout {
protected:
  /**
   * left button below the list
   */
  PixelRect left_button_rc;

  /**
   * right button below the list.
   */
  PixelRect right_button_rc;

  /**
   * middle button below the list.
   */
  PixelRect middle_button_rc;

  /**
   * The widget area
   */
  PixelRect widget_rc;

protected:
  /*
   * Sizes the rectangles for the layout in the center
   * of the rc
   * @param parent_rc The rc of the parent window's usable area
   */
  void CalculateLayout(const PixelRect &rc);
};

/**
 * A layout with a widget area and 4 command buttons under it
 */
class FourCommandButtonWidgetLayout {
protected:
  /**
   * left button below the list
   */
  PixelRect left_button_rc;

  /**
   * right button below the list.
   */
  PixelRect right_button_rc;

  /**
   * middle button below the list.
   */
  PixelRect middle_button_rc;

  /**
   * second button from left
   */
  PixelRect left_middle_button_rc;

  /**
   * The widget area
   */
  PixelRect widget_rc;

protected:
  /*
   * Sizes the rectangles for the layout in the center
   * of the rc
   * @param parent_rc The rc of the parent window's usable area
   */
  void CalculateLayout(const PixelRect &rc);
};


/**
 * A layout with a list and two command buttons under it;
 * E.g. For alternates list with "Goto" and "Details" below.
 */
class TwoCommandButtonListLayout {
protected:
  /**
   * left button below the list
   */
  PixelRect left_button_rc;

  /**
   * right button below the list.
   */
  PixelRect right_button_rc;

  /**
   * The list is in the upper area
   */
  PixelRect list_rc;

protected:
  /*
   * Sizes the rectangles for the layout in the center
   * of the rc
   * @param parent_rc The rc of the parent window's usable area
   */
  void CalculateLayout(const PixelRect &rc);
};

/**
 *  Base class that sizes the rectangles needed to layout
 *  two buttons or frames of button size in the middle of the
 *  box, full width.
 */
class TwoButtonLayout {
protected:
  /**
   * upper of the two rects
   */
  PixelRect upper_rc;

  /**
   * lower of the two rects
   */
  PixelRect lower_rc;


protected:
  /*
   * Sizes the rectangles for the layout in the center
   * of the rc
   * @param rc The rc of the parent window's usable area
   */
  void CalculateLayout(const PixelRect &rc);
};

/**
 *  Base class that sizes the rectangles needed to layout
 *  three buttons in two rows.
 *  Vertically in the center of the screen.
 *  Upper button is full width.
 *  Lower two buttons are left/right half width bdlow upper button
 */
class ThreeButtonLayout {
protected:
  /**
   * upper rect, full width
   */
  PixelRect upper_rc;

  /**
   * lower two rects, half screen width
   */
  PixelRect lower_left_rc;
  PixelRect lower_right_rc;


protected:
  /*
   * Sizes the rectangles for the layout in the center
   * of the rc
   * @param rc The rc of the parent window's usable area
   */
  void CalculateLayout(const PixelRect &rc);
};

#endif
