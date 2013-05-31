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

#ifndef XCSOAR_SCREEN_PAINT_WINDOW_HXX
#define XCSOAR_SCREEN_PAINT_WINDOW_HXX

#include "Screen/Canvas.hpp"
#include "Screen/Window.hpp"

class ContainerWindow;

/**
 * A #Window implementation for custom drawing.  Call get_canvas()
 * whenever you want to draw something.
 */
class PaintWindow : public Window {
private:
  /* hide this method */
  void InstallWndProc();

public:
  PaintWindow() {
    EnableCustomPainting();
  }

#ifdef USE_GDI
  static bool register_class(HINSTANCE hInstance);
#endif

#ifndef USE_GDI
  using Window::set;

  void set(ContainerWindow &parent,
           PixelScalar left, PixelScalar top,
           UPixelScalar width, UPixelScalar height,
           const WindowStyle style=WindowStyle()) {
    set(&parent, left, top, width, height, style);
  }
#else /* USE_GDI */
  void set(ContainerWindow *parent, const TCHAR *cls,
           PixelScalar left, PixelScalar top,
           UPixelScalar width, UPixelScalar height,
           const WindowStyle style=WindowStyle()) {
    Window::set(parent, cls, NULL,
                left, top, width, height, style);
  }

  void set(ContainerWindow &parent, const TCHAR *cls,
           PixelScalar left, PixelScalar top,
           UPixelScalar width, UPixelScalar height,
           const WindowStyle style=WindowStyle()) {
    set(&parent, cls, left, top, width, height, style);
  }

  void set(ContainerWindow &parent,
           PixelScalar left, PixelScalar top,
           UPixelScalar width, UPixelScalar height,
           const WindowStyle style=WindowStyle()) {
    set(parent, _T("PaintWindow"), left, top, width, height, style);
  }
#endif /* USE_GDI */

  void set(ContainerWindow &parent, PixelRect rc,
           const WindowStyle window_style=WindowStyle()) {
    set(parent, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        window_style);
  }

  constexpr
  static bool SupportsPartialRedraw() {
#ifdef USE_GDI
    /* we can use the GDI function InvalidateRect() with a non-NULL
       RECT */
    return true;
#else
    /* SDL and OpenGL can't do partial redraws, they always repaint
       the whole screen */
    return false;
#endif
  }

  /**
   * Invalidates the visible area and schedules a repaint (which will
   * occur in the main thread).
   */
  void Invalidate() {
#ifndef USE_GDI
    Window::Invalidate();
#else
    ::InvalidateRect(hWnd, NULL, false);
#endif
  }

  /**
   * Invalidates a part of the visible area and schedules a repaint
   * (which will occur in the main thread).
   */
  void Invalidate(const PixelRect &rect) {
#ifndef USE_GDI
    Invalidate();
#else
    ::InvalidateRect(hWnd, &rect, false);
#endif
  }
};

#endif
