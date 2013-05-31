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

#include "Screen/Canvas.hpp"
#include "Screen/Bitmap.hpp"

#ifndef NDEBUG
#include "Util/UTF8.hpp"
#endif

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Cache.hpp"
#endif

#include <algorithm>
#include <assert.h>
#include <string.h>
#include <winuser.h>

#include "Screen/Util.hpp"
#ifndef ENABLE_OPENGL

#include <SDL_rotozoom.h>
#include <SDL_imageFilter.h>
#endif

#ifndef ENABLE_OPENGL

void
Canvas::reset()
{
  if (surface != NULL) {
    SDL_FreeSurface(surface);
    surface = NULL;
  }
}

void
Canvas::DrawPolyline(const RasterPoint *lppt, unsigned cPoints)
{
  for (unsigned i = 1; i < cPoints; ++i)
    DrawLine(lppt[i - 1].x, lppt[i - 1].y, lppt[i].x, lppt[i].y);
}

void
Canvas::DrawPolygon(const RasterPoint *lppt, unsigned cPoints)
{
  if (brush.IsHollow() && !pen.IsDefined())
    return;

  Sint16 vx[cPoints], vy[cPoints];

  for (unsigned i = 0; i < cPoints; ++i) {
    vx[i] = x_offset + lppt[i].x;
    vy[i] = y_offset + lppt[i].y;
  }

  if (!brush.IsHollow())
    ::filledPolygonColor(surface, vx, vy, cPoints,
                         brush.GetColor().GFXColor());

  if (pen_over_brush())
    ::polygonColor(surface, vx, vy, cPoints, pen.GetColor().GFXColor());
}

void
Canvas::DrawCircle(PixelScalar x, PixelScalar y, UPixelScalar radius)
{
  x += x_offset;
  y += y_offset;

  if (!brush.IsHollow())
    ::filledCircleColor(surface, x, y, radius,
                        brush.GetColor().GFXColor());

  if (pen_over_brush())
    ::circleColor(surface, x, y, radius, pen.GetColor().GFXColor());
}

void
Canvas::DrawSegment(PixelScalar x, PixelScalar y, UPixelScalar radius,
                Angle start, Angle end, bool horizon)
{
  // XXX horizon

  x += x_offset;
  y += y_offset;

  if (!brush.IsHollow())
    ::filledPieColor(surface, x, y, radius, 
                     (int)start.Degrees() - 90,
                     (int)end.Degrees() - 90,
                     brush.GetColor().GFXColor());

  if (pen_over_brush())
    ::pieColor(surface, x, y, radius, 
               (int)start.Degrees() - 90,
               (int)end.Degrees() - 90,
               pen.GetColor().GFXColor());
}

void
Canvas::DrawAnnulus(PixelScalar x, PixelScalar y,
                UPixelScalar small_radius, UPixelScalar big_radius,
                Angle start, Angle end)
{
  assert(IsDefined());

  ::Annulus(*this, x, y, big_radius, start, end, small_radius);
}

void
Canvas::DrawKeyhole(PixelScalar x, PixelScalar y,
                UPixelScalar small_radius, UPixelScalar big_radius,
                Angle start, Angle end)
{
  assert(IsDefined());

  ::KeyHole(*this, x, y, big_radius, start, end, small_radius);
}

#endif /* !OPENGL */

#ifdef ENABLE_OPENGL

void
Canvas::DrawButton(PixelRect rc, bool down)
{
  Color gray = COLOR_LIGHT_GRAY;
  Pen bright_pen(1, LightColor(gray));
  Pen dark_pen(1, DarkColor(gray));

  DrawButtonFancy(*this, rc, dark_pen, bright_pen, gray, false, down, false);
}

#else

