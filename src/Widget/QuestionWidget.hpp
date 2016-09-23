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

#ifndef XCSOAR_QUESTION_WIDGET_HPP
#define XCSOAR_QUESTION_WIDGET_HPP

#include "SolidWidget.hpp"
#include "Util/StaticArray.hpp"

#include <tchar.h>

class ActionListener;

/**
 * A #Widget that displays a message and a number of buttons.  It is
 * used by XCSoar to display context-sensitive dialogs in the "bottom
 * area".
 */
class QuestionWidget : public SolidWidget {
  struct Button {
    const TCHAR *caption;
    int id;
  };

  const TCHAR *message;

  ActionListener &listener;

  StaticArray<Button, 8> buttons;

public:
  QuestionWidget(const TCHAR *_message, ActionListener &_listener);
  QuestionWidget(const TCHAR *_message, ActionListener &_listener,
                 unsigned num_rows_text);

  void SetMessage(const TCHAR *_message);

  void AddButton(const TCHAR *caption, int id) {
    buttons.append({caption, id});
  }

  /**
   * update the widget's message
   */
  void UpdateMessage(const TCHAR *_message);


  void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
};

#endif
