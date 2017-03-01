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
  FontInfoBoxComment,
  FontOverlayButton,
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
  const unsigned min_scale_range = 40;

  AddInteger(_("Nav bar waypoint name"),
             nullptr,
             _T("%d %%"), _T("%d"), min_scale_range, max_scale_range, 5,
             settings.font_scale_nav_bar_waypoint_name);

  AddInteger(_("Nav bar distance, altitude"),
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

  AddInteger(_("Infobox value"),
             nullptr,
             _T("%d %%"), _T("%d"), min_scale_range, max_scale_range, 5,
             settings.font_scale_infobox_value);

  AddInteger(_("Infobox title"),
             nullptr,
             _T("%d %%"), _T("%d"), min_scale_range, max_scale_range, 5,
             settings.font_scale_infobox_title);

  AddInteger(_("Infobox footer"),
             nullptr,
             _T("%d %%"), _T("%d"), min_scale_range, max_scale_range, 5,
             settings.font_scale_infobox_comment);

  AddInteger(_("Main menu button size"),
             nullptr,
             _T("%d %%"), _T("%d"), min_scale_range, max_scale_range, 5,
             settings.font_scale_overlay_button);

  AddInteger(_("Dialog and menu text"),
             nullptr,
             _T("%d %%"), _T("%d"), min_scale_range, max_scale_range, 5,
             settings.font_scale_dialog);
}

bool
FontConfigPanel::Save(bool &_changed)
{
  UISettings &settings = CommonInterface::SetUISettings();

  fonts_changed |= SaveValueEnum(FontNavBarWaypointName,
                                 ProfileKeys::FontNavBarWaypointName,
                                 settings.font_scale_nav_bar_waypoint_name);

  fonts_changed |= SaveValueEnum(FontNavBarDistance,
                                 ProfileKeys::FontNavBarDistance,
                                 settings.font_scale_nav_bar_distance);

  fonts_changed |= SaveValueEnum(FontMapWaypointName,
                                 ProfileKeys::FontMapWaypointName,
                                 settings.font_scale_map_waypoint_name);

  fonts_changed |= SaveValueEnum(FontMapPlaceName,
                                 ProfileKeys::FontMapPlaceName,
                                 settings.font_scale_map_place_name);

  fonts_changed |= SaveValueEnum(FontInfoBoxValue,
                                 ProfileKeys::FontInfoBoxValue,
                                 settings.font_scale_infobox_value);

  fonts_changed |= SaveValueEnum(FontInfoBoxTitle,
                                 ProfileKeys::FontInfoBoxTitle,
                                 settings.font_scale_infobox_title);

  fonts_changed |= SaveValueEnum(FontInfoBoxComment,
                                 ProfileKeys::FontInfoBoxComment,
                                 settings.font_scale_infobox_comment);

  fonts_changed |= SaveValueEnum(FontOverlayButton,
                                 ProfileKeys::FontOverlayButton,
                                 settings.font_scale_overlay_button);

  bool dialog_font_changed = SaveValueEnum(FontDialog,
                                           ProfileKeys::FontDialog,
                                           settings.font_scale_dialog);
  require_restart |= dialog_font_changed;
  _changed |= dialog_font_changed;

  _changed |= fonts_changed;


  return true;
}

Widget *
CreateFontConfigPanel()
{
  return new FontConfigPanel();
}
