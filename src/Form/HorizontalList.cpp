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

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Scope.hpp"
#include "Screen/OpenGL/Globals.hpp"
#elif defined(USE_GDI)
#include "Screen/WindowCanvas.hpp"
#endif

#include "LogFile.hpp"


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
  kinetic_timer.Cancel();
//  LogDebug(_T("HorizontalListControl::EnsureVisible i:%u"), i);
  //SetOrigin(i);
  //bounce_pixel_origin = i * item_height - over_scroll_max;
  //kinetic_timer.Schedule(30);

  //SetPixelOrigin(i * item_height - over_scroll_max);
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

  //LogDebug(_T("HorizontalListControl::GetItemFromPixelOrigin origin:%i item_hght:%u index:%u modulus:%u [item_height/2-over_scroll_max/2:%u"),
  //         origin, item_height, index, modulus, item_height / 2 - over_scroll_max / 2);
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
//  const PixelScalar old_origin = GetPixelOrigin();
//  const PixelScalar delta = (PixelScalar)pixels - (PixelScalar)over_scroll_max;
  over_scroll_max = pixels;
//  SetPixelOrigin(old_origin + delta);
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
   //const int pixel_origin_debug = pixel_origin;
   int max = length * item_height - GetHeight() + over_scroll_max;

//   LogDebug(_T("HorizontalListControl::SetPixelOrigin ENTER pixel_origin:%i overscroll_max:%u max:%i pixel_pan:%i"),
//            pixel_origin, over_scroll_max, max, pixel_pan);

   if (pixel_origin > max)
     pixel_origin = max;

   if (pixel_origin < -1 * (int)over_scroll_max)
     pixel_origin = -1 * (int)over_scroll_max;

    UPixelScalar pixel_origin_unsigned = (UPixelScalar) (pixel_origin < 0 ? 0 : pixel_origin);
    SetOrigin(pixel_origin_unsigned / item_height);
//   SetOrigin(GetItemFromPixelOrigin(pixel_origin));
    SetPixelPan(pixel_origin % (int)item_height);

   /* LogDebug(_T("HorizontalListControl::SetPixelOrigin pixel_origin:%i item_height:%u pixel_origin/item_height:%i"),
            pixel_origin, item_height, pixel_origin/(int)item_height);
   LogDebug(_T("HorizontalListControl::SetPixelOrigin EXIT pixel_origin:%i, pixel_origin:%i pixel_pan:%i"),
            pixel_origin, pixel_origin_debug, pixel_pan);
            */
 }

void
HorizontalListControl::SetPixelOriginAndCenter(int pixel_origin)
{
   //const int pixel_origin_debug = pixel_origin;
   int max = length * item_height - GetHeight() + over_scroll_max;

   //LogDebug(_T("HorizontalListControl::SetPixelOrigin ENTER pixel_origin:%i overscroll_max:%u max:%i pixel_pan:%i"),
   //         pixel_origin, over_scroll_max, max, pixel_pan);

   if (pixel_origin > max)
     pixel_origin = max;

   if (pixel_origin < -1 * (int)over_scroll_max)
     pixel_origin = -1 * (int)over_scroll_max;

//    UPixelScalar pixel_origin_unsigned = (UPixelScalar) (pixel_origin < 0 ? 0 : pixel_origin);
//    SetOrigin(pixel_origin_unsigned / item_height);
   SetOrigin(GetItemFromPixelOrigin(pixel_origin));
   SetPixelPan(-1 * (int)over_scroll_max); //TODO : this pan value should be adjusted for centering
/*
   LogDebug(_T("HorizontalListControl::SetPixelOrigin pixel_origin:%i item_height:%u pixel_origin/item_height:%i"),
            pixel_origin, item_height, pixel_origin/(int)item_height);
   LogDebug(_T("HorizontalListControl::SetPixelOrigin EXIT pixel_origin:%i, pixel_pan:%i"),
            pixel_origin, pixel_pan);
*/

}

void
HorizontalListControl::OnPaint(Canvas &canvas)
{
  if (handler != NULL || paint_item_callback != NULL) {
    unsigned first = origin;
    if (first > 0)
      first--;
//    LogDebug(_T("ListControl::OnPaint() origin:%u items_visible:%u first:%i"),
//             origin, items_visible, first);

    DrawItems(canvas, first, first + items_visible + 3);
  }
}

