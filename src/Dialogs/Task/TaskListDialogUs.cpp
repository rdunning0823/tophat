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

#include "Dialogs/Task/Manager/TaskListPanel.hpp"
#include "Screen/Layout.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Boolean.hpp"
#include "Form/DataField/Float.hpp"
#include "Dialogs/Task/Manager/TaskListPanel.hpp"
#include "Dialogs/Task/dlgTaskHelpers.hpp"
#include "Dialogs/Task/TaskDialogs.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Form/Edit.hpp"
#include "Form/Draw.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Task/TaskNationalities.hpp"
#include "Units/Units.hpp"
#include "Language/Language.hpp"
#include "Components.hpp"
#include "Interface.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Form/DataField/Listener.hpp"
#include "Engine/Task/TaskBehaviour.hpp"
#include "UIGlobals.hpp"
#include "Screen/SingleWindow.hpp"


void
dlgTaskListUsShowModal(OrderedTask **_active_task, bool &_task_modified)
{

  WidgetDialog dialog(UIGlobals::GetDialogLook());
  Widget *instance = CreateTaskListPanel(nullptr,
                                         _active_task, &_task_modified,
                                         &dialog);

  ButtonPanel::ButtonPanelPosition position = ButtonPanel::ButtonPanelPosition::Bottom;

  dialog.CreateFull(UIGlobals::GetMainWindow(), _("Browse tasks"),
                    instance, nullptr, 0, position);
  dialog.ShowModal();
}
