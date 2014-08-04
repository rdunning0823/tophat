/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2011 The XCSoar Project
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

#include "Dialogs/Task/StartTimeEntry.hpp"
#include "Dialogs/Message.hpp"
#include "Widget/Widget.hpp"
#include "Widget/ManagedWidget.hpp"
#include "Form/Form.hpp"
#include "Form/Button.hpp"
#include "Form/DigitEntry.hpp"
#include "Form/Frame.hpp"
#include "Screen/SingleWindow.hpp"
#include "Screen/Timer.hpp"
#include "Screen/Layout.hpp"
#include "Language/Language.hpp"
#include "UIGlobals.hpp"
#include "Components.hpp"
#include "Time/RoughTime.hpp"
#include "Time/BrokenDateTime.hpp"
#include "Time/LocalTime.hpp"
#include "Language/Language.hpp"
#include "Interface.hpp"
#include "Util/StaticString.hpp"
#include "Formatter/LocalTimeFormatter.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Engine/Task/Ordered/Points/OrderedTaskPoint.hpp"
#include "Task/TaskType.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Look/DialogLook.hpp"
#include "Look/Look.hpp"
#include "NMEA/Aircraft.hpp"
#include "Navigation/Aircraft.hpp"

gcc_const
static WindowStyle
GetDialogStyle()
{
  WindowStyle style;
  style.Hide();
  style.ControlParent();
  return style;
}

enum {
  CLEAR = 100,
};

enum {
  CURRENTTIME,
  STARTTIME,
  DIGITENTRY,
  CLOSEBUTTON,
  CANCELBUTTON,
};


class StartTimeEntry : public NullWidget, public WndForm
{
protected:

  const NMEAInfo &basic;
  const ComputerSettings &settings;

  /**
   * This timer updates the clock
   */
  WindowTimer dialog_timer;

  /* the time being edited stored as utc */
  RoughTime &utc_time;

  PixelRect rc_digit_entry, rc_label_start, rc_label_current, rc_label_help;
  PixelRect rc_cancel, rc_close;

  WndButton *cancel, *close;
  WndFrame *label_start, *label_current, *label_help;
  DigitEntry *digit_entry;

  StaticString<50> start_label_text;
  unsigned start_label_width;


public:
  StartTimeEntry(RoughTime &_utc_time)
    :WndForm(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
             UIGlobals::GetMainWindow().GetClientRect(),
             _T(""), GetDialogStyle()),
     basic(CommonInterface::Basic()),
     settings(CommonInterface::GetComputerSettings()),
     dialog_timer(*this),
     utc_time(_utc_time)
  {}

  /* override from Timer */
  virtual bool OnTimer(WindowTimer &timer);


  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();
  virtual bool Save(bool &changed) {
    return true;
  }
  virtual void Show(const PixelRect &rc) {};
  virtual void Hide() {};
  virtual void Move(const PixelRect &rc) {};

  /* overrides from WndForm */
  virtual void OnResize(PixelSize new_size) override;
  virtual void ReinitialiseLayout(const PixelRect &parent_rc) override;

  /**
   * from ActionListener
   */
  virtual void OnAction(int id);

protected:
  /**
   * sets up rectangles for layout of screen
   * @param rc. rect of dialog
   */
  void SetRectangles(const PixelRect &rc);
  void UpdateCurrentTime();
};

void
StartTimeEntry::ReinitialiseLayout(const PixelRect &parent_rc)
{
  WndForm::Move(parent_rc);
}

void
StartTimeEntry::OnResize(PixelSize new_size)
{
  WndForm::OnResize(new_size);
  SetRectangles(GetClientRect());

  label_current->Move(rc_label_current);
  label_help->Move(rc_label_help);
  label_start->Move(rc_label_start);
  close->Move(rc_close);
  cancel->Move(rc_cancel);
  digit_entry->Move(rc_digit_entry);
}

void
StartTimeEntry::SetRectangles(const PixelRect &rc_outer)
{
  PixelRect rc;
  UPixelScalar side_margin = Layout::landscape ? Layout::Scale(10) : Layout::Scale(4);
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  unsigned line_height = dialog_look.text_font->TextSize(_T("A")).cy * 1.5;

  rc.left = rc_outer.left + side_margin;
  rc.top = rc_outer.left + Layout::Scale(2);
  rc.right = rc_outer.right - side_margin;
  rc.bottom = rc_outer.bottom - rc_outer.top -
      Layout::Scale(2) - GetTitleHeight();

  const PixelSize sz_button { (unsigned)(rc.right - rc.left) / 2,
    (unsigned)Layout::Scale(40) };

  rc_cancel = rc_close = rc_digit_entry = rc_label_start = rc_label_current
      = rc_label_help = rc;

  rc_cancel.top = rc_close.top = rc.bottom - sz_button.cy;
  rc_cancel.left = rc_cancel.right - sz_button.cx;
  rc_close.right = rc_cancel.left;

  PixelSize sz_entry = digit_entry->GetRecommendedSize();
  rc_label_start.right = rc_label_start.left + start_label_width;
  rc_label_start.top =
      rc.top + (rc.GetSize().cy - sz_button.cy - line_height) / 2;
  rc_label_start.bottom = rc_label_start.top + line_height;

  rc_digit_entry.left = rc_label_start.right;
  rc_digit_entry.right = rc_digit_entry.left + sz_entry.cx;
  rc_digit_entry.top =
        rc.top + (rc.GetSize().cy - sz_button.cy - sz_entry.cy) / 2;
  rc_digit_entry.bottom = rc_digit_entry.top + sz_entry.cy;

  rc_label_current.bottom = rc_digit_entry.top - line_height;
  rc_label_current.top = rc_label_current.bottom - line_height;

  rc_label_help.top += line_height;
  rc_label_help.bottom = rc_label_help.top + line_height;
}

