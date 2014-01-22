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

#include "Dialogs/TextEntry.hpp"
#include "Look/DialogLook.hpp"
#include "Form/Form.hpp"
#include "Form/Button.hpp"
#include "Form/Edit.hpp"
#include "Widget/KeyboardWidget.hpp"
#include "Widget/KeyboardNumericWidget.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Key.h"
#include "Util/StringUtil.hpp"
#include "Util/NumberParser.hpp"
#include "UIGlobals.hpp"
#include "Language/Language.hpp"
#include "Dialogs/HelpDialog.hpp"

#include <algorithm>
#include <assert.h>

static WndProperty *editor;
static KeyboardWidget *kb = nullptr;
static KeyboardNumericWidget *knb = nullptr;

static AllowedCharacters AllowedCharactersCallback;

static constexpr size_t MAX_TEXTENTRY = 40;
static unsigned int cursor = 0;
static size_t max_width;
static TCHAR edittext[MAX_TEXTENTRY];
static const TCHAR* caption;
static const TCHAR* help_text;

/**
 * has the value been edited by the user
 */
bool has_been_edited;

static void ClearText();

/**
 * clears the field and marks the field as having been modified
 */
static void
ClearTextFirstTime()
{
  ClearText();
  has_been_edited = true;
}

static void
UpdateAllowedCharacters()
{
  if (AllowedCharactersCallback && kb != nullptr)
    kb->SetAllowedCharacters(AllowedCharactersCallback(edittext));
  else if (AllowedCharactersCallback && knb != nullptr)
    knb->SetAllowedCharacters(AllowedCharactersCallback(edittext));
}

static void
UpdateTextboxProp()
{
  editor->SetHighlight(!has_been_edited);
  editor->SetText(edittext);

  UpdateAllowedCharacters();
}

static bool
DoBackspace()
{
  if (!has_been_edited)
    ClearTextFirstTime();

  if (cursor < 1)
    return false;

  cursor--;
  edittext[cursor] = 0;
  UpdateTextboxProp();
  return true;
}

static void
OnBackspace()
{
  DoBackspace();
}

static bool
DoCharacter(TCHAR character)
{
  if (!has_been_edited)
    ClearTextFirstTime();

  if (cursor >= max_width - 1)
    return false;

  edittext[cursor++] = character;
  edittext[cursor] = 0;
  UpdateTextboxProp();
  return true;
}

static bool
FormKeyDown(unsigned key_code)
{
  switch (key_code) {
  case KEY_RIGHT:
    return true;
  case KEY_LEFT:
  case KEY_BACK:
    DoBackspace();
    return true;
  }

  return false;
}

static bool
FormCharacter(unsigned ch)
{
  if (ch < 0x20)
    return false;

#ifndef _UNICODE
  if (ch >= 0x80)
    /* TODO: ASCII only for now, because we don't have proper UTF-8
       support yet */
    return false;
#endif

  DoCharacter((TCHAR)ch);
  return true;
}

static void
ClearText()
{
  cursor = 0;
  edittext[0] = 0;
  UpdateTextboxProp();
}

static void
ShowHelp()
{
  HelpDialog(caption, help_text);
}

bool
TouchTextEntry(TCHAR *text, size_t width,
               const TCHAR *caption,
               AllowedCharacters accb,
               bool default_shift_state)
{
  if (width == 0)
    width = MAX_TEXTENTRY;

  has_been_edited = true; // disable clearing of value on first key for alpha

  max_width = std::min(MAX_TEXTENTRY, width);

  const DialogLook &look = UIGlobals::GetDialogLook();
  WndForm form(look);
  form.Create(UIGlobals::GetMainWindow(), caption);
  form.SetKeyDownFunction(FormKeyDown);
  form.SetCharacterFunction(FormCharacter);

  ContainerWindow &client_area = form.GetClientAreaWindow();
  const PixelRect rc = client_area.GetClientRect();

  const PixelScalar client_height = rc.bottom - rc.top;

  const PixelScalar padding = Layout::Scale(2);
  const PixelScalar backspace_width = Layout::Scale(36);
  const PixelScalar backspace_left = rc.right - padding - backspace_width;
  const PixelScalar editor_height = Layout::Scale(22);
  const PixelScalar editor_bottom = padding + editor_height;
  const PixelScalar button_height = Layout::Scale(40);
  constexpr unsigned keyboard_rows = 5;
  const PixelScalar keyboard_top = editor_bottom + padding;
  const PixelScalar keyboard_height = keyboard_rows * button_height;
  const PixelScalar keyboard_bottom = keyboard_top + keyboard_height;

  const bool vertical = client_height >= keyboard_bottom + button_height;

  const PixelScalar button_top = vertical
    ? rc.bottom - button_height
    : keyboard_bottom - button_height;
  const PixelScalar button_bottom = vertical
    ? rc.bottom
    : keyboard_bottom;

  const PixelScalar ok_left = vertical ? 0 : padding;
  const PixelScalar ok_right = vertical
    ? rc.right / 3
    : ok_left + Layout::Scale(80);

  const PixelScalar cancel_left = vertical
    ? ok_right
    : Layout::Scale(175);
  const PixelScalar cancel_right = vertical
    ? rc.right * 2 / 3
    : cancel_left + Layout::Scale(60);

  const PixelScalar clear_left = vertical
    ? cancel_right
    : Layout::Scale(235);
  const PixelScalar clear_right = vertical
    ? rc.right
    : clear_left + Layout::Scale(50);

  WndProperty _editor(client_area, look, _T(""),
                      { 0, padding, backspace_left - padding, editor_bottom },
                      0, WindowStyle());
  _editor.SetReadOnly();
  editor = &_editor;

  ButtonWindowStyle button_style;
  button_style.TabStop();

  WndButton ok_button(client_area, look.button, _("OK"),
                      { ok_left, button_top, ok_right, button_bottom },
                      button_style, form, mrOK);

  WndButton cancel_button(client_area, look.button, _("Cancel"),
                          { cancel_left, button_top,
                              cancel_right, button_bottom },
                          button_style, form, mrCancel);

  WndButton clear_button(client_area, look.button, _("Clear"),
                         { clear_left, button_top,
                             clear_right, button_bottom },
                         button_style, ClearText);

  KeyboardWidget keyboard(look.button, FormCharacter, !accb,
                          default_shift_state);

  const PixelRect keyboard_rc = {
    padding, keyboard_top,
    rc.right - padding, keyboard_bottom
  };

  keyboard.Initialise(client_area, keyboard_rc);
  keyboard.Prepare(client_area, keyboard_rc);
  keyboard.Show(keyboard_rc);

  kb = &keyboard;
  knb = nullptr;

  WndButton backspace_button(client_area, look.button, _T("<-"),
                             { backspace_left, padding, rc.right - padding,
                                 editor_bottom },
                             button_style, OnBackspace);

  AllowedCharactersCallback = accb;

  cursor = 0;
  ClearText();

  if (!StringIsEmpty(text)) {
    CopyString(edittext, text, width);
    cursor = _tcslen(text);
  }

  UpdateTextboxProp();
  bool result = form.ShowModal() == mrOK;

  keyboard.Hide();
  keyboard.Unprepare();

  if (result) {
    CopyString(text, edittext, width);
  }

  return result;
}