void
Canvas::DrawButton(PixelRect rc, bool down)
{
  const Pen old_pen = pen;

  Color gray = COLOR_LIGHT_GRAY;
  DrawFilledRectangle(rc, gray);

  Pen bright(1, LightColor(gray));
  Pen dark(1, DarkColor(gray));

  Select(down ? dark : bright);
  DrawTwoLines(rc.left, rc.bottom - 2, rc.left, rc.top,
            rc.right - 2, rc.top);
  DrawTwoLines(rc.left + 1, rc.bottom - 3, rc.left + 1, rc.top + 1,
            rc.right - 3, rc.top + 1);

  Select(down ? bright : dark);
  DrawTwoLines(rc.left + 1, rc.bottom - 1, rc.right - 1, rc.bottom - 1,
            rc.right - 1, rc.top + 1);
  DrawTwoLines(rc.left + 2, rc.bottom - 2, rc.right - 2, rc.bottom - 2,
            rc.right - 2, rc.top + 2);

  pen = old_pen;
}

#endif /* !OPENGL */

const PixelSize
Canvas::CalcTextSize(const TCHAR *text, size_t length) const
{
  assert(text != NULL);

  TCHAR *duplicated = _tcsdup(text);
  duplicated[length] = 0;

#ifndef UNICODE
  assert(ValidateUTF8(duplicated));
#endif

  const PixelSize size = CalcTextSize(duplicated);
  free(duplicated);

  return size;
}

const PixelSize
Canvas::CalcTextSize(const TCHAR *text) const
{
  assert(text != NULL);
#ifndef UNICODE
  assert(ValidateUTF8(text));
#endif

  PixelSize size = { 0, 0 };

  if (font == NULL)
    return size;

#ifdef ENABLE_OPENGL
  /* see if the TextCache can handle this request */
  size = TextCache::LookupSize(*font, text);
  if (size.cy > 0)
    return size;

  return TextCache::GetSize(*font, text);
#else
  return font->TextSize(text);
#endif
}

#ifndef ENABLE_OPENGL

void
Canvas::text(PixelScalar x, PixelScalar y, const TCHAR *text)
{
  assert(text != NULL);
#ifndef UNICODE
  assert(ValidateUTF8(text));
#endif

  SDL_Surface *s;

  if (font == NULL)
    return;

#ifdef UNICODE
  s = ::TTF_RenderUNICODE_Solid(font->Native(), (const Uint16 *)text,
                                COLOR_BLACK);
#else
  s = ::TTF_RenderUTF8_Solid(font->Native(), text, COLOR_BLACK);
#endif
  if (s == NULL)
    return;

  if (s->format->palette != NULL && s->format->palette->ncolors >= 2) {
    s->format->palette->colors[1] = text_color;

    if (background_mode == OPAQUE) {
      s->flags &= ~SDL_SRCCOLORKEY;
      s->format->palette->colors[0] = background_color;
    }
  }

  copy(x, y, s);
  ::SDL_FreeSurface(s);
}

void
Canvas::text_transparent(PixelScalar x, PixelScalar y, const TCHAR *text)
{
  assert(text != NULL);
#ifndef UNICODE
  assert(ValidateUTF8(text));
#endif

  SDL_Surface *s;

  if (font == NULL)
    return;

#ifdef UNICODE
  s = ::TTF_RenderUNICODE_Solid(font->Native(), (const Uint16 *)text,
                                COLOR_BLACK);
#else
  s = ::TTF_RenderUTF8_Solid(font->Native(), text, COLOR_BLACK);
#endif
  if (s == NULL)
    return;

  if (s->format->palette != NULL && s->format->palette->ncolors >= 2)
    s->format->palette->colors[1] = text_color;

  copy(x, y, s);
  ::SDL_FreeSurface(s);
}

#endif /* !OPENGL */

