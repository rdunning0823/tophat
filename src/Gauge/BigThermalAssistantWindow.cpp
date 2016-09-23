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

#include "BigThermalAssistantWindow.hpp"
#include "Input/InputEvents.hpp"
#include "Screen/Layout.hpp"
#include "Asset.hpp"
#include "LogFile.hpp" //debug

bool
BigThermalAssistantWindow::OnMouseDown(PixelScalar x, PixelScalar y)
{
  if (!dragging) {
    dragging = true;
    SetCapture();
    gestures.Start(x, y, Layout::Scale(20));
  }

  return true;
}

bool
BigThermalAssistantWindow::OnMouseUp(PixelScalar x, PixelScalar y)
{
  InputEvents::HideMenu();
  if (dragging) {
    StopDragging();

    const TCHAR *gesture = gestures.Finish();
    if (gesture && InputEvents::processGesture(gesture))
      return true;
  }

  return false;
}

bool
BigThermalAssistantWindow::OnMouseMove(PixelScalar x, PixelScalar y,
                                       gcc_unused unsigned keys)
{
  if (!dragging && HasTouchScreen())
    OnMouseDown(x, y);

  if (dragging)
    gestures.Update(x, y);

  return true;
}

void
BigThermalAssistantWindow::OnCancelMode()
{
  ThermalAssistantWindow::OnCancelMode();
  StopDragging();
}

bool
BigThermalAssistantWindow::OnKeyDown(unsigned key_code)
{
  return InputEvents::processKey(key_code);
}