void
HorizontalListControl::OnPaint(Canvas &canvas, const PixelRect &dirty)
{
  LogDebug(_T("HorizontalListControl::OnPaint() dirty left:%u top:%u right:%u bot:%u"),
           dirty.left, dirty.top, dirty.right, dirty.bottom);

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
//  LogDebug(_T("HorizontalListControl::DrawItems (%i-%i) startrc L:%i T:%i R:%i B:%i origin:%i"),
//           start, end, rc.left, rc.top, rc.right, rc.bottom, origin);
  //canvas.SetBackgroundColor(look.list.background_color);
  canvas.SetBackgroundTransparent();
  canvas.SelectNullPen();
  canvas.Select(*look.list.font);
#ifdef _WIN32
  //canvas.Clear();
#endif

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
    //if (rc.right > 0 && rc.left < (PixelScalar)GetWidth()) {
      if (handler != NULL)
        handler->OnPaintItem(canvas, rc, i);
      else
        paint_item_callback(canvas, rc, i);
    //}
    //if (focused && selected)
      //canvas.DrawFocusRectangle(rc);

    ::MoveRect(rc, rc.right - rc.left, 0);
  }

  /* paint the bottom part below the last item */
  rc.bottom = canvas.get_height();
//  if (rc.bottom > rc.top)
//    canvas.DrawFilledRectangle(rc, look.list.background_color);
}

void
HorizontalListControl::ScrollToItem(unsigned i)
{
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
//    LogDebug(_T("HorizontalListControl::OnMouseUp CURSOR"));
    if (mouse_down_clock.Elapsed() > (int)click_duration) {
      drag_end();
      ActivateItem();
    }
    //return true;
  }

  if (drag_mode == DragMode::SCROLL || drag_mode == DragMode::CURSOR) {
    //EnsureVisible(GetCenteredItem());
    drag_end();

//    drag_y = GetPixelOrigin() + x;
//    drag_y_window = x;
//#ifndef _WIN32_WCE
    //assume x is getting bigger:

    //const int stop_distance = (PixelScalar)item_height -
    //  abs(GetPixelOrigin() - start_pixel_origin) + start_pixel_origin;
    kinetic.MouseUp(GetPixelOrigin(), (item_height));
    kinetic_timer.Schedule(30);
//#endif

    // figure out which item is in center of screen and "recenter" that item.
//    LogDebug(_T("HorizontalListControl::OnMouseUp x:%i pixelorigin:%i drag_y:%i, drag_y_w:%i (item_height*4)/3:%u, old_px_orig:%i"),
//             x, GetPixelOrigin(), drag_y, drag_y_window, (item_height * 3) / 3, drag_y - drag_y_window);


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
      //LogDebug(_T("HorizontalListControl::OnMouseMove exit 1: mode Cursor to Scroll"));
    } else
      //LogDebug(_T("HorizontalListControl::OnMouseMove exit 2: mode Cursor"));
      return true;
  }

  if (drag_mode == DragMode::SCROLL) {
    cursor_down_index = -1;
//    LogDebug(_T("HorizontalListControl::OnMouseMove cursor_down_index:%i"), cursor_down_index);
    int new_origin = drag_y - x;
    SetPixelOrigin(new_origin);
//#ifndef _WIN32_WCE
    kinetic.MouseMove(GetPixelOrigin());
//#endif
    if (handler != nullptr)
      handler->OnPixelMove();
    //int item_index = GetPixelOrigin() / item_height;
    //unsigned scroll_offset = GetPixelOrigin() % item_height;
    //LogDebug(_T("HorizontalListControl::OnMousemove index:%i offset:%u screenwidth:%u"),
    //         item_index, scroll_offset, GetHeight());
    return true;
  }
  //LogDebug(_T("HorizontalListControl::OnMouseMove exit 4: mode not scroll or cursor"));
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

//#ifndef _WIN32_WCE
  kinetic_timer.Cancel();
