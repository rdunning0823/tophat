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

#include "AltitudeSimulator.hpp"
#include "Altitude.hpp"
#include "Base.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Form/Button.hpp"
#include "Form/Frame.hpp"
#include "Form/CheckBox.hpp"
#include "Look/GlobalFonts.hpp"
#include "Screen/Timer.hpp"
#include "Formatter/UserUnits.hpp"
#include "Units/Units.hpp"
#include "UIGlobals.hpp"
#include "Interface.hpp"
#include "Screen/Layout.hpp"
#include "Util/StaticString.hpp"
#include "Renderer/FinalGlideBarRenderer.hpp"
#include "Look/Look.hpp"
#include "Screen/Canvas.hpp"
#include "Profile/Profile.hpp"
#include "ActionInterface.hpp"
#include "Language/Language.hpp"
#include "Task/Points/TaskWaypoint.hpp"
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
  class FinalGlideChart: public PaintWindow
  {
  public:
    FinalGlideChart(ContainerWindow &parent,
                    PixelScalar x, PixelScalar y,
                    UPixelScalar Width, UPixelScalar Height,
                    WindowStyle style,
                    const Look&);
  protected:
    const Look& look;

    /**
     * draws the Final Glide Bar on the MC widget so the pilot can
     * adjust his MC with respect to the arrival height
     */
    FinalGlideBarRenderer *final_glide_bar_renderer;

    /**
     * draws the the Final Glide bar
     */
    virtual void OnPaint(Canvas &canvas);
  };


protected:
  /**
   * These buttons and the altitude_value and altitude_type frames use the layout rectangles
   * calculated in NumberButtonSubNumberLayout
   */
  WndButton *big_plus, *big_minus, *little_plus, *little_minus;
  WndFrame *altitude_value;
  WndFrame *altitude_type;
  CheckBoxControl *show_alternate_units;
  /**
   * draws the Final Glide Bar on the MC widget so the pilot can
   * adjust his MC with respect to the arrival height
   */
  FinalGlideChart *final_glide_chart;

  /**
   * Area where canvas will draw the final glide bar
   */
  PixelRect fg_rc;

  /**
   * This timer updates the data for the final glide
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

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();
  /* Move must discard rc and use GetMainWindow()'s ClientRect */
  virtual void Move(const PixelRect &rc) override;
  void CalculateLayout(const PixelRect &rc);
  void Refresh();

protected:
  /**
   * render the final glide periodically because
   * latency in the blackboards causes the final glide
   * renderer to not always use value updated with the
   * buttons
   */
  virtual bool OnTimer(WindowTimer &timer);

  /* methods from ActionListener */
  virtual void OnAction(int id);
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
  case LittlePlus:    Profile::SetModified(true);
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

  altitude_value->SetCaption(altitude_string.get());
  final_glide_chart->Invalidate();
  altitude_type->SetCaption(is_agl ? _("AGL") : _("MSL"));
}

void
AltitudeSimulatorPanel::Move(const PixelRect &rc_unused)
{
  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();

  BaseAccessPanel::Move(rc);
  CalculateLayout(rc);
  final_glide_chart->Move(fg_rc);
  big_plus->Move(big_plus_rc);
  little_plus->Move(little_plus_rc);
  big_minus->Move(big_minus_rc);
  little_minus->Move(little_minus_rc);
  altitude_value->Move(value_rc);
  altitude_type->Move(sub_number_rc);
  show_alternate_units->Move(show_alternate_units_rc);
}

