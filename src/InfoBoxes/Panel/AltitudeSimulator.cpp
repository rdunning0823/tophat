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

#include "AltitudeSimulator.hpp"
#include "Altitude.hpp"
#include "Base.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "Components.hpp"
#include "Form/Button.hpp"
#include "Form/Frame.hpp"
#include "Form/CheckBox.hpp"
#include "Screen/Timer.hpp"
#include "Formatter/UserUnits.hpp"
#include "Units/Units.hpp"
#include "UIGlobals.hpp"
#include "Interface.hpp"
#include "Screen/Layout.hpp"
#include "Util/StaticString.hxx"
#include "Look/Look.hpp"
#include "Screen/Canvas.hpp"
#include "Profile/Profile.hpp"
#include "ActionInterface.hpp"
#include "Language/Language.hpp"
#include "Blackboard/DeviceBlackboard.hpp"
#include "Formatter/UserUnits.hpp"
#include "Screen/SingleWindow.hpp"

enum ControlIndex {
  BigPlus,
  LittlePlus,
  LittleMinus,
  BigMinus,
  AlternateUnits,
};


class AltitudeSimulatorPanel : public BaseAccessPanel, NumberButtonSubNumberLayout {

protected:
  /**
   * These buttons and the altitude_value and altitude_type frames use the layout rectangles
   * calculated in NumberButtonSubNumberLayout
   */
  Button *big_plus, *big_minus, *little_plus, *little_minus;
  WndFrame *altitude_value;
  WndFrame *altitude_type;
  CheckBoxControl show_alternate_units;
  unsigned value_font_height;

  /**
   * This timer syncs the altitude data
   */
  WindowTimer dialog_timer;

  /**
   * Dialog look with large text font
   */
  DialogLook big_dialog_look;

  /**
   * ButtonLook with large font
   */
  ButtonLook big_button_look;

  PixelRect show_alternate_units_rc;

  /* is the altitude displayed AGL? */
  bool is_agl;

public:
  AltitudeSimulatorPanel(unsigned id, bool _is_agl)
    :BaseAccessPanel(id), dialog_timer(*this), is_agl(_is_agl) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Unprepare() override;
  /* Move must discard rc and use GetMainWindow()'s ClientRect */
  virtual void Move(const PixelRect &rc) override;
  void CalculateLayout(const PixelRect &rc, unsigned value_height);
  void Refresh();

protected:
  /**
   * sync blackboards
   */
  virtual bool OnTimer(WindowTimer &timer) override;

  /* methods from ActionListener */
  virtual void OnAction(int id) override;
};


bool
AltitudeSimulatorPanel::OnTimer(WindowTimer &timer)
{
  if (timer == dialog_timer) {
    Refresh();
    return true;
  }
  return BaseAccessPanel::OnTimer(timer);
}

void
AltitudeSimulatorPanel::OnAction(int action_id)
{
  InfoBoxSettings &settings_info_boxes = CommonInterface::SetUISettings().info_boxes;
  const NMEAInfo &basic = CommonInterface::Basic();
  fixed altitude = basic.gps_altitude;

  fixed step = Units::ToSysAltitude(fixed(10));

  switch (action_id) {
  case BigPlus:
    altitude += step * fixed(10);
    break;
  case LittlePlus:
    altitude += step;
    break;
  case LittleMinus:
    altitude -= step;
    break;
  case BigMinus:
    altitude -= step * fixed(10);
    break;
  case AlternateUnits:
    settings_info_boxes.show_alternative_altitude_units =
        !settings_info_boxes.show_alternative_altitude_units;
    Profile::Set(ProfileKeys::ShowAlternateAltitudeUnits,
                 settings_info_boxes.show_alternative_altitude_units);
    Profile::SetModified(true);
  default:
    BaseAccessPanel::OnAction(action_id);
    return;
  }
  device_blackboard->SetAltitude(altitude);
  Refresh();
}

void
AltitudeSimulatorPanel::Refresh()
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const DerivedInfo &calculated = CommonInterface::Calculated();
  bool use_agl = is_agl && calculated.altitude_agl_valid;
  fixed altitude = use_agl ? calculated.altitude_agl : basic.gps_altitude;


  StaticString<50> altitude_string;
  FormatUserAltitude(altitude, altitude_string.buffer(), true);

  altitude_value->SetCaption(altitude_string.c_str());
  altitude_type->SetCaption(is_agl ? _("AGL") : _("MSL"));
}

