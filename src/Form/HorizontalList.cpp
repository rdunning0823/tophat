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

#include "Form/HorizontalList.hpp"
#include "Screen/Layout.hpp"
#include "Language/Language.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/ContainerWindow.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Screen/Key.h"
#include "Screen/Point.hpp"
#include "Asset.hpp"

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Scissor.hpp"
#include "Screen/OpenGL/Globals.hpp"
#elif defined(USE_GDI)
#include "Screen/WindowCanvas.hpp"
#endif

#include <assert.h>
#include <algorithm>

/**
 * Can the user scroll with pixel precision?  This is used on fast
 * displays to give more instant feedback, which feels more slick.  On
 * slow e-paper screens, this is not a good idea.
 */
gcc_const
static bool
UsePixelPan()
{
  return HasDraggableScreen();
}

HorizontalListControl::HorizontalListControl(ContainerWindow &parent,
                                             const DialogLook &_look,
                                             PixelRect _rc,
                                             const WindowStyle _style,
                                             UPixelScalar _item_height)
  :look(_look),
   item_height(_item_height),
   length(0),
   origin(0), pixel_pan(0),
   cursor(0),
   drag_mode(DragMode::NONE),
   item_renderer(nullptr), cursor_handler(nullptr),
 #ifndef _WIN32_WCE
   kinetic(1000),
   kinetic_timer(*this),
 #endif
   over_scroll_max(0),
   cursor_down_index(-1),
   click_duration(0)
{
  Create(parent, _rc, _style, _item_height);
}

HorizontalListControl::~HorizontalListControl() {
  /* we must override ~Window(), because in ~Window(), our own
     OnDestroy() method won't be called (during object destruction,
     this object loses its identity) */
  Destroy();
}

void
HorizontalListControl::Create(ContainerWindow &parent,
                              PixelRect rc, const WindowStyle style,
                              unsigned _item_height)
{
  item_height = _item_height;
  PaintWindow::Create(parent, rc, style);
}

bool
HorizontalListControl::CanActivateItem() const
{
  if (IsEmpty())
    return false;

  return cursor_handler != nullptr &&
    cursor_handler->CanActivateItem(GetCursorIndex());
}

void
HorizontalListControl::ActivateItem()
{
  assert(CanActivateItem());

  unsigned index = GetCursorIndex();
  assert(index < GetLength());
  if (cursor_handler != nullptr)
    cursor_handler->OnActivateItem(index);
}

void
HorizontalListControl::OnResize(PixelSize new_size)
{
  PaintWindow::OnResize(new_size);

  items_visible = new_size.cy / item_height;

  if (unsigned(new_size.cy) >= length * item_height) {
    /* after the resize, there is enough room for all list items -
       scroll back to the top */
    origin = pixel_pan = 0;
  }

  if (length > 0)
    /* make sure the cursor is still visible */
    EnsureVisible(GetCursorIndex());
}

void
HorizontalListControl::OnSetFocus()
{
  PaintWindow::OnSetFocus();
  Invalidate_item(cursor);
}

void
HorizontalListControl::EnsureVisible(unsigned i)
{
  assert(i < length);

#ifndef _WIN32_WCE
  if (HasDraggableScreen())
    kinetic_timer.Cancel();
#endif

  SetPixelOriginAndCenter(i * item_height - over_scroll_max);
}

unsigned
HorizontalListControl::GetItemFromPixelOrigin(PixelScalar pixel_origin)
{
  if (pixel_origin < 0)
    return 0;

  unsigned index = pixel_origin / item_height;
  unsigned modulus = pixel_origin % item_height;

  if (modulus > (unsigned)(item_height / 2 - over_scroll_max / 2))
    index++;
  return (index < length) ? index : length - 1;
}

unsigned
HorizontalListControl::GetCenteredItem()
{
  return GetItemFromPixelOrigin(GetPixelOrigin());
}

void
HorizontalListControl::SetOverScrollMax(UPixelScalar pixels)
{
  over_scroll_max = pixels;
}

void
HorizontalListControl::SetLength(unsigned n)
{
  if (n == length)
    return;

  unsigned cursor = GetCursorIndex();

  length = n;

  if (n == 0)
    cursor = 0;
  else if (cursor >= n)
    cursor = n - 1;

  items_visible = GetHeight() / item_height;

  if (n <= items_visible)
    origin = 0;
  else if (origin + items_visible > n)
    origin = n - items_visible;
  else if (cursor < origin)
    origin = cursor;

  Invalidate();

  SetCursorIndex(cursor);

  if (n > 0)
    EnsureVisible(origin);
}

