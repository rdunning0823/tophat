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

#ifndef XCSOAR_FORM_TABBAR_HPP
#define XCSOAR_FORM_TABBAR_HPP

#include "Screen/ContainerWindow.hpp"
#include "PagerWidget.hpp"

struct DialogLook;
class Bitmap;
class ContainerWindow;
class TabDisplay;
class TabButton;

/** TabBarControl displays tabs that show/hide the windows
 * associated with each tab.  For example a "Panel" control.
 * It can also display buttons with no associated Window.
 * Supports pre and post- tab click callbacks
 * Each tab must be added via code (not via XML)
 * ToDo: support lazy loading
 */
class TabBarControl : public ContainerWindow {
  typedef void (*PageFlippedCallback)();

  PagerWidget pager;

  TabDisplay * tab_display;
  const UPixelScalar tab_line_height;
  bool flip_orientation;

  PageFlippedCallback page_flipped_callback;

public:
  /**
   * Constructor used for stand-alone TabBarControl
   * @param parent
   * @param x, y Location of the tab bar (unused)
   * @param width, height.  Size of the tab bar
   * @param style
   * @return
   */
  TabBarControl(ContainerWindow &parent, const DialogLook &look,
                PixelScalar x, PixelScalar y,
                UPixelScalar width, UPixelScalar height,
                const WindowStyle style = WindowStyle(),
                bool _flipOrientation = false);

  ~TabBarControl();

  void SetPageFlippedCallback(PageFlippedCallback _page_flipped_callback) {
    assert(page_flipped_callback == NULL);
    assert(_page_flipped_callback != NULL);

    page_flipped_callback = _page_flipped_callback;
  }

private:
  static constexpr unsigned TabLineHeightInitUnscaled = 5;

public:
  unsigned AddTab(Widget *widget, const TCHAR *caption, const Bitmap *bmp = NULL);

public:
  gcc_pure
  unsigned GetTabCount() const {
    return pager.GetSize();
  }

  gcc_pure
  unsigned GetCurrentPage() const {
    return pager.GetCurrentIndex();
  }

  gcc_pure
  const Widget &GetCurrentWidget() const {
    return pager.GetCurrentWidget();
  }

  void ClickPage(unsigned i);

  void SetCurrentPage(unsigned i);
  void NextPage();
  void PreviousPage();

  bool Save(bool &changed, bool &require_restart) {
    return pager.Save(changed, require_restart);
  }

  gcc_pure
  UPixelScalar GetTabHeight() const;

  gcc_pure
  UPixelScalar GetTabWidth() const;

  gcc_pure
  const TCHAR *GetButtonCaption(unsigned i) const;

  UPixelScalar GetTabLineHeight() const {
    return tab_line_height;
  }

protected:
  virtual void OnCreate();
  virtual void OnDestroy();

#ifdef HAVE_CLIPPING
  virtual void OnPaint(Canvas &canvas) gcc_override;
#endif
};

#endif
