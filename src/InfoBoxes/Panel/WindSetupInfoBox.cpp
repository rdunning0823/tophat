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

#include "WindSetupInfoBox.hpp"
#include "Base.hpp"
#include "Dialogs/WindSettingsPanel.hpp"
#include "Form/Edit.hpp"
#include "Form/DataField/Float.hpp"
#include "Interface.hpp"
#include "Units/Units.hpp"
#include "Units/Group.hpp"
#include "Form/RowFormWidget.hpp"
#include "Form/DataField/Listener.hpp"
#include "UIGlobals.hpp"
#include "Language/Language.hpp"
#include "Screen/Layout.hpp"

enum ControlIndex {
  WindSpeed,
  WindDirection,
  LastItemInList,
};

class WindSetupInfoBoxPanel: public BaseAccessPanel {
  protected:
  WindSettingsPanel *wind_settings_panel;

public:
  WindSetupInfoBoxPanel(unsigned id)
    :BaseAccessPanel(id, new WindSettingsPanel(true, true)) {
  };
  virtual void Hide();

protected:
};

void
WindSetupInfoBoxPanel::Hide()
{
  WindSettingsPanel *wind_settings_panel =
      (WindSettingsPanel *)managed_widget.Get();
  assert(wind_settings_panel);
  bool changed, restart_required;
  wind_settings_panel->Save(changed, restart_required);

  BaseAccessPanel::Hide();
}

Widget *
LoadWindSetupInfoBoxPanel(unsigned id)
{
  return new WindSetupInfoBoxPanel(id);
}
