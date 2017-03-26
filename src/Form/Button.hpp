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

#ifndef XCSOAR_FORM_BUTTON_HPP
#define XCSOAR_FORM_BUTTON_HPP

#include "Screen/PaintWindow.hpp"
#include "Util/StaticString.hxx"
#include "Renderer/SymbolButtonRenderer.hpp"



#include <tchar.h>

struct ButtonLook;
class ContainerWindow;
class ActionListener;

/**
 * This class is used for creating buttons.
 */
class Button : public PaintWindow {
  bool dragging, down;

  ButtonRenderer *renderer;

  ActionListener *listener;
  int id;
  /** draw no background when not pressed, regardless of look */
  bool transparent_background_force;

  /**
   * This flag specifies whether the button is "selected".  The
   * "selected" button in a #ButtonPanel is the button that will be
   * triggered by the #KEY_RETURN.  On some devices without touch
   * screen, cursor keys left/right can be used to navigate the
   * #ButtonPanel.
   */
  bool selected;

public:
  Button(ContainerWindow &parent, const PixelRect &rc,
         WindowStyle style, ButtonRenderer *_renderer,
         ActionListener &_listener, int _id)
  :transparent_background_force(false) {
    Create(parent, rc, style, _renderer, _listener, _id);
  }

  Button(ContainerWindow &parent, const ButtonLook &look,
         const TCHAR *caption, const PixelRect &rc,
         WindowStyle style,
         ActionListener &_listener, int _id)
  :transparent_background_force(false) {
    Create(parent, look, caption, rc, style, _listener, _id);
  }

  Button():listener(nullptr), transparent_background_force(false) {}

  virtual ~Button();

  void Create(ContainerWindow &parent, const PixelRect &rc,
              WindowStyle style, ButtonRenderer *_renderer);

  void Create(ContainerWindow &parent, const ButtonLook &look,
              const TCHAR *caption, const PixelRect &rc,
              WindowStyle style, bool use_large_font = false);

  void Create(ContainerWindow &parent, const PixelRect &rc,
              WindowStyle style, ButtonRenderer *_renderer,
              ActionListener &listener, int id);

  void Create(ContainerWindow &parent, const ButtonLook &look,
              const TCHAR *caption, const PixelRect &rc,
              WindowStyle style,
              ActionListener &listener, int id);

  /**
   * Set the object that will receive click events.
   */
  void SetListener(ActionListener &_listener, int _id) {
    id = _id;
    listener = &_listener;
  }

  ButtonRenderer &GetRenderer() {
    return *renderer;
  }

  /**
   * Set a new caption.  This method is a wrapper for
   * #TextButtonRenderer and may only be used if created with a
   * #TextButtonRenderer instance.
   */
  void SetCaption(const TCHAR *caption);

  StaticString<96>::const_pointer GetCaption();

  void SetSelected(bool _selected);

  gcc_pure
  unsigned GetMinimumWidth() const;

  /**
   * will not paint the background when not pressed regardless of
   * settings in look
   */
  void SetForceTransparent(bool value) {
    transparent_background_force = value;
  }

  /**
   * Simulate a click on this button.
   */
  void Click();

protected:
  /**
   * Called when the button is clicked (either by mouse or by
   * keyboard).  The default implementation invokes the OnClick
   * callback.
   */
  virtual bool OnClicked();

/* virtual methods from class Window */
  void OnDestroy() override;

  bool OnKeyCheck(unsigned key_code) const override;
  bool OnKeyDown(unsigned key_code) override;
  bool OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys) override;
  bool OnMouseDown(PixelScalar x, PixelScalar y) override;
  bool OnMouseUp(PixelScalar x, PixelScalar y) override;
  void OnSetFocus() override;
  void OnKillFocus() override;
  void OnCancelMode() override;

  void OnPaint(Canvas &canvas) override;
  bool IsDown() {
    return down;
  }

private:
  void SetDown(bool _down);
};

class WndSymbolButton : public Button {
public:
  WndSymbolButton(ContainerWindow &parent,
                  const ButtonLook &look,
                  const TCHAR *caption,
                  const PixelRect &rc,
                  WindowStyle style,
                  ActionListener &_listener,
                  int _id);
  WndSymbolButton() {};

  void Create(ContainerWindow &parent,
              const ButtonLook &look,
              const TCHAR *caption,
              const PixelRect &rc,
              WindowStyle style,
              ActionListener &_listener,
              int _id);

  void SetCaption(const TCHAR *caption);
  void SetPrefixIcon(SymbolButtonRenderer::PrefixIcon);

};

#endif
