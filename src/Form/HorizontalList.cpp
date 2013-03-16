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

#include "Form/HorizontalList.hpp"
#include "Screen/Layout.hpp"
#include "Language/Language.hpp"
#include "Screen/Canvas.hpp"

#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Asset.hpp"

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Scope.hpp"
#include "Screen/OpenGL/Globals.hpp"
#elif defined(USE_GDI)
#include "Screen/WindowCanvas.hpp"
#endif

HorizontalListControl::HorizontalListControl(ContainerWindow &parent,
                                             const DialogLook &_look,
                                             PixelRect _rc,
                                             const WindowStyle _style,
                                             UPixelScalar _item_height)
  :ListControl(parent, _look, _rc, _style, _item_height, 1000),
   over_scroll_max(0),
   cursor_down_index(-1),
   click_duration(0)
{
  SetHasScrollBar(false);
}

void
HorizontalListControl::EnsureVisible(unsigned i)
{
  assert(i < length);
  if (!IsOldWindowsCE())
    kinetic_timer.Cancel();

  SetPixelOriginAndCenter(i * item_height - over_scroll_max);
}

unsigned
HorizontalListControl::GetItemFromPixelOrigin(PixelScalar pixel_origin)
{
  if (pixel_origin < 0)
    return 0;

  unsigned index = pixel_origin / item_height;
  unsigned modulus = pixel_origin % item_height;

  if (modulus > (item_height / 2 - over_scroll_max / 2))
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
  ListControl::SetLength(n);
  if (n > 0)
    EnsureVisible(origin);
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
HorizontalListControl::OnPaint(Canvas &canvas)
{
  if (handler != NULL || paint_item_callback != NULL) {
    unsigned first = origin;
    if (first > 0)
      first--;
    DrawItems(canvas, first, first + items_visible + 3);
  }
}

void
HorizontalListControl::OnPaint(Canvas &canvas, const PixelRect &dirty)
{
  if (handler != NULL || paint_item_callback != NULL) {
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
HorizontalListControl::DrawItems(Canvas &canvas, unsigned start, unsigned end) const
{
  PixelRect rc = item_rect(start);
  canvas.SetBackgroundTransparent();
  canvas.SelectNullPen();
  canvas.Select(*look.list.font);

#ifdef ENABLE_OPENGL
  /* enable clipping */

  GLScissor scissor(OpenGL::translate_x,
                    OpenGL::screen_height - OpenGL::translate_y - canvas.get_height() - 1,
                    scroll_bar.GetLeft(GetSize()), canvas.get_height());

#endif

  unsigned last_item = min(length, end);

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
      if (handler != NULL)
        handler->OnPaintItem(canvas, rc, i);
      else
        paint_item_callback(canvas, rc, i);

    ::MoveRect(rc, rc.right - rc.left, 0);
  }

  /* paint the bottom part below the last item */
  rc.bottom = canvas.get_height();
}

void
HorizontalListControl::ScrollToItem(unsigned i)
{
  if (IsOldWindowsCE()) {
    SetCursorIndex(i);
    return;
  }

  if (i == origin)
    return;

  assert(i < length);
  kinetic_timer.Cancel();
  int offset = GetPixelOrigin() - origin * item_height;
  kinetic.MoveTo(i * item_height + offset);
  kinetic_timer.Schedule(30);
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
    //EnsureVisible(GetCenteredItem());
    drag_end();

    if (!IsOldWindowsCE()) {
      kinetic.MouseUp(GetPixelOrigin(), (item_height));
      kinetic_timer.Schedule(30);
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
    if (!IsOldWindowsCE())
      kinetic.MouseMove(GetPixelOrigin());
    if (handler != nullptr)
      handler->OnPixelMove();
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
  if (!IsOldWindowsCE())
    kinetic_timer.Cancel();

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
  }
  if (!IsOldWindowsCE())
    kinetic.MouseDown(GetPixelOrigin());
  SetCapture();

  return true;
}

bool
HorizontalListControl::ScrollAdvance(bool forward)
{
  if (!IsOldWindowsCE())
    return false;

    unsigned old_item = GetItemFromPixelOrigin(GetPixelOrigin());
  if ((forward && (old_item >= (GetLength() - 1))) ||
      (!forward && old_item == 0))
    return false;

  unsigned new_item = old_item + (forward ? 1 : -1);
  int to_location = new_item * GetItemHeight() - over_scroll_max;
  kinetic.MoveTo(to_location);
  kinetic_timer.Schedule(30);
  return true;
}

bool
HorizontalListControl::OnTimer(WindowTimer &timer)
{
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

  return PaintWindow::OnTimer(timer);
}
