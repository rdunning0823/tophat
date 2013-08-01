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

#include "TaskButtonWidget.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Look/Look.hpp"
#include "Look/GlobalFonts.hpp"
#include "Form/SymbolButton.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Components.hpp"
#include "Widgets/MapOverlayButton.hpp"

void
TaskButtonWidget::Prepare(ContainerWindow &parent,
                          const PixelRect &rc)
{
  white_look.Initialise(Fonts::map_bold, Fonts::map, Fonts::map_label,
                        Fonts::map_bold, Fonts::map_bold, Fonts::map_bold);
  white_look.SetBackgroundColor(COLOR_WHITE);
  white_look.button.standard.background_color = COLOR_WHITE;
  white_look.button.focused.background_color = COLOR_WHITE;
  white_look.button.focused.foreground_color = COLOR_BLACK;
  white_look.button.focused.foreground_brush.Set(COLOR_BLACK);
  CreateButton(parent, white_look, rc);
  Move(rc);
}

void
TaskButtonWidget::Unprepare()
{
  WindowWidget::Unprepare();
  DeleteWindow();
}

void
TaskButtonWidget::Show(const PixelRect &rc)
{
  GetWindow().Show();
}

void
TaskButtonWidget::Hide()
{
  GetWindow().Hide();
}

void
TaskButtonWidget::Move(const PixelRect &rc)
{
  WindowWidget::Move(rc);
}

WndSymbolButton &
TaskButtonWidget::CreateButton(ContainerWindow &parent,
                               const DialogLook &dialog_look,
                               const PixelRect &rc_map)
{
  ButtonWindowStyle button_style;
  button_style.multiline();

  WndSymbolButton *button = new WndSymbolButton(parent, dialog_look, caption,
                                                rc_map, button_style, *this, 0);

  SetWindow(button);
  return *button;
}
