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

#ifndef XCSOAR_SCREEN_SDL_CANVAS_HPP
#define XCSOAR_SCREEN_SDL_CANVAS_HPP

#ifdef ENABLE_OPENGL
#error Please include OpenGL/Canvas.hpp
#endif

#include "Util/NonCopyable.hpp"
#include "Math/fixed.hpp"
#include "Math/Angle.hpp"
#include "Screen/Brush.hpp"
#include "Screen/Font.hpp"
#include "Screen/Pen.hpp"
#include "Screen/SDL/Color.hpp"
#include "Screen/SDL/Point.hpp"
#include "Compiler.h"

#include <assert.h>
#include <tchar.h>

#include <SDL_gfxPrimitives.h>

#ifdef WIN32
/* those are WIN32 macros - undefine, or Canvas::background_mode will
   break */
#undef OPAQUE
#undef TRANSPARENT
#endif

class Bitmap;

/**
 * Base drawable canvas class
 * 
 */
class Canvas : private NonCopyable {
  friend class WindowCanvas;
  friend class SubCanvas;

protected:
  SDL_Surface *surface;

  PixelScalar x_offset, y_offset;
  UPixelScalar width, height;

  Pen pen;
  Brush brush;
  const Font *font;
  Color text_color, background_color;
  enum {
    OPAQUE, TRANSPARENT
  } background_mode;

public:
  Canvas()
    :surface(NULL), x_offset(0), y_offset(0), width(0), height(0),
     font(NULL), background_mode(OPAQUE) {}
  explicit Canvas(SDL_Surface *_surface)
    :surface(_surface), width(_surface->w), height(_surface->h),
     font(NULL), background_mode(OPAQUE) {}

  void set(SDL_Surface *_surface) {
    reset();

    surface = _surface;
    width = surface->w;
    height = surface->h;
  }

  void reset();

protected:
  /**
   * Returns true if the outline should be drawn after the area has
   * been filled.  As an optimization, this function returns false if
   * brush and pen share the same color.
   */
  bool pen_over_brush() const {
    return pen.IsDefined() &&
      (brush.IsHollow() || brush.GetColor() != pen.GetColor());
  }

public:
  bool IsDefined() const {
    return surface != NULL;
  }

  UPixelScalar get_width() const {
    return width;
  }

  UPixelScalar get_height() const {
    return height;
  }

  gcc_pure
  const HWColor map(const Color color) const
  {
    return HWColor(::SDL_MapRGB(surface->format, color.value.r,
                                color.value.g, color.value.b));
  }

  void SelectNullPen() {
    pen = Pen(0, COLOR_BLACK);
  }

  void SelectWhitePen() {
    pen = Pen(1, COLOR_WHITE);
  }

  void SelectBlackPen() {
    pen = Pen(1, COLOR_BLACK);
  }

  void SelectHollowBrush() {
    brush.Reset();
  }

  void SelectWhiteBrush() {
    brush = Brush(COLOR_WHITE);
  }

  void SelectBlackBrush() {
    brush = Brush(COLOR_BLACK);
  }

  void Select(const Pen &_pen) {
    pen = _pen;
  }

  void Select(const Brush &_brush) {
    brush = _brush;
  }

  void Select(const Font &_font) {
    font = &_font;
  }

  void SetTextColor(const Color c) {
    text_color = c;
  }

  Color GetTextColor() const {
    return text_color;
  }

  void SetBackgroundColor(const Color c) {
    background_color = c;
  }

  Color GetBackgroundColor() const {
    return background_color;
  }

  void SetBackgroundOpaque() {
    background_mode = OPAQUE;
  }

  void SetBackgroundTransparent() {
    background_mode = TRANSPARENT;
  }

  void DrawOutlineRectangle(PixelScalar left, PixelScalar top,
                         PixelScalar right, PixelScalar bottom,
                         Color color) {
    ::rectangleColor(surface, left + x_offset, top + y_offset,
                     right + x_offset, bottom + y_offset, color.GFXColor());
  }

  void Rectangle(PixelScalar left, PixelScalar top,
                 PixelScalar right, PixelScalar bottom) {
    DrawFilledRectangle(left, top, right, bottom, brush);

    if (pen_over_brush())
      DrawOutlineRectangle(left, top, right, bottom, pen.GetColor());
  }

