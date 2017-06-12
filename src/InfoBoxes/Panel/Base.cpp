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

#include "Base.hpp"
#include "Form/Button.hpp"
#include "Form/Frame.hpp"
#include "Form/Form.hpp"
#include "Dialogs/HelpDialog.hpp"
#include "Interface.hpp"
#include "Units/Units.hpp"
#include "Units/Group.hpp"
#include "UIGlobals.hpp"
#include "UIState.hpp"
#include "Look/IconLook.hpp"
#include "Look/DialogLook.hpp"
#include "Look/GlobalFonts.hpp"
#include "Language/Language.hpp"
#include "Screen/Layout.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "InfoBoxes/Content/Factory.hpp"
#include "MainWindow.hpp"

enum ControlIndex {
  SetUp = 1000,
  HelpButton = 1001,
  CloseButton = 1002,
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

BaseAccessPanel::BaseAccessPanel(unsigned _id, Widget *_widget)
  :WndForm(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
           UIGlobals::GetMainWindow().GetClientRect(),
           _T(""), GetDialogStyle()),
   id(_id), managed_widget(GetClientAreaWindow(), _widget) {}

BaseAccessPanel::BaseAccessPanel(unsigned _id)
  :WndForm(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
           UIGlobals::GetMainWindow().GetClientRect(),
           _T(""), GetDialogStyle()),
   id(_id), managed_widget(GetClientAreaWindow()) {}

gcc_pure PixelScalar
BaseAccessPanel::GetHeaderHeight() {
  const IconLook &icons = UIGlobals::GetIconLook();
  const MaskedIcon &icon = icons.hBmpTabSettings;
  const PixelSize icon_size = icon.GetSize();
  return (PixelScalar)std::max((unsigned)Layout::Scale(20),
                               (unsigned)icon_size.cy + (unsigned)Layout::Scale(2));
}

static gcc_pure PixelScalar
GetFooterHeight() {
  return Layout::GetMinimumControlHeight();
}

void
BaseAccessPanel::Close()
{
  SetModalResult(mrOK);
}

void
BaseAccessPanel::OnPaint(Canvas &canvas)
{
  WndForm::OnPaint(canvas);
}

void
BaseAccessPanel::Hide()
{
  Close();
}

void
BaseAccessPanel::OnAction(int action_id)
{
  switch (action_id) {
  case SetUp:
    InfoBoxManager::ShowInfoBoxPicker(id);
    Close();
    break;
  case CloseButton:
    Close();
    break;
  case HelpButton:
    ShowHelp();
  }
}

void
BaseAccessPanel::ShowHelp()
{
  assert(HasCustomContent());

  InfoBoxSettings &settings = CommonInterface::SetUISettings().info_boxes;
  const unsigned panel_index = CommonInterface::GetUIState().panel_index;
  const InfoBoxSettings::Panel &panel = settings.panels[panel_index];
  const InfoBoxFactory::Type old_type = panel.contents[id];

  StaticString<256> help_text;
  help_text = gettext(InfoBoxFactory::GetDescription(old_type));
  /* get caption in case it is not already set */
  caption_text = gettext(InfoBoxFactory::GetName(old_type));
  HelpDialog(caption_text.c_str(), help_text.c_str());
}

void
BaseAccessPanel::Show(const PixelRect &rc)
{
  if (managed_widget.IsDefined())
    managed_widget.Show();
  WndForm::ShowModal();
}

void
BaseAccessPanel::Move(const PixelRect &rc_unused)
{
  const PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();
  WndForm::Move(rc);

  CalculateLayout(rc);

  close_button->Move(close_button_rc);
  setup_button->Move(setup_button_rc);
  if (HasCustomContent())
    help_button->Move(help_button_rc);

  header_text->Move(frame_rc);

  managed_widget.Move(content_rc);
}

void
BaseAccessPanel::CalculateLayout(const PixelRect &rc)
{
  base_rc.left = base_rc.top = 0;
  base_rc.right = rc.right - rc.left;
  base_rc.bottom = rc.bottom - rc.top;

  close_button_rc = base_rc;
  close_button_rc.top = close_button_rc.bottom - GetFooterHeight();

  PixelScalar setup_button_width = 0.2 * (rc.right - rc.left);
  setup_button_rc = help_button_rc = base_rc;
  setup_button_rc.left = setup_button_rc.right - setup_button_width;
  setup_button_rc.bottom = help_button_rc.bottom =
      setup_button_rc.top + GetHeaderHeight();

  help_button_rc.right = setup_button_rc.left;
  help_button_rc.left = help_button_rc.right - setup_button_width;

  frame_rc = base_rc;
  frame_rc.right = HasCustomContent() ? help_button_rc.left : setup_button_rc.left;
  frame_rc.bottom = setup_button_rc.bottom;

  content_rc = base_rc;
  content_rc.top += GetHeaderHeight();
  content_rc.bottom -= GetFooterHeight();
}

void BaseAccessPanel::Unprepare()
{
  if (HasCustomContent())
    delete(help_button);

  delete(close_button);
  delete(setup_button);
  delete(header_text);
}

void
BaseAccessPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  NullWidget::Prepare(parent, rc);

