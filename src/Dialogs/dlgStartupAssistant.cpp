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

#include "Dialogs/Dialogs.h"
#include "Form/Button.hpp"
#include "Form/Form.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Form/Frame.hpp"
#include "Look/InfoBoxLook.hpp"
#include "Look/Look.hpp"
#include "Look/DialogLook.hpp"
#include "UIGlobals.hpp"
#include "Form/ButtonPanel.hpp"
#include "Language/Language.hpp"
#include "Screen/SingleWindow.hpp"
#include "Screen/Layout.hpp"
#include "Profile/Profile.hpp"
#include "DisplaySettings.hpp"
#include "Util/Macros.hpp"
#include "Interface.hpp"
#include "UISettings.hpp"
#include "Form/CheckBox.hpp"
#include "Version.hpp"
#include "Widget/Widget.hpp"
#include "Widget/ManagedWidget.hpp"
#include "Look/GlobalFonts.hpp"
#include "Util/StaticString.hxx"
#include "Util/ConvertString.hpp"
#include "Task/TaskNationalities.hpp"
#include "Task/TaskBehaviour.hpp"
#include "Dialogs/Message.hpp"

const TCHAR* tips [] = {
    N_("Click the Nav Bar at the top to show the navigation menu."),
    N_("Slide the Nav Bar to the left or right to advance the task turnpoint."),
    N_("Click the 'M' button to show the Main menu.  Try clicking it again."),
    N_("Need to show the 'Setup Top Hat' screen?  Click the 'M' button 3x, and click the Gear icon."),
    N_("Need a map, waypoint or airspace file?  Top Hat Setup downloads these from the internet."),
    N_("To cancel a menu, click anywhere on the screen in the background to hide it."),
    N_("After you've entered a task cylinder, the Nav Bar shows a checkmark by the turnpoint name."),
    N_("You can suspend your task by clicking on a waypoint and selecting 'Goto.'"),
    N_("While you are in 'Goto' mode, the Nav Bar and all your Infoboxes provide information solely about getting to the waypoint."),
    N_("While in 'Goto' mode, click the Nav Bar and 'Resume' to navigate your task again."),
    N_("If you are flying a MAT task, just create the start and finish.  Add or remove task points to the task by clicking them on the map."),
    N_("Create tasks in See You desktop and save them as a .CUP file.  Copy the .CUP file to XCSoarData.  Load using Top Hat's task manager."),
    N_("Click on an item in the map to show its details."),
    N_("Click on a waypoint on the map to Goto it."),
    N_("Click on an Infobox to see what it does, or change its function."),
    N_("Click on the Wind Infobox to change your wind settings."),
    N_("Click on the MacCready Infobox to change your MacCready setting."),
    N_("You must have the MacCready Infobox visible to change the MacCready setting.  Unless you have a logger that changes Top Hat's MacCready setting."),
    N_("If you increase your MacCready setting, the map will increase the altitude you need to reach any waypoint.  It assumes you fly at the appropriate MacCready speed."),
    N_("Set your Nationality using the Setup menu.  This sets up your contest rules and your units."),
    N_("Altitudes displayed on the map are the altitudes required to reach the waypoint at your safety height flying at the current MacCready setting / MacCready speed."),
    N_("The large vertical arrow on the left is your final glide indicator.  It indicates the amount of altitude above or below final glide."),
    N_("If you're flying a task, the final glide indicator indicates your arrival height above the finish height plus your safety height."),
    N_("Too many waypoint labels?  Not enough?  Click the 'Labels...' button on the menu to toggle more, fewer, less options.'"),
    N_("While flying, you should be able to do anything from either the Nav Bar or the primary 'M' menu."),
    N_("Enter the pilot's name for the .IGC file with the Setup menu"),
    N_("Too many buttons on the screen? Move the 'S' button to the menu in Setup > Screens"),
    N_("Clean up the screen?  Click the Wind Infobox and select to show the Wind arrow only in the Infobox"),
    N_("To EXIT Top Hat, click the 'M' menu button four times"),
    N_("Task: to fly a TAT/AAT task with big cylinders, the Nav Bar points to the CENTER of the TP.  The arrow on the map points to the target."),
    N_("Task: when flying a TAT/AAT task with big cylinders, consider the 'Target arrow' Infobox to help navigate to the target."),

};