void
HorizontalListControl::MoveOrigin(int delta)
{
  if (UsePixelPan()) {
    int pixel_origin = (int)GetPixelOrigin();
    SetPixelOrigin(pixel_origin + delta * (int)item_height);
  } else {
    SetOrigin(origin + delta);
  }
}

void
HorizontalListControl::SetOrigin(int i)
{
  if (length <= items_visible)
    return;

  if (i < 0)
    i = 0;
  else if ((unsigned)i + items_visible > length)
    i = length - items_visible;

  if ((unsigned)i == origin)
    return;

#ifdef USE_GDI
  int delta = origin - i;
#endif

  origin = i;

#ifdef USE_GDI
  if ((unsigned)abs(delta) < items_visible) {
    PixelRect rc = GetClientRect();
    Scroll(0, delta * item_height, rc);
    return;
  }
#endif

  Invalidate();
}

void
HorizontalListControl::SetPixelOrigin(int pixel_origin)
{
  int max = length * item_height - GetHeight() + over_scroll_max;
  if (pixel_origin > max)
    pixel_origin = max;

  if (pixel_origin < -1 * (int)over_scroll_max)
    pixel_origin = -1 * (int)over_scroll_max;

  UPixelScalar pixel_origin_unsigned = (UPixelScalar) (pixel_origin < 0 ? 0 : pixel_origin);
  SetOrigin(pixel_origin_unsigned / item_height);
  SetPixelPan(pixel_origin % (int)item_height);
}

void
HorizontalListControl::SetPixelOriginAndCenter(int pixel_origin)
{
  int max = length * item_height - GetHeight() + over_scroll_max;

  if (pixel_origin > max)
    pixel_origin = max;

  if (pixel_origin < -1 * (int)over_scroll_max)
    pixel_origin = -1 * (int)over_scroll_max;

  SetOrigin(GetItemFromPixelOrigin(pixel_origin));
  SetPixelPan(-1 * (int)over_scroll_max); //TODO : this pan value should be adjusted for centering
}

void
HorizontalListControl::SetPixelPan(PixelScalar _pixel_pan)
{
  if (pixel_pan == _pixel_pan)
    return;

  pixel_pan = _pixel_pan;
  Invalidate();
}

void
HorizontalListControl::OnPaint(Canvas &canvas)
{
  if (item_renderer != nullptr) {
    unsigned first = origin;
    if (first > 0)
      first--;

    DrawItems(canvas, first, first + items_visible + 3);
  }
}

void
HorizontalListControl::OnPaint(Canvas &canvas, const PixelRect &dirty)
{
  if (item_renderer != nullptr) {
    unsigned first = origin + (dirty.top + GetPixelPanUnsigned()) / item_height;
    unsigned last = origin + (dirty.bottom + GetPixelPanUnsigned() + item_height - 1) / item_height;
    if (last < GetLength())
      last++;
    if (first > 0)
         first--;
    DrawItems(canvas, first, last);
  }
}

void
HorizontalListControl::SetItemHeight(UPixelScalar _item_height)
{
  item_height = _item_height;
  items_visible = GetHeight() / item_height;

  Invalidate();
}

bool
HorizontalListControl::SetCursorIndex(unsigned i, bool ensure_visible)
{
  if (i >= length)
    return false;

  if (i == GetCursorIndex())
    return true;

  if (ensure_visible)
    EnsureVisible(i);

  Invalidate_item(cursor);
  cursor = i;
  Invalidate_item(cursor);

  if (cursor_handler != nullptr)
    cursor_handler->OnCursorMoved(i);
  return true;
}

void
HorizontalListControl::MoveCursor(int delta)
{
  if (length == 0)
    return;

  int new_cursor = cursor + delta;
  if (new_cursor < 0)
    new_cursor = 0;
  else if ((unsigned)new_cursor >= length)
    new_cursor = length - 1;

  SetCursorIndex(new_cursor);
}

