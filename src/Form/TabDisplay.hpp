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

#ifndef XCSOAR_FORM_TAB_DISPLAY_HPP
#define XCSOAR_FORM_TAB_DISPLAY_HPP

#include "Screen/PaintWindow.hpp"
#include "Util/StaticArray.hpp"
#include "Util/StaticString.hpp"

struct DialogLook;
class Bitmap;
class ContainerWindow;
class TabBarControl;

/**
 * TabButton class holds display and callbacks data for a single tab
 */
class TabButton {
public:
  StaticString<32> caption;
  const Bitmap *bitmap;
  PixelRect rc;

public:
  TabButton(const TCHAR* _caption, const Bitmap *_bitmap)
    :bitmap(_bitmap)
  {
    caption = _caption;
    rc.left = 0;
    rc.top = 0;
    rc.right = 0;
    rc.bottom = 0;
  };
};

/**
 * TabDisplay class handles onPaint callback for TabBar UI
 * and handles Mouse and key events
 * TabDisplay uses a pointer to TabBarControl
 * to show/hide the appropriate pages in the Container Class
 */
class TabDisplay: public PaintWindow
{
protected:
  TabBarControl& tab_bar;
  const DialogLook &look;

  StaticArray<TabButton *, 32> buttons;

  bool dragging; // tracks that mouse is down and captured
  int down_index; // index of tab where mouse down occurred
  bool drag_off_button; // set by mouse_move
  bool flip_orientation;

public:
  /**
   *
   * @param parent
   * @param _theTabBar. An existing TabBar object
   * @param left. Left position of the tab bar box in the parent window
   * @param top Top position of the tab bar box in the parent window
   * @param width Width of tab bar box in the parent window
   * @param height Height of tab bar box in the parent window
   */
  TabDisplay(TabBarControl& _theTabBar, const DialogLook &look,
             ContainerWindow &parent,
             PixelScalar left, PixelScalar top,
             UPixelScalar width, UPixelScalar height,
             bool _flipOrientation = false);

  virtual ~TabDisplay();

  const DialogLook &GetLook() const {
    return look;
  }

  /**
   * Paints one button
   */
  static void PaintButton(Canvas &canvas, const unsigned CaptionStyle,
                          const TCHAR *caption, const PixelRect &rc,
                          const Bitmap *bmp, const bool isDown, bool inverse);

  unsigned GetSize() const {
    return buttons.size();
  }

  void Add(const TCHAR *caption, const Bitmap *bmp = NULL);

  gcc_pure
  const TCHAR *GetCaption(unsigned i) const {
    return buttons[i]->caption.c_str();
  }

  /**
   * @return -1 if there is no button at the specified position
   */
  gcc_pure
  int GetButtonIndexAt(RasterPoint p) const;

public:
  UPixelScalar GetTabHeight() const {
    return this->GetHeight();
  }

  UPixelScalar GetTabWidth() const {
    return this->GetWidth();
  }

private:
  /**
   * calculates the size and position of ith button
   * works in landscape or portrait mode
   * @param i index of button
   * @return Rectangle of button coordinates
   */
  gcc_pure
  const PixelRect &GetButtonSize(unsigned i) const;

protected:
  /**
   * paints the tab buttons
   * @param canvas
   */
  virtual void OnPaint(Canvas &canvas);
  //ToDo: support function buttons

  /**
   * track key presses to navigate without mouse
   * @param key_code
   * @return
   */
  virtual void OnKillFocus();
  virtual void OnSetFocus();
  virtual bool OnKeyCheck(unsigned key_code) const;
  virtual bool OnKeyDown(unsigned key_code);

  /**
   * track mouse clicks
   */
  virtual bool OnMouseDown(PixelScalar x, PixelScalar y);
  virtual bool OnMouseUp(PixelScalar x, PixelScalar y);
  virtual bool OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys);

  void EndDrag();
};

#endif
