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

#ifndef XCSOAR_TASK_MANAGER_DIALOG_US_HPP
#define XCSOAR_TASK_MANAGER_DIALOG_US_HPP

#include "Form/Form.hpp"
#include "Screen/PaintWindow.hpp"
#include "Widget/Widget.hpp"

class OrderedTask;
class SingleWindow;
class WndOwnerDrawFrame;
class WndFrame;
class Button;
class Canvas;
struct DialogLook;

enum Controls {
  TASKVIEW,
  SAVE_AS,
  FLY,
  BACK,
};


void TaskManagerDialogUsShowModal();

class TaskManagerDialogUs : public NullWidget, public WndForm
{
  class TaskView : public PaintWindow
  {
    OrderedTask **ordered_task_pointer;
    bool fullscreen;
    PixelRect rc_full_screen, rc_partial_screen;

  public:
    TaskView(OrderedTask **_ordered_task_pointer);
    /**
     * draws the the Final Glide bar
     */
    virtual void OnPaint(Canvas &canvas);

    virtual bool OnMouseUp(PixelScalar x, PixelScalar y);

    /**
     * initializes the rectangle for full screen display
     */
    void SetFullScreenRect(PixelRect _rc_full_screen);

    /**
     * initializes the rectangle for partial screen display
     */
    void SetPartialScreenRect(PixelRect _rc_partial_screen);

    /**
     * restores to partial or full screen display based on fullscreen property
     */
    void Restore();
  };

protected:
  OrderedTask** active_task_pointer;
  OrderedTask* active_task;
  bool task_modified;
  bool fullscreen;

  PixelRect rc_task_view;
  PixelRect rc_fly_button, rc_save_as_button, rc_back_button;
  PixelRect rc_task_summary;

  Button *fly_button, *save_as_button, *back_button;
  WndFrame *task_summary;

public:
  TaskView task_view;

  TaskManagerDialogUs(const DialogLook &look,
                      OrderedTask **_active_task_pointer,
                      bool _task_modified);

  /**
   * Validates task and prompts if change or error
   * Commits task if no error
   * CheckGeometry() Must be called before calling this
   *   if any turnpoint locations have changed
   * @return True if task manager should close
   *         False if window should remain open
   */
  bool CommitTaskChanges();

  /**
   * restores task view rect
   */
  void TaskViewRestore(WndOwnerDrawFrame *wTaskView);
  //void ResetTaskView(WndOwnerDrawFrame *task_view);

  /**
   * toggles maximize or restore state of the TaskView frame
   */
  bool OnTaskViewClick(WndOwnerDrawFrame *Sender,
                              PixelScalar x, PixelScalar y);

  /**
   * saves the current task to a file
   */
  void SaveTask();

  /**
   * enables or disables buttons as appropriate
   */
  void UpdateButtons();

  /* virtual methods from Widget */
  void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  void Unprepare() override;
  void Show(const PixelRect &rc) override {};
  void Hide() override {};

  /* overrides from WndForm */
  void OnResize(PixelSize new_size) override;
  void ReinitialiseLayout(const PixelRect &parent_rc) override;

  bool Save(bool &changed) override;

  /**
   * prompts pilot to declare to any attached loggers
   */
  void Declare();
  /**
   * from ActionListener (WndForm)
   */
  virtual void OnAction(int id) override;

  /**
   * sets the caption on the dialog based on the task's name
   */
  void SetDialogCaption();

  /**
   * sets up rectangles for layout of screen
   * @param rc. rect of dialog
   */
  void SetRectangles(const PixelRect &rc);
};

#endif
