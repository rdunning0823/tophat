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

#include "TaskButtonWidget.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Look/Look.hpp"
#include "Look/GlobalFonts.hpp"
#include "Form/Button.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Components.hpp"
#include "TophatWidgets/MapOverlayButton.hpp"

void
TaskButtonWidget::Prepare(ContainerWindow &parent,
                          const PixelRect &rc)
{
  white_look.Initialise(Fonts::map_bold);

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
                               const ButtonLook &button_look,
                               const PixelRect &rc_map)
{
  WindowStyle button_style;

  WndSymbolButton *button = new WndSymbolButton(parent, button_look, caption,
                                                rc_map, button_style, *this, 0);

  SetWindow(button);
  return *button;
}