  void DrawFilledRectangle(PixelScalar left, PixelScalar top,
                      PixelScalar right, PixelScalar bottom,
                      const HWColor color) {
    if (left >= right || top >= bottom)
      return;

    left += x_offset;
    right += x_offset;
    top += y_offset;
    bottom += y_offset;

    SDL_Rect r = { (Sint16)left, (Sint16)top,
                   (Uint16)(right - left), (Uint16)(bottom - top) };
    SDL_FillRect(surface, &r, color);
  }

  void DrawFilledRectangle(PixelScalar left, PixelScalar top,
                      PixelScalar right, PixelScalar bottom,
                      const Color color) {
    DrawFilledRectangle(left, top, right, bottom, map(color));
  }

  void DrawFilledRectangle(PixelScalar left, PixelScalar top,
                      PixelScalar right, PixelScalar bottom,
                      const Brush &brush) {
    if (brush.IsHollow())
      return;

    DrawFilledRectangle(left, top, right, bottom, brush.GetColor());
  }

  void DrawFilledRectangle(const PixelRect &rc, const HWColor color) {
    DrawFilledRectangle(rc.left, rc.top, rc.right, rc.bottom, color);
  }

  void DrawFilledRectangle(const PixelRect &rc, const Color color) {
    DrawFilledRectangle(rc.left, rc.top, rc.right, rc.bottom, color);
  }

  void DrawFilledRectangle(const PixelRect rc, const Brush &brush) {
    DrawFilledRectangle(rc.left, rc.top, rc.right, rc.bottom, brush);
  }

  void Clear() {
    Rectangle(0, 0, get_width(), get_height());
  }

  void Clear(const HWColor color) {
    DrawFilledRectangle(0, 0, get_width(), get_height(), color);
  }

  void Clear(const Color color) {
    DrawFilledRectangle(0, 0, get_width(), get_height(), color);
  }

  void Clear(const Brush &brush) {
    DrawFilledRectangle(0, 0, get_width(), get_height(), brush);
  }

  void ClearWhite() {
    Clear(COLOR_WHITE);
  }

  void DrawRoundRectangle(PixelScalar left, PixelScalar top,
                       PixelScalar right, PixelScalar bottom,
                       UPixelScalar ellipse_width, UPixelScalar ellipse_height);

  void DrawRaisedEdge(PixelRect &rc) {
    Pen bright(1, Color(240, 240, 240));
    Select(bright);
    DrawTwoLines(rc.left, rc.bottom - 2, rc.left, rc.top,
              rc.right - 2, rc.top);

    Pen dark(1, Color(128, 128, 128));
    Select(dark);
    DrawTwoLines(rc.left + 1, rc.bottom - 1, rc.right - 1, rc.bottom - 1,
              rc.right - 1, rc.top + 1);

    ++rc.left;
    ++rc.top;
    --rc.right;
    --rc.bottom;
  }

  void DrawPolyline(const RasterPoint *points, unsigned num_points);
  void DrawPolygon(const RasterPoint *points, unsigned num_points);

  void DrawTriangleFan(const RasterPoint *points, unsigned num_points) {
    DrawPolygon(points, num_points);
  }

  void DrawLine(PixelScalar ax, PixelScalar ay, PixelScalar bx, PixelScalar by) {
    ax += x_offset;
    bx += x_offset;
    ay += y_offset;
    by += y_offset;

#if SDL_GFXPRIMITIVES_MAJOR > 2 || \
  (SDL_GFXPRIMITIVES_MAJOR == 2 && (SDL_GFXPRIMITIVES_MINOR > 0 || \
                                    SDL_GFXPRIMITIVES_MICRO >= 22))
    /* thickLineColor() was added in SDL_gfx 2.0.22 */
    if (pen.GetWidth() > 1)
      ::thickLineColor(surface, ax, ay, bx, by,
                       pen.GetWidth(), pen.GetColor().GFXColor());
    else
#endif
      ::lineColor(surface, ax, ay, bx, by, pen.GetColor().GFXColor());
  }

  void DrawLine(const RasterPoint a, const RasterPoint b) {
    DrawLine(a.x, a.y, b.x, b.y);
  }

  void DrawLinePiece(const RasterPoint a, const RasterPoint b) {
    DrawLine(a, b);
  }

