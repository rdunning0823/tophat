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

#include "LayoutConfigPanel.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Profile/Profile.hpp"
#include "Form/DataField/Enum.hpp"
#include "Hardware/Display.hpp"
#include "Interface.hpp"
#include "MainWindow.hpp"
#include "LogFile.hpp"
#include "Language/Language.hpp"
#include "Dialogs/XML.hpp"
#include "Form/RowFormWidget.hpp"
#include "UIGlobals.hpp"

enum ControlIndex {
  DisplayOrientation,
  AppInfoBoxGeom,
  AppInverseInfoBox,
};

static const StaticEnumChoice display_orientation_list[] = {
  { (unsigned)DisplaySettings::Orientation::DEFAULT,
    N_("Default") },
  { (unsigned)DisplaySettings::Orientation::PORTRAIT,
    N_("Portrait") },
  { (unsigned)DisplaySettings::Orientation::LANDSCAPE,
    N_("Landscape") },
  { (unsigned)DisplaySettings::Orientation::REVERSE_PORTRAIT,
    N_("Reverse Portrait") },
  { (unsigned)DisplaySettings::Orientation::REVERSE_LANDSCAPE,
    N_("Reverse Landscape") },
  { 0 }
};

static const StaticEnumChoice info_box_geometry_list[] = {
  { (unsigned)InfoBoxSettings::Geometry::TOP_4_BOTTOM_4,
    N_("8 Top + Bottom (Portrait)") },
  { (unsigned)InfoBoxSettings::Geometry::BOTTOM_8,
    N_("8 Bottom (Portrait)") },
  { (unsigned)InfoBoxSettings::Geometry::BOTTOM_8_VARIO,
    N_("8 Bottom + Vario (Portrait)") },
  { (unsigned)InfoBoxSettings::Geometry::TOP_8,
    N_("8 Top (Portrait)") },
  { (unsigned)InfoBoxSettings::Geometry::LEFT_4_RIGHT_4,
    N_("8 Left + Right (Landscape)") },
  { (unsigned)InfoBoxSettings::Geometry::LEFT_8,
    N_("8 Left (Landscape)") },
  { (unsigned)InfoBoxSettings::Geometry::RIGHT_8,
    N_("8 Right (Landscape)") },
  { (unsigned)InfoBoxSettings::Geometry::RIGHT_9_VARIO,
    N_("9 Right + Vario (Landscape)") },
  { (unsigned)InfoBoxSettings::Geometry::LEFT_6_RIGHT_3_VARIO,
    N_("9 Left + Right + Vario (Landscape)") },
  { (unsigned)InfoBoxSettings::Geometry::RIGHT_5,
    N_("5 Right (Square)") },
  { (unsigned)InfoBoxSettings::Geometry::RIGHT_12,
    N_("12 Right (Landscape)") },
  { (unsigned)InfoBoxSettings::Geometry::BOTTOM_12,
    N_("12 Bottom (Portrait)") },
  { (unsigned)InfoBoxSettings::Geometry::TOP_12,
    N_("12 Top (Portrait)") },
  { (unsigned)InfoBoxSettings::Geometry::RIGHT_24,
    N_("24 Right (Landscape)") },
  { (unsigned)InfoBoxSettings::Geometry::TOP_4,
    N_("4 Top (Portrait)") },
  { (unsigned)InfoBoxSettings::Geometry::BOTTOM_4,
    N_("4 Bottom (Portrait)") },
  { (unsigned)InfoBoxSettings::Geometry::LEFT_4,
    N_("4 Left (Landscape)") },
  { (unsigned)InfoBoxSettings::Geometry::RIGHT_4,
    N_("4 Right (Landscape)") },
  { 0 }
};

class LayoutConfigPanel : public RowFormWidget {
public:
  LayoutConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);
};

void
LayoutConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const UISettings &ui_settings = CommonInterface::GetUISettings();

  RowFormWidget::Prepare(parent, rc);

  if (Display::RotateSupported())
    AddEnum(_("Display orientation"), _("Rotate the display on devices that support it."),
            display_orientation_list, (unsigned)ui_settings.display.orientation);
  else
    AddDummy();

  AddEnum(_("InfoBox geometry"),
          _("A list of possible InfoBox layouts. Do some trials to find the best for your screen size."),
          info_box_geometry_list, (unsigned)ui_settings.info_boxes.geometry);

  AddBoolean(_("Inverse InfoBoxes"), _("If true, the InfoBoxes are white on black, otherwise black on white."),
             ui_settings.info_boxes.inverse);
  SetExpertRow(AppInverseInfoBox);
}

bool
LayoutConfigPanel::Save(bool &_changed, bool &_require_restart)
{
  bool changed = false, require_restart = false;

  UISettings &ui_settings = CommonInterface::SetUISettings();

  bool orientation_changed = false;

  if (Display::RotateSupported()) {
    orientation_changed =
      SaveValueEnum(DisplayOrientation, ProfileKeys::DisplayOrientation,
                    ui_settings.display.orientation);
    changed |= orientation_changed;
  }

  bool info_box_geometry_changed = false;

  info_box_geometry_changed |=
    SaveValueEnum(AppInfoBoxGeom, ProfileKeys::InfoBoxGeometry,
                  ui_settings.info_boxes.geometry);

  changed |= info_box_geometry_changed;

  changed |= require_restart |=
    SaveValue(AppInverseInfoBox, ProfileKeys::AppInverseInfoBox, ui_settings.info_boxes.inverse);

  if (orientation_changed) {
    assert(Display::RotateSupported());

    if (ui_settings.display.orientation == DisplaySettings::Orientation::DEFAULT)
      Display::RotateRestore();
    else {
      if (!Display::Rotate(ui_settings.display.orientation))
        LogStartUp(_T("Display rotation failed"));
    }
  } else if (info_box_geometry_changed)
    CommonInterface::main_window->ReinitialiseLayout();

  _changed |= changed;
  _require_restart |= require_restart;

  return true;
}

Widget *
CreateLayoutConfigPanel()
{
  return new LayoutConfigPanel();
}