void
Canvas::formatted_text(PixelRect *rc, const TCHAR *text, unsigned format) {
  assert(text != NULL);
#ifndef UNICODE
  assert(ValidateUTF8(text));
#endif

  if (font == NULL)
    return;

  UPixelScalar skip = font->GetLineSpacing();
  unsigned max_lines = (format & DT_CALCRECT) ? -1 :
                       (rc->bottom - rc->top + skip - 1) / skip;

  size_t len = _tcslen(text);
  TCHAR *duplicated = new TCHAR[len + 1], *p = duplicated;
  unsigned lines = 1;
  for (const TCHAR *i = text; *i != _T('\0'); ++i) {
    TCHAR ch = *i;
    if (ch == _T('\n')) {
      /* explicit line break */

      if (++lines >= max_lines)
        break;

      ch = _T('\0');
    } else if (ch == _T('\r'))
      /* skip */
      continue;
    else if ((unsigned)ch < 0x20)
      /* replace non-printable characters */
      ch = _T(' ');

    *p++ = ch;
  }

  *p = _T('\0');
  len = p - duplicated;

  // simple wordbreak algorithm. looks for single spaces only, no tabs,
  // no grouping of multiple spaces
  if (format & DT_WORDBREAK) {
    for (size_t i = 0; i < len; i += _tcslen(duplicated + i) + 1) {
      PixelSize sz = CalcTextSize(duplicated + i);
      TCHAR *prev_p = NULL;

      // remove words from behind till line fits or no more space is found
      while (sz.cx > rc->right - rc->left &&
             (p = _tcsrchr(duplicated + i, _T(' '))) != NULL) {
        if (prev_p)
          *prev_p = _T(' ');
        *p = _T('\0');
        prev_p = p;
        sz = CalcTextSize(duplicated + i);
      }

      if (prev_p) {
        lines++;
        if (lines >= max_lines)
          break;
      }
    }
  }

  if (format & DT_CALCRECT) {
    rc->bottom = rc->top + lines * skip;
    delete[] duplicated;
    return;
  }

  PixelScalar y = (format & DT_VCENTER) && lines < max_lines
    ? (PixelScalar)(rc->top + rc->bottom - lines * skip) / 2
    : rc->top;
  for (size_t i = 0; i < len; i += _tcslen(duplicated + i) + 1) {
    if (duplicated[i] != _T('\0')) {
      PixelScalar x;
      if (format & (DT_RIGHT | DT_CENTER)) {
        PixelSize sz = CalcTextSize(duplicated + i);
        x = (format & DT_CENTER) ? (rc->left + rc->right - sz.cx)/2 :
                                    rc->right - sz.cx;  // DT_RIGHT
      } else {  // default is DT_LEFT
        x = rc->left;
      }

      TextAutoClipped(x, y, duplicated + i);
    }
    y += skip;
    if (y >= rc->bottom)
      break;
  }

  delete[] duplicated;
}

void
Canvas::text(PixelScalar x, PixelScalar y, const TCHAR *_text, size_t length)
{
  assert(_text != NULL);

  TCHAR copy[length + 1];
  std::copy(_text, _text + length, copy);
  copy[length] = _T('\0');

#ifndef UNICODE
  assert(ValidateUTF8(copy));
#endif

  text(x, y, copy);
}

void
Canvas::text_opaque(PixelScalar x, PixelScalar y, const PixelRect &rc,
                    const TCHAR *_text)
{
  assert(_text != NULL);
#ifndef UNICODE
  assert(ValidateUTF8(_text));
#endif

  DrawFilledRectangle(rc, background_color);
  text_transparent(x, y, _text);
}

#ifndef ENABLE_OPENGL

static bool
clip(PixelScalar &position, UPixelScalar &length, UPixelScalar max,
     PixelScalar &src_position)
{
  if (position < 0) {
    if (length <= (UPixelScalar)-position)
      return false;

    length -= -position;
    src_position -= position;
    position = 0;
  }

  if ((UPixelScalar)position >= max)
    return false;

  if (position + length >= max)
    length = max - position;

  return true;
}

