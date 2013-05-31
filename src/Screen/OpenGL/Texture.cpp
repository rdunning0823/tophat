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

#include "Screen/OpenGL/Texture.hpp"
#include "Screen/OpenGL/Globals.hpp"
#include "Screen/OpenGL/Features.hpp"
#include "Asset.hpp"
#include "Scope.hpp"
#include "Compiler.h"

#ifdef HAVE_OES_DRAW_TEXTURE
#include <GLES/glext.h>
#endif

#include <assert.h>

#ifndef NDEBUG
unsigned num_textures;
#endif

gcc_const gcc_unused
static GLsizei
NextPowerOfTwo(GLsizei i)
{
  GLsizei p = 1;
  while (p < i)
    p <<= 1;
  return p;
}

gcc_const
static inline GLsizei
ValidateTextureSize(GLsizei i)
{
  return OpenGL::texture_non_power_of_two ? i : NextPowerOfTwo(i);
}

gcc_const
static inline PixelSize
ValidateTextureSize(PixelSize size)
{
  return { PixelScalar(ValidateTextureSize(size.cx)),
      PixelScalar(ValidateTextureSize(size.cy)) };
}

#ifndef ANDROID

/**
 * Checks if the specified palette consists of gray shades 0x00..0xff.
 */
gcc_pure
static bool
IsLuminancePalette(const SDL_Palette *palette)
{
  if (palette->ncolors != 0x100)
    return false;

  for (unsigned i = 0; i < 0x100; ++i)
    if (palette->colors[i].r != i ||
        palette->colors[i].g != i ||
        palette->colors[i].b != i)
      return false;

  return true;
}

/**
 * Checks if the specified format is a 8 bit grayscale format.
 */
gcc_pure
static bool
IsLuminanceFormat(const SDL_PixelFormat *format)
{
  return format->palette != NULL && format->BitsPerPixel == 8 &&
    format->Rloss == 8 && format->Gloss == 8 && format->Bloss == 8 &&
    format->Rshift == 0 && format->Gshift == 0 && format->Bshift == 0 &&
    format->Rmask == 0 && format->Gmask == 0 && format->Bmask == 0 &&
    IsLuminancePalette(format->palette);
}

/**
 * Load data into the current texture.  Fixes alignment to the next
 * power of two if needed.
 */
static void
LoadTextureAutoAlign(GLint internal_format,
                     GLsizei width, GLsizei height,
                     GLenum format, GLenum type, const GLvoid *pixels)
{
  GLsizei width2 = ValidateTextureSize(width);
  GLsizei height2 = ValidateTextureSize(height);

  if (width2 == width && height2 == height)
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0,
                 format, type, pixels);
  else {
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width2, height2, 0,
                 format, type, NULL);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                    format, type, pixels);
  }
}

static bool
LoadLuminanceTexture(const SDL_Surface *surface)
{
  assert(IsLuminanceFormat(surface->format));

  glPixelStorei(GL_UNPACK_ROW_LENGTH,
                surface->pitch / surface->format->BytesPerPixel);
  LoadTextureAutoAlign(GL_LUMINANCE, surface->w, surface->h,
                       GL_LUMINANCE, GL_UNSIGNED_BYTE,
                       surface->pixels);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  return true;
}

/**
 * Loads a SDL_Surface into the current texture.  Attempts to
 * auto-detect the pixel format.
 *
 * @return false if the pixel format is not supported
 */
static bool
LoadSurfaceIntoTexture(const SDL_Surface *surface)
{

  assert(surface != NULL);
  assert(surface->format != NULL);

  const SDL_PixelFormat *fmt = surface->format;
  if (IsLuminanceFormat(fmt))
    return LoadLuminanceTexture(surface);

  if (fmt->palette != NULL)
    /* OpenGL does not support a hardware palette */
    return false;

  GLenum format, type;
  if (fmt->BitsPerPixel == 16 && fmt->BytesPerPixel == 2 &&
      fmt->Rmask == 0xf800 && fmt->Gmask == 0x07e0 &&
      fmt->Bmask == 0x1f) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    format = GL_RGB;
    type = GL_UNSIGNED_SHORT_5_6_5;
  } else if (fmt->BitsPerPixel == 24 && fmt->BytesPerPixel == 3 &&
             fmt->Rmask == 0xff0000 && fmt->Gmask == 0xff00 &&
             fmt->Bmask == 0xff) {
#ifdef ANDROID
    /* big endian */
    format = GL_RGB;
#else
    /* little endian */
    format = GL_BGR;
#endif
    type = GL_UNSIGNED_BYTE;
#ifndef ANDROID
  } else if ((fmt->BitsPerPixel == 24 || fmt->BitsPerPixel == 32) &&
             fmt->BytesPerPixel == 4 && fmt->Rmask == 0xff0000 &&
             fmt->Gmask == 0xff00 && fmt->Bmask == 0xff) {
    format = GL_BGRA;
    type = GL_UNSIGNED_BYTE;
#endif
  } else
    return false;

  glPixelStorei(GL_UNPACK_ROW_LENGTH,
                surface->pitch / surface->format->BytesPerPixel);
  LoadTextureAutoAlign(GL_RGB, surface->w, surface->h,
                       format, type, surface->pixels);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  return true;
}

#endif

GLTexture::GLTexture(UPixelScalar _width, UPixelScalar _height)
  :width(_width), height(_height)
#ifndef HAVE_OES_DRAW_TEXTURE
  , allocated_width(ValidateTextureSize(_width)),
   allocated_height(ValidateTextureSize(_height))
