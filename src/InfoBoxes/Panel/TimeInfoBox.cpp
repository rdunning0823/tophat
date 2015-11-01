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

#include "TimeInfoBox.hpp"
#include "Base.hpp"
#include "Dialogs/Settings/Panels/TimeConfigPanel.hpp"
#include "Form/Edit.hpp"
#include "Form/DataField/Float.hpp"
#include "Interface.hpp"
#include "Units/Units.hpp"
#include "Units/Group.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Form/DataField/Listener.hpp"
#include "UIGlobals.hpp"
#include "Language/Language.hpp"
#include "Screen/Layout.hpp"

enum ControlIndex {
  WindSpeed,
  WindDirection,
  LastItemInList,
};

class TimeInfoBoxPanel: public BaseAccessPanel {

public:
  TimeInfoBoxPanel(unsigned id, TimeConfigPanel *time_config_panel)
    :BaseAccessPanel(id, time_config_panel) {
  };
  virtual void Hide();

protected:
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
  //inner_widget->SetForm(outer_widget);
  return outer_widget;
}