void
Canvas::copy(PixelScalar dest_x, PixelScalar dest_y,
             UPixelScalar dest_width, UPixelScalar dest_height,
             SDL_Surface *src_surface, PixelScalar src_x, PixelScalar src_y)
{
  assert(src_surface != NULL);

  if (!clip(dest_x, dest_width, width, src_x) ||
      !clip(dest_y, dest_height, height, src_y))
    return;

  SDL_Rect src_rect = { src_x, src_y, dest_width, dest_height };
  SDL_Rect dest_rect = { PixelScalar(x_offset + dest_x),
                         PixelScalar(y_offset + dest_y) };

  ::SDL_BlitSurface(src_surface, &src_rect, surface, &dest_rect);
}

void
Canvas::copy(const Canvas &src, PixelScalar src_x, PixelScalar src_y)
{
  copy(0, 0, src.get_width(), src.get_height(), src, src_x, src_y);
}

void
Canvas::copy(const Canvas &src)
{
  copy(src, 0, 0);
}

void
Canvas::copy(PixelScalar dest_x, PixelScalar dest_y,
             UPixelScalar dest_width, UPixelScalar dest_height,
             const Bitmap &src, PixelScalar src_x, PixelScalar src_y)
{
  copy(dest_x, dest_y, dest_width, dest_height,
       src.GetNative(), src_x, src_y);
}

void
Canvas::copy(const Bitmap &src)
{
  SDL_Surface *surface = src.GetNative();
  copy(0, 0, surface->w, surface->h, surface, 0, 0);
}

void
Canvas::copy_transparent_white(const Canvas &src)
{
  assert(src.surface != NULL);

  ::SDL_SetColorKey(src.surface, SDL_SRCCOLORKEY, src.map(COLOR_WHITE));
  copy(src);
  ::SDL_SetColorKey(src.surface, 0, 0);
}

void
Canvas::copy_transparent_black(const Canvas &src)
{
  assert(src.surface != NULL);

  ::SDL_SetColorKey(src.surface, SDL_SRCCOLORKEY, src.map(COLOR_BLACK));
  copy(src);
  ::SDL_SetColorKey(src.surface, 0, 0);
}

void
Canvas::stretch_transparent(const Bitmap &src, Color key)
{
  assert(src.IsDefined());

  SDL_Surface *surface = src.GetNative();

  ::SDL_SetColorKey(surface, SDL_SRCCOLORKEY,
                    ::SDL_MapRGB(surface->format, key.value.r,
                                 key.value.g, key.value.b));
  Stretch(surface);
  ::SDL_SetColorKey(surface, 0, 0);
}

void
Canvas::invert_stretch_transparent(const Bitmap &src, Color key)
{
  assert(src.IsDefined());

  SDL_Surface *src_surface = src.GetNative();
  const UPixelScalar src_x = 0, src_y = 0;
  const UPixelScalar src_width = src_surface->w;
  const UPixelScalar src_height = src_surface->h;
  const UPixelScalar dest_x = 0, dest_y = 0;
  const UPixelScalar dest_width = get_width();
  const UPixelScalar dest_height = get_height();

  SDL_Surface *zoomed =
    ::zoomSurface(src_surface, (double)dest_width / (double)src_width,
                  (double)dest_height / (double)src_height,
                  SMOOTHING_OFF);

  if (zoomed == NULL)
    return;

  ::SDL_SetColorKey(zoomed, SDL_SRCCOLORKEY,
                    ::SDL_MapRGB(zoomed->format, key.value.r,
                                 key.value.g, key.value.b));

  CopyNot(dest_x, dest_y, dest_width, dest_height,
           zoomed, (src_x * dest_width) / src_width,
           (src_y * dest_height) / src_height);
  ::SDL_FreeSurface(zoomed);
}