enum ControlIndex {
  TipText,
  DeclineCheckbox,
  NextTip,
  PrevTip,
  CloseDialog,
};

enum Actions {
  NextTipClick = 100,
  PrevTipClick,
  DeclineCheckboxClick,
  CloseDialogClick,
};

gcc_const
static WindowStyle
GetDialogStyle()
{
  WindowStyle style;
  style.Hide();
  style.ControlParent();
  return style;
}

class StartupAssistant : public NullWidget, public WndForm
{
public:
  PixelRect rc_tip_text, rc_chkb_decline;
  PixelRect rc_prev_tip, rc_next_tip, rc_close;

  Button *prev_tip, *next_tip, *close;
  WndFrame *tip_text;
  CheckBoxControl chkb_decline;

  StartupAssistant()
    :WndForm(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
             UIGlobals::GetMainWindow().GetClientRect(),
             _T(""), GetDialogStyle())
  {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Unprepare() override;
  virtual bool Save(bool &changed) override;
  virtual void Show(const PixelRect &rc) override;
  virtual void Hide() override {};
  virtual void Move(const PixelRect &rc) override;

  /**
   * returns height of TaskNavSlider bar
   */
  UPixelScalar GetNavSliderHeight();

  /**
   * from ActionListener
   */
  virtual void OnAction(int id) override;

  /**
   * sets the current tip and advances ui_settings.last_startup_tip
   * to the next tip in the array
   */
  void SetTip(bool forward);

  /**
   * sets up rectangles for layout of screen
   * @param rc. rect of dialog
   */
  void SetRectangles(const PixelRect &rc);

#ifndef _WIN32_WCE
  /* This crashes Prepare on PPC2003 */

  /* overrides from WndForm */
  virtual void OnResize(PixelSize new_size) override;
#endif
  virtual void ReinitialiseLayout(const PixelRect &parent_rc) override;
};

void
StartupAssistant::ReinitialiseLayout(const PixelRect &parent_rc)
{
  WndForm::Move(parent_rc);
}

#ifndef _WIN32_WCE
void
StartupAssistant::OnResize(PixelSize new_size)
{
  WndForm::OnResize(new_size);
  SetRectangles(GetClientRect());

  tip_text->Move(rc_tip_text);
  close->Move(rc_close);
  next_tip->Move(rc_next_tip);
  prev_tip->Move(rc_prev_tip);
  chkb_decline.Move(rc_chkb_decline);
}
#endif

void
StartupAssistant::Move(const PixelRect &rc)
{
}

void
StartupAssistant::Show(const PixelRect &rc)
{
}

UPixelScalar
StartupAssistant::GetNavSliderHeight()
{
  UPixelScalar large_font_height = UIGlobals::GetLook().info_box.value_font.GetHeight();
  UPixelScalar small_font_height = UIGlobals::GetDialogLook().list.font->GetHeight();

  return large_font_height + 2 * small_font_height - Layout::Scale(9);
}

void
StartupAssistant::SetTip(bool forward)
{
  UISettings &ui_settings = CommonInterface::SetUISettings();
  assert(ARRAY_SIZE(tips) > 1);

  // 1-based index.  move to value between 1 and ARRAY_SIZE(tips)
  if (forward) {
    if (ui_settings.last_startup_tip < ARRAY_SIZE(tips))
      ui_settings.last_startup_tip++;
    else
      ui_settings.last_startup_tip = 1;
  } else {
    if (ui_settings.last_startup_tip > 1)
      ui_settings.last_startup_tip--;
    else
      ui_settings.last_startup_tip = ARRAY_SIZE(tips);
  }
  assert(ui_settings.last_startup_tip >= 1 &&
         ui_settings.last_startup_tip <= ARRAY_SIZE(tips));

  StaticString<512> tip;
  tip.Format(_T("%s %u:\n%s"), _("Tip"), ui_settings.last_startup_tip, gettext(tips[ui_settings.last_startup_tip - 1]));

  tip_text->SetCaption(tip.c_str());
}

void
StartupAssistant::OnAction(int id)
{
  if (id == NextTipClick)
    SetTip(true);

  else if (id == PrevTipClick)
    SetTip(false);

  else if (id == CloseDialogClick) {
    bool changed;
    Save(changed);
    SetModalResult(mrOK);
  }
}

void
StartupAssistant::SetRectangles(const PixelRect &rc_outer)
{
  PixelRect rc;
  rc.left = rc.top = Layout::Scale(2);
  rc.right = rc_outer.right - rc_outer.left - Layout::Scale(2);
  rc.bottom = rc_outer.bottom - rc_outer.top - Layout::Scale(2) - WndForm::GetTitleHeight();

  UPixelScalar button_height = Layout::GetMinimumControlHeight();

  rc_close = rc_tip_text = rc_chkb_decline =
      rc_prev_tip = rc_next_tip = rc;
  rc_close.top = rc_close.bottom - button_height;

  rc_prev_tip.bottom = rc_next_tip.bottom =  rc_close.top - 1;
  rc_prev_tip.top = rc_next_tip.top = rc_next_tip.bottom -
      button_height;

  rc_prev_tip.right = (rc.right - rc.left) / 2;
  rc_next_tip.left = rc_prev_tip.right - 1;

  rc_chkb_decline.bottom = rc_prev_tip.top - 1;
  rc_chkb_decline.top = rc_chkb_decline.bottom - button_height / 2;
  rc_chkb_decline.right = rc_chkb_decline.left + (rc.right - rc.left); /*
      Layout::landscape ? 3 : 2;*/

  rc_tip_text.bottom = rc_chkb_decline.top - 1 - Layout::Scale(2);
  rc_tip_text.top = Layout::landscape ? rc.top + Layout::Scale(2) :
      (rc_chkb_decline.top - rc.top) / 2;
  rc_tip_text.left += Layout::Scale(2);
  rc_tip_text.right -= Layout::Scale(2);
}

void
StartupAssistant::Unprepare()
{
  delete prev_tip;
  delete next_tip;
  delete close;
  delete tip_text;
}

void
StartupAssistant::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const PixelRect rc_form = rc;
  NullWidget::Prepare(parent, rc_form);

