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

#ifndef XCSOAR_OVERLAY_BUTTON_WIDGET_HPP
#define XCSOAR_OVERLAY_BUTTON_WIDGET_HPP

#include "TophatWidgets/MapOverlayButton.hpp"
#include "TophatWidgets/MapOverlayBaseWidget.hpp"
#include "Form/ActionListener.hpp"

struct IconLook;
class ContainerWindow;
class Bitmap;
struct PixelRect;
struct OverlayButtonLook;

class OverlayButtonWidget : public MapOverlayBaseWidget, protected ActionListener {
protected:
  bool prepared;

  /**
   * a customized copy of button_look
   */
  ButtonLook white_look;

  /**
   * the bitmap displayed in the button
   */
  const Bitmap *bmp;

  const OverlayButtonLook &overlay_button_look;

public:
  OverlayButtonWidget();

  /**
   * Shows or hides the widgets based on these parameters
   * @rc. the rc of the map
   * @is_panning.  is the map in panning mode
   * @is_main_window_widget. is the mainWindow's widget non-NULL
   * @is_map. is the map non-NULL
   */
  virtual void UpdateVisibility(const PixelRect &rc, bool is_panning,
                                bool is_main_window_widget, bool is_map,
                                bool is_top_widget) override = 0;

  /**
   * Must call Prepare() at end of derived class Prepare()
   * Derived class Prepare() must call SetBitmap()
   */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Unprepare() override;
  virtual void Show(const PixelRect &rc) override;
  virtual void Hide() override;

  /* moves the widget only if it needs to be moved
   * This eliminates performance issues
   * @param rc new position
   */
  virtual void Move(const PixelRect &rc) override;
  MapOverlayButton& CreateButton(ContainerWindow &parent,
                                 const ButtonLook &button_look,
                                 const IconLook &icon_look,
                                 const PixelRect &rc);

  /**
   * How much height does this widget use at the bottom right of the map screen
   */
  virtual UPixelScalar HeightFromBottomRight() override;

  /**
   * returns width of button
   */
  UPixelScalar GetWidth() const;

  /**
   * returns height of button
   */
  UPixelScalar GetHeight() const;

  /**
   * Position of the button displayed
   */
  PixelRect GetPosition() const;

  /**
   * The OnAction is derived from ActionListener
   */
  virtual void OnAction(int id) override = 0;

protected:
  /**
   * Button must be created before this is called
   */
  void SetBitmap(const Bitmap *_bmp) {
//    assert(GetButton().GetCaption().length() == 0); TODO Fix syntax
    bmp = _bmp;
  }

  /**
   * Button must be created before this is called
   */
  MapOverlayButton &GetButton() {
    assert(prepared == true);
    return (MapOverlayButton&)WindowWidget::GetWindow();
  }

  /**
   * Button must be created before this is called
   */
  void SetText(const TCHAR * _text) {
    assert(bmp == nullptr);
    assert(prepared == true);
    GetButton().SetCaption(_text);
  }

  const TCHAR *GetText() {
    return GetButton().GetCaption();
  }

  /**
   * Updates subscript text of button
   * Button must be created before this is called
   */
  void SetSubscriptText(const TCHAR * _text) {
    assert(bmp == nullptr);
    assert(prepared == true);
    GetButton().SetSubscripText(_text);
  }

  /**
   * Updates line two text of button
   * Button must be created before this is called
   */
  void SetLineTwoText(const TCHAR * _text) {
    assert(bmp == nullptr);
    assert(prepared == true);
    GetButton().SetLineTwoText(_text);
  }
};

#endif
