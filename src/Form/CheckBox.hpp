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

#ifndef XCSOAR_FORM_CHECK_BOX_HPP
#define XCSOAR_FORM_CHECK_BOX_HPP

#include "Screen/CheckBox.hpp"

struct DialogLook;
class ContainerWindow;
class ActionListener;

/**
 * This class is used for creating buttons.
 * It is based on the WindowControl class.
 */
class CheckBoxControl : public CheckBox {
public:

  typedef void (*ClickNotifyCallback)(CheckBoxControl &button);

private:
#ifdef USE_GDI
  int id;
#endif
  ActionListener *listener;

public:
  /**
   * Constructor of the WndButton class
   * @param Parent Parent window/ContainerControl
   * @param Caption Text on the button
   * @param Function The function that should be called
   * when the button is clicked
   */
  CheckBoxControl(ContainerWindow &parent, const DialogLook &look,
                  const TCHAR *caption,
                  const PixelRect &rc,
                  const CheckBoxStyle style,
                  ClickNotifyCallback click_notify_callback = NULL);

  CheckBoxControl(ContainerWindow &parent, const DialogLook &look,
                  const TCHAR *caption,
                  const PixelRect &rc,
                  const CheckBoxStyle style,
                  ActionListener *listener, int id);

  /**
   * Set the object that will receive click events.
   */
  void SetListener(ActionListener *_listener) {
    assert(click_notify_callback == NULL);

    listener = _listener;
  }

  /**
   * Sets the function that should be called when the button is pressed
   * @param Function Pointer to the function to be called
   */
  void
  SetOnClickNotify(ClickNotifyCallback _click_notify_callback)
  {
    assert(listener == NULL);

    click_notify_callback = _click_notify_callback;
  }

  /**
   * Called when the button is clicked (either by mouse or by
   * keyboard).  The default implementation invokes the OnClick
   * callback.
   */
  virtual bool OnClicked();

#ifdef _WIN32_WCE
protected:
  virtual bool OnKeyCheck(unsigned key_code) const;
  virtual bool OnKeyDown(unsigned key_code);
#endif

private:
  /**
   * The callback-function that should be called when the button is pressed
   * @see SetOnClickNotify()
   */
  ClickNotifyCallback click_notify_callback;
};

#endif