void
StartTimeEntry::OnAction(int id)
{
  switch (id) {
  case CANCELBUTTON:
    SetModalResult(mrCancel);
    break;

  case CLOSEBUTTON:
    if (!CommonInterface::Calculated().flight.flying) {
      ShowMessageBox(_("Must by flying to update task start time."),
                     _T("Not flying!"),
                     MB_ICONEXCLAMATION | MB_OK);
      SetModalResult(mrCancel);
    } else {
      utc_time = digit_entry->GetTimeValue() - settings.utc_offset;
      if (utc_time.IsValid())
        SetModalResult(mrOK);
      else
        SetModalResult(mrCancel);
    }
    break;
  }
}

void
StartTimeEntry::UpdateCurrentTime()
{
  if (!dialog_timer.IsActive())
    dialog_timer.Schedule(1000);

  StaticString<50> now_string1;
  StaticString<50> now_string2;
  FormatSignedTimeHHMM(now_string1.buffer(), TimeLocal((int)basic.time, settings.utc_offset));
  now_string2.Format(_T("%s  %s"), _("Current time"), now_string1.c_str());
  label_current->SetText(now_string2.c_str());
}

bool
StartTimeEntry::OnTimer(WindowTimer &timer)
{
  if (timer == dialog_timer) {
    UpdateCurrentTime();
    return true;
  }
  return WndForm::OnTimer(timer);
}

void
StartTimeEntry::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  const ButtonLook &button_look = dialog_look.button;

  PixelRect rc_form = rc; //GetSize(rc);
  NullWidget::Prepare(parent, rc_form);
  WndForm::Move(rc_form);

  SetCaption(_("Task start"));
  start_label_text.Format(_T("%s:  "), _("Start time"));
  start_label_width = dialog_look.text_font->TextSize(start_label_text.c_str()).cx;

  /* create the input control widget*/
  WindowStyle control_style;
  control_style.TabStop();
  digit_entry = new DigitEntry(look);
  digit_entry->CreateTime(GetClientAreaWindow(), rc, control_style);
  digit_entry->Resize(digit_entry->GetRecommendedSize());
  digit_entry->SetActionListener(*this, CLOSEBUTTON);
  AddDestruct(digit_entry);

  const BrokenTime bt = BrokenDateTime::NowUTC();
  utc_time = utc_time.FromMinuteOfDayChecked(bt.hour * 60 + bt.minute);
  {
    ProtectedTaskManager::Lease task_manager(*protected_task_manager);

    if (task_manager->GetMode() == TaskType::ORDERED &&
        (task_manager->TaskSize() > 1)) {
      const OrderedTask &ordered_task = task_manager->GetOrderedTask();
      const OrderedTaskPoint &start = ordered_task.GetTaskPoint(0);

      if (start.HasExited())
        utc_time = utc_time.FromSecondOfDayChecked(
            (unsigned)ordered_task.GetStats().start.time);
    }
  }
  digit_entry->SetValue(utc_time + settings.utc_offset);

  // call after digit_entry is created
  SetRectangles(rc_form);
  digit_entry->Move(rc_digit_entry);

  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();
  WindowStyle style_frame;

  close = new WndButton(GetClientAreaWindow(), button_look,
                        _("Update"),
                        rc_close,
                        button_style, *this, CLOSEBUTTON);
  AddDestruct(close);

  cancel = new WndButton(GetClientAreaWindow(), button_look,
                         _("Cancel"),
                         rc_cancel,
                         button_style, *this, CANCELBUTTON);
  AddDestruct(cancel);

  label_help = new WndFrame(GetClientAreaWindow(), dialog_look,
                            rc_label_help, style_frame);
  AddDestruct(label_help);
  label_help->SetCaption(_("Adjust time you started the task"));

  label_current = new WndFrame(GetClientAreaWindow(), dialog_look,
                              rc_label_current, style_frame);
  AddDestruct(label_current);

  label_start = new WndFrame(GetClientAreaWindow(), dialog_look,
                              rc_label_start, style_frame);
  label_start->SetCaption(start_label_text.c_str());
  AddDestruct(label_start);

  UpdateCurrentTime();

  dialog_timer.Schedule(1000);
}

void
StartTimeEntry::Unprepare()
{
  dialog_timer.Cancel();
}

static void
UpdateStart(RoughTime &utc_time)
{
  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  if (task_manager->GetMode() == TaskType::ORDERED &&
      (task_manager->TaskSize() > 1)) {
    const MoreData &basic = CommonInterface::Basic();
    const DerivedInfo &calculated = CommonInterface::Calculated();
    const AircraftState aircraft_state =
        ToAircraftState(basic, calculated);

    task_manager->OverrideStartTime(aircraft_state,
                                    fixed(utc_time.GetMinuteOfDay() * 60));

    const OrderedTask &ordered_task = task_manager->GetOrderedTask();

    RoughTime time = time.FromSecondOfDayChecked((unsigned)ordered_task.GetStats().start.time);
  }
}

void
StartTimeEntryDialog()
{
  ContainerWindow &w = UIGlobals::GetMainWindow();
  RoughTime utc_time;
  StartTimeEntry *instance = new StartTimeEntry(utc_time);
  ManagedWidget managed_widget(w, instance);
  managed_widget.Move(w.GetClientRect());
  managed_widget.Show();
  if (instance->ShowModal() == mrOK) {
    UpdateStart(utc_time);
  }
}

