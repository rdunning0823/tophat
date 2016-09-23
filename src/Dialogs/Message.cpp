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

#include "Dialogs/Message.hpp"
#include "Language/Language.hpp"
#include "Form/Button.hpp"
#include "Form/Form.hpp"
#include "Form/Frame.hpp"
#include "Look/DialogLook.hpp"
#include "Screen/Layout.hpp"
#include "Screen/SingleWindow.hpp"
#include "Util/StaticArray.hpp"
#include "UIGlobals.hpp"
#include "Screen/Font.hpp"

#include <assert.h>
#include <limits.h>

int
ShowMessageBox(const TCHAR *text, const TCHAR *caption, unsigned flags)
{
  assert(text != NULL);

  SingleWindow &main_window = UIGlobals::GetMainWindow();

  const unsigned dialog_width = Layout::Scale(220);
  unsigned dialog_height = Layout::Scale(160);

  const unsigned button_width = Layout::Scale(60u);
  const unsigned button_height = Layout::Scale(32u);

  // Create dialog
  WindowStyle style;
  style.Hide();
  style.ControlParent();

  const DialogLook &dialog_look = UIGlobals::GetDialogLook();

  PixelRect form_rc;
  form_rc.left = 0;
  form_rc.top = 0;
  form_rc.right = dialog_width;
  form_rc.bottom = dialog_height;

  WndForm wf(main_window, dialog_look, form_rc, caption, style);

  ContainerWindow &client_area = wf.GetClientAreaWindow();

  // add this margin at top and bottom of the window and above buttons
  UPixelScalar top_bottom_margin = dialog_look.text_font.TextSize(_T("A")).cy;

  // Create text element
  PixelRect text_rc = form_rc;
  text_rc.top += top_bottom_margin;
  WndFrame *text_frame = new WndFrame(client_area, dialog_look, text_rc);

  text_frame->SetCaption(text);
  text_frame->SetAlignCenter();

  const unsigned text_height = text_frame->GetTextHeight();
  text_frame->Resize(dialog_width, text_height + Layout::GetTextPadding());

  const PixelSize root_size = main_window.GetSize();

  dialog_height = wf.GetTitleHeight() + Layout::Scale(10) + text_height +
      button_height + 3 * top_bottom_margin;
  const int  dialog_x = (root_size.cx - dialog_width) / 2;
  const int  dialog_y = (root_size.cy - dialog_height) / 2;
  wf.Move(dialog_x, dialog_y, dialog_width, dialog_height);

  PixelRect button_rc;
  button_rc.left = 0;
  button_rc.top = Layout::Scale(6) + text_height + 2 * top_bottom_margin;
  button_rc.right = button_rc.left + button_width;
  button_rc.bottom = button_rc.top + button_height;

  // Create buttons
  WindowStyle button_style;
  button_style.TabStop();

  StaticArray<Button *, 10> buttons;

  unsigned button_flags = flags & 0x000f;
  if (button_flags == MB_OK ||
      button_flags == MB_OKCANCEL)
    buttons.append() =
      new WndSymbolButton(client_area, dialog_look.button, _T("_X"), button_rc,
                          button_style, wf, IDOK);


  if (button_flags == MB_YESNO ||
      button_flags == MB_YESNOCANCEL ||
      button_flags == MB_YESNOALL) {
    buttons.append() =
      new Button(client_area, dialog_look.button, _("Yes"), button_rc,
                 button_style, wf, IDYES);

    if (button_flags == MB_YESNOALL) {
      buttons.append() =
          new Button(client_area, dialog_look.button, _("All"), button_rc,
                        button_style, wf, IDALL);
    }

    buttons.append() =
      new Button(client_area, dialog_look.button, _("No"), button_rc,
                 button_style, wf, IDNO);
  }

  if (button_flags == MB_ABORTRETRYIGNORE ||
      button_flags == MB_RETRYCANCEL)
    buttons.append() =
      new Button(client_area, dialog_look.button, _("Retry"), button_rc,
                 button_style, wf, IDRETRY);

  if (button_flags == MB_OKCANCEL ||
      button_flags == MB_RETRYCANCEL ||
      button_flags == MB_YESNOCANCEL)
    buttons.append() =
      new Button(client_area, dialog_look.button, _("Cancel"), button_rc,
                 button_style, wf, IDCANCEL);

  if (button_flags == MB_ABORTRETRYIGNORE) {
    buttons.append() =
      new Button(client_area, dialog_look.button, _("Abort"), button_rc,
                 button_style, wf, IDABORT);

    buttons.append() =
      new Button(client_area, dialog_look.button, _("Ignore"), button_rc,
                 button_style, wf, IDIGNORE);
  }

  const unsigned max_button_width = dialog_width / buttons.size();
  int button_x = max_button_width / 2 - button_width / 2;

  // Move buttons to the right positions
  for (unsigned i = 0; i < buttons.size(); i++) {
    buttons[i]->Move(button_x, button_rc.top);
    button_x += max_button_width;
  }

  // Show MessageBox and save result
  unsigned res = wf.ShowModal();

  delete text_frame;
  for (unsigned i = 0; i < buttons.size(); ++i)
    delete buttons[i];

  wf.Destroy();

  return res;
}
