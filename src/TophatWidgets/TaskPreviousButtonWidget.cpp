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

#include "TaskPreviousButtonWidget.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Look/Look.hpp"
#include "Look/GlobalFonts.hpp"
#include "Renderer/SymbolButtonRenderer.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Engine/Task/TaskType.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Components.hpp"
#include "TophatWidgets/MapOverlayButton.hpp"

void
TaskPreviousButtonWidget::Move(const PixelRect &rc_map)
{
  slider_shape.Resize(rc_map.right - rc_map.left);

  height = slider_shape.GetHeight();
  width = slider_shape.GetHintWidth() * 2;

  PixelRect rc = rc_map;
  rc.bottom = rc.top + height;
  rc.right = rc.left + width;

  WindowWidget::Move(rc);
}

void
TaskPreviousButtonWidget::OnAction(int id)
{
  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  if (task_manager->GetActiveTaskPointIndex() > 0)
    task_manager->SetActiveTaskPoint(task_manager->GetActiveTaskPointIndex()
                                     - 1);
}

static bool
DoesTaskNeedButton()
{
  ProtectedTaskManager::Lease task_manager(*protected_task_manager);
  return (task_manager->GetMode() == TaskType::ORDERED &&
      task_manager->GetActiveTaskPointIndex() > 0);
}

void
TaskPreviousButtonWidget::UpdateVisibility(const PixelRect &rc,
                                           bool is_panning,
                                           bool is_main_window_widget,
                                           bool is_map,
                                           bool is_top_widget)
{
  if (is_map && !is_main_window_widget && !is_panning && DoesTaskNeedButton()) {
    Show(rc);
    return;
  }

  Hide();
}
