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

#include "MainMenuButtonWidget.hpp"
#include "MapOverlayButton.hpp"
#include "Form/SymbolButton.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Look/IconLook.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/Layout.hpp"
#include "Language/Language.hpp"
#include "Input/InputEvents.hpp"
#include "Interface.hpp"
#include "MainWindow.hpp"
#include "Look/Look.hpp"
#include "Screen/Fonts.hpp"


void
MainMenuButton::OnPaint(Canvas &canvas)
{
  PixelRect rc = {
    PixelScalar(0), PixelScalar(0), PixelScalar(canvas.get_width()),
    PixelScalar(canvas.get_height())
  };

  bool pressed = is_down();
#ifdef ENABLE_OPENGL
  bool transparent = true;
#else
  bool transparent = false;
#endif
  //Todo fix the GDI rendering so it draws transparent correctly
  renderer.DrawButton(canvas, rc, HasFocus(), pressed, transparent);
  rc = renderer.GetDrawingRect(rc, pressed);

  canvas.SelectNullPen();
  if (!IsEnabled())
    canvas.Select(look.button.disabled.brush);
  else
    canvas.Select(look.button.standard.foreground_brush);
  const Bitmap *bmp = &icon_look.hBmpMenuButton;
  const PixelSize bitmap_size = bmp->GetSize();
  const int offsetx = (rc.right - rc.left - bitmap_size.cx / 2) / 2;
  const int offsety = (rc.bottom - rc.top - bitmap_size.cy) / 2;
  canvas.CopyAnd(rc.left + offsetx,
                  rc.top + offsety,
                  bitmap_size.cx / 2,
                  bitmap_size.cy,
                  *bmp,
                  bitmap_size.cx / 2, 0);
}

void
MainMenuButtonWidget::UpdateVisibility(const PixelRect &rc,
                                       bool is_panning,
                                       bool is_main_window_widget,
                                       bool is_map)
{
  if (is_map && !is_main_window_widget && !is_panning)
    Show(rc);
  else
    Hide();
}

UPixelScalar
MainMenuButtonWidget::HeightFromBottomRight()
{
  return bitmap_size_raw.cy * MapOverlayButton::GetScale() + Layout::Scale(3);
}

void
MainMenuButtonWidget::Prepare(ContainerWindow &parent,
                              const PixelRect &rc)
{
  const IconLook &icon_look = CommonInterface::main_window->GetLook().icon;

  white_look.Initialise(Fonts::map_bold, Fonts::map, Fonts::map_label,
                        Fonts::map_bold, Fonts::map_bold);
  white_look.SetBackgroundColor(COLOR_WHITE);
  white_look.button.standard.background_color = COLOR_WHITE;
  white_look.button.focused.background_color = COLOR_WHITE;

  const IconLook &icons = UIGlobals::GetIconLook();
  const Bitmap *bmp = &icons.hBmpMenuButton;
  bitmap_size_raw = bmp->GetSize();

  CreateButton(parent, white_look, icon_look, rc);
  Move(rc);
}

void
MainMenuButtonWidget::Unprepare()
{
  WindowWidget::Unprepare();
  DeleteWindow();
}

void
MainMenuButtonWidget::Show(const PixelRect &rc)
{
  GetWindow()->Show();
}

void
MainMenuButtonWidget::Hide()
{
  GetWindow()->Hide();
}

void
MainMenuButtonWidget::Move(const PixelRect &rc_map)
{
  UPixelScalar clear_border_width = Layout::Scale(2);
  PixelRect rc;
  rc.right = rc_map.right;
  rc.left = rc.right - GetWidth() - 2 * clear_border_width;
  rc.bottom = rc_map.bottom;
  rc.top = rc.bottom - GetHeight() - 2 * clear_border_width;

  WindowWidget::Move(rc);
  GetWindow()->Move(rc);
}

UPixelScalar
MainMenuButtonWidget::GetWidth() const
{
  return bitmap_size_raw.cx / 2 * MapOverlayButton::GetScale() / 2.5;
}

UPixelScalar
MainMenuButtonWidget::GetHeight() const
{
  return bitmap_size_raw.cy * MapOverlayButton::GetScale() / 2.5;
}

void
MainMenuButtonWidget::OnAction(int id)
{
  StaticString<20> menu;
  StaticString<20> menu_1;
  StaticString<20> menu_2;
  StaticString<20> menu_last;

  menu = _T("Menu");
  menu_1 = _T("Menu1");
  menu_2 = _T("Menu2");
  menu_last = _T("MenuLast");

  if (InputEvents::IsMode(menu.buffer()))
    InputEvents::setMode(menu_1.buffer());

  else if (InputEvents::IsMode(menu_1.buffer()))
    InputEvents::setMode(menu_2.buffer());

  else if (InputEvents::IsMode(menu_2.buffer()))
    InputEvents::setMode(menu_last.buffer());

  else if (InputEvents::IsMode(menu_last.buffer()))
    InputEvents::HideMenu();

  else InputEvents::setMode(menu.buffer());

  return;
}

MainMenuButton &
MainMenuButtonWidget::CreateButton(ContainerWindow &parent,
                                   const DialogLook &dialog_look,
                                   const IconLook &icon_look,
                                   const PixelRect &rc_map)
{
  ButtonWindowStyle button_style;
  button_style.multiline();

  MainMenuButton *button =
    new MainMenuButton(parent, dialog_look, icon_look, _T(""), // TODO replace "" with NULL
                       rc_map, button_style, this, 0);
  SetWindow(button);
  return *button;
}
