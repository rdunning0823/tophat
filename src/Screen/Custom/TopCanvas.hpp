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

#ifndef XCSOAR_SCREEN_TOP_CANVAS_HPP
#define XCSOAR_SCREEN_TOP_CANVAS_HPP

#include "Compiler.h"

#ifdef USE_MEMORY_CANVAS
#include "Screen/Memory/PixelTraits.hpp"
#include "Screen/Memory/ActivePixelTraits.hpp"
#include "Screen/Memory/Buffer.hpp"
#else
#include "Screen/Canvas.hpp"
#endif

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Features.hpp"
#endif

#ifdef USE_EGL
#include "Screen/EGL/System.hpp"

#ifdef MESA_KMS
#include <drm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#endif
#endif

#ifdef USE_GLX
#include "Screen/GLX/System.hpp"

#define Font X11Font
#define Window X11Window
#define Display X11Display
#include <X11/X.h>
#undef Font
#undef Window
#undef Display
#undef Expose
#undef KeyPress
struct _XDisplay;
#endif

#ifdef DITHER
#include "../Memory/Dither.hpp"
#endif

#include <stdint.h>

#ifdef ENABLE_SDL
#include <SDL_version.h>
#endif

#ifdef SOFTWARE_ROTATE_DISPLAY
enum class DisplayOrientation : uint8_t;
#endif

struct SDL_Surface;
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
class Canvas;
struct PixelSize;
struct PixelRect;

#if (defined(USE_FB) && !defined(KOBO)) || (defined USE_EGL && (defined(USE_VIDEOCORE) || defined(HAVE_MALI)))
/* defined if we need to initialise /dev/tty to graphics mode, see
   TopCanvas::InitialiseTTY() */
#define USE_TTY
#endif

class TopCanvas
#ifndef USE_MEMORY_CANVAS
  : public Canvas
#endif
{
#ifdef USE_EGL
#if defined(USE_X11) || defined(USE_WAYLAND)
#elif defined(USE_VIDEOCORE)
  /* for Raspberry Pi */
  DISPMANX_DISPLAY_HANDLE_T vc_display;
  DISPMANX_UPDATE_HANDLE_T vc_update;
  DISPMANX_ELEMENT_HANDLE_T vc_element;
  EGL_DISPMANX_WINDOW_T vc_window;
#elif defined(HAVE_MALI)
  struct mali_native_window mali_native_window;
#elif defined(MESA_KMS)
  struct gbm_device *native_display;
  struct gbm_surface *native_window;

  int dri_fd;

  struct gbm_bo *current_bo;

  drmEventContext evctx;

  drmModeConnector *connector;
  drmModeEncoder *encoder;
  drmModeModeInfo mode;

  drmModeCrtc* saved_crtc;
#endif

  EGLDisplay display;
  EGLContext context;
  EGLSurface surface;
#endif

#ifdef USE_GLX
  _XDisplay *x_display;
  GLXContext glx_context;
  GLXWindow glx_window;
#endif

#ifdef ENABLE_SDL
#if SDL_MAJOR_VERSION >= 2
  SDL_Window *window;
#endif

#ifdef USE_MEMORY_CANVAS
#if SDL_MAJOR_VERSION >= 2
  SDL_Renderer *renderer;
  SDL_Texture *texture;
#else
  SDL_Surface *surface;
#endif
#endif
#endif

#ifdef USE_MEMORY_CANVAS

#ifdef GREYSCALE
  WritableImageBuffer<GreyscalePixelTraits> buffer;

#ifdef DITHER
  Dither dither;
#endif

#else /* !GREYSCALE */
  WritableImageBuffer<ActivePixelTraits> buffer;
#endif /* !GREYSCALE */
#endif /* USE_MEMORY_CANVAS */

#ifdef USE_TTY
  /**
   * A file descriptor for /dev/tty, or -1 if /dev/tty could not be
   * opened.  This is used on Linux to switch to graphics mode
   * (KD_GRAPHICS) or restore text mode (KD_TEXT).
   */
  int tty_fd;
#endif

#ifdef USE_FB
  int fd;

  void *map;
  unsigned map_pitch, map_bpp;

  uint32_t epd_update_marker;
#endif

#ifdef KOBO
  /**
   * Runtime flag that can be used to disable dithering at runtime for
   * some situations.
   */
  bool enable_dither;
#endif

public:
#ifdef USE_FB
  TopCanvas()
    :
#ifdef USE_TTY
    tty_fd(-1),
#endif
    fd(-1), map(nullptr)
#ifdef KOBO
    , enable_dither(true)
#endif
  {}
#elif defined(USE_TTY)
  TopCanvas():tty_fd(-1) {}
#endif

#ifndef ANDROID
  ~TopCanvas() {
    Destroy();
  }

  void Destroy();
#endif

#ifdef USE_MEMORY_CANVAS
  bool IsDefined() const {
#ifdef ENABLE_SDL
#if SDL_MAJOR_VERSION >= 2
    return window != nullptr;
#else
    return surface != nullptr;
#endif
#elif defined(USE_VFB)
    return true;
#else
    return fd >= 0;
#endif
  }

  gcc_pure
  PixelRect GetRect() const;
#endif

#if defined(ENABLE_SDL) && (SDL_MAJOR_VERSION >= 2)
  void Create(const char *text, PixelSize new_size,
              bool full_screen, bool resizable);
#elif defined(USE_GLX)
  void Create(_XDisplay *x_display,
              X11Window x_window,
              GLXFBConfig *fb_cfg) {
    CreateGLX(x_display, x_window, fb_cfg);
  }
#elif defined(USE_X11) || defined(USE_WAYLAND)
  void Create(EGLNativeDisplayType native_display,
              EGLNativeWindowType native_window) {
    CreateEGL(native_display, native_window);
  }
#else
  void Create(PixelSize new_size,
              bool full_screen, bool resizable);
#endif

#ifdef USE_FB
  /**
   * Check if the screen has been resized.
   *
   * @return true if the screen has been resized
   */
  bool CheckResize();

  gcc_pure
  unsigned GetWidth() const {
    return buffer.width;
  }

  gcc_pure
  unsigned GetHeight() const {
    return buffer.height;
  }
#endif

#ifdef ENABLE_OPENGL
  /**
   * Initialise the new OpenGL context.
   */
  void Resume();
#endif

  void OnResize(PixelSize new_size);

#ifdef USE_MEMORY_CANVAS
  Canvas Lock();
  void Unlock();
#endif

  void Flip();

#ifdef KOBO
  /**
   * Wait until the screen update is complete.
   */
  void Wait();

  void SetEnableDither(bool _enable_dither) {
    enable_dither = _enable_dither;
  }
#endif

#ifdef SOFTWARE_ROTATE_DISPLAY
  void SetDisplayOrientation(DisplayOrientation orientation);
#endif

private:
#ifdef USE_GLX
  void InitGLX(_XDisplay *x_display);
  void CreateGLX(_XDisplay *x_display,
                 X11Window x_window,
                 GLXFBConfig *fb_cfg);
#elif defined(USE_EGL)
  void CreateEGL(EGLNativeDisplayType native_display,
                 EGLNativeWindowType native_window);
#endif

  void InitialiseTTY();
  void DeinitialiseTTY();
};

#endif
