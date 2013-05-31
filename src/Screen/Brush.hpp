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

#ifndef XCSOAR_SCREEN_BRUSH_HPP
#define XCSOAR_SCREEN_BRUSH_HPP

#include "Util/NonCopyable.hpp"
#include "Screen/Color.hpp"
#include "Screen/Features.hpp"
#include "Compiler.h"

#ifdef USE_GDI
class Bitmap;
#endif

/**
 * A Brush is used for drawing filled circles, rectangles and so on
 */
class Brush
#ifdef USE_GDI
  : private NonCopyable
#endif
{
protected:
#ifndef USE_GDI
  bool hollow;
  Color color;
#else
  HBRUSH brush;
#endif

public:
#ifndef USE_GDI
  Brush():hollow(true) {}

  constexpr
  explicit Brush(const Color _color):hollow(false), color(_color)  {}
#else
  /** Base Constructor of the Brush class */
  Brush():brush(NULL) {}

  /**
   * Constructor (creates a Brush object of the given Color
   * @param c Color of the Brush
   */
  explicit Brush(const Color c) : brush(NULL) { Set(c); }

  /** Destructor */
  ~Brush() { Reset(); }
  #endif

public:
  /**
   * Sets the Color of the Brush
   * @param c The new Color
   */
  void Set(const Color c);

#ifdef HAVE_HATCHED_BRUSH

  /**
   * Creates a bitmap-based Brush
   * @param bitmap The bitmap the new Brush will be based on
   */
  void Set(const Bitmap &bitmap);

#endif

  /**
   * Resets the Brush to NULL
   */
  void Reset();

  /**
   * Returns whether the Brush is defined (!= NULL)
   * @return True if the Brush is defined, False otherwise
   */
  bool
  IsDefined() const
  {
#ifndef USE_GDI
    return !hollow;
#else
    return brush != NULL;
#endif
  }

#ifndef USE_GDI
  bool IsHollow() const { return hollow; }
  const Color GetColor() const { return color; }
#else
  /**
   * Returns the native HBRUSH object
   * @return The native HBRUSH object
   */
  HBRUSH Native() const {
    return brush;
  }
#endif

#ifdef ENABLE_OPENGL
  /**
   * Configures this brush in the OpenGL context.
   */
  void Set() const {
    color.Set();
  }
#endif /* OPENGL */
};

#endif