void
Canvas::Stretch(PixelScalar dest_x, PixelScalar dest_y,
                UPixelScalar dest_width, UPixelScalar dest_height,
                SDL_Surface *src,
                PixelScalar src_x, PixelScalar src_y,
                UPixelScalar src_width, UPixelScalar src_height)
{
  assert(src != NULL);
  assert(dest_width < 0x4000);
  assert(dest_height < 0x4000);

  if (dest_width == src_width && dest_height == src_height) {
    /* fast path: no zooming needed */
    copy(dest_x, dest_y, dest_width, dest_height, src, src_x, src_y);
    return;
  }

  if (dest_width >= 0x4000 || dest_height >= 0x4000)
    /* paranoid sanity check; shouldn't ever happen */
    return;

  SDL_Surface *zoomed =
    ::zoomSurface(src, (double)dest_width / (double)src_width,
                  (double)dest_height / (double)src_height,
                  SMOOTHING_OFF);

  if (zoomed == NULL)
    return;

  ::SDL_SetColorKey(zoomed, 0, 0);

  copy(dest_x, dest_y, dest_width, dest_height,
       zoomed, (src_x * dest_width) / src_width,
       (src_y * dest_height) / src_height);
  ::SDL_FreeSurface(zoomed);
}

void
Canvas::Stretch(const Canvas &src,
                PixelScalar src_x, PixelScalar src_y,
                UPixelScalar src_width, UPixelScalar src_height)
{
  // XXX
  Stretch(0, 0, get_width(), get_height(),
          src, src_x, src_y, src_width, src_height);
}

void
Canvas::Stretch(PixelScalar dest_x, PixelScalar dest_y,
                UPixelScalar dest_width, UPixelScalar dest_height,
                const Bitmap &src,
                PixelScalar src_x, PixelScalar src_y,
                UPixelScalar src_width, UPixelScalar src_height)
{
  assert(IsDefined());
  assert(src.IsDefined());

  Stretch(dest_x, dest_y, dest_width, dest_height,
          src.GetNative(),
          src_x, src_y, src_width, src_height);
}

void
Canvas::Stretch(PixelScalar dest_x, PixelScalar dest_y,
                UPixelScalar dest_width, UPixelScalar dest_height,
                const Bitmap &src)
{
  assert(IsDefined());
  assert(src.IsDefined());

  SDL_Surface *surface = src.GetNative();
  Stretch(dest_x, dest_y, dest_width, dest_height,
          surface, 0, 0, surface->w, surface->h);
}

void
Canvas::StretchMono(PixelScalar dest_x, PixelScalar dest_y,
                    UPixelScalar dest_width, UPixelScalar dest_height,
                    const Bitmap &src,
                    PixelScalar src_x, PixelScalar src_y,
                    UPixelScalar src_width, UPixelScalar src_height,
                    Color fg_color, Color bg_color)
{
  assert(IsDefined());
  assert(src.IsDefined());
  assert(dest_width < 0x4000);
  assert(dest_height < 0x4000);

  if (dest_width >= 0x4000 || dest_height >= 0x4000)
    /* paranoid sanity check; shouldn't ever happen */
    return;

  SDL_Surface *src_surface = src.GetNative();
  assert(src_surface->format->palette != NULL &&
         src_surface->format->palette->ncolors == 2);

  SDL_Surface *zoomed =
    ::zoomSurface(src_surface, (double)dest_width / (double)src_width,
                  (double)dest_height / (double)src_height,
                  SMOOTHING_OFF);
  if (zoomed == NULL)
    return;

  assert(zoomed->format->palette != NULL &&
         zoomed->format->palette->ncolors == 2);

  ::SDL_SetColorKey(zoomed, 0, 0);
  zoomed->format->palette->colors[0] = text_color;
  zoomed->format->palette->colors[1] = bg_color;

  copy(dest_x, dest_y, dest_width, dest_height,
       zoomed, (src_x * dest_width) / src_width,
       (src_y * dest_height) / src_height);
  ::SDL_FreeSurface(zoomed);
}

static bool
clip_range(PixelScalar &a, UPixelScalar a_size,
           PixelScalar &b, UPixelScalar b_size,
           UPixelScalar &size)
{
  if (a < 0) {
    b -= a;
    size += a;
    a = 0;
  }

  if (b < 0) {
    a -= b;
    size += b;
    b = 0;
  }

  if ((PixelScalar)size <= 0)
    return false;

  if (a + size > a_size)
    size = a_size - a;

  if ((PixelScalar)size <= 0)
    return false;

  if (b + size > b_size)
    size = b_size - b;

  return (PixelScalar)size > 0;
}

