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

#ifndef XCSOAR_HORIZONTAL_LIST_HPP
#define XCSOAR_HORIZONTAL_LIST_HPP

#include "Form/List.hpp"
#include "Time/PeriodClock.hpp"

struct DialogLook;
class ContainerWindow;
struct PixelRect;
class WndSymbolButton;

/**
 * List that is drawn and scrolls horizontally
 * Scrollbar is not supported

 */
class HorizontalListControl : public ListControl {

protected:

  /**
   * amount the screen can scroll beyond the min/max scroll positions of the list
   */
  UPixelScalar over_scroll_max;

  /**
   * the origin to which to bounce
   */
  PixelScalar bounce_pixel_origin;

  /**
   * index of item with cursor down (mouse down)
   * -1 if no item has mouse down.
   */
  int cursor_down_index;

  /**
   * measures the duration of the mouse down
   */
  PeriodClock mouse_down_clock;

  /**
   * minimum duration (ms) for click before calls OnActivateItem
   */
  unsigned click_duration;

public:

  /**
   * allows the list to over & underscroll scroll by this number of pixels
   */
  void SetOverScrollMax(UPixelScalar pixels);

  /**
   * return index of item centered or closest to the center
   */
  unsigned GetCenteredItem();

  /**
   * returns index of list item that has dragmode cursor
   * meaning it is mousedown but not moving
   * @return -1 if no item has mousedown on it
   */
  int GetCursorDownIndex() {
    return cursor_down_index;
  }

  /**
   * Sets the minimum duration (ms) of a mouse down required
   * for the mouse up event to trigger OnActivateItem
   */
  void SetClickDuration(unsigned duration) {
    click_duration = duration;
  }


  /**
   * @param parent the parent window
   * @param _item_height Height of an item of the ListFrameControl
   */
  HorizontalListControl(ContainerWindow &parent, const DialogLook &_look,
                        PixelRect _rc, const WindowStyle _style,
                        UPixelScalar _item_height);

  /**
   * visually scrolls the list forward
   * @param forward. true if forward
   * @return true if list advances
   */
  virtual bool ScrollAdvance(bool forward);

  /**
   * positions the ith item in the middle of the screen
   */
  virtual void EnsureVisible(unsigned i);

  /**
   * sets length, and correctly positions origin item
   */
  virtual void SetLength(unsigned n);

  virtual void DrawItems(Canvas &canvas, unsigned start, unsigned end) const;

  virtual void OnPaint(Canvas &canvas);
  virtual void OnPaint(Canvas &canvas, const PixelRect &dirty);
  /**
   * handles bouncing to center an item
   */
  virtual bool OnTimer(WindowTimer &timer);


  void KillFocus() {
    PaintWindow::OnKillFocus();
  }
  /**
   * reverse the height/width components of the list
   */
  virtual UPixelScalar GetHeight()
  {
    //LogDebug(_T("HorizontalListControl::GetHeight"));
    return PaintWindow::GetWidth();
  }
  virtual UPixelScalar GetWidth()
  {
    //LogDebug(_T("HorizontalListControl::GetWidth"));
    return PaintWindow::GetHeight();
  }

  virtual void SetPixelOrigin(int pixel_origin);

  /**
   * Sets the pixel origin and origin based on the item that is
   * closed to the center of the screen given the input pixel_origin
   * @param pixel_origin
   */
  void SetPixelOriginAndCenter(int pixel_origin);
  /**
   * returns item index that would be nearest the center of the screen
   * given the input parameter pixel_origin
   * @param pixel_origin
   */ //TODO what does this return if out of bounds????
  unsigned GetItemFromPixelOrigin(PixelScalar pixel_origin);


  virtual gcc_pure
  PixelRect item_rect(unsigned i) const {
    PixelRect rc;
    rc.top = 0;
    rc.left = (int)(i - origin) * item_height - pixel_pan;
    rc.bottom = rc.top + PaintWindow::GetHeight();
    rc.right = rc.left + (PixelScalar)item_height;

    return rc;
  }

  /**
   * returns true if the kinetic scroll is not moving
   */
  bool IsSteady() {
#ifndef _WIN32_WCE
    return kinetic.IsSteady();
#else
    return true;
#endif
  }

  /**
   * scrolls to the index i using kinetic scroller
   * @param i. the item in the list to scroll to
   */
  void ScrollToItem(unsigned i);

  /**
   * The OnMouseDown event is called when the mouse is pressed over the button
   * (derived from Window)
   */
  virtual bool OnMouseDown(PixelScalar x, PixelScalar y);
  /**
   * The OnMouseUp event is called when the mouse is released over the button
   * (derived from Window)
   */
  virtual bool OnMouseUp(PixelScalar x, PixelScalar y);
  /**
   * The OnMouseMove event is called when the mouse is moved over the button
   * (derived from Window)
   */
  virtual bool OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys);

};

#endif