//#endif

  // if click in ListBox area
  // -> select appropriate item

  int index = ItemIndexAt(x);
  // If mouse was clicked outside the list items -> cancel
  if (index < 0) {
//      LogDebug(_T("HorizontalListControl::OnMouseDown 1 - error - click outside of list"));
    //assert(false);
    return false;
  }

  drag_y = GetPixelOrigin() + x;
  drag_y_window = x;
  drag_mode = DragMode::SCROLL;
  cursor_down_index = GetItemFromPixelOrigin(GetPixelOrigin());
//    LogDebug(_T("HorizontalListControl::OnMouseDown 1.5 index:%i GetCursorIndex():%i CanActIt():%i"),
//             index, GetCursorIndex(), CanActivateItem());
  Invalidate_item(cursor);
  if ((unsigned)index == GetCursorIndex() &&
      CanActivateItem()) {
//      LogDebug(_T("HorizontalListControl::OnMouseDown 2 setting mode to CURSOR"));
    drag_mode = DragMode::CURSOR;

    Invalidate_item(cursor);
  } else {
    // If item was not selected before
    // -> select it
    //SetCursorIndex(index); // dont set cursor index when item is clicked.  SHould be done on mouse up
    //    of whatever location is in the center of the screen.
    /*
    LogDebug(_T("HorizontalListControl::OnMouseDown cursor_down_index:%i"), cursor_down_index);
    LogDebug(_T("HorizontalListControl::OnMouseDown 3 setting mode to SCROLL"));
    LogDebug(_T("HorizontalListControl::OnMouseDown x:%u pixelorigin:%i drag_y:%i, drag_y_w:%i item_height:%u"),
             x, GetPixelOrigin(), drag_y, drag_y_window, item_height);
             */
  }
//#ifndef _WIN32_WCE
  kinetic.MouseDown(GetPixelOrigin());
//#endif
  SetCapture();

  return true;
}

bool
HorizontalListControl::ScrollAdvance(bool forward)
{
//  LogDebug(_T("HorizontalListControl::ScrollAdvance 1"));
  unsigned old_item = GetItemFromPixelOrigin(GetPixelOrigin());
  if ((forward && (old_item >= (GetLength() - 1))) ||
      (!forward && old_item == 0))
    return false;

  unsigned new_item = old_item + (forward ? 1 : -1);
  int to_location = new_item * GetItemHeight() - over_scroll_max;
//  LogDebug(_T("HorizontalListControl::ScrollAdvance old:%i new:%i to_loc:%i"),
//           old_item, new_item, to_location);
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
//        LogDebug(_T("HorizontalListControl::OnTimer steady# setting cursor index:%i"),
//                 new_item);
        SetCursorIndex(new_item);
        EnsureVisible(new_item);
//        LogDebug(_T("HorizontalListControl::OnTimer steady#1 new_item:%i origin:%i, pixel_pan:%i GetPixelOrigin:%i"),
//                 new_item, origin, pixel_pan, GetPixelOrigin());
        break;
      }
      case KineticManager::NORMAL:
        if (GetItemFromPixelOrigin(GetPixelOrigin()) ==
              GetItemFromPixelOrigin(kinetic.GetMouseDownX())) {
          kinetic.Reverse();
 //         LogDebug(_T("HorizontalListControl::OnTimer steady#2-just reversed origin:%i, pixel_pan:%i GetPixelOrigin:%i"),
 //                  origin, pixel_pan, GetPixelOrigin());
       } else {// move to next item!
        int item_temp = GetItemFromPixelOrigin(GetPixelOrigin());
        int to_location = item_temp * GetItemHeight() - over_scroll_max;
        kinetic.MoveTo(to_location);
//        LogDebug(_T("HorizontalListControl::OnTimer steady#3-just MoveTo'd origin:%i, pixel_pan:%i GetPixelOrigin:%i new-item:%i i_height:%i, item:%u over_scroll:%i to_pixels:%i"),
//                 origin, pixel_pan, GetPixelOrigin(), item_temp, GetItemHeight(),
//                 GetItemFromPixelOrigin(GetPixelOrigin()), over_scroll_max, to_location);
        }
      }
    } else // continue scrolling
      SetPixelOrigin(kinetic.GetPosition());

    return true;
  }

  return PaintWindow::OnTimer(timer);
}



