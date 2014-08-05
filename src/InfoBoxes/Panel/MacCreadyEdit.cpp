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

#include "MacCreadyEdit.hpp"
#include "Base.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Form/SymbolButton.hpp"
#include "Form/CheckBox.hpp"
#include "Form/Frame.hpp"
#include "Screen/Timer.hpp"
#include "Look/GlobalFonts.hpp"
#include "Formatter/UserUnits.hpp"
#include "Units/Units.hpp"
#include "UIGlobals.hpp"
#include "Interface.hpp"
#include "Screen/Layout.hpp"
#include "Util/StaticString.hpp"
#include "Renderer/FinalGlideBarRenderer.hpp"
#include "Look/Look.hpp"
#include "Look/DialogLook.hpp"
#include "Screen/Canvas.hpp"
#include "Profile/Profile.hpp"
#include "ActionInterface.hpp"
#include "Language/Language.hpp"
#include "Task/Points/TaskWaypoint.hpp"
#include "Screen/SingleWindow.hpp"

enum ControlIndex {
  BigPlus,
  LittlePlus,
  LittleMinus,
  BigMinus,
  AutoMc,
};


class MacCreadyEditPanel : public BaseAccessPanel, NumberButtonLayout {
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
   * These 4 buttons and the mc_value frame use the layout rectangles
   * calculated in NumberButtonLayout
   */
  WndSymbolButton *big_plus, *big_minus, *little_plus, *little_minus;
  WndFrame *mc_value;
  CheckBoxControl *auto_mc;
  PixelRect checkbox_rc;

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

public:
  MacCreadyEditPanel(unsigned _id)
    :BaseAccessPanel(_id), dialog_timer(*this) {}

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
MacCreadyEditPanel::OnTimer(WindowTimer &timer)
{
  if (timer == dialog_timer) {
    Refresh();
    return true;
  }
  return BaseAccessPanel::OnTimer(timer);
}

void
MacCreadyEditPanel::OnAction(int action_id)
{
  if (protected_task_manager == NULL)
    return;

  const ComputerSettings &settings_computer =
    CommonInterface::GetComputerSettings();
  const GlidePolar &polar = settings_computer.polar.glide_polar_task;
  TaskBehaviour &task_behaviour = CommonInterface::SetComputerSettings().task;
  fixed mc = polar.GetMC();

  switch (action_id) {
  case BigPlus:
    mc = std::min(mc + Units::ToSysVSpeed(GetUserVerticalSpeedStep() * 5),
                  fixed(5));
    ActionInterface::SetManualMacCready(mc);
    break;
  case LittlePlus:
    mc = std::min(mc + Units::ToSysVSpeed(GetUserVerticalSpeedStep()),
                  fixed(5));
    ActionInterface::SetManualMacCready(mc);
    break;
  case LittleMinus:
    mc = std::max(mc - Units::ToSysVSpeed(GetUserVerticalSpeedStep()),
                  fixed(0));
    ActionInterface::SetManualMacCready(mc);
    break;
  case BigMinus:
    mc = std::max(mc - Units::ToSysVSpeed(GetUserVerticalSpeedStep() * 5),
                  fixed(0));
    ActionInterface::SetManualMacCready(mc);
    break;
  case AutoMc:
    task_behaviour.auto_mc = !task_behaviour.auto_mc;
    Profile::Set(ProfileKeys::AutoMc, task_behaviour.auto_mc);
    if (task_behaviour.auto_mc)
      Close();
    break;
  default:
    BaseAccessPanel::OnAction(action_id);
    return;
  }
  Refresh();
}

void
MacCreadyEditPanel::Refresh()
{
  const ComputerSettings &settings_computer =
    CommonInterface::GetComputerSettings();
  auto_mc->SetState(XCSoarInterface::GetComputerSettings().task.auto_mc);
  fixed mc = settings_computer.polar.glide_polar_task.GetMC();
  StaticString<32> buffer;
  FormatUserVerticalSpeed(mc, buffer.buffer(), false);
  mc_value->SetCaption(buffer.c_str());
  mc_value->SetEnabled(!auto_mc->GetState());
  final_glide_chart->Invalidate();
}

void
MacCreadyEditPanel::Move(const PixelRect &rc_unused)
{
  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();

  BaseAccessPanel::Move(rc);
  CalculateLayout(rc);
  final_glide_chart->Move(fg_rc);
  big_plus->Move(big_plus_rc);
  little_plus->Move(little_plus_rc);
  big_minus->Move(big_minus_rc);
  little_minus->Move(little_minus_rc);

  mc_value->Move(value_rc);
  auto_mc->Move(checkbox_rc);
}

void
MacCreadyEditPanel::CalculateLayout(const PixelRect &rc)
{
  NumberButtonLayout::CalculateLayout(content_rc);

  PixelRect content_right_rc = content_rc;
  PixelRect content_left_rc = content_rc;

  // split content area into two columns, buttons on the right, fg on left
  content_right_rc.left += Layout::Scale(50);

  NumberButtonLayout::CalculateLayout(content_right_rc);
  content_left_rc.right = big_plus_rc.left - 1;
  fg_rc = content_left_rc;

  checkbox_rc.bottom = content_rc.bottom -
    (content_rc.bottom - big_minus_rc.bottom) / 4;
  checkbox_rc.top = big_minus_rc.bottom +
    (content_rc.bottom - big_minus_rc.bottom) / 4;
  checkbox_rc.left = big_minus_rc.left;
  checkbox_rc.right = little_minus_rc.right;
}

void
MacCreadyEditPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);
  CalculateLayout(rc);

  WindowStyle style;
  const Look &look = UIGlobals::GetLook();
  final_glide_chart =
      new FinalGlideChart(GetClientAreaWindow(),
                          fg_rc.left, fg_rc.top,
                          (UPixelScalar)(fg_rc.right - fg_rc.left),
                          (UPixelScalar)(fg_rc.bottom - fg_rc.top),
                          style, look);

  const ButtonLook &button_look = UIGlobals::GetDialogLook().button;
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();
  big_plus = new WndSymbolButton(GetClientAreaWindow(), button_look, _T("^"),
                                 big_plus_rc,
                                 button_style, *this, BigPlus);

  little_plus = new WndSymbolButton(GetClientAreaWindow(), button_look,
                                    _T("^"), little_plus_rc,
                                    button_style, *this, LittlePlus);

  big_minus = new WndSymbolButton(GetClientAreaWindow(), button_look,
                                  _T("v"), big_minus_rc,
                                  button_style, *this, BigMinus);

  little_minus = new WndSymbolButton(GetClientAreaWindow(), button_look,
                                     _T("v"), little_minus_rc,
                                     button_style, *this, LittleMinus);

  WindowStyle style_frame;
  big_dialog_look.Initialise(Fonts::map_bold, Fonts::infobox, Fonts::map_label,
                             Fonts::infobox, Fonts::map_bold,
                             Fonts::map_bold);
  mc_value = new WndFrame(GetClientAreaWindow(), big_dialog_look,
                          value_rc, style_frame);
  mc_value->SetAlignCenter();
  mc_value->SetVAlignCenter();

  ButtonWindowStyle checkbox_style;
  checkbox_style.TabStop();

  auto_mc = new CheckBoxControl(GetClientAreaWindow(), dialog_look, _T("Auto MC"),
                                checkbox_rc, checkbox_style,
                                this, AutoMc);
  dialog_timer.Schedule(500);
  Refresh();
}

void
MacCreadyEditPanel::Unprepare()
{
  delete big_plus;
  delete big_minus;
  delete little_plus;
  delete little_minus;
  delete mc_value;
  delete auto_mc;
  delete final_glide_chart;
  dialog_timer.Cancel();
}

Widget *
LoadMacCreadyEditPanel(unsigned id)
{
  return new MacCreadyEditPanel(id);
}


MacCreadyEditPanel::FinalGlideChart::FinalGlideChart(ContainerWindow &parent,
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
MacCreadyEditPanel::FinalGlideChart::OnPaint(Canvas &canvas)
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