void
AltitudeSimulatorPanel::CalculateLayout(const PixelRect &rc)
{
  NumberButtonSubNumberLayout::CalculateLayout(content_rc);

  PixelRect content_right_rc = content_rc;
  PixelRect content_left_rc = content_rc;

  // split content area into two columns, buttons on the right, fg on left
  content_right_rc.left += Layout::Scale(50);

  NumberButtonSubNumberLayout::CalculateLayout(content_right_rc);
  content_left_rc.right = big_plus_rc.left - 1;
  fg_rc = content_left_rc;

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
  CalculateLayout(rc);

  WindowStyle style;
  const Look &look = UIGlobals::GetLook();
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();

  final_glide_chart =
      new FinalGlideChart(GetClientAreaWindow(),
                          fg_rc.left, fg_rc.top,
                          (UPixelScalar)(fg_rc.right - fg_rc.left),
                          (UPixelScalar)(fg_rc.bottom - fg_rc.top),
                          style, look);
  WndForm::AddDestruct(final_glide_chart);

  big_button_look.Initialise(Fonts::map_bold);

  big_dialog_look.Initialise(Fonts::map_bold, Fonts::infobox, Fonts::map_label,
                             Fonts::infobox, Fonts::map_bold,
                             Fonts::map_bold);
  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();
  big_plus = new WndButton(GetClientAreaWindow(), big_button_look, _T("+100"),
                           big_plus_rc,
                           button_style, *this, BigPlus);
  WndForm::AddDestruct(big_plus);

  little_plus = new WndButton(GetClientAreaWindow(), big_button_look,
                              _T("+10"), little_plus_rc,
                              button_style, *this, LittlePlus);
  WndForm::AddDestruct(little_plus);

  big_minus = new WndButton(GetClientAreaWindow(), big_button_look,
                            _T("-100"), big_minus_rc,
                            button_style, *this, BigMinus);
  WndForm::AddDestruct(big_minus);

  little_minus = new WndButton(GetClientAreaWindow(), big_button_look,
                               _T("-10"), little_minus_rc,
                               button_style, *this, LittleMinus);
  WndForm::AddDestruct(little_minus);

  WindowStyle style_frame;

  altitude_value = new WndFrame(GetClientAreaWindow(), big_dialog_look,
                                value_rc, style_frame);
  WndForm::AddDestruct(altitude_value);

  altitude_value->SetAlignCenter();
  altitude_value->SetVAlignCenter();

  altitude_type = new WndFrame(GetClientAreaWindow(), dialog_look,
                                sub_number_rc, style_frame);
  WndForm::AddDestruct(altitude_type);
  altitude_type->SetAlignCenter();
  altitude_type->SetVAlignCenter();

  ButtonWindowStyle checkbox_style;
  checkbox_style.TabStop();
  show_alternate_units = new CheckBoxControl(GetClientAreaWindow(),
                                             dialog_look,
                                             _T("Show feet and meters"),
                                             show_alternate_units_rc,
                                             checkbox_style,
                                             this, AlternateUnits);
  const InfoBoxSettings &settings_info_boxes =
      CommonInterface::GetUISettings().info_boxes;
  show_alternate_units->SetState(settings_info_boxes.show_alternative_altitude_units);
  WndForm::AddDestruct(show_alternate_units);

  dialog_timer.Schedule(500);
  Refresh();
}

void
AltitudeSimulatorPanel::Unprepare()
{
  dialog_timer.Cancel();
}

AltitudeSimulatorPanel::FinalGlideChart::FinalGlideChart(
    ContainerWindow &parent,
    PixelScalar X,
    PixelScalar Y,
    UPixelScalar width,
    UPixelScalar height,
    WindowStyle style,
    const Look& _look)
:look(_look)
{
  PixelRect rc (X, Y, X + width, Y + height);
  Create(parent, rc, style);
  final_glide_bar_renderer = new FinalGlideBarRenderer(look.final_glide_bar,
                                                       look.map.task);
}

void
AltitudeSimulatorPanel::FinalGlideChart::OnPaint(Canvas &canvas)
{
  PaintWindow::OnPaint(canvas);
  canvas.SelectNullPen();
  canvas.Clear(look.dialog.background_color);
  StaticString<64> description;

  ProtectedTaskManager::Lease task_manager(*protected_task_manager);
  if (task_manager->GetMode() == TaskType::ORDERED)
    description = _("Task");
  else {
    const TaskWaypoint* wp = task_manager->GetActiveTaskPoint();
    if (wp != nullptr)
      description = wp->GetWaypoint().name.c_str();
    else
      description.clear();
  }

  final_glide_bar_renderer->Draw(canvas, GetClientRect(),
                                 CommonInterface::Calculated(),
                                 CommonInterface::GetComputerSettings().task.glide,
  CommonInterface::GetUISettings().map.final_glide_bar_mc0_enabled,
  description.c_str());
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
