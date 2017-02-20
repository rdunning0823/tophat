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

#include "SpeedSimulator.hpp"
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

enum SpeedType {
  TAS = 0,
  IAS,
  GroundSpeed,
};

enum ControlIndex {
  BigPlus,
  LittlePlus,
  LittleMinus,
  BigMinus,
  AlternateUnits,
};


class SpeedSimulatorPanel : public BaseAccessPanel, NumberButton2SubNumberLayout {

protected:
  /**
   * These buttons and the Speed_value and speed_type_label frames use the layout rectangles
   * calculated in NumberButton2SubNumberLayout
   */
  Button *big_plus, *big_minus, *little_plus, *little_minus;
  WndFrame *speed_value, *speed_type_label, *best_ld;
  unsigned value_font_height, sub_number_height;

  /**
   * This timer syncs the speed data
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

  /* is the speed IAS, TAS or Ground? */
  SpeedType speed_type;

public:
  SpeedSimulatorPanel(unsigned id, SpeedType _speed_type)
    :BaseAccessPanel(id), dialog_timer(*this), speed_type(_speed_type) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Unprepare() override;
  /* Move must discard rc and use GetMainWindow()'s ClientRect */
  virtual void Move(const PixelRect &rc) override;
  void CalculateLayout(const PixelRect &rc);
  void Refresh();

  void SetSpeed(fixed user_air_speed);
  /** Gets speed from blackboard based on SpeedType */
  fixed GetBlackboardSpeed();

protected:
  /**
   * sync blackboards
   */
  virtual bool OnTimer(WindowTimer &timer) override;

  /* methods from ActionListener */
  virtual void OnAction(int id) override;
};


bool
SpeedSimulatorPanel::OnTimer(WindowTimer &timer)
{
  if (timer == dialog_timer) {
    Refresh();
    return true;
  }
  return BaseAccessPanel::OnTimer(timer);
}

fixed
SpeedSimulatorPanel::GetBlackboardSpeed()
{
  const NMEAInfo &basic = CommonInterface::Basic();

  switch (speed_type) {
  case SpeedType::TAS:
    return basic.airspeed_available.IsValid() ?
        basic.true_airspeed : basic.ground_speed;
  case SpeedType::IAS:
    return basic.airspeed_available.IsValid() ?
        basic.indicated_airspeed : basic.ground_speed;
  case SpeedType::GroundSpeed:
    return basic.ground_speed;
  }
  return basic.ground_speed;
}

void
SpeedSimulatorPanel::OnAction(int action_id)
{
  fixed speed = GetBlackboardSpeed();

  fixed step = Units::ToSysSpeed(fixed(1));

  switch (action_id) {
  case BigPlus:
    speed += step * fixed(10);
    break;
  case LittlePlus:
    speed += step;
    break;
  case LittleMinus:
    speed -= step;
    break;
  case BigMinus:
    speed -= step * fixed(10);
    break;
  default:
    BaseAccessPanel::OnAction(action_id);
    return;
  }
  if (!positive(speed))
    speed = fixed(0);

  SetSpeed(speed);
  Refresh();
}

void SpeedSimulatorPanel::SetSpeed(fixed speed)
{
  switch (speed_type) {
  case SpeedType::TAS:
    device_blackboard->SetSpeedFromTAS(speed);
    break;
  case SpeedType::IAS:
    device_blackboard->SetSpeedFromIAS(speed);
    break;
  case SpeedType::GroundSpeed:
    device_blackboard->SetSpeed(speed);
    break;
  }
  device_blackboard->SkipNextGlideSpeedCalculation();
}

