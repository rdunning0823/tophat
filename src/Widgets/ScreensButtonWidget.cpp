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

#include "ScreensButtonWidget.hpp"
#include "MapOverlayButton.hpp"
#include "Form/SymbolButton.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Look/IconLook.hpp"
#include "Screen/Bitmap.hpp"
#include "Screen/Layout.hpp"
#include "Interface.hpp"
#include "MainWindow.hpp"
#include "Look/Look.hpp"
#include "Look/GlobalFonts.hpp"
#include "Screen/Canvas.hpp"
#include "Interface.hpp"
#include "Input/InputEvents.hpp"
#include "InfoBoxes/InfoBoxLayout.hpp"

enum ButtonPosition {
  Bottom,
  Left,
  Right,
};

static ButtonPosition
GetButtonPosition(InfoBoxSettings::Geometry geometry, bool landscape)
{
  if (landscape)
    switch (geometry) {
    case InfoBoxSettings::Geometry::SPLIT_8:
    case InfoBoxSettings::Geometry::LEFT_6_RIGHT_3_VARIO:
    case InfoBoxSettings::Geometry::TOP_8_VARIO:
    case InfoBoxSettings::Geometry::OBSOLETE_SPLIT_8:
        return ButtonPosition::Bottom;

    case InfoBoxSettings::Geometry::BOTTOM_RIGHT_8:
    case InfoBoxSettings::Geometry::BOTTOM_RIGHT_12:
    case InfoBoxSettings::Geometry::BOTTOM_RIGHT_4:
    case InfoBoxSettings::Geometry::BOTTOM_8_VARIO:
    case InfoBoxSettings::Geometry::RIGHT_5:
    case InfoBoxSettings::Geometry::RIGHT_24:
    case InfoBoxSettings::Geometry::RIGHT_9_VARIO:
    case InfoBoxSettings::Geometry::OBSOLETE_BOTTOM_RIGHT_8:
    case InfoBoxSettings::Geometry::OBSOLETE_BOTTOM_RIGHT_12:
    case InfoBoxSettings::Geometry::OBSOLETE_BOTTOM_RIGHT_4:
      return ButtonPosition::Left;

    case InfoBoxSettings::Geometry::TOP_LEFT_8:
    case InfoBoxSettings::Geometry::TOP_LEFT_12:
    case InfoBoxSettings::Geometry::TOP_LEFT_4:
    case InfoBoxSettings::Geometry::OBSOLETE_TOP_LEFT_8:
    case InfoBoxSettings::Geometry::OBSOLETE_TOP_LEFT_4:
      return ButtonPosition::Right;
    }

  return ButtonPosition::Left;
}

bool
ScreensButton::OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys)
{
  if (IsInside(x, y))
    return WndButton::OnMouseMove(x, y, keys);
  else
    OnCancelMode();
  return true;
}