static void
blit_not(SDL_Surface *dest, PixelScalar dest_x, PixelScalar dest_y,
         UPixelScalar dest_width, UPixelScalar dest_height,
         SDL_Surface *_src, PixelScalar src_x, PixelScalar src_y)
{
  int ret;

  /* obey the dest and src surface borders */

  if (!clip_range(dest_x, dest->w, src_x, _src->w, dest_width) ||
      !clip_range(dest_y, dest->h, src_y, _src->h, dest_height))
    return;

  ret = ::SDL_LockSurface(dest);
  if (ret != 0)
    return;

  /* convert src's pixel format */

  SDL_Surface *src = ::SDL_ConvertSurface(_src, dest->format, SDL_SWSURFACE);
  if (src == NULL) {
    ::SDL_UnlockSurface(dest);
    return;
  }

  ret = ::SDL_LockSurface(src);
  if (ret != 0) {
    ::SDL_FreeSurface(src);
    ::SDL_UnlockSurface(dest);
    return;
  }

  /* get pointers to the upper left dest/src pixel */

  unsigned char *dest_buffer = (unsigned char *)dest->pixels;
  dest_buffer += dest_y * dest->pitch +
    dest_x * dest->format->BytesPerPixel;

  unsigned char *src_buffer = (unsigned char *)src->pixels;
  src_buffer += src_y * src->pitch +
    src_x * src->format->BytesPerPixel;

  /* copy line by line */

  for (UPixelScalar y = 0; y < dest_height; ++y) {
    ::SDL_imageFilterBitNegation(src_buffer, dest_buffer,
                                 dest_width * dest->format->BytesPerPixel);
    src_buffer += src->pitch;
    dest_buffer += dest->pitch;
  }

  /* cleanup */

  ::SDL_UnlockSurface(src);
  ::SDL_FreeSurface(src);
  ::SDL_UnlockSurface(dest);
}

static void
blit_or(SDL_Surface *dest, PixelScalar dest_x, PixelScalar dest_y,
        UPixelScalar dest_width, UPixelScalar dest_height,
        SDL_Surface *_src, PixelScalar src_x, PixelScalar src_y)
{
  int ret;

  /* obey the dest and src surface borders */

  if (!clip_range(dest_x, dest->w, src_x, _src->w, dest_width) ||
      !clip_range(dest_y, dest->h, src_y, _src->h, dest_height))
    return;

  ret = ::SDL_LockSurface(dest);
  if (ret != 0)
    return;

  /* convert src's pixel format */

  SDL_Surface *src = ::SDL_ConvertSurface(_src, dest->format, SDL_SWSURFACE);
  if (src == NULL) {
    ::SDL_UnlockSurface(dest);
    return;
  }

  ret = ::SDL_LockSurface(src);
  if (ret != 0) {
    ::SDL_FreeSurface(src);
    ::SDL_UnlockSurface(dest);
    return;
  }

  /* get pointers to the upper left dest/src pixel */

  unsigned char *dest_buffer = (unsigned char *)dest->pixels;
  dest_buffer += dest_y * dest->pitch +
    dest_x * dest->format->BytesPerPixel;

  unsigned char *src_buffer = (unsigned char *)src->pixels;
  src_buffer += src_y * src->pitch +
    src_x * src->format->BytesPerPixel;

  /* copy line by line */

  for (UPixelScalar y = 0; y < dest_height; ++y) {
    ::SDL_imageFilterBitOr(src_buffer, dest_buffer, dest_buffer,
                           dest_width * dest->format->BytesPerPixel);
    src_buffer += src->pitch;
    dest_buffer += dest->pitch;
  }

  /* cleanup */

  ::SDL_UnlockSurface(src);
  ::SDL_FreeSurface(src);
  ::SDL_UnlockSurface(dest);
}