  SetCaption(_("Top Hat Tips"));
  SetRectangles(rc_form);

  WindowStyle style_frame;
  tip_text = new WndFrame(GetClientAreaWindow(), look,
                          rc_tip_text,
                          style_frame);

  const ButtonLook &button_look = UIGlobals::GetDialogLook().button;
  WindowStyle button_style;
  button_style.TabStop();
  close = new WndSymbolButton(GetClientAreaWindow(), button_look, _T("_X"),
                              rc_close,
                              button_style, *this, CloseDialogClick);

  next_tip = new Button(GetClientAreaWindow(), button_look, _("Next tip"),
                           rc_next_tip,
                           button_style, *this, NextTipClick);

  prev_tip = new Button(GetClientAreaWindow(), button_look, _("Previous tip"),
                           rc_prev_tip,
                           button_style, *this, PrevTipClick);

  WindowStyle checkbox_style;
  checkbox_style.TabStop();
  chkb_decline.Create(GetClientAreaWindow(),
                      UIGlobals::GetDialogLook(),
                      _("Don't show tips at startup"),
                      rc_chkb_decline, checkbox_style,
                      *this, DeclineCheckboxClick);

  WndForm::Move(rc_form);
  SetTip(true);
}

bool
StartupAssistant::Save(bool &changed)
{
  UISettings &ui_settings = CommonInterface::SetUISettings();
  Profile::Set(ProfileKeys::StartupTipId, ui_settings.last_startup_tip);
  bool declined = chkb_decline.GetState();
  Profile::Set(ProfileKeys::StartupTipDeclineVersion,
               declined ? TopHat_ProductToken : _T(""));

  ui_settings.restart_gesture_help =  true;

  Profile::Save();
  return true;
}

void
dlgStartupAssistantShowModal(bool conditional)
{
  if (conditional)
    ShowDialogSetupQuick();

  StaticString<64> decline_ver(_T(""));
  UTF8ToWideConverter text2(Profile::Get(ProfileKeys::StartupTipDeclineVersion, ""));
  if (text2.IsValid())
    decline_ver = text2;

  if ((decline_ver == TopHat_ProductToken) && conditional)
    return;

  ContainerWindow &w = UIGlobals::GetMainWindow();
  StartupAssistant *instance = new StartupAssistant();
  ManagedWidget managed_widget(w, instance);
  managed_widget.Move(w.GetClientRect());
  managed_widget.Show();
  instance->ShowModal();
}
