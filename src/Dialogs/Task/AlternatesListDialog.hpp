/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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

#ifndef DIALOG_ALTERNATES_LIST_HPP
#define DIALOG_ALTERNATES_LIST_HPP

#include "Widget/ListWidget.hpp"
#include "Engine/Task/Unordered/AlternateList.hpp"
#include "Form/ActionListener.hpp"


class WndButton;
class WidgetDialog;
class Canvas;
struct DialogLook;

class AlternatesListWidget
  : public ListWidget, private ActionListener {
  enum Buttons {
    SETTINGS,
    GOTO,
  };

  const DialogLook &dialog_look;

  WndButton *details_button, *cancel_button, *goto_button;

public:
  AlternateList alternates;

public:
  void CreateButtons(WidgetDialog &dialog);

public:
  AlternatesListWidget(const DialogLook &_dialog_look)
    :dialog_look(_dialog_look) {}

  unsigned GetCursorIndex() const {
    return GetList().GetCursorIndex();
  }

  void Update();

public:
  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Unprepare() override {
    DeleteWindow();
  }

  /* virtual methods from class List::Handler */
  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned index) override;

  virtual bool CanActivateItem(unsigned index) const {
    return true;
  }

  virtual void OnActivateItem(unsigned index) override;

  /* virtual methods from class ActionListener */
  virtual void OnAction(int id) override;
};

#endif


