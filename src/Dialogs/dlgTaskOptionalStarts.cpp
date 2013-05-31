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

#include "Dialogs/Task.hpp"
#include "Dialogs/Waypoint.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/XML.hpp"
#include "Form/Form.hpp"
#include "Form/List.hpp"
#include "Form/Button.hpp"
#include "Screen/Layout.hpp"
#include "LocalPath.hpp"
#include "Task/Factory/AbstractTaskFactory.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Engine/Task/Ordered/Points/OrderedTaskPoint.hpp"
#include "Dialogs/dlgTaskHelpers.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"

#include <assert.h>
#include <stdio.h>

static WndForm *wf = NULL;
static ListControl* wOptionalStartPoints = NULL;
static bool task_modified = false;
static OrderedTask* ordered_task = NULL;
static bool RealStartExists = false;

static void
OnCloseClicked(gcc_unused WndButton &Sender)
{
  wf->SetModalResult(mrOK);
}

static void
RefreshView()
{
  wOptionalStartPoints->SetLength(ordered_task->GetOptionalStartPointCount()
      + (RealStartExists ? 2 : 1));

  wOptionalStartPoints->Invalidate();
}

static void
OnOptionalStartPaintListItem(Canvas &canvas, const PixelRect rc,
                             unsigned DrawListIndex)
{
  assert(DrawListIndex <= ordered_task->GetOptionalStartPointCount()
      +  (RealStartExists ? 2 : 1));
  assert(wOptionalStartPoints->GetLength() ==
         ordered_task->GetOptionalStartPointCount() + (RealStartExists ? 2 : 1));

  const unsigned index_optional_starts = DrawListIndex - (RealStartExists ? 1 : 0);

  if (DrawListIndex == wOptionalStartPoints->GetLength() - 1) {
    canvas.text(rc.left + Layout::FastScale(2),
                rc.top + Layout::FastScale(2), _("(Add Alternate Start)"));
  } else {
    RasterPoint pt = { PixelScalar(rc.left + Layout::FastScale(2)),
                       PixelScalar(rc.top + Layout::FastScale(2)) };

    const OrderedTaskPoint *tp;
    if (DrawListIndex == 0 && RealStartExists) {
      tp = &ordered_task->GetPoint(0);
      canvas.text(pt.x, pt.y, _T("*"));
      pt.x += canvas.CalcTextWidth(_T("*"));
    } else
      tp = &ordered_task->GetOptionalStartPoint(index_optional_starts);

    assert(tp != NULL);

    canvas.text(pt.x, pt.y, tp->GetWaypoint().name.c_str());
  }
}


static void
OnOptionalStartListEnter(unsigned ItemIndex)
{
  assert(ItemIndex <= ordered_task->GetOptionalStartPointCount()
      +  (RealStartExists ? 2 : 1));
  assert(wOptionalStartPoints->GetLength() ==
         ordered_task->GetOptionalStartPointCount() + (RealStartExists ? 2 : 1));

  if (ItemIndex == 0 && RealStartExists)
    return;

  const unsigned index_optional_starts = ItemIndex - (RealStartExists ? 1 : 0);

  if (index_optional_starts < ordered_task->GetOptionalStartPointCount()) {
    const GeoPoint &location = ordered_task->TaskSize() > 0
      ? ordered_task->GetPoint(0).GetLocation()
      : XCSoarInterface::Basic().location;
    const Waypoint* way_point = ShowWaypointListDialog(wf->GetMainWindow(),
                                                       location);
    if (!way_point)
      return;

    if (ordered_task->RelocateOptionalStart(index_optional_starts, *way_point))
      task_modified = true;

  } else if (!ordered_task->IsFull()) {

    const GeoPoint &location = ordered_task->TaskSize() > 0
      ? ordered_task->GetPoint(0).GetLocation()
      : XCSoarInterface::Basic().location;
    const Waypoint* way_point =
      ShowWaypointListDialog(wf->GetMainWindow(), location);
    if (!way_point)
      return;

    AbstractTaskFactory &factory = ordered_task->GetFactory();
    if (factory.AppendOptionalStart(*way_point)) {
      task_modified = true;
    }
  }
  RefreshView();
}

static void
OnRelocateClicked(gcc_unused WndButton &Sender)
{
  OnOptionalStartListEnter(wOptionalStartPoints->GetCursorIndex());
  RefreshView();
}

static void
OnRemoveClicked(gcc_unused WndButton &Sender)
{
  const unsigned index_optional_starts = wOptionalStartPoints->GetCursorIndex()
      - (RealStartExists ? 1 : 0);
  if (ordered_task->RemoveOptionalStart(index_optional_starts)) {
    RefreshView();
    task_modified = true;
  }
}

static constexpr CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnRemoveClicked),
  DeclareCallBackEntry(OnRelocateClicked),
  DeclareCallBackEntry(NULL)
};

bool
dlgTaskOptionalStarts(SingleWindow &parent, OrderedTask** task)
{
  ordered_task = *task;
  task_modified = false;

  if (Layout::landscape)
    wf = LoadDialog(CallBackTable, parent, _T("IDR_XML_TASKOPTIONALSTARTS_L"));
  else
    wf = LoadDialog(CallBackTable, parent, _T("IDR_XML_TASKOPTIONALSTARTS"));

  if (!wf)
    return false;

  assert(wf != NULL);

  wOptionalStartPoints = (ListControl*)wf->FindByName(_T("frmOptionalStarts"));
  assert(wOptionalStartPoints != NULL);

  RealStartExists = ordered_task->TaskSize();

  wOptionalStartPoints->SetActivateCallback(OnOptionalStartListEnter);
  wOptionalStartPoints->SetPaintItemCallback(OnOptionalStartPaintListItem);

  RefreshView();

  wf->ShowModal();

  delete wf;
  wf = NULL;

  return task_modified;
}
