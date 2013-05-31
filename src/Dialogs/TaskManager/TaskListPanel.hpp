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

#ifndef XCSOAR_TASK_LIST_PANEL_HPP
#define XCSOAR_TASK_LIST_PANEL_HPP

#include "Form/XMLWidget.hpp"
#include "Form/List.hpp"

class TabBarControl;
class WndButton;
class WndOwnerDrawFrame;
class TabbedControl;
class Canvas;
class OrderedTask;
class TaskStore;
class WndForm;

class TaskListPanel : public XMLWidget, private ListControl::Handler {
  /**
   * a pointer to the tab bar if run in a tabbed environment
   */
  TabBarControl *tab_bar;

  OrderedTask **active_task;
  bool *task_modified;

  TaskStore *task_store;

  bool lazy_loaded; // if store has been loaded first time tab displayed

  /**
   * Showing all task files?  (including *.igc, *.cup)
   */
  bool more;

  ListControl *wTasks;
  WndButton *more_button;
  WndButton *cancel_button;
  WndOwnerDrawFrame* wTaskView;

  /**
   * an instance to a parent form.  This form is closed by the Load function
   * if it is not nullptr
   * If the form is nullptr, it's assume the widget is shown in a tabbed window
   */
  WndForm *parent_form;

public:
  TaskListPanel(TabBarControl *_tab_bar,
                OrderedTask **_active_task, bool *_task_modified)
    :tab_bar(_tab_bar),
     active_task(_active_task), task_modified(_task_modified),
     more(false),
     wTaskView(NULL), parent_form(nullptr) {}

  void SetTaskView(WndOwnerDrawFrame *_task_view) {
    assert(wTaskView == NULL);
    assert(_task_view != NULL);

    wTaskView = _task_view;
  }

  void RefreshView();
  void DirtyList();

  void SaveTask();
  void LoadTask();
  void DeleteTask();
  void RenameTask();

  /**
   * closes the parent form when this is not run in a tabbed environment
   */
  void CancelForm();

  void OnTaskPaint(WndOwnerDrawFrame *Sender, Canvas &canvas);

  void OnMoreClicked();

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();
  virtual void Show(const PixelRect &rc);
  virtual void Hide();

  /**
   * set the tatic parent form
   * and indicates the widget is being shown in a form
   * instead of in a tabbed window
   * also add a cancel button
   */
  void SetParentForm(WndForm *_parent_form);

protected:
  OrderedTask *get_cursor_task();

  gcc_pure
  const TCHAR *get_cursor_name();

private:
  /* virtual methods from class ListControl::Handler */
  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned idx) gcc_override;

  virtual void OnCursorMoved(unsigned index) gcc_override {
    RefreshView();
  }

  virtual bool CanActivateItem(unsigned index) const gcc_override {
      return true;
  }

  virtual void OnActivateItem(unsigned index) gcc_override {
    LoadTask();
  }
};

#endif
