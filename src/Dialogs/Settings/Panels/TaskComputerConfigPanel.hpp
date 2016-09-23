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

#ifndef TASK_COMPUTER_CONFIG_PANEL_HPP
#define TASK_COMPUTER_CONFIG_PANEL_HPP

#include "Widget/RowFormWidget.hpp"
#include "Form/ActionListener.hpp"
#include "Form/DataField/Listener.hpp"
#include "Blackboard/BlackboardListener.hpp"
#include "Form/Form.hpp"
#include "Task/TaskBehaviour.hpp"
#include "Event/Timer.hpp"


class TaskComputerConfigPanel final
  : public RowFormWidget, DataFieldListener, Timer {
private:
  // have any of the parameters changed
  bool changed;
public:
  TaskComputerConfigPanel();

protected:
  void UpdateVisibility(TaskBehaviour::TaskPlanningSpeedMode mode);
  void UpdateValues();

  /** Inherited from class Timer */
  void OnTimer() override;

public:
  /* methods from Widget */
  void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  bool Save(bool &changed) override;
  void Hide() override;
  void Show(const PixelRect &rc) override;

private:
  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df) override;
};

Widget *
CreateTaskComputerConfigPanel();

#endif