  const DialogLook &look = UIGlobals::GetDialogLook();

  WindowStyle button_style;
  button_style.TabStop();

  CalculateLayout(rc);

  close_button = new WndSymbolButton(GetClientAreaWindow(), look.button,
                                     _T("_X"), close_button_rc,
                                     button_style,
                                     *this, CloseButton);
  setup_button = new WndSymbolButton(GetClientAreaWindow(), look.button,
                                     _("Setup"), setup_button_rc,
                                     button_style,
                                     *this, SetUp);

  if (HasCustomContent())
    help_button = new WndSymbolButton(GetClientAreaWindow(), look.button,
                                      _("Help"), help_button_rc,
                                      button_style,
                                      *this, HelpButton);

  WindowStyle style_frame;
  header_text = new WndFrame(GetClientAreaWindow(), look,
                             frame_rc, style_frame);
  header_text->SetVAlignCenter();
  SetCaption();

  managed_widget.Move(content_rc);
  WndForm::Move(rc);
}

void
BaseAccessPanel::SetCaption()
{
  InfoBoxSettings &settings = CommonInterface::SetUISettings().info_boxes;
  const unsigned panel_index = CommonInterface::GetUIState().panel_index;
  const InfoBoxSettings::Panel &panel = settings.panels[panel_index];
  const InfoBoxFactory::Type old_type = panel.contents[id];

  caption_text = gettext(InfoBoxFactory::GetName(old_type));
  header_text->SetText(caption_text);
}

void
NumberButton2SubNumberLayout::CalculateLayout(const PixelRect &parent_rc,
                                              unsigned min_value_height,
                                              unsigned sub_number_height)
{
  NumberButtonSubNumberLayout::CalculateLayout(parent_rc, min_value_height);

  // set left and right.  Now stack on top and center with big valud field
  sub_number_top_rc = sub_number_bottom_rc = sub_number_rc;

  // split into two sub numbers
  unsigned middle = sub_number_rc.top + sub_number_rc.GetSize().cy / 2;
  sub_number_top_rc.bottom = middle - Layout::GetTextPadding();
  sub_number_top_rc.top = std::max(0, (int)sub_number_top_rc.bottom - (int)sub_number_height);
  sub_number_bottom_rc.top = middle + Layout::GetTextPadding();
  sub_number_bottom_rc.bottom = sub_number_bottom_rc.bottom + sub_number_height;

  sub_number_rc = value_rc;
  sub_number_rc.left = little_plus_rc.right + Layout::GetTextPadding();
  sub_number_rc.right = parent_rc.right;
}

void
NumberButtonSubNumberLayout::CalculateLayout(const PixelRect &parent_rc, unsigned min_value_height)
{
  NumberButtonLayout::CalculateLayout(parent_rc, min_value_height);

  sub_number_rc = value_rc;
  sub_number_rc.left = little_plus_rc.right + Layout::GetTextPadding();
  sub_number_rc.right = parent_rc.right;
}

void
NumberButtonLayout::CalculateLayout(const PixelRect &parent_rc, unsigned min_value_height)
{
  fixed ratio = Layout::landscape ? fixed(0.65) : fixed(0.6);
  PixelRect rc_bound = parent_rc;
  PixelSize sz_parent;
  sz_parent.cx = (PixelScalar)((parent_rc.right - parent_rc.left) * ratio);
  sz_parent.cy = (PixelScalar)((parent_rc.bottom - parent_rc.top) * ratio);

  rc_bound.top = parent_rc.top + (parent_rc.bottom - parent_rc.top - sz_parent.cy)
    / 2;
  rc_bound.bottom = rc_bound.top + sz_parent.cy;
  rc_bound.left = parent_rc.left + (parent_rc.right - parent_rc.left - sz_parent.cx)
    / 2;
  rc_bound.right = rc_bound.left + sz_parent.cx;

  big_plus_rc.left = rc_bound.left;
  big_plus_rc.top = rc_bound.top;
  big_plus_rc.bottom = big_plus_rc.top + sz_parent.cy / 3;
  big_plus_rc.right = big_plus_rc.left + sz_parent.cx / 2;

  little_plus_rc.right = rc_bound.right;
  little_plus_rc.top = rc_bound.top;
  little_plus_rc.bottom = little_plus_rc.top + sz_parent.cy / 3;
  little_plus_rc.left = little_plus_rc.right - sz_parent.cx / 2;

  big_minus_rc.left = rc_bound.left;
  big_minus_rc.bottom = rc_bound.bottom;
  big_minus_rc.top = big_minus_rc.bottom - sz_parent.cy / 3;
  big_minus_rc.right = big_minus_rc.left + sz_parent.cx / 2;

  little_minus_rc.right = rc_bound.right;
  little_minus_rc.bottom = rc_bound.bottom;
  little_minus_rc.top = little_minus_rc.bottom - sz_parent.cy / 3;
  little_minus_rc.left = little_minus_rc.right - sz_parent.cx / 2;

  double_size_plus_rc = big_plus_rc;
  double_size_plus_rc.right = little_plus_rc.right;
  double_size_minus_rc = big_minus_rc;
  double_size_minus_rc.right = little_minus_rc.right;

  value_rc.left = big_plus_rc.left;
  value_rc.right = little_plus_rc.right;
  value_rc.top = big_plus_rc.bottom;
  value_rc.bottom = big_minus_rc.top;
  if (value_rc.GetSize().cy < (int)min_value_height) {
    unsigned nudge_y = (min_value_height - value_rc.GetSize().cy) / 2;
    unsigned new_value_top = std::max((int)(big_plus_rc.top + 10),
                                      (int)(big_plus_rc.bottom - nudge_y));
    unsigned new_value_bottom = std::min((int)(big_minus_rc.bottom - 10),
                                         (int)(big_minus_rc.top + nudge_y));
    value_rc.top = big_plus_rc.bottom = little_plus_rc.bottom =
        double_size_plus_rc.bottom = new_value_top;
    value_rc.bottom = big_minus_rc.top = little_minus_rc.top =
        double_size_plus_rc.top = new_value_bottom;
  }
}

