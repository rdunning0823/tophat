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

#include "CheckBoxWidget.hpp"
#include "Form/CheckBox.hpp"
#include "Renderer/TextButtonRenderer.hpp"
#include "Screen/Layout.hpp"
#include "Screen/ContainerWindow.hpp"

CheckBoxWidget::CheckBoxWidget(const DialogLook &_look, const TCHAR *_caption,
                               ActionListener &_listener, int _id)
  :look(_look), listener(_listener), caption(_caption),
   id(_id), cb(nullptr) {}

CheckBoxWidget::~CheckBoxWidget()
{
  if (IsDefined())
    DeleteWindow();
}

void
CheckBoxWidget::Invalidate()
{
  assert(IsDefined());

  ((CheckBoxControl &)GetWindow()).Invalidate();
}

PixelSize
CheckBoxWidget::GetMinimumSize() const
{
  return PixelSize(Layout::GetMaximumControlHeight(),
                   Layout::GetMaximumControlHeight());
}

PixelSize
CheckBoxWidget::GetMaximumSize() const
{
  return PixelSize(Layout::GetMaximumControlHeight() * 2,
                   Layout::GetMaximumControlHeight());
}

void
CheckBoxWidget::Initialise(ContainerWindow &parent, const PixelRect &rc)
{
  WindowStyle style;
  style.Hide();
  style.TabStop();

  cb = new CheckBoxControl();
  cb->Create(parent, look, caption, parent.GetClientRect(), style,
             listener, id);
  SetWindow(cb);
}

bool
CheckBoxWidget::SetFocus()
{
  GetWindow().SetFocus();
  return true;
}

void
CheckBoxWidget::SetState(bool state)
{
  ((CheckBoxControl&)GetWindow()).SetState(state);
}

bool
CheckBoxWidget::GetState()
{
  return ((CheckBoxControl&)GetWindow()).GetState();
}