static void
BlitNotOr(SDL_Surface *dest, PixelScalar dest_x, PixelScalar dest_y,
          UPixelScalar dest_width, UPixelScalar dest_height,
          SDL_Surface *_src, PixelScalar src_x, PixelScalar src_y)
{
  int ret;

  /* obey the dest and src surface borders */

  if (!clip_range(dest_x, dest->w, src_x, _src->w, dest_width) ||
      !clip_range(dest_y, dest->h, src_y, _src->h, dest_height))
    return;

  ret = ::SDL_LockSurface(dest);
  if (ret != 0)
    return;

  /* convert src's pixel format */

  SDL_Surface *src = ::SDL_ConvertSurface(_src, dest->format, SDL_SWSURFACE);
  if (src == NULL) {
    ::SDL_UnlockSurface(dest);
    return;
  }

  ret = ::SDL_LockSurface(src);
  if (ret != 0) {
    ::SDL_FreeSurface(src);
    ::SDL_UnlockSurface(dest);
    return;
  }

  /* get pointers to the upper left dest/src pixel */

  unsigned char *dest_buffer = (unsigned char *)dest->pixels;
  dest_buffer += dest_y * dest->pitch +
    dest_x * dest->format->BytesPerPixel;

  unsigned char *src_buffer = (unsigned char *)src->pixels;
  src_buffer += src_y * src->pitch +
    src_x * src->format->BytesPerPixel;

  /* copy line by line */

  const size_t line_size = dest_width * dest->format->BytesPerPixel;
  unsigned char *tmp = new unsigned char[line_size];

  for (UPixelScalar y = 0; y < dest_height; ++y) {
    ::SDL_imageFilterBitNegation(src_buffer, tmp, line_size);
    src_buffer += src->pitch;
    ::SDL_imageFilterBitOr(tmp, dest_buffer, dest_buffer, line_size);
    dest_buffer += dest->pitch;
  }

  delete[] tmp;

  /* cleanup */

  ::SDL_UnlockSurface(src);
  ::SDL_FreeSurface(src);
  ::SDL_UnlockSurface(dest);
}

static void
blit_and(SDL_Surface *dest, PixelScalar dest_x, PixelScalar dest_y,
         UPixelScalar dest_width, UPixelScalar dest_height,
         SDL_Surface *_src, PixelScalar src_x, PixelScalar src_y)
{
  int ret;

  /* obey the dest and src surface borders */

  if (!clip_range(dest_x, dest->w, src_x, _src->w, dest_width) ||
      !clip_range(dest_y, dest->h, src_y, _src->h, dest_height))
    return;

  ret = ::SDL_LockSurface(dest);
  if (ret != 0)
    return;

  /* convert src's pixel format */

  SDL_Surface *src = ::SDL_ConvertSurface(_src, dest->format, SDL_SWSURFACE);
  if (src == NULL) {
    ::SDL_UnlockSurface(dest);
    return;
  }

  ret = ::SDL_LockSurface(src);
  if (ret != 0) {
    ::SDL_FreeSurface(src);
    ::SDL_UnlockSurface(dest);
    return;
  }

  /* get pointers to the upper left dest/src pixel */

  unsigned char *dest_buffer = (unsigned char *)dest->pixels;
  dest_buffer += dest_y * dest->pitch +
    dest_x * dest->format->BytesPerPixel;

  unsigned char *src_buffer = (unsigned char *)src->pixels;
  src_buffer += src_y * src->pitch +
    src_x * src->format->BytesPerPixel;

  /* copy line by line */

  for (UPixelScalar y = 0; y < dest_height; ++y) {
    ::SDL_imageFilterBitAnd(src_buffer, dest_buffer, dest_buffer,
                            dest_width * dest->format->BytesPerPixel);
    src_buffer += src->pitch;
    dest_buffer += dest->pitch;
  }

  /* cleanup */

  ::SDL_UnlockSurface(src);
  ::SDL_FreeSurface(src);
  ::SDL_UnlockSurface(dest);
}

