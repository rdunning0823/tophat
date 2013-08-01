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
#include "Look/GlobalFonts.hpp"
#include "Util/StaticString.hpp"
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
    N_("While you are in 'Goto' mode, the Nav Bar and all your Info boxes provide information solely about getting to the waypoint."),
    N_("While in 'Goto' mode, click the Nav Bar and 'Resume' to navigate your task again."),
    N_("If you are flying a MAT task, just create the start and finish.  Task points will be added as you fly over them."),
    N_("Create tasks in See You desktop and save them as a .CUP file.  Copy the .CUP file to XCSoarData.  Load using Top Hat's task manager."),
    N_("Click on an item in the map to show its details."),
    N_("Click on a waypoint on the map to Goto it."),
    N_("Click on an InfoBox to see what it does, or change its function."),
    N_("Click on the Wind InfoBox to change your wind settings."),
    N_("Click on the MacCready InfoBox to change your MacCready setting."),
    N_("You must have the MacCready InfoBox visible to change the MacCready setting.  Unless you have a logger that changes Top Hat's MacCready setting."),
    N_("If you increase your MacCready setting, the map will increase the altitude you need to reach any waypoint.  It assumes you fly at the appropriate MacCready speed."),
    N_("Set your Nationality using the Setup menu.  This sets up your contest rules and your units."),
    N_("Altitudes displayed on the map are the altitudes required to reach the waypoint at your safety height flying at the current MacCready setting / MacCready speed."),
    N_("The large vertical arrow on the left is your final glide indicator.  It indicates the amount of altitude above or below final glide."),
    N_("If you're flying a task, the final glide indicator indicates your arrival height above the finish height plus your safety height."),
    N_("Too many waypoint labels?  Not enough?  Click the 'Labels...' button on the menu to toggle more, fewer, less options.'"),
    N_("While flying, you should be able to do anything from either the Nav Bar or the primary 'M' menu."),
    N_("Enter the pilot's name for the .IGC file with the Setup menu"),
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

  WndButton *prev_tip, *next_tip, *close;
  WndFrame *tip_text;
  CheckBoxControl *chkb_decline;

  StartupAssistant()
    :WndForm(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
             UIGlobals::GetMainWindow().GetClientRect(),
             _T(""), GetDialogStyle())
  {}

  void OnTimer();

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();
  virtual bool Save(bool &changed);
  virtual void Show(const PixelRect &rc);
  virtual void Hide() {};
  virtual void Move(const PixelRect &rc);

  /**
   * returns on full screen less height of the NavSliderWidget
   */
  virtual PixelRect GetSize(const PixelRect &rc);

  /**
   * returns height of TaskNavSlider bar
   */
  UPixelScalar GetNavSliderHeight();

  /**
   * from ActionListener
   */
  virtual void OnAction(int id);

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
};

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
  UPixelScalar large_font_height = UIGlobals::GetLook().info_box.value.font->GetHeight();
  UPixelScalar small_font_height = UIGlobals::GetDialogLook().list.font->GetHeight();

  return large_font_height + 2 * small_font_height - Layout::Scale(9);
}

PixelRect
StartupAssistant::GetSize(const PixelRect &rc)
{
  UPixelScalar nav_slider_height = GetNavSliderHeight();

  PixelRect rc_form = rc;
  rc_form.left += Layout::landscape ? nav_slider_height / 2 :
      Layout::Scale(3);
  rc_form.right -= Layout::landscape ? nav_slider_height / 2 :
      Layout::Scale(3);
  rc_form.top = nav_slider_height + Layout::Scale(3);

  UPixelScalar height = std::min(rc.bottom - rc_form.top,
                            (PixelScalar)nav_slider_height * 6);
  rc_form.bottom = rc_form.top + height;

  return rc_form;
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
  tip.Format(_T("Tip %u:\n%s"), ui_settings.last_startup_tip, tips[ui_settings.last_startup_tip - 1]);

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
StartupAssistant::OnTimer()
{
}

void
StartupAssistant::SetRectangles(const PixelRect &rc_outer)
{
  PixelRect rc;
  rc.left = rc.top = Layout::Scale(2);
  rc.right = rc_outer.right - rc_outer.left - Layout::Scale(2);
  rc.bottom = rc_outer.bottom - rc_outer.top - Layout::Scale(2);

  UPixelScalar button_height = std::min(GetNavSliderHeight(),
                                        UPixelScalar(rc.bottom / 5));

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
  rc_tip_text.top = rc.top + Layout::Scale(2);
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
  const PixelRect rc_form = GetSize(rc);
  NullWidget::Prepare(parent, rc_form);
  WndForm::Move(rc_form);

  SetRectangles(rc_form);

  WindowStyle style_frame;
  tip_text = new WndFrame(GetClientAreaWindow(), look,
                          rc_tip_text,
                          style_frame);
  tip_text->SetFont(Fonts::infobox_small);

  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();
  close = new WndButton(GetClientAreaWindow(), dialog_look, _T("Close"),
                        rc_close,
                        button_style, *this, CloseDialogClick);

  next_tip = new WndButton(GetClientAreaWindow(), dialog_look, _T("Next Tip"),
                           rc_next_tip,
                           button_style, *this, NextTipClick);

  prev_tip = new WndButton(GetClientAreaWindow(), dialog_look, _T("Previous Tip"),
                           rc_prev_tip,
                           button_style, *this, PrevTipClick);

  ButtonWindowStyle checkbox_style;
  checkbox_style.TabStop();
  chkb_decline = new CheckBoxControl(GetClientAreaWindow(),
                                     UIGlobals::GetDialogLook(),
                                     _("Don't show tips at startup"),
                                     rc_chkb_decline, checkbox_style,
                                     this, DeclineCheckboxClick);

  SetTip(true);
}

bool
StartupAssistant::Save(bool &changed)
{
  UISettings &ui_settings = CommonInterface::SetUISettings();
  Profile::Set(ProfileKeys::StartupTipId, ui_settings.last_startup_tip);
  bool declined = chkb_decline->GetState();
  Profile::Set(ProfileKeys::StartupTipDeclineVersion,
               declined ? TopHat_ProductToken : _T(""));

  Profile::Save();
  return true;
}

void
dlgStartupAssistantShowModal(bool conditional)
{
  if (conditional)
    ShowDialogSetupQuick(true);

  StaticString<32> decline_ver(_T(""));
  UTF8ToWideConverter text2(Profile::Get(ProfileKeys::StartupTipDeclineVersion, ""));
  if (text2.IsValid())
    decline_ver = text2;

  if ((decline_ver == TopHat_ProductToken) && conditional)
    return;

  ContainerWindow &w = UIGlobals::GetMainWindow();
  StartupAssistant *instance = new StartupAssistant();
  instance->Initialise(w, instance->GetSize(w.GetClientRect()));
  instance->Prepare(w, instance->GetSize(w.GetClientRect()));
  instance->ShowModal();
  instance->Hide();
  instance->Unprepare();
  delete instance;
}
