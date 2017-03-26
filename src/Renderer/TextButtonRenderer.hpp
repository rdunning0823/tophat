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

#ifndef XCSOAR_TEXT_BUTTON_RENDERER_HPP
#define XCSOAR_TEXT_BUTTON_RENDERER_HPP

#include "ButtonRenderer.hpp"
#include "TextRenderer.hpp"
#include "Util/StaticString.hxx"

class Font;

/**
 * A #ButtonRenderer instance that renders a regular button frame and
 * some text.
 */
class TextButtonRenderer : public ButtonRenderer {
protected:
  ButtonFrameRenderer frame_renderer;

  TextRenderer text_renderer;

  StaticString<96> caption;

  bool use_large_font;

public:
  explicit TextButtonRenderer(const ButtonLook &_look)
    :frame_renderer(_look),
     use_large_font(false) {
    text_renderer.SetCenter();
    text_renderer.SetVCenter();
    text_renderer.SetControl();
  }

  TextButtonRenderer(const ButtonLook &_look,
                     StaticString<96>::const_pointer _caption,
                     bool _use_large_font = false)
    :frame_renderer(_look), caption(_caption),
     use_large_font(_use_large_font) {
    text_renderer.SetCenter();
    text_renderer.SetVCenter();
    text_renderer.SetControl();
  }

  const ButtonLook &GetLook() const {
    return frame_renderer.GetLook();
  }

  StaticString<96>::const_pointer GetCaption() const {
    return caption;
  }

  void SetCaption(StaticString<96>::const_pointer _caption) {
    caption = _caption;
    text_renderer.InvalidateLayout();
  }

  void SetRounded(bool rounded) override {
    frame_renderer.SetRounded(rounded);
  }

  gcc_pure
  virtual unsigned GetMinimumButtonWidth() const override;



  /**
   * @param force_transparent_background: draws transparent background and
   * font with white outline around black text
   */
  void DrawButton(Canvas &canvas, const PixelRect &rc,
                  bool enabled, bool focused, bool pressed,
                  bool force_transparent_background) const override;

protected:

  const Font& GetFont() const;

  /**
   * @param rc: rc of text
   * @return rc required to print text (adjusts for multiple lines)
   */
  PixelSize GetCaptionSize(Canvas &canvas, PixelRect rc,
                           const TCHAR *text) const;

  void DrawCaption(Canvas &canvas, const PixelRect &rc,
                   bool enabled, bool focused, bool pressed,
                   bool outlined_text = false) const;

  /**
   * @param outlined_text. draws white outline around black text
   */
  void DrawCaption(Canvas &canvas, StaticString<96>::const_pointer _caption,
                   const PixelRect &rc,
                   bool enabled, bool focused, bool pressed,
                   bool outlined_text) const;

};

#endif
