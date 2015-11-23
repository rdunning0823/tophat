/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
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

#include "FontConfigPanel.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Profile/Profile.hpp"
#include "Form/DataField/Enum.hpp"
#include "Hardware/RotateDisplay.hpp"
#include "Interface.hpp"
#include "MainWindow.hpp"
#include "LogFile.hpp"
#include "Language/Language.hpp"
#include "Widget/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "UtilsSettings.hpp"
#include "Asset.hpp"
#include "UtilsSettings.hpp"


enum ControlIndex {
  FontNavBarWaypointName,
  FontNavBarDistance,
  FontMapWaypointName,
  FontMapPlaceName,
  FontInfoBoxValue,
  FontInfoBoxTitle,
/*  FontMenuButton,*/
  FontDialog,
};

class FontConfigPanel final : public RowFormWidget {
public:
  FontConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;
};

void
FontConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const UISettings &settings = CommonInterface::GetUISettings();

  RowFormWidget::Prepare(parent, rc);

  const unsigned max_scale_range = 150;
  const unsigned min_scale_range = 75;

  AddInteger(_("Nav bar waypoint name"),
             nullptr,
             _T("%d %%"), _T("%d"), min_scale_range, max_scale_range, 5,
             settings.font_scale_nav_bar_waypoint_name);

  AddInteger(_("Nav bar waypoint distance"),
             nullptr,
             _T("%d %%"), _T("%d"), min_scale_range, max_scale_range, 5,
             settings.font_scale_nav_bar_distance);

  AddInteger(_("Map waypoint name"),
             nullptr,
             _T("%d %%"), _T("%d"), min_scale_range, max_scale_range, 5,
             settings.font_scale_map_waypoint_name);

  AddInteger(_("Map place name"),
             nullptr,
             _T("%d %%"), _T("%d"), min_scale_range, max_scale_range, 5,
             settings.font_scale_map_place_name);

  AddInteger(_("Infbox value"),
             nullptr,
             _T("%d %%"), _T("%d"), min_scale_range, max_scale_range, 5,
             settings.font_scale_infobox_value);

  AddInteger(_("Infbox title"),
             nullptr,
             _T("%d %%"), _T("%d"), min_scale_range, max_scale_range, 5,
             settings.font_scale_infobox_title);

/*  AddInteger(_("Map menu button size"),
             nullptr,
             _T("%d %%"), _T("%d"), min_scale_range, max_scale_range, 5,
             settings.font_scale_menu_button);*/

  AddInteger(_("Dialog and menu text"),
             nullptr,
             _T("%d %%"), _T("%d"), min_scale_range, max_scale_range, 5,
             settings.font_scale_dialog);
}

bool
FontConfigPanel::Save(bool &_changed)
{
  bool changed = false;

  UISettings &settings = CommonInterface::SetUISettings();

  if (SaveValueEnum(FontNavBarWaypointName, ProfileKeys::FontNavBarWaypointName,
                    settings.font_scale_nav_bar_waypoint_name))
    require_restart = changed = true;

  if (SaveValueEnum(FontNavBarDistance, ProfileKeys::FontNavBarDistance,
                    settings.font_scale_nav_bar_distance))
    require_restart = changed = true;

  if (SaveValueEnum(FontMapWaypointName, ProfileKeys::FontMapWaypointName,
                    settings.font_scale_map_waypoint_name))
    require_restart = changed = true;

  if (SaveValueEnum(FontMapPlaceName, ProfileKeys::FontMapPlaceName,
                    settings.font_scale_map_place_name))
    require_restart = changed = true;

  if (SaveValueEnum(FontInfoBoxValue, ProfileKeys::FontInfoBoxValue,
                    settings.font_scale_infobox_value))
    require_restart = changed = true;

  if (SaveValueEnum(FontInfoBoxTitle, ProfileKeys::FontInfoBoxTitle,
                    settings.font_scale_infobox_title))
    require_restart = changed = true;

/*  if (SaveValueEnum(FontMenuButton, ProfileKeys::FontMenuButton,
                    settings.font_scale_menu_button))
    require_restart = changed = true;*/

  if (SaveValueEnum(FontDialog, ProfileKeys::FontDialog,
                    settings.font_scale_dialog))
    require_restart = changed = true;

  _changed |= changed;

  return true;
}

Widget *
CreateFontConfigPanel()
{
  return new FontConfigPanel();
}
