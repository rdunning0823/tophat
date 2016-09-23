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

#include "QuestionWidget.hpp"
#include "ButtonPanelWidget.hpp"
#include "TextWidget.hpp"
#include "Form/ButtonPanel.hpp"

QuestionWidget::QuestionWidget(const TCHAR *_message,
                               ActionListener &_listener)
  :SolidWidget(new ButtonPanelWidget(new TextWidget(2),
                                     ButtonPanelWidget::Alignment::BOTTOM)),
   message(_message), listener(_listener) {}

QuestionWidget::QuestionWidget(const TCHAR *_message,
                               ActionListener &_listener, unsigned num_rows_text)
  :SolidWidget(new ButtonPanelWidget(new TextWidget(num_rows_text),
                                     ButtonPanelWidget::Alignment::BOTTOM)),
   message(_message), listener(_listener) {}

void
QuestionWidget::UpdateMessage(const TCHAR *_message)
{
  message = _message;

  ButtonPanelWidget &bpw = (ButtonPanelWidget &)GetWidget();
  ((TextWidget &)bpw.GetWidget()).SetText(message);
}

void
QuestionWidget::SetMessage(const TCHAR *_message)
{
  auto &bpw = (ButtonPanelWidget &)GetWidget();
  auto &tw = (TextWidget &)bpw.GetWidget();
  tw.SetText(_message);
}

void
QuestionWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  SolidWidget::Prepare(parent, rc);

  ButtonPanelWidget &bpw = (ButtonPanelWidget &)GetWidget();

  ((TextWidget &)bpw.GetWidget()).SetText(message);

  ButtonPanel &panel = bpw.GetButtonPanel();

  for (auto button : buttons)
    panel.Add(button.caption, listener, button.id);
}


