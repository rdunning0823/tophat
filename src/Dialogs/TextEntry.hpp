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

#ifndef DIALOGS_TEXT_ENTRY_HPP
#define DIALOGS_TEXT_ENTRY_HPP

#include "Util/StringBuffer.hxx"
#include "Math/fixed.hpp"

#include <functional>
#include <tchar.h>

class WndProperty;

typedef std::function<const TCHAR *(const TCHAR *)> AllowedCharacters;
typedef void (*HelpCallback)();


bool
TextEntryDialog(TCHAR *text, size_t size,
                const TCHAR *caption=nullptr,
                AllowedCharacters ac=AllowedCharacters(),
                bool show_shift_key = true,
                bool default_shift_state = true);

template<size_t N>
static inline bool
TextEntryDialog(StringBuffer<TCHAR, N> &text,
                const TCHAR *caption=NULL,
                AllowedCharacters accb=AllowedCharacters(),
                bool default_shift_state = true)
{
  return TextEntryDialog(text.data(), text.capacity(),
                         caption, accb, default_shift_state);
}

template<size_t N>
static inline bool
TextEntryDialog(StringBuffer<TCHAR, N> &text,
                const TCHAR *caption,
                bool default_shift_state)
{
  AllowedCharacters accb=AllowedCharacters();
  return TextEntryDialog(text.data(), text.capacity(),
                         caption, accb, true, default_shift_state);
}

void
KnobTextEntry(TCHAR *text, size_t width,
              const TCHAR *caption);

bool
TouchTextEntry(TCHAR *text, size_t size,
               const TCHAR *caption=nullptr,
               AllowedCharacters ac=AllowedCharacters(),
               bool show_shift_keyy = true,
               bool default_shift_state = true);

/**
 * show a numeric keypad with Help if available
 * @param value containing value before and after edit
 * @param caption caption of field for editing
 * @param help text or nullptr if no help text
 * @param show minus sign
 * @param optional character filter
 */
bool
TouchNumericEntry(fixed &value,
                  const TCHAR *caption,
                  const TCHAR *help_text,
                  bool show_minus,
                  AllowedCharacters accb=AllowedCharacters());

#endif
