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

#include "NextWaypoint.hpp"
#include "Base.hpp"
#include "Dialogs/Waypoint/WaypointInfoWidget.hpp"
#include "Engine/Waypoint/Waypoint.hpp"
#include "UIGlobals.hpp"
#include "Form/Frame.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Components.hpp"
#include "Language/Language.hpp"

class NextWaypointPanel : public BaseAccessPanel {
protected:
  /**
   * the next waypoint
   */
  const Waypoint *waypoint;
public:

  NextWaypointPanel(unsigned _id, const Waypoint &_waypoint)
    :BaseAccessPanel(_id, new WaypointInfoWidget(UIGlobals::GetDialogLook(),
                                                 _waypoint)),
    waypoint(&_waypoint) {}

  virtual void SetCaption() override;
};

void
NextWaypointPanel::SetCaption()
{
  assert(waypoint != nullptr);
  StaticString<255> caption;
  caption.Format(_T("%s: %s"), _("Next waypoint"), waypoint->name.c_str());
  header_text->SetText(caption.get());
}

Widget *
LoadNextWaypointPanel(unsigned id)
{
  const Waypoint * wp = protected_task_manager->GetActiveWaypoint();
  if (wp == nullptr)
    return nullptr;

  return new NextWaypointPanel(id, *wp);
}