void
AltitudeSimulatorPanel::Move(const PixelRect &rc_unused)
{
  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();

  BaseAccessPanel::Move(rc);
  CalculateLayout(rc, value_font_height);
  big_plus->Move(big_plus_rc);
  little_plus->Move(little_plus_rc);
  big_minus->Move(big_minus_rc);
  little_minus->Move(little_minus_rc);
  altitude_value->Move(value_rc);
  altitude_type->Move(sub_number_rc);
  show_alternate_units.Move(show_alternate_units_rc);
}

void
AltitudeSimulatorPanel::CalculateLayout(const PixelRect &rc, unsigned value_height)
{
  NumberButtonSubNumberLayout::CalculateLayout(content_rc, value_height);


  NumberButtonSubNumberLayout::CalculateLayout(content_rc, value_height);
  content_rc.right = big_plus_rc.left - 1;

  show_alternate_units_rc.bottom = content_rc.bottom -
    (content_rc.bottom - big_minus_rc.bottom) / 4;
  show_alternate_units_rc.top = big_minus_rc.bottom +
    (content_rc.bottom - big_minus_rc.bottom) / 4;
  show_alternate_units_rc.left = big_minus_rc.left;
  show_alternate_units_rc.right = little_minus_rc.right;
}

void
AltitudeSimulatorPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);

  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  const MapLook &map_look = UIGlobals::GetLook().map;
  big_dialog_look.Initialise(320);
  big_button_look.Initialise(map_look.overlay_font);
  value_font_height = big_dialog_look.text_font.GetHeight();
  CalculateLayout(rc, value_font_height);

  WindowStyle style;

  WindowStyle button_style;
  button_style.TabStop();
  big_plus = new Button(GetClientAreaWindow(), big_button_look, _T("+100"),
                           big_plus_rc,
                           button_style, *this, BigPlus);

  little_plus = new Button(GetClientAreaWindow(), big_button_look,
                              _T("+10"), little_plus_rc,
                              button_style, *this, LittlePlus);

  big_minus = new Button(GetClientAreaWindow(), big_button_look,
                            _T("-100"), big_minus_rc,
                            button_style, *this, BigMinus);

  little_minus = new Button(GetClientAreaWindow(), big_button_look,
                               _T("-10"), little_minus_rc,
                               button_style, *this, LittleMinus);

  WindowStyle style_frame;

  altitude_value = new WndFrame(GetClientAreaWindow(), big_dialog_look,
                                value_rc, style_frame);

  altitude_value->SetAlignCenter();
  altitude_value->SetVAlignCenter();

  altitude_type = new WndFrame(GetClientAreaWindow(), dialog_look,
                                sub_number_rc, style_frame);
  altitude_type->SetVAlignCenter();

  WindowStyle checkbox_style;
  checkbox_style.TabStop();
  show_alternate_units.Create(GetClientAreaWindow(),
                              dialog_look,
                              _("Show feet and meters"),
                              show_alternate_units_rc,
                              checkbox_style,
                              *this, AlternateUnits);

  const InfoBoxSettings &settings_info_boxes =
      CommonInterface::GetUISettings().info_boxes;
  show_alternate_units.SetState(settings_info_boxes.show_alternative_altitude_units);

  dialog_timer.Schedule(500);
  Refresh();
}

void
AltitudeSimulatorPanel::Unprepare()
{
  dialog_timer.Cancel();
  delete(big_plus);
  delete(little_plus);
  delete(big_minus);
  delete(little_minus);
  delete(altitude_value);
  delete(altitude_type);
}

static Widget*
LoadAltitudeSimulatorPanel(unsigned id, bool is_agl)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  if (!basic.gps.simulator)
    return LoadAltitudePanel(id);

  return new AltitudeSimulatorPanel(id, is_agl);
}

Widget*
LoadAglAltitudeSimulatorPanel(unsigned id)
{
  return LoadAltitudeSimulatorPanel(id, true);
}

Widget*
LoadGpsAltitudeSimulatorPanel(unsigned id)
{
  return LoadAltitudeSimulatorPanel(id, false);
}
