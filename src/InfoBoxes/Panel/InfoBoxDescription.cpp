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

#include "InfoBoxDescription.hpp"
#include "Form/Frame.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Language/Language.hpp"
#include "Screen/Layout.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "Interface.hpp"


void
InfoBoxDescriptionPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);

  const DialogLook &look = UIGlobals::GetDialogLook();
  WindowStyle style_frame;
  PixelRect frame_rc = content_rc;
  frame_rc.right = content_rc.right - Layout::Scale(10);
  frame_rc.left = content_rc.left + Layout::Scale(10);
  frame_rc.top = content_rc.top + Layout::Scale(10);
  description_text = new WndFrame(GetClientAreaWindow(), look,
                                  frame_rc.left, frame_rc.top,
                                  frame_rc.right - frame_rc.left,
                                  frame_rc.bottom - frame_rc.top,
                                  style_frame);

  InfoBoxSettings &settings = CommonInterface::SetUISettings().info_boxes;
  const unsigned panel_index = InfoBoxManager::GetCurrentPanel();
  InfoBoxSettings::Panel &panel = settings.panels[panel_index];
  const InfoBoxFactory::Type old_type = panel.contents[id];

  const TCHAR *desc = InfoBoxFactory::GetDescription(old_type);
  if (desc != NULL)
    description_text->SetText(gettext(desc));
}