void
RatchetListLayout::CalculateLayout(const PixelRect &rc)
{
  PixelScalar right_buttons_width;
  right_buttons_width = 0.2 * (rc.right - rc.left);
  ratchet_list_rc = rc;
  ratchet_list_rc.right = rc.right - right_buttons_width;
  ratchet_list_rc.bottom = rc.bottom;

  ratchet_up_rc = rc;
  ratchet_up_rc.left = ratchet_list_rc.right - ratchet_list_rc.left + 1;
  ratchet_up_rc.bottom = rc.top + (rc.bottom - rc.top) / 2;

  ratchet_down_rc = rc;
  ratchet_down_rc.left = ratchet_up_rc.left;
  ratchet_down_rc.top = ratchet_up_rc.bottom + 1;
  ratchet_down_rc.bottom = rc.bottom;
}

void
ThreeCommandButtonWidgetLayout::CalculateLayout(const PixelRect &rc)
{
  const unsigned height = GetFooterHeight();
  const unsigned width = (rc.right - rc.left) / 3;

  widget_rc = left_button_rc = right_button_rc = middle_button_rc = rc;

  left_button_rc.top = right_button_rc.top = middle_button_rc.top =
      rc.bottom - height;
  left_button_rc.right = middle_button_rc.left = width;
  middle_button_rc.right = right_button_rc.left = width * 2;

  widget_rc.bottom = rc.bottom - height;
}

void
FourCommandButtonWidgetLayout::CalculateLayout(const PixelRect &rc)
{
  const unsigned height = (GetFooterHeight() * 5) / 4;;
  const unsigned width = (rc.right - rc.left) / 4;

  widget_rc = left_button_rc = left_middle_button_rc = right_button_rc
      = middle_button_rc = rc;

  left_button_rc.top = left_middle_button_rc.top = right_button_rc.top
      = middle_button_rc.top = rc.bottom - height;

  left_button_rc.right = left_middle_button_rc.left = width;
  left_middle_button_rc.right = middle_button_rc.left = width * 2;
  middle_button_rc.right = right_button_rc.left = width * 3;

  widget_rc.bottom = rc.bottom - height;
}


void
TwoCommandButtonListLayout::CalculateLayout(const PixelRect &rc)
{
  const UPixelScalar height = GetFooterHeight();
  list_rc = left_button_rc = right_button_rc = rc;

  left_button_rc.top = right_button_rc.top = rc.bottom - height;
  left_button_rc.right = right_button_rc.left = (rc.right - rc.left) / 2;

  list_rc.bottom = rc.bottom - height;
}

void
TwoButtonLayout::CalculateLayout(const PixelRect &rc)
{
  upper_rc = rc;
  upper_rc.Grow(-2, 0);
  lower_rc = upper_rc;
  const UPixelScalar height = GetFooterHeight();
  upper_rc.top = rc.top + (rc.bottom - rc.top - 2 * height) / 2;
  upper_rc.bottom = upper_rc.top + height;

  lower_rc.top = upper_rc.bottom + 1;
  lower_rc.bottom = lower_rc.top + height;
}

void
ThreeButtonLayout::CalculateLayout(const PixelRect &rc)
{
  upper_rc = rc;
  upper_rc.Grow(-2, 0);
  lower_left_rc = upper_rc;
  lower_left_rc.right = lower_left_rc.left +
      (upper_rc.right - upper_rc.left) / 2;
  lower_right_rc = lower_left_rc;
  lower_right_rc.Offset(lower_left_rc.GetSize().cx, 0);
  const UPixelScalar height = GetFooterHeight();
  upper_rc.top = rc.top + (rc.bottom - rc.top - 2 * height) / 2;
  upper_rc.bottom = upper_rc.top + height;

  lower_left_rc.top = lower_right_rc.top = upper_rc.bottom + 1;
  lower_left_rc.bottom = lower_right_rc.bottom = lower_left_rc.top + height;
}