  void DrawTwoLines(PixelScalar ax, PixelScalar ay,
                 PixelScalar bx, PixelScalar by,
                 PixelScalar cx, PixelScalar cy)
  {
    DrawLine(ax, ay, bx, by);
    DrawLine(bx, by, cx, cy);
  }

  void DrawTwoLines(const RasterPoint a, const RasterPoint b,
                 const RasterPoint c) {
    DrawTwoLines(a.x, a.y, b.x, b.y, c.x, c.y);
  }

  void DrawCircle(PixelScalar x, PixelScalar y, UPixelScalar radius);

  void DrawSegment(PixelScalar x, PixelScalar y, UPixelScalar radius,
               Angle start, Angle end, bool horizon=false);

  void DrawAnnulus(PixelScalar x, PixelScalar y,
               UPixelScalar small_radius, UPixelScalar big_radius,
               Angle start, Angle end);

  void DrawKeyhole(PixelScalar x, PixelScalar y,
               UPixelScalar small_radius, UPixelScalar big_radius,
               Angle start, Angle end);

  void DrawFocusRectangle(PixelRect rc) {
    DrawOutlineRectangle(rc.left, rc.top, rc.right, rc.bottom,
                      COLOR_DARK_GRAY);
  }

  void DrawButton(PixelRect rc, bool down);

  gcc_pure
  const PixelSize CalcTextSize(const TCHAR *text, size_t length) const;

  gcc_pure
  const PixelSize CalcTextSize(const TCHAR *text) const;

  gcc_pure
  UPixelScalar CalcTextWidth(const TCHAR *text) const {
    return CalcTextSize(text).cx;
  }

  gcc_pure
  UPixelScalar GetFontHeight() const {
    return font != NULL ? font->GetHeight() : 0;
  }

  void text(PixelScalar x, PixelScalar y, const TCHAR *text);
  void text(PixelScalar x, PixelScalar y, const TCHAR *text, size_t length);

  void text_transparent(PixelScalar x, PixelScalar y, const TCHAR *text);

  void text_opaque(PixelScalar x, PixelScalar y, const PixelRect &rc,
                   const TCHAR *text);

  void text_clipped(PixelScalar x, PixelScalar y, const PixelRect &rc,
                    const TCHAR *text) {
    // XXX
    this->text(x, y, text);
  }

  void text_clipped(PixelScalar x, PixelScalar y, UPixelScalar width,
                    const TCHAR *text) {
    // XXX
    this->text(x, y, text);
  }

  /**
   * Render text, clip it within the bounds of this Canvas.
   */
  void TextAutoClipped(PixelScalar x, PixelScalar y, const TCHAR *t) {
    text(x, y, t);
  }

  void formatted_text(PixelRect *rc, const TCHAR *text, unsigned format);

  void copy(PixelScalar dest_x, PixelScalar dest_y,
            UPixelScalar dest_width, UPixelScalar dest_height,
            SDL_Surface *surface, PixelScalar src_x, PixelScalar src_y);

  void copy(PixelScalar dest_x, PixelScalar dest_y, SDL_Surface *surface) {
    copy(dest_x, dest_y, surface->w, surface->h, surface, 0, 0);
  }

  void copy(PixelScalar dest_x, PixelScalar dest_y,
            UPixelScalar dest_width, UPixelScalar dest_height,
            const Canvas &src, PixelScalar src_x, PixelScalar src_y) {
    copy(dest_x, dest_y, dest_width, dest_height,
         src.surface, src_x, src_y);
  }

  void copy(const Canvas &src, PixelScalar src_x, PixelScalar src_y);
  void copy(const Canvas &src);

  void copy(PixelScalar dest_x, PixelScalar dest_y,
            UPixelScalar dest_width, UPixelScalar dest_height,
            const Bitmap &src, PixelScalar src_x, PixelScalar src_y);
  void copy(const Bitmap &src);

  void copy_transparent_white(const Canvas &src);
  void copy_transparent_black(const Canvas &src);

  void stretch_transparent(const Bitmap &src, Color key);
  void invert_stretch_transparent(const Bitmap &src, Color key);

  void Stretch(PixelScalar dest_x, PixelScalar dest_y,
               UPixelScalar dest_width, UPixelScalar dest_height,
               SDL_Surface *src,
               PixelScalar src_x, PixelScalar src_y,
               UPixelScalar src_width, UPixelScalar src_height);

  void Stretch(SDL_Surface *src) {
    Stretch(0, 0, get_width(), get_height(),
            src, 0, 0, src->w, src->h);
  }

