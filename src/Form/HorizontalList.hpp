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

#ifndef XCSOAR_HORIZONTAL_LIST_HPP
#define XCSOAR_HORIZONTAL_LIST_HPP

#include "Screen/PaintWindow.hpp"
#include "List.hpp" // for listeners & handlers
#include "Time/PeriodClock.hpp"
#include "Compiler.h"

#ifndef _WIN32_WCE
#include "Screen/Timer.hpp"
#include "UIUtil/KineticManager.hpp"
#endif

struct DialogLook;
class ContainerWindow;
struct PixelRect;
class WndSymbolButton;

class HorizontalListCursorHandler {
public:
  virtual void OnCursorMoved(gcc_unused unsigned index) {}

  gcc_pure
  virtual bool CanActivateItem(gcc_unused unsigned index) const {
    return false;
  }

  /**
   * called when the list scrolls a pixel
   */
  virtual void OnPixelMove() {}
  virtual void OnActivateItem(gcc_unused unsigned index) {}
};

/**
 * List that is drawn and scrolls horizontally
 * Scrollbar is not supported
 */
class HorizontalListControl : public PaintWindow {

protected:
  const DialogLook &look;

  /** The height of one item on the screen, in pixels. */
  UPixelScalar item_height;
  /** The number of items in the list. */
  unsigned length;
  /** The index of the topmost item currently being displayed. */
  unsigned origin;

  /**
   * Which pixel row of the "origin" item is being displayed at the
   * top of the Window?
   * May be negative if used by HorizontalList with over_scroll_max
   */
  PixelScalar pixel_pan;

  /** The number of items visible at a time. */
  unsigned items_visible;
  /** The index of the selected item on the screen. */
  unsigned cursor;

  /**
   * Tracks the state of the mouse dragging over the list items
   */
  enum class DragMode {

    /**
     * No dragging in progress
     */
    NONE,

    /**
     * The user is currently scrolling the map.
     */
    SCROLL,

    /**
     * The user is dragging the selected item.
     */
    CURSOR,
  };

  DragMode drag_mode;

  /**
   * the vertical distance from the start of the drag relative to the
   * top of the list (not the top of the screen)
   */
  int drag_y;

  /**
   * The vertical distance from the start of the drag relative to the
   * top of the window
   */
  int drag_y_window;

  ListItemRenderer *item_renderer;
  HorizontalListCursorHandler *cursor_handler;

#ifndef _WIN32_WCE
  KineticManager kinetic;
  WindowTimer kinetic_timer;
#endif

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
   * @param parent the parent window
   * @param _item_height Height of an item of the ListFrameControl
   */
  HorizontalListControl(ContainerWindow &parent, const DialogLook &_look,
                        PixelRect _rc, const WindowStyle _style,
                        UPixelScalar _item_height);

  virtual ~HorizontalListControl();

  void Create(ContainerWindow &parent,
              PixelRect rc, const WindowStyle style,
              unsigned _item_height);

  void SetItemRenderer(ListItemRenderer *_item_renderer) {
    assert(_item_renderer != nullptr);
    assert(item_renderer == nullptr);

    item_renderer = _item_renderer;
  }

  void SetCursorHandler(HorizontalListCursorHandler *_cursor_handler) {
    assert(_cursor_handler != nullptr);
    assert(cursor_handler == nullptr);

    cursor_handler = _cursor_handler;
  }

  /**
   * Returns the height of list items
   * @return height of list items in pixel
   */
  unsigned GetItemHeight() const {
    return item_height;
  }

  void SetItemHeight(UPixelScalar _item_height);

  bool IsEmpty() const {
    return length == 0;
  }

  /**
   * Returns the number of items in the list
   * @return The number of items in the list
   */
  unsigned GetLength() const {
    return length;
  }

  /**
   * Check whether the length of the list is below a certain
   * threshold.  Small lists may have different behaviour on some
   * platforms (e.g. Altair).
   */
  bool IsShort() const {
    return length <= 8 || length <= items_visible;
  }

