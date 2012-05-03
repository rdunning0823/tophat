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

#include "MacCreadyEdit.hpp"
#include "Base.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Form/SymbolButton.hpp"
#include "Form/CheckBox.hpp"
#include "Form/Frame.hpp"
#include "Form/Panel.hpp"
#include "Screen/Fonts.hpp"
#include "Formatter/UserUnits.hpp"
#include "Units/Units.hpp"
#include "UIGlobals.hpp"
#include "Interface.hpp"
#include "Screen/Layout.hpp"
#include "Screen/PaintWindow.hpp"
#include "Renderer/FinalGlideBarRenderer.hpp"
#include "Look/Look.hpp"
#include "Profile/Profile.hpp"

enum ControlIndex {
  BigPlus,
  LittlePlus,
  LittleMinus,
  BigMinus,
  AutoMc,
};

class MacCreadyEditPanel : public BaseAccessPanel, NumberButtonLayout {
protected:

  /**
   * These 4 buttons and the mc_value frame use the layout rectangles
   * calculated in NumberButtonLayout
   */
  WndSymbolButton *big_plus, *big_minus, *little_plus, *little_minus;
  WndFrame *mc_value;
  CheckBoxControl *auto_mc;

public:
  MacCreadyEditPanel(unsigned _id)
    :BaseAccessPanel(_id) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();

  void Refresh();

protected:
  /* methods from ActionListener */
  virtual void OnAction(int id);
};

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
                  fixed_zero);
    ActionInterface::SetManualMacCready(mc);
    break;
  case BigMinus:
    mc = std::max(mc - Units::ToSysVSpeed(GetUserVerticalSpeedStep() * 5),
                  fixed_zero);
    ActionInterface::SetManualMacCready(mc);
    break;
  case AutoMc:
    task_behaviour.auto_mc = !task_behaviour.auto_mc;
    Profile::Set(ProfileKeys::AutoMc, task_behaviour.auto_mc);
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
  TCHAR buffer[32];
  fixed value = settings_computer.polar.glide_polar_task.GetMC();
  FormatUserVerticalSpeed(value, buffer, false);
  mc_value->SetCaption(buffer);
  mc_value->SetEnabled(!auto_mc->GetState());
}

void
MacCreadyEditPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);
  NumberButtonLayout::Prepare(parent, content_rc);

  const DialogLook &look = UIGlobals::GetDialogLook();

  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();
  big_plus = new WndSymbolButton(GetClientAreaWindow(), look, _T("^"),
                                 big_plus_rc,
                                 button_style, this, BigPlus);
  big_plus->SetFont(Fonts::infobox);

  little_plus = new WndSymbolButton(GetClientAreaWindow(), look, _T("^"),
                                    little_plus_rc,
                                    button_style, this, LittlePlus);
  little_plus->SetFont(Fonts::infobox);

  big_minus = new WndSymbolButton(GetClientAreaWindow(), look, _T("v"),
                                  big_minus_rc,
                                  button_style, this, BigMinus);
  big_minus->SetFont(Fonts::infobox);

  little_minus = new WndSymbolButton(GetClientAreaWindow(), look, _T("v"),
                                     little_minus_rc,
                                     button_style, this, LittleMinus);
  little_minus->SetFont(Fonts::infobox);

  WindowStyle style_frame;
  mc_value = new WndFrame(GetClientAreaWindow(), look,
                          value_rc.left, value_rc.top,
                          value_rc.right - value_rc.left,
                          value_rc.bottom - value_rc.top,
                          style_frame);
  mc_value->SetVAlignCenter();
  mc_value->SetAlignCenter();
  mc_value->SetFont(Fonts::infobox);

  PixelRect checkbox_rc;
  checkbox_rc.bottom = content_rc.bottom -
    (content_rc.bottom - big_minus_rc.bottom) / 4;
  checkbox_rc.top = big_minus_rc.bottom +
    (content_rc.bottom - big_minus_rc.bottom) / 4;
  checkbox_rc.left = big_minus_rc.left;
  checkbox_rc.right = big_minus_rc.right;

  ButtonWindowStyle checkbox_style;
  checkbox_style.TabStop();

  auto_mc = new CheckBoxControl(GetClientAreaWindow(), look, _T("Auto MC"),
                                checkbox_rc, checkbox_style,
                                this, AutoMc);
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
  BaseAccessPanel::Unprepare();
}

Widget *
LoadMacCreadyEditPanel(unsigned id)
{
  return new MacCreadyEditPanel(id);
}
