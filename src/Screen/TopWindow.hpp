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

#ifndef XCSOAR_SCREEN_TOP_WINDOW_HXX
#define XCSOAR_SCREEN_TOP_WINDOW_HXX

#include "Screen/ContainerWindow.hpp"

#ifdef HAVE_AYGSHELL_DLL
#include "OS/AYGShellDLL.hpp"
#endif

#ifndef USE_GDI
#include "Thread/Mutex.hpp"
#include "Screen/SDL/TopCanvas.hpp"
#include "Screen/SDL/DoubleClick.hpp"
#endif

#ifdef ANDROID
#include "Thread/Mutex.hpp"
#include "Thread/Cond.hpp"

struct Event;
#endif

class TopWindowStyle : public WindowStyle {
#if defined(ENABLE_SDL) && !defined(ANDROID)
  bool full_screen;
  bool resizable;
#endif

public:
  TopWindowStyle()
#if defined(ENABLE_SDL) && !defined(ANDROID)
    :full_screen(false), resizable(false)
#endif
  {
    Popup();
  }

  TopWindowStyle(const WindowStyle other)
    :WindowStyle(other)
#if defined(ENABLE_SDL) && !defined(ANDROID)
    , full_screen(false), resizable(false)
#endif
  {
    Popup();
  }

  void FullScreen() {
#if defined(ENABLE_SDL) && !defined(ANDROID)
    full_screen = true;
#endif
  }

  bool GetFullScreen() const {
#if defined(ENABLE_SDL) && !defined(ANDROID)
    return full_screen;
#else
    return false;
#endif
  }

  void Resizable() {
#if defined(ENABLE_SDL) && !defined(ANDROID)
    resizable = true;
#elif defined(USE_GDI)
    style &= ~WS_BORDER;
    style |= WS_THICKFRAME;
#endif
  }

  bool GetResizable() const {
#if defined(ENABLE_SDL) && !defined(ANDROID)
    return resizable;
#else
    return false;
#endif
  }
};

/**
 * A top-level full-screen window.
 */
class TopWindow : public ContainerWindow {
#ifndef USE_GDI
  TopCanvas screen;

  Mutex invalidated_lock;
  bool invalidated;

#ifdef ANDROID
  Mutex paused_mutex;
  Cond paused_cond;

  /**
   * Is the application currently paused?  While this flag is set, no
   * OpenGL operations are allowed, because the OpenGL surface does
   * not exist.
   */
  bool paused;

  /**
   * Has the application been resumed?  When this flag is set,
   * TopWindow::Expose() attempts to reinitialize the OpenGL surface.
   */
  bool resumed;

  /**
   * Was the application view resized while paused?  If true, then
   * new_width and new_height contain the new display dimensions.
   */
  bool resized;

  UPixelScalar new_width, new_height;
#endif

  DoubleClick double_click;

#else /* USE_GDI */

#ifdef _WIN32_WCE
  /**
   * A handle to the task bar that was manually hidden.  This is a
   * hack when aygshell.dll is not available (Windows CE Core).
   */
  HWND task_bar;
#endif

  /**
   * On WM_ACTIVATE, the focus is returned to this window.
   */
  HWND hSavedFocus;

#ifdef HAVE_AYGSHELL_DLL
  SHACTIVATEINFO s_sai;
#endif
#endif /* USE_GDI */

public:
#ifdef HAVE_AYGSHELL_DLL
  const AYGShellDLL ayg_shell_dll;
#endif

public:
  TopWindow();

  static bool find(const TCHAR *cls, const TCHAR *text);

  void set(const TCHAR *cls, const TCHAR *text, PixelRect rc,
           TopWindowStyle style=TopWindowStyle());

#ifdef _WIN32_WCE
  void reset();
#endif

  /**
   * Triggers an OnCancelMode() call on the focused #Window and/or the
   * #Window currently capturing the mouse.
   */
  void CancelMode();

#if defined(USE_GDI) && !defined(_WIN32_WCE)
  gcc_pure
  const PixelRect GetClientRect() const {
    if (::IsIconic(hWnd)) {
      /* for a minimized window, GetClientRect() returns the
         dimensions of the icon, which is not what we want */
      WINDOWPLACEMENT placement;
      if (::GetWindowPlacement(hWnd, &placement) &&
          (placement.showCmd == SW_MINIMIZE ||
           placement.showCmd == SW_SHOWMINIMIZED)) {
        placement.rcNormalPosition.right -= placement.rcNormalPosition.left;
        placement.rcNormalPosition.bottom -= placement.rcNormalPosition.top;
        placement.rcNormalPosition.left = 0;
        placement.rcNormalPosition.top = 0;
        return placement.rcNormalPosition;
      }
    }

    return ContainerWindow::GetClientRect();
  }

  gcc_pure
  const PixelSize GetSize() const {
    /* this is implemented again because Window::get_size() would call
       Window::GetClientRect() (method is not virtual) */
    PixelRect rc = GetClientRect();
    PixelSize s;
    s.cx = rc.right;
    s.cy = rc.bottom;
    return s;
  }
#endif

  void Fullscreen();

#ifndef USE_GDI
  virtual void Invalidate();

protected:
  void Expose();

public:
#endif /* !USE_GDI */

  /**
   * Synchronously refresh the screen by handling all pending repaint
   * requests.
   */
  void Refresh();

  void Close() {
    AssertNoneLocked();

#ifndef USE_GDI
    OnClose();
#else /* ENABLE_SDL */
    ::SendMessage(hWnd, WM_CLOSE, 0, 0);
#endif
  }

#ifdef ANDROID
  bool OnEvent(const Event &event);
#elif defined(ENABLE_SDL)
  bool OnEvent(const SDL_Event &event);
#endif

#ifdef ANDROID
  /**
   * The Android OpenGL surface has been resized; notify the TopWindow
   * that this has happened.  The caller should also submit the RESIZE
   * event to the event queue.  This method is thread-safe.
   */
  void AnnounceResize(UPixelScalar width, UPixelScalar height);

  bool ResumeSurface();

  /**
   * Reinitialise the OpenGL surface if the Android Activity has been
   * resumed.
   *
   * @return true if there is a valid OpenGL surface
   */
  bool CheckResumeSurface();

  /**
   * Synchronously update the size of the TopWindow to the new OpenGL
   * surface dimensions.
   */
  void RefreshSize();
#else
  void RefreshSize() {}
#endif

protected:
  virtual bool OnActivate();
  virtual bool OnDeactivate();

#ifdef ENABLE_SDL
  virtual bool OnClose();
#else
  virtual LRESULT OnMessage(HWND _hWnd, UINT message,
                             WPARAM wParam, LPARAM lParam);
#endif /* !ENABLE_SDL */

#ifndef USE_GDI
  virtual void OnResize(UPixelScalar width, UPixelScalar height);
#endif

#ifdef ANDROID
  /**
   * @see Event::PAUSE
   */
  virtual void OnPause();

  /**
   * @see Event::RESUME
   */
  virtual void OnResume();

public:
  void Pause();
  void Resume();
#endif

public:
  void PostQuit();

  /**
   * Runs the event loop until the application quits.
   */
  int RunEventLoop();
};

#endif