  /**
   * Returns the current cursor position
   * @return The current cursor position
   */
  unsigned GetCursorIndex() const {
    return cursor;
  }

  /**
   * Moves the cursor to the specified position.
   *
   * @param ensure_visible.  If false, does not scroll to item
   * @return true if the cursor was moved to the specified position,
   * false if the position was invalid
   */
  bool SetCursorIndex(unsigned i, bool ensure_visible = true);

  /**
   * Move the cursor this many items up (negative delta) or down
   * (positive delta).  Scrolls if necessary.
   */
  void MoveCursor(int delta);

  /**
   * Pan the "origin item" to the specified pixel position.
   */
  void SetPixelPan(PixelScalar _pixel_pan);


  /**
   * returns max of 0 or pixel_pan
   */
  UPixelScalar GetPixelPanUnsigned() const {
    return (UPixelScalar)std::max((PixelScalar)0, pixel_pan);
  }

  /**
   * Scrolls to the specified index.
   */
  void SetOrigin(int i);

  PixelScalar GetPixelOrigin() const {
    return origin * item_height + pixel_pan;
  }

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
   * Scrolls a number of items up (negative delta) or down (positive
   * delta).  The cursor is not moved.
   */
  void MoveOrigin(int delta);

  /**
   * Sets the minimum duration (ms) of a mouse down required
   * for the mouse up event to trigger OnActivateItem
   */
  void SetClickDuration(unsigned duration) {
    click_duration = duration;
  }

  /**
   * visually scrolls the list forward
   * @param forward. true if forward
   * @return true if list advances
   */
  virtual bool ScrollAdvance(bool forward);

  /**
   * sets length, and correctly positions origin item
   */
  virtual void SetLength(unsigned n);

  virtual void DrawItems(Canvas &canvas, unsigned start, unsigned end) const;

  virtual void OnPaint(Canvas &canvas) override;
  virtual void OnPaint(Canvas &canvas, const PixelRect &dirty) override;
  /**
   * handles bouncing to center an item
   */
  virtual bool OnTimer(WindowTimer &timer) override;
#ifndef _WIN32_WCE
  void OnDestroy() override;
#endif

  void KillFocus() {
    PaintWindow::OnKillFocus();
  }

  void OnResize(PixelSize new_size) override;
  void OnSetFocus() override;

  /**
   * reverse the height/width components of the list
   */
  virtual UPixelScalar GetHeight()
  {
    return PaintWindow::GetWidth();
  }
  virtual UPixelScalar GetWidth()
  {
    return PaintWindow::GetHeight();
  }

protected:
  gcc_pure
  bool CanActivateItem() const;
  void ActivateItem();

  /**
   * Scroll to the ListItem defined by i
   * @param i The ListItem array id
   */
  virtual void EnsureVisible(unsigned i);

  /**
   * Determine which list item resides at the specified pixel row.
   * Returns -1 if there is no such list item.
   */
  gcc_pure
  int ItemIndexAt(int y) const {
    int i = (y + GetPixelPanUnsigned()) / item_height + origin;
    return i >= 0 && (unsigned)i < length ? i : -1;
  }

  void Invalidate_item(unsigned i) {
    Invalidate(item_rect(i));
  }

  void drag_end();

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

public:
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
  virtual bool OnMouseDown(PixelScalar x, PixelScalar y) override;
  /**
   * The OnMouseUp event is called when the mouse is released over the button
   * (derived from Window)
   */
  virtual bool OnMouseUp(PixelScalar x, PixelScalar y) override;
  /**
   * The OnMouseMove event is called when the mouse is moved over the button
   * (derived from Window)
   */
  virtual bool OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys) override;
  bool OnMouseWheel(PixelScalar x, PixelScalar y, int delta) override;

  bool OnKeyCheck(unsigned key_code) const override;
  bool OnKeyDown(unsigned key_code) override;

  void OnCancelMode() override;
};

#endif

