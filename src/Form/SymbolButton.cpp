/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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

#include "Form/SymbolButton.hpp"
#include "Formatter/HexColor.hpp"
#include "Look/DialogLook.hpp"
#include "Screen/Canvas.hpp"
#include "Look/IconLook.hpp"
#include "Screen/Bitmap.hpp"
#include "Renderer/SymbolRenderer.hpp"
#include "Language/Language.hpp"
#include "Resources.hpp"
#include "Asset.hpp"
#include "Screen/Layout.hpp"
#include "UIGlobals.hpp"

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Texture.hpp"
#include "Screen/OpenGL/Scope.hpp"
#include "Screen/OpenGL/Compatibility.hpp"
#endif

static void
DrawIcon1(Canvas &canvas, PixelRect rc, const Bitmap &bmp, bool pressed)
{

  const PixelSize bitmap_size = bmp.GetSize();
  const int offsety = (rc.bottom - rc.top - bitmap_size.cy) / 2;

#ifdef ENABLE_OPENGL
    const int offsetx = (rc.right - rc.left - bitmap_size.cx) / 2;

    if (pressed) {
      OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

      /* invert the texture color */
      OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
      OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
      OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_ONE_MINUS_SRC_COLOR);

      /* copy the texture alpha */
      OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
      OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
      OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
    } else
      /* simple copy */
      OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    const GLEnable scope(GL_TEXTURE_2D);
    const GLBlend blend(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLTexture &texture = *bmp.GetNative();
    texture.Bind();
    texture.Draw(rc.left + offsetx, rc.top + offsety);

#else
    const int offsetx = (rc.right - rc.left - bitmap_size.cx / 2) / 2;
    if (pressed) // black background
      canvas.CopyNotOr(rc.left + offsetx,
                       rc.top + offsety,
                       bitmap_size.cx / 2,
                       bitmap_size.cy,
                       bmp,
                       bitmap_size.cx / 2, 0);

    else
      canvas.CopyAnd(rc.left + offsetx,
                      rc.top + offsety,
                      bitmap_size.cx / 2,
                      bitmap_size.cy,
                      bmp,
                      bitmap_size.cx / 2, 0);
#endif
}

