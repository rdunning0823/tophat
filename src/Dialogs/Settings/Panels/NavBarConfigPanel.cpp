/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2016 The XCSoar Project
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

#include "NavBarConfigPanel.hpp"
#include "Profile/Profile.hpp"
#include "Language/Language.hpp"
#include "Interface.hpp"
#include "Widget/RowFormWidget.hpp"

enum ControlIndex {
  DisplayGR,
  DisplayTpIndex,
  DisplayTpName
};

void
NavBarConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const UISettings &ui_settings =
    CommonInterface::GetUISettings();

  RowFormWidget::Prepare(parent, rc);

  AddBoolean(_("Display GR"),
             _("Enable/disable displaying glide ratio required to reach TP."),
             ui_settings.navbar_enable_gr);

  AddBoolean(_("Display TP index"),
             _("Enable/disable displaying TP index number in TopHat NavBar."),
             ui_settings.navbar_enable_tp_index);

  AddBoolean(_("Display TP name"),
             _("Enable/disable displaying TP name in TopHat NavBar."),
             ui_settings.navbar_enable_tp_name);
}

bool
NavBarConfigPanel::Save(bool &_changed)
{
  bool changed = false;
  UISettings &ui_settings = CommonInterface::SetUISettings();

  changed |= SaveValue(DisplayGR, ProfileKeys::NavBarDisplayGR,
                       ui_settings.navbar_enable_gr);

  changed |= SaveValue(DisplayTpIndex, ProfileKeys::NavBarDisplayTpIndex,
                       ui_settings.navbar_enable_tp_index);

  changed |= SaveValue(DisplayTpName, ProfileKeys::NavBarDisplayTpName,
                       ui_settings.navbar_enable_tp_name);

  _changed |= changed;

  return true;
}

Widget *
CreateNavBarConfigPanel()
{
  return new NavBarConfigPanel();
}