void
HorizontalListControl::DrawItems(Canvas &canvas, unsigned start, unsigned end) const
{
  PixelRect rc = item_rect(start);
  canvas.SetBackgroundTransparent();
  canvas.SelectNullPen();
  canvas.Select(*look.list.font);

#ifdef ENABLE_OPENGL
  /* enable clipping */
  GLCanvasScissor scissor(canvas);
#endif

  unsigned last_item = std::min(length, end);

  for (unsigned i = start; i < last_item; i++) {
    const bool selected = i == cursor;
    const bool pressed = selected && drag_mode == DragMode::CURSOR;
#ifdef _WIN32
    canvas.DrawFilledRectangle(rc,
                               look.list.GetBackgroundColor(false,
                                                            false,
                                                            false));
#endif
    canvas.SetTextColor(look.list.GetTextColor(selected, false, pressed));
      if (item_renderer != nullptr)
        item_renderer->OnPaintItem(canvas, rc, i);

    rc.Offset(rc.right - rc.left, 0);
  }

  /* paint the bottom part below the last item */
  rc.bottom = canvas.GetHeight();
}

void
HorizontalListControl::ScrollToItem(unsigned i)
{
  if (!HasDraggableScreen()) {
    SetCursorIndex(i);
    return;
  }

  if (i == origin)
    return;

  assert(i < length);
#ifndef _WIN32_WCE
  kinetic_timer.Cancel();
  int offset = GetPixelOrigin() - origin * item_height;
  kinetic.MoveTo(i * item_height + offset);
  kinetic_timer.Schedule(30);
#endif
}

/**
 * use x as input for scrolling instead of y
 */
bool
HorizontalListControl::OnMouseUp(PixelScalar x, PixelScalar y)
{
  cursor_down_index = -1;

  if (drag_mode == DragMode::CURSOR &&
      y >= 0 && y <= ((PixelScalar)GetWidth())) {
    if (mouse_down_clock.Elapsed() > (int)click_duration) {
      drag_end();
      ActivateItem();
    }
  }

  if (drag_mode == DragMode::SCROLL || drag_mode == DragMode::CURSOR) {
    drag_end();

    if (HasDraggableScreen()) {
#ifndef _WIN32_WCE
      kinetic.MouseUp(GetPixelOrigin(), (item_height));
      kinetic_timer.Schedule(30);
#endif
    } else {
      SetCursorIndex(GetCenteredItem());
    }

    return true;
  } else
    return PaintWindow::OnMouseUp(x, y);
}

/**
 * use x as input for scrolling instead of y
 */
bool
HorizontalListControl::OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys)
{
  if (drag_mode == DragMode::CURSOR) {
    if (abs(x - drag_y_window) > ((int)item_height / 25)) {  /** TODO: adjust this behaviour for horizontal **/
      drag_mode = DragMode::SCROLL;
      Invalidate_item(cursor);
    } else
      return true;
  }

  if (drag_mode == DragMode::SCROLL) {
    cursor_down_index = -1;
    int new_origin = drag_y - x;
    SetPixelOrigin(new_origin);
#ifndef _WIN32_WCE
    if (HasDraggableScreen())
      kinetic.MouseMove(GetPixelOrigin());
#endif
    if (cursor_handler != nullptr)
      cursor_handler->OnPixelMove();
    return true;
  }
  return PaintWindow::OnMouseMove(x, y, keys);
}

/**
 * use x as input for scrolling instead of y
 */
bool
HorizontalListControl::OnMouseDown(PixelScalar x, PixelScalar y)
{
  // End any previous drag
  drag_end();

  mouse_down_clock.Update();
#ifndef _WIN32_WCE
  if (HasDraggableScreen())
    kinetic_timer.Cancel();
#endif
  // if click in ListBox area
  // -> select appropriate item

  int index = ItemIndexAt(x);
  // If mouse was clicked outside the list items -> cancel
  if (index < 0) {
    return false;
  }

  drag_y = GetPixelOrigin() + x;
  drag_y_window = x;
  drag_mode = DragMode::SCROLL;
  cursor_down_index = GetItemFromPixelOrigin(GetPixelOrigin());
  Invalidate_item(cursor);
  if ((unsigned)index == GetCursorIndex() &&
      CanActivateItem()) {
    drag_mode = DragMode::CURSOR;

    Invalidate_item(cursor);
  } else {
    // If item was not selected before
    // -> select it
  }
#ifndef _WIN32_WCE
  if (HasDraggableScreen())
    kinetic.MouseDown(GetPixelOrigin());
#endif
  SetCapture();

  return true;
}

void
HorizontalListControl::drag_end()
{
  if (drag_mode != DragMode::NONE) {
    if (drag_mode == DragMode::CURSOR)
      Invalidate_item(cursor);

    drag_mode = DragMode::NONE;
    ReleaseCapture();
  }
}