void
ScreensButton::OnPaint(Canvas &canvas)
{
  PixelRect rc = {
    PixelScalar(0), PixelScalar(0), PixelScalar(canvas.GetWidth()),
    PixelScalar(canvas.GetHeight())
  };

  bool pressed = IsDown();
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
    canvas.Select(button_look.disabled.brush);
  else
    canvas.Select(button_look.standard.foreground_brush);
  const Bitmap *bmp = &icon_look.hBmpScreensButton;
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
ScreensButtonWidget::UpdateVisibility(const PixelRect &rc,
                                       bool is_panning,
                                       bool is_main_window_widget,
                                       bool is_map)
{
  if (!is_panning) {
    const UISettings &ui_settings = CommonInterface::GetUISettings();
    const InfoBoxLayout::Layout ib_layout =
      InfoBoxLayout::Calculate(UIGlobals::GetMainWindow().GetClientRect(),
                               ui_settings.info_boxes.geometry);
    bool full_screen = CommonInterface::main_window->GetFullScreen();

    if (false && !Layout::landscape && full_screen &&
        InfoBoxLayout::HasInfoBoxesOnBottom(ib_layout.geometry) &&
        (PixelScalar)CommonInterface::main_window->GetHeight() >
        ib_layout.remaining.bottom)
      height = CommonInterface::main_window->GetHeight() -
        ib_layout.remaining.bottom;
    else
      height = 0;

    Show(rc);
  } else
    Hide();
}

UPixelScalar
ScreensButtonWidget::HeightFromBottomRight()
{
  return bitmap_size_raw.cy * MapOverlayButton::GetScale() + Layout::Scale(3);
}

void
ScreensButtonWidget::Prepare(ContainerWindow &parent,
                              const PixelRect &rc)
{
  const IconLook &icon_look = CommonInterface::main_window->GetLook().icon;
  white_look.Initialise(Fonts::map_bold);

  const IconLook &icons = UIGlobals::GetIconLook();
  const Bitmap *bmp = &icons.hBmpScreensButton;
  bitmap_size_raw = bmp->GetSize();

  the_button = &CreateButton(parent, white_look, icon_look, rc);
  Move(rc);
}

void
ScreensButtonWidget::Unprepare()
{
  WindowWidget::Unprepare();
  DeleteWindow();
}

void
ScreensButtonWidget::Show(const PixelRect &rc)
{
  GetWindow().Show();
}

void
ScreensButtonWidget::Hide()
{
  GetWindow().Hide();
}

void
ScreensButtonWidget::Move(const PixelRect &rc_map)
{
  const UISettings &ui_settings = CommonInterface::GetUISettings();
  const PixelRect rc_main = UIGlobals::GetMainWindow().GetClientRect();

  UPixelScalar clear_border_width = Layout::Scale(2);
  PixelRect rc;

  switch (GetButtonPosition(ui_settings.info_boxes.geometry,
                            Layout::landscape)) {
  case ButtonPosition::Left:
    rc.left = 0;
    rc.right = rc.left + GetWidth() + clear_border_width;
    rc.bottom = rc_main.GetCenter().y;
    rc.top = rc.bottom - GetHeight() - 2 * clear_border_width;
  break;

  case ButtonPosition::Right:
    rc.right = rc_main.right;
    rc.left = rc.right - GetWidth() - clear_border_width;
    rc.bottom = rc_main.GetCenter().y;
    rc.top = rc.bottom - GetHeight() - 2 * clear_border_width;
  break;

  case ButtonPosition::Bottom:
    rc.left = rc_main.GetCenter().x - GetWidth() / 2;
    rc.right = rc.left + GetWidth() + clear_border_width;
    rc.bottom = rc_main.bottom;
    rc.top = rc.bottom - GetHeight() - 2 * clear_border_width;
  break;
}

  WindowWidget::Move(rc);
  GetWindow().Move(rc);
}

UPixelScalar
ScreensButtonWidget::GetWidth() const
{
  return bitmap_size_raw.cx / 2 * MapOverlayButton::GetScale() / 2.5;
}

UPixelScalar
ScreensButtonWidget::GetHeight() const
{
  const UPixelScalar base_height = bitmap_size_raw.cy *
      MapOverlayButton::GetScale() / 2.5;

  if (height == 0)
    return base_height;
  else
   return height + base_height / 2;
}

void
ScreensButtonWidget::OnAction(int id)
{
  InputEvents::HideMenu();
  InputEvents::eventScreenModes(_T("next"));
}

ScreensButton &
ScreensButtonWidget::CreateButton(ContainerWindow &parent,
                                   const ButtonLook &button_look,
                                   const IconLook &icon_look,
                                   const PixelRect &rc_map)
{
  ButtonWindowStyle button_style;
  button_style.multiline();

  ScreensButton *button =
    new ScreensButton(parent, button_look, icon_look, _T(""), // TODO replace "" with NULL
                       rc_map, button_style, *this, 0);
  SetWindow(button);
  return *button;
}