bool
TouchNumericEntry(fixed &value,
                  const TCHAR *_caption,
                  const TCHAR *_help_text,
                  bool show_minus,
                  AllowedCharacters accb)
{
  caption = _caption;
  help_text = _help_text;
  has_been_edited = false;

  StaticString<12> buffer;
  buffer.Format(_T("%.1f"), (double)value);

  const TCHAR zero = '0';
  const TCHAR point = '.';

  // remove trailing zeros
  unsigned decimal_location = 0;
  for (unsigned i = 0; i < buffer.length(); ++i)
    if (buffer[i] == point) {
      decimal_location = i;
      break;
    }
  for (unsigned i = buffer.length(); i > decimal_location; --i)
    if (buffer[i] == zero)
      buffer.Truncate(i - 1);

  max_width = MAX_TEXTENTRY;

  const DialogLook &look = UIGlobals::GetDialogLook();
  WndForm form(look);
  form.Create(UIGlobals::GetMainWindow(), caption);
  form.SetKeyDownFunction(FormKeyDown);
  form.SetCharacterFunction(FormCharacter);

  ContainerWindow &client_area = form.GetClientAreaWindow();
  const PixelRect rc = client_area.GetClientRect();

  const PixelScalar client_height = rc.bottom - rc.top;

  const PixelScalar padding = Layout::Scale(2);
  const PixelScalar backspace_width = (rc.right - rc.left) / 4;
  const PixelScalar backspace_left = rc.right - padding - backspace_width;

  const PixelScalar button_height = (client_height - padding * 2) / 6;

  const PixelScalar keyboard_top = rc.top + padding + button_height;
  const PixelScalar keyboard_bottom = rc.bottom - button_height - padding;


  WndProperty _editor(client_area, look, _T(""),
                      { 0, padding, backspace_left - padding, rc.top + button_height },
                      0, WindowStyle());
  _editor.SetReadOnly();
  editor = &_editor;

  ButtonWindowStyle button_style;
  button_style.TabStop();

  PixelScalar button_width = (rc.right - rc.left) / 3;
  WndButton ok_button(client_area, look.button, _("OK"),
                      { 0, rc.top + 5 * button_height, button_width,
                        rc.top + 6 * button_height},
                      button_style, form, mrOK);

  WndButton cancel_button(client_area, look.button, _("Cancel"),
                          { button_width,
                            rc.top + 5 * button_height, 2 * button_width,
                            rc.top + 6 * button_height},
                          button_style, form, mrCancel);

  WndButton help_button(client_area, look.button, _("Help"),
                          { button_width * 2,
                            rc.top + 5 * button_height, 3 * button_width,
                            rc.top + 6 * button_height},
                          button_style, ShowHelp);

  WndButton clear_button(client_area, look.button, _("Clear"),
                         { button_width * 2,
                           rc.top + 5 * button_height, 3 * button_width,
                           rc.top + 6 * button_height},
                         button_style, ClearText);

  help_button.SetVisible(help_text != nullptr);
  clear_button.SetVisible(help_text == nullptr);

  KeyboardNumericWidget keyboard(look.button, FormCharacter, show_minus);

  const PixelRect keyboard_rc = {
    padding, keyboard_top,
    rc.right - padding, keyboard_bottom
  };

  keyboard.Initialise(client_area, keyboard_rc);
  keyboard.Prepare(client_area, keyboard_rc);
  keyboard.Show(keyboard_rc);

  kb = nullptr;
  knb = &keyboard;

  WndButton backspace_button(client_area, look.button, _T("<-"),
                             { backspace_left, padding, rc.right - padding,
                               rc.top + button_height },
                             button_style, OnBackspace);

  AllowedCharactersCallback = accb;

  ClearText();

  if (buffer.length() == 1 && buffer[0u] == zero)
    buffer.clear();

  if (!buffer.empty()) {
    CopyString(edittext, buffer.c_str(), max_width);
    cursor = buffer.length();
  }

  UpdateTextboxProp();
  bool result = form.ShowModal() == mrOK;

  if (result) {
    value = fixed(ParseDouble(edittext));
  }

  return result;
}