bool
HorizontalListControl::ScrollAdvance(bool forward)
{
  if (!HasDraggableScreen())
    return false;

    unsigned old_item = GetItemFromPixelOrigin(GetPixelOrigin());
  if ((forward && (old_item >= (GetLength() - 1))) ||
      (!forward && old_item == 0))
    return false;

#ifndef _WIN32_WCE
  unsigned new_item = old_item + (forward ? 1 : -1);
  int to_location = new_item * GetItemHeight() - over_scroll_max;
  kinetic.MoveTo(to_location);
  kinetic_timer.Schedule(30);
#endif
  return true;
}

bool
HorizontalListControl::OnTimer(WindowTimer &timer)
{
#ifndef _WIN32_WCE
  if (timer == kinetic_timer) {
    if (kinetic.IsSteady()) {
      // if already reversed or in MoveTo mode,
      //  then stop completely and hard move to position
      // else, if we're still on the same item, Reverse motion back to start
      // else, MoveTo to animate advance to the next item.
      // else reverse
      switch (kinetic.GetMode()) {
      case KineticManager::MOVETO:
      case KineticManager::REVERSED:
      {
        kinetic_timer.Cancel();
        int new_item = GetItemFromPixelOrigin(GetPixelOrigin());
        SetCursorIndex(new_item);
        EnsureVisible(new_item);
        break;
      }
      case KineticManager::NORMAL:
        if (GetItemFromPixelOrigin(GetPixelOrigin()) ==
            GetItemFromPixelOrigin(kinetic.GetMouseDownX())) {
          kinetic.Reverse();
        } else { // move to next item!
          int item_temp = GetItemFromPixelOrigin(GetPixelOrigin());
          int to_location = item_temp * GetItemHeight() - over_scroll_max;
          kinetic.MoveTo(to_location);
        }
      }
    } else // continue scrolling
      SetPixelOrigin(kinetic.GetPosition());

    return true;
  }
#endif

  return PaintWindow::OnTimer(timer);
}

bool
HorizontalListControl::OnKeyCheck(unsigned key_code) const
{
  switch (key_code) {
  case KEY_RETURN:
    return CanActivateItem();

  case KEY_UP:
    return GetCursorIndex() > 0;

  case KEY_DOWN:
    return GetCursorIndex() + 1 < length;

  default:
    return false;
  }
}

bool
HorizontalListControl::OnKeyDown(unsigned key_code)
{
#ifndef _WIN32_WCE
  kinetic_timer.Cancel();
#endif

  switch (key_code) {
#ifdef GNAV
  // JMW added this to make data entry easier
  case KEY_APP4:
#endif
  case KEY_RETURN:
    if (CanActivateItem())
      ActivateItem();
    return true;

  case KEY_UP:
    // previous item
    if (GetCursorIndex() <= 0)
      break;

    MoveCursor(-1);
    return true;

  case KEY_DOWN:
    // next item
    if (GetCursorIndex() +1 >= length)
      break;

    MoveCursor(1);
    return true;

  case KEY_LEFT:
    // page up
    MoveCursor(-(int)items_visible);
    return true;

  case KEY_RIGHT:
    // page down
    MoveCursor(items_visible);
    return true;

  case KEY_HOME:
    SetCursorIndex(0);
    return true;

  case KEY_END:
    if (length > 0) {
      SetCursorIndex(length - 1);
    }
    return true;

  case KEY_PRIOR:
    MoveCursor(-(int)items_visible);
    return true;

  case KEY_NEXT:
    MoveCursor(items_visible);
    return true;
  }
  return PaintWindow::OnKeyDown(key_code);
}

bool
HorizontalListControl::OnMouseWheel(PixelScalar x, PixelScalar y, int delta)
{
  drag_end();

#ifndef _WIN32_WCE
  kinetic_timer.Cancel();
#endif

  if (delta > 0) {
    // scroll up
    MoveOrigin(-1);
  } else if (delta < 0) {
    // scroll down
    MoveOrigin(1);
  }

  return true;
}

void
HorizontalListControl::OnCancelMode()
{
  PaintWindow::OnCancelMode();

  drag_end();

#ifndef _WIN32_WCE
  kinetic_timer.Cancel();
#endif
}

#ifndef _WIN32_WCE
void
HorizontalListControl::OnDestroy()
{
  kinetic_timer.Cancel();

  PaintWindow::OnDestroy();
}
#endif