#endif
{
  /* enable linear filtering for the terrain texture */
  Initialise(true);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
               ValidateTextureSize(width), ValidateTextureSize(height),
               0, GL_RGB, GetType(), NULL);
}

void
GLTexture::ResizeDiscard(PixelSize new_size)
{
  const PixelSize validated_size = ValidateTextureSize(new_size);
  const PixelSize old_size = GetAllocatedSize();

  width = new_size.cx;
  height = new_size.cy;

  if (validated_size == old_size)
    return;

#ifndef HAVE_OES_DRAW_TEXTURE
  allocated_width = validated_size.cx;
  allocated_height = validated_size.cy;
#endif

  Bind();

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
               validated_size.cx, validated_size.cy,
               0, GL_RGB, GetType(), NULL);

}

#ifndef ANDROID

void
GLTexture::Load(SDL_Surface *src)
{
  width = src->w;
  height = src->h;

#ifndef HAVE_OES_DRAW_TEXTURE
  allocated_width = ValidateTextureSize(width);
  allocated_height = ValidateTextureSize(src->h);
#endif

  if (!LoadSurfaceIntoTexture(src)) {
    /* try again after conversion */
    SDL_PixelFormat format;
    format.palette = NULL;
#ifdef ANDROID
    format.BitsPerPixel = 16;
    format.BytesPerPixel = 2;
    format.Rloss = 3;
    format.Gloss = 2;
    format.Bloss = 3;
    format.Rshift = 11;
    format.Gshift = 5;
    format.Bshift = 0;
    format.Rmask = 0xf800;
    format.Gmask = 0x07e0;
    format.Bmask = 0x001f;
#else
    format.BitsPerPixel = 24;
    format.BytesPerPixel = 4;
    format.Rloss = format.Gloss = format.Bloss = 0;
    format.Rshift = 16;
    format.Gshift = 8;
    format.Bshift = 0;
    format.Rmask = 0xff0000;
    format.Gmask = 0x00ff00;
    format.Bmask = 0x0000ff;
#endif
    format.Aloss = 8;
    format.Ashift = 0;
    format.Amask = 0;
    format.colorkey = 0;
    format.alpha = 0xff;

    SDL_Surface *surface = ::SDL_ConvertSurface(src, &format, SDL_SWSURFACE);
    assert(surface != NULL);

    LoadSurfaceIntoTexture(surface);
    SDL_FreeSurface(surface);
  }
}

#endif

void
GLTexture::Draw(PixelScalar dest_x, PixelScalar dest_y,
                UPixelScalar dest_width, UPixelScalar dest_height,
                PixelScalar src_x, PixelScalar src_y,
                UPixelScalar src_width, UPixelScalar src_height) const
{
#ifdef HAVE_OES_DRAW_TEXTURE
  const GLint rect[4] = { src_x, src_y + src_height, src_width,
                          /* negative height to flip the texture */
                          -(int)src_height };
  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);

  /* glDrawTexiOES() circumvents the projection settings, thus we must
     roll our own translation */
  glDrawTexiOES(OpenGL::translate_x + dest_x,
                OpenGL::screen_height - OpenGL::translate_y - dest_y - dest_height,
                0, dest_width, dest_height);
#else
  const PixelScalar vertices[] = {
    dest_x, dest_y,
    PixelScalar(dest_x + dest_width), dest_y,
    dest_x, PixelScalar(dest_y + dest_height),
    PixelScalar(dest_x + dest_width), PixelScalar(dest_y + dest_height),
  };

  glVertexPointer(2, GL_VALUE, 0, vertices);

  const PixelSize allocated = GetAllocatedSize();
  GLfloat x0 = (GLfloat)src_x / allocated.cx;
  GLfloat y0 = (GLfloat)src_y / allocated.cy;
  GLfloat x1 = (GLfloat)(src_x + src_width) / allocated.cx;
  GLfloat y1 = (GLfloat)(src_y + src_height) / allocated.cy;

  const GLfloat coord[] = {
    x0, y0,
    x1, y0,
    x0, y1,
    x1, y1,
  };

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, 0, coord);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
}

void
GLTexture::DrawFlipped(PixelRect dest, PixelRect src) const
{
#ifdef HAVE_OES_DRAW_TEXTURE
  const GLint rect[4] = { src.left, src.top,
                          src.right - src.left, src.bottom - src.top };
  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);

  /* glDrawTexiOES() circumvents the projection settings, thus we must
     roll our own translation */
  glDrawTexiOES(OpenGL::translate_x + dest.left,
                OpenGL::screen_height - OpenGL::translate_y - dest.bottom,
                0, dest.right - dest.left, dest.bottom - dest.top);
#else
  const PixelScalar vertices[] = {
    dest.left, dest.top,
    dest.right, dest.top,
    dest.left, dest.bottom,
    dest.right, dest.bottom,
  };

  glVertexPointer(2, GL_VALUE, 0, vertices);

  const PixelSize allocated = GetAllocatedSize();
  GLfloat x0 = (GLfloat)src.left / allocated.cx;
  GLfloat y0 = (GLfloat)src.top / allocated.cy;
  GLfloat x1 = (GLfloat)src.right / allocated.cx;
  GLfloat y1 = (GLfloat)src.bottom / allocated.cy;

  const GLfloat coord[] = {
    x0, y1,
    x1, y1,
    x0, y0,
    x1, y0,
  };

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, 0, coord);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
}
