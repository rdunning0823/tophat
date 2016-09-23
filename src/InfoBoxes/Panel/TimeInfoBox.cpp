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

#include "TimeInfoBox.hpp"
#include "Base.hpp"
#include "Dialogs/Settings/Panels/TimeConfigPanel.hpp"
#include "Dialogs/DialogSettings.hpp"
#include "Form/Edit.hpp"
#include "Form/DataField/Float.hpp"
#include "Interface.hpp"
#include "Units/Units.hpp"
#include "Units/Group.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Form/DataField/Listener.hpp"
#include "UIGlobals.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Screen/Layout.hpp"


class TimeInfoBoxPanel: public BaseAccessPanel {
  bool expert_save;
public:
  TimeInfoBoxPanel(unsigned id, TimeConfigPanel *time_config_panel)
    :BaseAccessPanel(id, time_config_panel), expert_save(UIGlobals::GetDialogSettings().expert) {
    CommonInterface::SetUISettings().dialog.expert = false;
  };
  virtual void Hide() override;
  ~TimeInfoBoxPanel() {
    CommonInterface::SetUISettings().dialog.expert = expert_save;
  }
};

void
TimeInfoBoxPanel::Hide()
{
  TimeConfigPanel *time_config_panel =
      (TimeConfigPanel *)managed_widget.Get();
  assert(time_config_panel);
  bool changed;
  time_config_panel->Save(changed);

  BaseAccessPanel::Hide();
}

Widget *
LoadTimeInfoBoxPanel(unsigned id)
{
  TimeConfigPanel *inner_widget = new TimeConfigPanel();
  TimeInfoBoxPanel *outer_widget = new TimeInfoBoxPanel(id, inner_widget);
  return outer_widget;
}
