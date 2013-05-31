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

#include "Dialogs/WidgetDialog.hpp"
#include "Look/DialogLook.hpp"
#include "Form/Form.hpp"
#include "Form/ButtonPanel.hpp"
#include "Form/Widget.hpp"
#include "UIGlobals.hpp"
#include "Language/Language.hpp"
#include "Screen/SingleWindow.hpp"
#include "Screen/Layout.hpp"

gcc_const
static WindowStyle
GetDialogStyle()
{
  WindowStyle style;
  style.Hide();
  style.ControlParent();
  return style;
}

WidgetDialog::WidgetDialog(const TCHAR *caption, const PixelRect &rc,
                           Widget *_widget,
                           DialogFooter::Listener *_listener,
                           UPixelScalar _footer_height,
                           ButtonPanel::ButtonPanelPosition _button_position)
  :WndForm(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
           rc, caption, GetDialogStyle()),
   dialog_footer(GetClientAreaWindow(), _listener, _footer_height),
   buttons(GetClientAreaWindow(), UIGlobals::GetDialogLook(),
           _button_position),
   widget(GetClientAreaWindow(), _widget),
   auto_size(false),
   changed(false)
{
  widget.Move(buttons.UpdateLayout());
}

WidgetDialog::WidgetDialog(const TCHAR *caption, Widget *_widget,
                           DialogFooter::Listener *_listener,
                           UPixelScalar _footer_height,
                           ButtonPanel::ButtonPanelPosition _button_position)
  :WndForm(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
           UIGlobals::GetMainWindow().GetClientRect(),
           caption, GetDialogStyle()),
   dialog_footer(GetClientAreaWindow(), _listener, _footer_height),
   buttons(GetClientAreaWindow(), UIGlobals::GetDialogLook(),
           _button_position),
   widget(GetClientAreaWindow(), _widget),
   auto_size(true),
   changed(false)
{
  widget.Move(buttons.UpdateLayout());
}

WidgetDialog::DialogFooter::DialogFooter(ContainerWindow &parent,
                                         Listener *_listener,
                                         UPixelScalar _height)
  :listener(_listener), height(_height)
{
  WindowStyle style;
  // TODO: hack - this creates a 1 pixel footer if no footer exists
  set(parent, 0, parent.GetHeight() - height, parent.GetWidth(),
      (height > 0) ? height : 1, style);
}

void
WidgetDialog::AutoSize()
{
  const PixelRect parent_rc = GetParentClientRect();
  const PixelSize parent_size = GetPixelRectSize(parent_rc);

  widget.Prepare();
  PixelSize min_size = widget.Get()->GetMinimumSize();
  min_size.cy += GetTitleHeight();
  PixelSize max_size = widget.Get()->GetMaximumSize();
  max_size.cy += GetTitleHeight();

  const PixelScalar min_height_with_buttons =
    min_size.cy + Layout::GetMaximumControlHeight();
  const PixelScalar max_height_with_buttons =
    max_size.cy + Layout::GetMaximumControlHeight();
  if (/* need full dialog height even for minimum widget height? */
      /*landscape */
      min_height_with_buttons >= parent_size.cy ||
      /* try to avoid putting buttons left on portrait screens; try to
         comply with maximum widget height only on landscape
         screens */
      (parent_size.cx > parent_size.cy &&
       max_height_with_buttons >= parent_size.cy)) {
    /* need full height, buttons must be left */
    PixelRect rc = parent_rc;
    if (max_size.cy < parent_size.cy)
      rc.bottom = rc.top + max_size.cy;

    PixelRect remaining = buttons.LeftLayout(rc);
    PixelSize remaining_size = GetPixelRectSize(remaining);
    if (remaining_size.cx > max_size.cx)
      rc.right -= remaining_size.cx - max_size.cx;

    Move(rc);
    rc.bottom -= dialog_footer.GetHeight();
    rc.bottom -= GetTitleHeight();
    widget.Move(buttons.LeftLayout(rc));
    return;
  }

  /* see if buttons fit at the bottom */
  /* portrait */

  PixelRect rc = parent_rc;
  if (max_size.cx < parent_size.cx)
    rc.right = rc.left + max_size.cx;

  PixelRect remaining = buttons.BottomLayout(rc);
  PixelSize remaining_size = GetPixelRectSize(remaining);

  if (remaining_size.cy > max_size.cy)
    rc.bottom -= remaining_size.cy - max_size.cy;

  Move(rc);
  rc.bottom -= dialog_footer.GetHeight();
  rc.bottom -= GetTitleHeight();
  widget.Move(buttons.BottomLayout(rc));
}

int
WidgetDialog::ShowModal()
{
  if (auto_size)
    AutoSize();

  widget.Show();
  return WndForm::ShowModal();
}

void
WidgetDialog::OnAction(int id)
{
  if (id == mrOK) {
    bool require_restart;
    if (!widget.Get()->Save(changed, require_restart))
      return;
  }

  WndForm::OnAction(id);
}

void
WidgetDialog::OnDestroy()
{
  widget.Unprepare();
}


PixelRect
WidgetDialog::GetFooterRect()
{
  PixelRect rc;
  rc.left = 0;
  rc.right = GetWidth();
  rc.top = GetHeight() - dialog_footer.GetHeight();
  rc.bottom = GetHeight();
  return rc;
}

void
WidgetDialog::OnResize(UPixelScalar width, UPixelScalar height)
{
  WndForm::OnResize(width, height);

  if (auto_size)
    return;

  widget.Move(buttons.UpdateLayout());
  dialog_footer.Move(0, height - dialog_footer.GetHeight());
}

bool
DefaultWidgetDialog(const TCHAR *caption, const PixelRect &rc, Widget &widget)
{
  WidgetDialog dialog(caption, rc, &widget);
  dialog.AddButton(_("OK"), mrOK);
  dialog.AddButton(_("Cancel"), mrCancel);

  dialog.ShowModal();

  /* the caller manages the Widget */
  dialog.StealWidget();

  return dialog.GetChanged();
}

bool
DefaultWidgetDialog(const TCHAR *caption, Widget &widget)
{
  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();

  WidgetDialog dialog(caption, rc, &widget);
  dialog.AddButton(_("OK"), mrOK);
  dialog.AddButton(_("Cancel"), mrCancel);

  dialog.ShowModal();

  /* the caller manages the Widget */
  dialog.StealWidget();

  return dialog.GetChanged();
}
