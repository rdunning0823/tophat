/* Copyright_License {

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
#ifndef RT_TASK_FACTORY_HPP
#define RT_TASK_FACTORY_HPP

#include "AbstractTaskFactory.hpp"

/**
 * Factory for construction of legal FAI racing tasks
 */
class RTTaskFactory: 
  public AbstractTaskFactory 
{
public:
/** 
 * Constructor
 * 
 * @param _task Ordered task to be managed by this factory
 * @param tb Behaviour (options)
 */  
  RTTaskFactory(OrderedTask& _task,
                const TaskBehaviour &tb);

  RTTaskFactory(OrderedTask &_task, const TaskBehaviour &_behaviour,
                const LegalPointConstArray _start_types,
                const LegalPointConstArray _intermediate_types,
                const LegalPointConstArray _finish_types);

  ~RTTaskFactory() {};

/** 
 * Check whether task is complete and valid according to factory rules
 * 
 * @return True if task is valid according to factory rules
 */
  virtual bool Validate();

  /**
   * swaps AAT OZs for AST_CYLINDERs
   * @param tp
   * @return: point type compatible with current factory, most
   * similar to type of tp
   */
  virtual gcc_pure
  TaskPointFactoryType GetMutatedPointType(const OrderedTaskPoint &tp) const;
};

/**
 * a RT task factory that supports US rules
 */
class RTTaskFactoryUs:
  public RTTaskFactory
{
public:
/**
 * Constructor
 *
 * @param _task Ordered task to be managed by this factory
 * @param tb Behaviour (options)
 */
  RTTaskFactoryUs(OrderedTask& _task,
                  const TaskBehaviour &tb);

  ~RTTaskFactoryUs() {};

  /**
   * swaps AAT OZs for AST_CYLINDERs
   * @param tp
   * @return: point type compatible with current factory, most
   * similar to type of tp
   */
  virtual gcc_pure
  TaskPointFactoryType GetMutatedPointType(const OrderedTaskPoint &tp) const;
};

#endif
