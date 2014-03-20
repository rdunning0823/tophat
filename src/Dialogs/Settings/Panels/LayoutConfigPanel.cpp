/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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
#include "Hardware/RotateDisplay.hpp"
#include "Interface.hpp"
#include "MainWindow.hpp"
#include "LogFile.hpp"
#include "Language/Language.hpp"
#include "Dialogs/XML.hpp"
#include "Widget/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "UtilsSettings.hpp"
#include "Asset.hpp"
#include "Form/ActionListener.hpp"
#include "UtilsSettings.hpp"
#include "Dialogs/Settings/Panels/PagesConfigPanel.hpp"
#include "Dialogs/Settings/Panels/InfoBoxesConfigPanel.hpp"

#ifdef KOBO
#include "Event/Globals.hpp"
#include "Event/Queue.hpp"
#endif

enum ControlIndex {
  DisplayOrientation,
  AppInfoBoxGeom,
  ScreensButtonLocation,
  AppInverseInfoBox,
  AppInfoBoxColors,
  CustomizeScreens,
  CustomizeInfoBoxes,
};

static constexpr StaticEnumChoice display_orientation_list[] = {
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

#if defined(ENABLE_OPENGL) | defined(KOBO)
static constexpr StaticEnumChoice screens_button_location_list[] = {
  { (unsigned)UISettings::ScreensButtonLocation::MAP,
    N_("On the map"),
    N_("Show the Screens button on the map at all times.") },
  { (unsigned)UISettings::ScreensButtonLocation::MENU,
    N_("On the menu"),
    N_("Show the Screens button on the menu.") },
  { 0 }
};
#endif

static constexpr StaticEnumChoice info_box_geometry_list[] = {
  { (unsigned)InfoBoxSettings::Geometry::SPLIT_8,
    N_("8 split") },
  { (unsigned)InfoBoxSettings::Geometry::BOTTOM_RIGHT_8,
    N_("8 bottom or right") },
  { (unsigned)InfoBoxSettings::Geometry::BOTTOM_8_VARIO,
    N_("8 bottom + vario (portrait)") },
  { (unsigned)InfoBoxSettings::Geometry::TOP_LEFT_8,
    N_("8 top or left") },
  { (unsigned)InfoBoxSettings::Geometry::TOP_8_VARIO,
    N_("8 top + vario (portrait)") },
  { (unsigned)InfoBoxSettings::Geometry::RIGHT_9_VARIO,
    N_("9 right + vario (landscape)") },
  { (unsigned)InfoBoxSettings::Geometry::LEFT_6_RIGHT_3_VARIO,
    N_("9 left + right + vario (landscape)") },
  { (unsigned)InfoBoxSettings::Geometry::RIGHT_5,
    N_("5 right (Square)") },
  { (unsigned)InfoBoxSettings::Geometry::BOTTOM_RIGHT_12,
    N_("12 bottom or right") },
  { (unsigned)InfoBoxSettings::Geometry::TOP_LEFT_12,
    N_("12 top or left") },
  { (unsigned)InfoBoxSettings::Geometry::RIGHT_24,
    N_("24 right (landscape)") },
  { (unsigned)InfoBoxSettings::Geometry::TOP_LEFT_4,
    N_("4 top or left") },
  { (unsigned)InfoBoxSettings::Geometry::BOTTOM_RIGHT_4,
    N_("4 bottom or right") },
  { 0 }
};

class LayoutConfigPanel final : public RowFormWidget, public ActionListener {
  /**
   * are we displaying this via the quick setup screen?
   */
  bool quick_setup;
public:
  LayoutConfigPanel(bool _quick_setup)
    :RowFormWidget(UIGlobals::GetDialogLook()), quick_setup(_quick_setup) {}

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;

  /** from ActionListener */
  virtual void OnAction(int id);
};

void
LayoutConfigPanel::OnAction(int id)
{
  Widget *widget = nullptr;
  StaticString<120> title;

  switch (id) {
  case CustomizeScreens:
    widget = CreatePagesConfigPanel();
    title = _("Set up screens");
    break;
  case CustomizeInfoBoxes:
    widget = CreateInfoBoxesConfigPanel();
    title = _("Set up infoboxes");
    break;
  }
  assert(widget != nullptr);
  SystemConfiguration(*widget, title.get());
}

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

#if !defined(ENABLE_OPENGL) & !defined(KOBO)
  AddDummy();
#else
  AddEnum(_("Screens button location"),
             _("Show the 'S' Screens button on the map or in the first menu."),
             screens_button_location_list, (unsigned)ui_settings.screens_button_location);
#endif

  if (!quick_setup) {
    AddBoolean(_("Inverse InfoBoxes"), _("If true, the InfoBoxes are white on black, otherwise black on white."),
               ui_settings.info_boxes.inverse);
    SetExpertRow(AppInverseInfoBox);
  } else
    AddDummy();

  if (HasColors() && !quick_setup) {
    AddBoolean(_("Colored InfoBoxes"),
               _("If true, certain InfoBoxes will have coloured text.  For example, the active waypoint "
                 "InfoBox will be blue when the glider is above final glide."),
               ui_settings.info_boxes.use_colors);
    SetExpertRow(AppInfoBoxColors);
  } else
    AddDummy();

  if (quick_setup) {
    AddButton(_("Edit screens"), *this, CustomizeScreens);
    AddButton(_("Edit infoboxes"), *this, CustomizeInfoBoxes);

  } else {
    AddDummy();
    AddDummy();
  }
}

bool
LayoutConfigPanel::Save(bool &_changed)
{
  bool changed = false;

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

#if defined(ENABLE_OPENGL) | defined(KOBO)
  changed |= SaveValueEnum(ScreensButtonLocation, ProfileKeys::ScreensButtonLocation,
                           ui_settings.screens_button_location);
#endif

  if (!quick_setup)
    changed |= require_restart |=
      SaveValue(AppInverseInfoBox, ProfileKeys::AppInverseInfoBox, ui_settings.info_boxes.inverse);

  if (HasColors() && !quick_setup &&
      SaveValue(AppInfoBoxColors, ProfileKeys::AppInfoBoxColors,
                ui_settings.info_boxes.use_colors))
    require_restart = changed = true;

  if (orientation_changed) {
    assert(Display::RotateSupported());

    if (ui_settings.display.orientation == DisplaySettings::Orientation::DEFAULT)
      Display::RotateRestore();
    else {
      if (!Display::Rotate(ui_settings.display.orientation))
        LogFormat("Display rotation failed");
    }

#ifdef KOBO
    event_queue->SetMouseRotation(ui_settings.display.orientation);
#endif

    CommonInterface::main_window->CheckResize();
  } else if (info_box_geometry_changed)
    CommonInterface::main_window->ReinitialiseLayout();

  _changed |= changed;

  return true;
}

Widget *
CreateLayoutConfigPanel()
{
  return CreateLayoutConfigPanel(false);
}

Widget *
CreateLayoutConfigPanel(bool quick_setup)
{
  return new LayoutConfigPanel(quick_setup);
}
