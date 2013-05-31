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

#ifndef XCSOAR_TASK_EDIT_PANEL_HPP
#define XCSOAR_TASK_EDIT_PANEL_HPP

#include "Form/XMLWidget.hpp"
#include "Form/List.hpp"

class WndForm;
struct DialogLook;
struct TaskLook;
struct AirspaceLook;
class OrderedTask;
class ListControl;
class WndOwnerDrawFrame;
class WndFrame;
class Canvas;

class TaskEditPanel : public XMLWidget, private ListControl::Handler {
  WndForm &wf;

  const TaskLook &task_look;
  const AirspaceLook &airspace_look;

  OrderedTask **ordered_task_pointer, *ordered_task;
  bool *task_modified;

  ListControl *wTaskPoints;
  WndOwnerDrawFrame *wTaskView;
  WndFrame *wSummary;

public:
  TaskEditPanel(WndForm &_wf,
                const TaskLook &_task_look, const AirspaceLook &_airspace_look,
                OrderedTask **_active_task, bool *_task_modified)
    :wf(_wf),
     task_look(_task_look), airspace_look(_airspace_look),
     ordered_task_pointer(_active_task), task_modified(_task_modified),
     wTaskView(NULL) {}

  void SetTaskView(WndOwnerDrawFrame *_task_view) {
    assert(wTaskView == NULL);
    assert(_task_view != NULL);

    wTaskView = _task_view;
  }

  void UpdateButtons();

  void MoveUp();
  void MoveDown();

  void OnClearAllClicked();
  void OnEditTurnpointClicked();
  void EditTaskPoint(unsigned ItemIndex);
  void OnMakeFinish();

  bool OnKeyDown(unsigned key_code);

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void ReClick();
  virtual void Show(const PixelRect &rc);
  virtual void Hide();

protected:
  void RefreshView();

private:
  /* virtual methods from List::Handler */
  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned idx);
  virtual void OnCursorMoved(unsigned index);
  virtual bool CanActivateItem(unsigned index) const;
  virtual void OnActivateItem(unsigned index);
};

#endif