void
Canvas::CopyNot(PixelScalar dest_x, PixelScalar dest_y,
                 UPixelScalar dest_width, UPixelScalar dest_height,
                 SDL_Surface *src, PixelScalar src_x, PixelScalar src_y)
{
  assert(src != NULL);

  dest_x += x_offset;
  dest_y += y_offset;

  ::blit_not(surface, dest_x, dest_y, dest_width, dest_height,
             src, src_x, src_y);
}

void
Canvas::CopyOr(PixelScalar dest_x, PixelScalar dest_y,
                UPixelScalar dest_width, UPixelScalar dest_height,
                SDL_Surface *src, PixelScalar src_x, PixelScalar src_y)
{
  assert(src != NULL);

  dest_x += x_offset;
  dest_y += y_offset;

  ::blit_or(surface, dest_x, dest_y, dest_width, dest_height,
            src, src_x, src_y);
}

void
Canvas::CopyNotOr(PixelScalar dest_x, PixelScalar dest_y,
                  UPixelScalar dest_width, UPixelScalar dest_height,
                  SDL_Surface *src, PixelScalar src_x, PixelScalar src_y)
{
  assert(src != NULL);

  dest_x += x_offset;
  dest_y += y_offset;

  ::BlitNotOr(surface, dest_x, dest_y, dest_width, dest_height,
              src, src_x, src_y);
}

void
Canvas::CopyNotOr(PixelScalar dest_x, PixelScalar dest_y,
                  UPixelScalar dest_width, UPixelScalar dest_height,
                  const Bitmap &src, PixelScalar src_x, PixelScalar src_y)
{
  assert(src.IsDefined());

  CopyNotOr(dest_x, dest_y, dest_width, dest_height,
            src.GetNative(), src_x, src_y);
}

void
Canvas::CopyAnd(PixelScalar dest_x, PixelScalar dest_y,
                 UPixelScalar dest_width, UPixelScalar dest_height,
                 SDL_Surface *src, PixelScalar src_x, PixelScalar src_y)
{
  assert(src != NULL);

  dest_x += x_offset;
  dest_y += y_offset;

  ::blit_and(surface, dest_x, dest_y, dest_width, dest_height,
             src, src_x, src_y);
}

void
Canvas::CopyNot(PixelScalar dest_x, PixelScalar dest_y,
                 UPixelScalar dest_width, UPixelScalar dest_height,
                 const Bitmap &src, PixelScalar src_x, PixelScalar src_y)
{
  assert(src.IsDefined());

  CopyNot(dest_x, dest_y, dest_width, dest_height,
           src.GetNative(), src_x, src_y);
}

void
Canvas::CopyOr(PixelScalar dest_x, PixelScalar dest_y,
                UPixelScalar dest_width, UPixelScalar dest_height,
                const Bitmap &src, PixelScalar src_x, PixelScalar src_y)
{
  assert(src.IsDefined());

  CopyOr(dest_x, dest_y, dest_width, dest_height,
          src.GetNative(), src_x, src_y);
}

void
Canvas::CopyAnd(PixelScalar dest_x, PixelScalar dest_y,
                 UPixelScalar dest_width, UPixelScalar dest_height,
                 const Bitmap &src, PixelScalar src_x, PixelScalar src_y)
{
  assert(src.IsDefined());

  CopyAnd(dest_x, dest_y, dest_width, dest_height,
           src.GetNative(), src_x, src_y);
}

void
Canvas::DrawRoundRectangle(PixelScalar left, PixelScalar top,
                        PixelScalar right, PixelScalar bottom,
                        UPixelScalar ellipse_width,
                        UPixelScalar ellipse_height)
{
  UPixelScalar radius = std::min(ellipse_width, ellipse_height) / 2;
  ::RoundRect(*this, left, top, right, bottom, radius);
}

#endif /* !OPENGL */
