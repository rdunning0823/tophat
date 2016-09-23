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

#include "InfoBoxDescription.hpp"
#include "Form/Frame.hpp"
#include "UIGlobals.hpp"
#include "UIState.hpp"
#include "Look/DialogLook.hpp"
#include "Language/Language.hpp"
#include "Screen/Layout.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "InfoBoxes/Content/Factory.hpp"
#include "Interface.hpp"
#include "Screen/SingleWindow.hpp"

void
InfoBoxDescriptionPanel::Move(const PixelRect &rc_unused)
{
  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();
  BaseAccessPanel::Move(rc);
  CalculateLayout(rc);
  description_text->Move(frame_rc);
}

void
InfoBoxDescriptionPanel::CalculateLayout(const PixelRect &rc)
{
  frame_rc = content_rc;
  frame_rc.right = content_rc.right - Layout::Scale(10);
  frame_rc.left = content_rc.left + Layout::Scale(10);
  frame_rc.top = content_rc.top + Layout::Scale(10);
}

void
InfoBoxDescriptionPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);
  CalculateLayout(rc);
  const DialogLook &look = UIGlobals::GetDialogLook();
  WindowStyle style_frame;
  description_text = new WndFrame(GetClientAreaWindow(), look,
                                  frame_rc, style_frame);

  InfoBoxSettings &settings = CommonInterface::SetUISettings().info_boxes;
  const unsigned panel_index = CommonInterface::GetUIState().panel_index;
  const InfoBoxSettings::Panel &panel = settings.panels[panel_index];
  const InfoBoxFactory::Type old_type = panel.contents[id];

  StaticString<512> buffer;
  buffer = gettext(InfoBoxFactory::GetDescription(old_type));
  description_text->SetText(buffer);
}
