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

#include "Base.hpp"
#include "Form/Button.hpp"
#include "Form/SymbolButton.hpp"
#include "Form/Frame.hpp"
#include "Form/Form.hpp"
#include "Interface.hpp"
#include "Units/Units.hpp"
#include "Units/Group.hpp"
#include "UIGlobals.hpp"
#include "Look/IconLook.hpp"
#include "Look/DialogLook.hpp"
#include "Screen/Fonts.hpp"
#include "Language/Language.hpp"
#include "Screen/Layout.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "MainWindow.hpp"


enum ControlIndex {
  SetUp = 1000,
  CloseButton = 1001,
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
  const Bitmap *bmp = &icons.hBmpTabSettings;
  const PixelSize bitmap_size = bmp->GetSize();
  return (PixelScalar)max((unsigned)Layout::Scale(20),
                          (unsigned)bitmap_size.cy + (unsigned)Layout::Scale(2));
}

static gcc_pure PixelScalar
GetFooterHeight() {
  return Layout::Scale(40);
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
  if (action_id == SetUp) {
    InfoBoxManager::ShowInfoBoxPicker(id);
    Close();
  } else if (action_id == CloseButton)
    Close();
}
void
BaseAccessPanel::Show(const PixelRect &rc)
{
  if (managed_widget.IsDefined())
    managed_widget.Show();
  WndForm::ShowModeless();
}

void
BaseAccessPanel::Unprepare()
{
  delete header_text;
  delete setup_button;
  delete close_button;
  NullWidget::Unprepare();
  managed_widget.Clear();
}

void
BaseAccessPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  NullWidget::Prepare(parent, rc);
  WndForm::Move(rc);

  const DialogLook &look = UIGlobals::GetDialogLook();

  PixelRect base_rc;
  base_rc.left = base_rc.top = 0;
  base_rc.right = rc.right - rc.left;
  base_rc.bottom = rc.bottom - rc.top;

  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();


  PixelRect close_button_rc = base_rc;
  close_button_rc.top = close_button_rc.bottom - GetFooterHeight();
  close_button_rc.bottom -= Layout::Scale(2);
  close_button_rc.left += Layout::Scale(2);
  close_button_rc.right -= Layout::Scale(2);

  close_button = new WndButton(GetClientAreaWindow(), look,
                               _("Close"), close_button_rc,
                               button_style,
                               this, CloseButton);

  PixelScalar setup_button_width = 0.2 * (rc.right - rc.left);
  PixelRect setup_button_rc = base_rc;
  setup_button_rc.left = setup_button_rc.right - setup_button_width;
  setup_button_rc.bottom = setup_button_rc.top + GetHeaderHeight();
  setup_button = new WndSymbolButton(GetClientAreaWindow(), look,
                                     _("Setup"), setup_button_rc,
                                     button_style,
                                     this, SetUp);

  WindowStyle style_frame;
  PixelRect frame_rc = base_rc;
  frame_rc.right = setup_button_rc.left - 1;
  frame_rc.bottom = setup_button_rc.bottom;
  header_text = new WndFrame(GetClientAreaWindow(), look,
                             frame_rc.left, frame_rc.top,
                             frame_rc.right - frame_rc.left,
                             frame_rc.bottom - frame_rc.top,
                             style_frame);
  header_text->SetVAlignCenter();
  header_text->SetFont(Fonts::infobox_small);
  SetCaption();

  content_rc = base_rc;
  content_rc.top += GetHeaderHeight();
  content_rc.bottom -= GetFooterHeight();
  managed_widget.Move(content_rc);
}

void
BaseAccessPanel::SetCaption()
{
  InfoBoxSettings &settings = CommonInterface::SetUISettings().info_boxes;
  const unsigned panel_index = InfoBoxManager::GetCurrentPanel();
  InfoBoxSettings::Panel &panel = settings.panels[panel_index];
  const InfoBoxFactory::Type old_type = panel.contents[id];

  StaticString<32> buffer;
  buffer = gettext(InfoBoxFactory::GetName(old_type));
  header_text->SetText(buffer);
}

void
NumberButtonLayout::Prepare(ContainerWindow &parent, const PixelRect &parent_rc)
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

  value_rc.left = big_plus_rc.left;
  value_rc.right = little_plus_rc.right;
  value_rc.top = big_plus_rc.bottom;
  value_rc.bottom = big_minus_rc.top;

  double_size_plus_rc = big_plus_rc;
  double_size_plus_rc.right = little_plus_rc.right;
  double_size_minus_rc = big_minus_rc;
  double_size_minus_rc.right = little_minus_rc.right;
}

void
RatchetListLayout::Prepare(ContainerWindow &parent, const PixelRect &rc)
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
TwoButtonLayout::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  upper_rc = rc;
  GrowRect(upper_rc, -2, 0);
  lower_rc = upper_rc;
  const UPixelScalar height = GetFooterHeight();
  upper_rc.top = rc.top + (rc.bottom - rc.top - 2 * height) / 2;
  upper_rc.bottom = upper_rc.top + height;

  lower_rc.top = upper_rc.bottom + 1;
  lower_rc.bottom = lower_rc.top + height;
}