void
SpeedSimulatorPanel::Refresh()
{
  const ComputerSettings &settings_computer =
    CommonInterface::GetComputerSettings();
  const NMEAInfo &basic = CommonInterface::Basic();

  fixed speed = GetBlackboardSpeed();

  StaticString<50> speed_string;
  FormatUserSpeed(speed, speed_string.buffer(), true);

  speed_value->SetCaption(speed_string.c_str());

  switch (speed_type) {
  case SpeedType::TAS:
    speed_type_label->SetCaption(_("TAS"));
    break;
  case SpeedType::IAS:
    speed_type_label->SetCaption(_("IAS"));
    break;
  case SpeedType::GroundSpeed:
    speed_type_label->SetCaption(_("V GND"));
    break;
  }

  StaticString<10> ld_string;

  fixed ias = basic.airspeed_available.IsValid() ?
      basic.indicated_airspeed : basic.ground_speed;

  if ((speed_type != SpeedType::GroundSpeed) &&
      ias > settings_computer.polar.glide_polar_task.GetVTakeoff() * 2) {
    fixed ld = ias / settings_computer.polar.glide_polar_task.SinkRate(ias);
    ld_string.Format(_T("LD %.0f:1"), (double)ld);
    best_ld->SetCaption(ld_string.c_str());
  } else {
    best_ld->SetCaption(_T(""));
  }
}

void
SpeedSimulatorPanel::Move(const PixelRect &rc_unused)
{
  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();

  BaseAccessPanel::Move(rc);
  CalculateLayout(rc);
  big_plus->Move(big_plus_rc);
  little_plus->Move(little_plus_rc);
  big_minus->Move(big_minus_rc);
  little_minus->Move(little_minus_rc);
  speed_value->Move(value_rc);
  speed_type_label->Move(sub_number_top_rc);
  best_ld->Move(sub_number_bottom_rc);
}

void
SpeedSimulatorPanel::CalculateLayout(const PixelRect &rc)
{
  NumberButton2SubNumberLayout::CalculateLayout(content_rc, value_font_height,
                                                sub_number_height);
  content_rc.right = big_plus_rc.left - 1;
}

void
SpeedSimulatorPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);

  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  const MapLook &map_look = UIGlobals::GetLook().map;
  big_dialog_look.Initialise(320);
  big_button_look.Initialise(map_look.overlay_font);
  value_font_height = big_dialog_look.text_font.GetHeight();
  sub_number_height = dialog_look.text_font.GetHeight() + Layout::GetTextPadding();

  CalculateLayout(rc);

  WindowStyle style;

  WindowStyle button_style;
  button_style.TabStop();
  big_plus = new Button(GetClientAreaWindow(), big_button_look, _T("+10"),
                        big_plus_rc,
                        button_style, *this, BigPlus);

  little_plus = new Button(GetClientAreaWindow(), big_button_look,
                           _T("+1"), little_plus_rc,
                           button_style, *this, LittlePlus);

  big_minus = new Button(GetClientAreaWindow(), big_button_look,
                         _T("-10"), big_minus_rc,
                         button_style, *this, BigMinus);

  little_minus = new Button(GetClientAreaWindow(), big_button_look,
                            _T("-1"), little_minus_rc,
                            button_style, *this, LittleMinus);

  WindowStyle style_frame;

  speed_value = new WndFrame(GetClientAreaWindow(), big_dialog_look,
                             value_rc, style_frame);
  speed_value->SetAlignCenter();
  speed_value->SetVAlignCenter();

  speed_type_label = new WndFrame(GetClientAreaWindow(), dialog_look,
                                  sub_number_top_rc, style_frame);

  best_ld = new WndFrame(GetClientAreaWindow(), dialog_look,
                         sub_number_bottom_rc, style_frame);

  dialog_timer.Schedule(500);
  Refresh();
}

void
SpeedSimulatorPanel::Unprepare()
{
  dialog_timer.Cancel();
  delete(big_plus);
  delete(little_plus);
  delete(big_minus);
  delete(little_minus);
  delete(speed_value);
  delete(speed_type_label);
  delete(best_ld);
}

Widget*
LoadTrueSpeedSimulatorPanel(unsigned id)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  if (!basic.gps.simulator)
    return nullptr;

  return new SpeedSimulatorPanel(id, SpeedType::TAS);
}

Widget*
LoadIndicatedSpeedSimulatorPanel(unsigned id)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  if (!basic.gps.simulator)
    return nullptr;

  return new SpeedSimulatorPanel(id, SpeedType::IAS);
}

Widget*
LoadGroundSpeedSimulatorPanel(unsigned id)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  if (!basic.gps.simulator)
    return nullptr;

  return new SpeedSimulatorPanel(id, SpeedType::GroundSpeed);
}
