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

#include "TaskListPanel.hpp"
#include "Internal.hpp"
#include "Screen/Layout.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Boolean.hpp"
#include "Form/DataField/Float.hpp"
#include "Dialogs/dlgTaskHelpers.hpp"
#include "Dialogs/Task.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Form/Edit.hpp"
#include "Form/Draw.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Task/TaskNationalities.hpp"
#include "Units/Units.hpp"
#include "Language/Language.hpp"
#include "Components.hpp"
#include "Interface.hpp"
#include "Form/RowFormWidget.hpp"
#include "Form/DataField/Listener.hpp"
#include "Engine/Task/TaskBehaviour.hpp"
#include "UIGlobals.hpp"
#include "Screen/SingleWindow.hpp"


void
dlgTaskListUsShowModal(OrderedTask **_active_task, bool &_task_modified)
{

  TaskListPanel *instance;

  instance = new TaskListPanel(nullptr, _active_task, &_task_modified);

  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();
  ButtonPanel::ButtonPanelPosition position = ButtonPanel::ButtonPanelPosition::Auto;

  WidgetDialog dialog(_("Task rules"),
                      rc, instance, nullptr, 0, position);
  instance->SetParentForm(&dialog);
  dialog.ShowModal();
}