  void Stretch(PixelScalar dest_x, PixelScalar dest_y,
               UPixelScalar dest_width, UPixelScalar dest_height,
               const Canvas &src,
               PixelScalar src_x, PixelScalar src_y,
               UPixelScalar src_width, UPixelScalar src_height) {
    Stretch(dest_x, dest_y, dest_width, dest_height,
            src.surface,
            src_x, src_y, src_width, src_height);
  }

  void Stretch(const Canvas &src,
               PixelScalar src_x, PixelScalar src_y,
               UPixelScalar src_width, UPixelScalar src_height);

  void Stretch(PixelScalar dest_x, PixelScalar dest_y,
               UPixelScalar dest_width, UPixelScalar dest_height,
               const Bitmap &src,
               PixelScalar src_x, PixelScalar src_y,
               UPixelScalar src_width, UPixelScalar src_height);
  void Stretch(PixelScalar dest_x, PixelScalar dest_y,
               UPixelScalar dest_width, UPixelScalar dest_height,
               const Bitmap &src);

  void Stretch(const Bitmap &src) {
    Stretch(0, 0, width, height, src);
  }

  void StretchMono(PixelScalar dest_x, PixelScalar dest_y,
                   UPixelScalar dest_width, UPixelScalar dest_height,
                   const Bitmap &src,
                   PixelScalar src_x, PixelScalar src_y,
                   UPixelScalar src_width, UPixelScalar src_height,
                   Color fg_color, Color bg_color);

  void CopyNot(PixelScalar dest_x, PixelScalar dest_y,
                UPixelScalar dest_width, UPixelScalar dest_height,
                SDL_Surface *src, PixelScalar src_x, PixelScalar src_y);

  void CopyNot(PixelScalar dest_x, PixelScalar dest_y,
                UPixelScalar dest_width, UPixelScalar dest_height,
                const Bitmap &src, PixelScalar src_x, PixelScalar src_y);

  void CopyOr(PixelScalar dest_x, PixelScalar dest_y,
               UPixelScalar dest_width, UPixelScalar dest_height,
               SDL_Surface *src, PixelScalar src_x, PixelScalar src_y);

  void CopyOr(PixelScalar dest_x, PixelScalar dest_y,
               UPixelScalar dest_width, UPixelScalar dest_height,
               const Bitmap &src, PixelScalar src_x, PixelScalar src_y);

  void CopyOr(const Bitmap &src) {
    CopyOr(0, 0, get_width(), get_height(), src, 0, 0);
  }

  void CopyNotOr(PixelScalar dest_x, PixelScalar dest_y,
                 UPixelScalar dest_width, UPixelScalar dest_height,
                 SDL_Surface *src, PixelScalar src_x, PixelScalar src_y);

  void CopyNotOr(PixelScalar dest_x, PixelScalar dest_y,
                 UPixelScalar dest_width, UPixelScalar dest_height,
                 const Bitmap &src, PixelScalar src_x, PixelScalar src_y);

  void CopyAnd(PixelScalar dest_x, PixelScalar dest_y,
                UPixelScalar dest_width, UPixelScalar dest_height,
                SDL_Surface *src, PixelScalar src_x, PixelScalar src_y);

  void CopyAnd(PixelScalar dest_x, PixelScalar dest_y,
                UPixelScalar dest_width, UPixelScalar dest_height,
                const Canvas &src, PixelScalar src_x, PixelScalar src_y) {
    CopyAnd(dest_x, dest_y, dest_width, dest_height,
             src.surface, src_x, src_y);
  }

  void CopyAnd(const Canvas &src) {
    CopyAnd(0, 0, src.get_width(), src.get_height(), src, 0, 0);
  }

  void CopyAnd(PixelScalar dest_x, PixelScalar dest_y,
                UPixelScalar dest_width, UPixelScalar dest_height,
                const Bitmap &src, PixelScalar src_x, PixelScalar src_y);

  void CopyAnd(const Bitmap &src) {
    CopyAnd(0, 0, get_width(), get_height(), src, 0, 0);
  }

  void ScaleCopy(PixelScalar dest_x, PixelScalar dest_y,
                  const Bitmap &src,
                  PixelScalar src_x, PixelScalar src_y,
                  UPixelScalar src_width, UPixelScalar src_height);
};

#endif