void
WndSymbolButton::OnPaint(Canvas &canvas)
{
  const ButtonLook &look = renderer.GetLook();

  const bool pressed = IsDown();
  const bool focused = draw_focus_override || (HasCursorKeys() ? HasFocus() : pressed);

  PixelRect rc = canvas.GetRect();
  renderer.DrawButton(canvas, rc, focused, pressed, transparent);
  // If button has text on it
  const tstring caption = GetText();
  if (caption.empty())
    return;

  rc = renderer.GetDrawingRect(rc, pressed);

  canvas.SelectNullPen();
  if (!IsEnabled())
    canvas.Select(look.disabled.brush);
  else if (focused)
    canvas.Select(look.focused.foreground_brush);
  else
    canvas.Select(look.standard.foreground_brush);

  const char ch = (char)caption[0];

  RGB8Color color;

  // Draw arrow symbol instead of <
  if (caption == _T("<"))
    SymbolRenderer::DrawArrow(canvas, rc, SymbolRenderer::LEFT);

  // Draw arrow symbol instead of >
  else if (caption == _T(">"))
    SymbolRenderer::DrawArrow(canvas, rc, SymbolRenderer::RIGHT);

  // Draw arrow symbol instead of ^
  else if (ch == '^')
    SymbolRenderer::DrawArrow(canvas, rc, SymbolRenderer::UP);

  // Draw arrow symbol instead of v
  else if (ch == '^' || ch == 'v')
    SymbolRenderer::DrawArrow(canvas, rc, SymbolRenderer::DOWN);

  // Draw symbols instead of + and -
  else if (ch == '+' || ch == '-')
    SymbolRenderer::DrawSign(canvas, rc, ch == '+');

  /* "play" buttons */
  else if (caption == _T("<<"))
    SymbolRenderer::DrawDoubleArrow(canvas, rc, SymbolRenderer::LEFT);
  else if (caption == _T(">>"))
    SymbolRenderer::DrawDoubleArrow(canvas, rc, SymbolRenderer::RIGHT);
  else if (caption == _T("||"))
    SymbolRenderer::DrawPause(canvas, rc);
  else if (caption == _T("[]"))
    SymbolRenderer::DrawStop(canvas, rc);

  // Draw Fly bitmap
  else if (caption == _T("Fly")) {
    Bitmap launcher1_bitmap(IDB_LAUNCHER1);
    launcher1_bitmap.EnableInterpolation();
    canvas.ClearWhite();
    if (pressed)
      canvas.StretchNot(launcher1_bitmap);
    else
      canvas.Stretch(launcher1_bitmap);
  }

  // Draw Simulator bitmap
  else if (caption == _T("Simulator")) {
    Bitmap launcher2_bitmap(IDB_LAUNCHER2);
    launcher2_bitmap.EnableInterpolation();
    canvas.ClearWhite();
    if (pressed)
      canvas.StretchNot(launcher2_bitmap);
    else
      canvas.Stretch(launcher2_bitmap);
  }

  else if (ParseHexColor(caption.c_str(), color)) {
    rc.Grow(-3);
    canvas.DrawFilledRectangle(rc, Color(color));

    //draw search icon
  } else if (caption == _("Search") || caption == _("SearchChecked")) {
    const IconLook &icon_look = UIGlobals::GetIconLook();
    const Bitmap *bmp;
    bmp = caption == _("Search") ? &icon_look.hBmpSearch :
        &icon_look.hBmpSearchChecked;

    DrawIcon1(canvas, rc, *bmp, pressed);
  }

  //draw gear for set up icon
  else if (caption == _("Setup")) {
    const IconLook &icon_look = UIGlobals::GetIconLook();
    const Bitmap &bmp = icon_look.hBmpTabSettings;
    DrawIcon1(canvas, rc, bmp, pressed);
  }
  else if (caption == _("_X")) {
    const IconLook &icon_look = UIGlobals::GetIconLook();
    const Bitmap &bmp = icon_look.hBmpClose;
    DrawIcon1(canvas, rc, bmp, pressed);

  } else if (caption.compare(0, 9, _T("_chkmark_")) == 0) { // new
    const Font &font = *look.font;
    tstring text = caption.substr(9, 99).c_str();
    PixelSize sz_text = font.TextSize(text.c_str());
    UPixelScalar padding = Layout::GetTextPadding();


    const IconLook &icon_look = UIGlobals::GetIconLook();
    const Bitmap &bmp = icon_look.hBmpCheckMark;
    PixelSize sz_icon = bmp.GetSize();
    PixelRect rc_icon = rc;
    rc_icon.left = (rc.GetSize().cx - sz_icon.cx - sz_text.cx - padding) / 2;
    rc_icon.right = rc_icon.left + sz_icon.cx;

    PixelRect rc_caption = rc;
    rc_caption.left = rc_icon.right + padding;
    rc_caption.right = rc_caption.left + sz_text.cx + padding;

    DrawIcon1(canvas, rc_icon, bmp, pressed);

    canvas.SetBackgroundTransparent();
    if (!IsEnabled())
      canvas.SetTextColor(look.disabled.color);
    else if (focused)
      canvas.SetTextColor(look.focused.foreground_color);
    else
      canvas.SetTextColor(look.standard.foreground_color);

    canvas.Select(*look.font);

#ifndef USE_GDI
  unsigned style = GetTextStyle();

  canvas.DrawFormattedText(&rc_caption, text.c_str(), style);
#else
  unsigned style = DT_CENTER | DT_NOCLIP | DT_WORDBREAK;

  PixelRect text_rc = rc_caption;
  canvas.DrawFormattedText(&text_rc, text.c_str(), style | DT_CALCRECT);
  text_rc.right = rc.right;

  PixelScalar offset = rc.bottom - text_rc.bottom;
  if (offset > 0) {
    offset /= 2;
    text_rc.top += offset;
    text_rc.bottom += offset;
  }

  canvas.DrawFormattedText(&text_rc, text.c_str(), style);
#endif



  } else if (caption == _("More") || caption == _("Less")) {
    bool up = caption == _("Less");
    // Draw arrow symbols instead of v and ^
    const Font &font = *look.font;
    canvas.Select(font);
    canvas.SetBackgroundTransparent();
    PixelSize text_size = font.TextSize(caption.c_str());

    UPixelScalar size = std::min(rc.right - rc.left, rc.bottom - rc.top) / 8;
    size = std::min(size, (UPixelScalar)(font.GetHeight() / 2));
    unsigned offset_x = (rc.right - rc.left -
        (text_size.cx + size * 2 + Layout::Scale(1))) / 2;

    unsigned left = rc.left + offset_x;

    RasterPoint Arrow[3];
    Arrow[0].x = left + size;
    Arrow[0].y = (rc.top + rc.bottom) / 2 +
                 (int)(up ? size : -size) * 2 / 3;
    Arrow[1].x = left;
    Arrow[1].y = (rc.top + rc.bottom) / 2 +
                 (up ? -size : size);
    Arrow[2].x = left - size;
    Arrow[2].y = (rc.top + rc.bottom) / 2 +
                 (int)(up ? size : -size) * 2 / 3;

    canvas.DrawText(left + size * 2 + Layout::Scale(1),
                    (rc.bottom - rc.top - text_size.cy) / 2 , caption.c_str());
    canvas.DrawTriangleFan(Arrow, 3);


    canvas.Select(Pen(Layout::Scale(1), COLOR_BLACK));
    canvas.SelectHollowBrush();
    canvas.DrawCircle(left, (rc.top + rc.bottom) / 2, (UPixelScalar)(size * 1.5));
  } else {
    WndButton::OnPaint(canvas);
  }
}
