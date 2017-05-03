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

#ifndef DIALOG_ALTERNATES_LIST_HPP
#define DIALOG_ALTERNATES_LIST_HPP

#include "Widget/ListWidget.hpp"
#include "Widget/TwoWidgets.hpp"
#include "Widget/TextWidget.hpp"
#include "Engine/Task/Unordered/AlternateList.hpp"
#include "Form/ActionListener.hpp"
#include "Screen/Layout.hpp"
#include "Form/CheckBox.hpp"
#include "Computer/Settings.hpp"

class Button;
class CheckBoxWidget;
class WidgetDialog;
class Canvas;
class WndForm;

struct DialogLook;
struct DerivedInfo;

enum Buttons {
  Goto = 100,
  Details,
};

class AlternatesListWidget
  : public ListWidget, private ActionListener {
  enum Buttons {
    SETTINGS,
    GOTO,
  };

  const DialogLook &dialog_look;

  Button *details_button, *cancel_button, *goto_button;

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

  virtual bool CanActivateItem(unsigned index) const override {
    return true;
  }

  virtual void OnActivateItem(unsigned index) override;

  /* virtual methods from class ActionListener */
  virtual void OnAction(int id) override;
};

/**
 * a widget that lists the alternates and executes the actions
 * but has no buttons visible
 */
class AlternatesListWidgetNoButtons : public AlternatesListWidget
{
protected:
  WndForm *form;
public:
  AlternatesListWidgetNoButtons(const DialogLook &_dialog_look)
    :AlternatesListWidget(_dialog_look) {}

  void SetForm(WndForm *_form) {
    assert(_form != nullptr);
    form = _form;
  }

  /* virtual methods from class Widget */
  virtual PixelSize GetMinimumSize() const override {
    return GetMaximumSize();
  }
  virtual void Move(const PixelRect &rc) override;
  bool DoDetails();
  bool DoGoto();
  void UpdateAirfieldsOnly(bool airfields_only);
  const Waypoint* GetWaypoint();

  virtual void OnActivateItem(unsigned index) override;

  void Refresh();
};


/**
 * A widget class that displays headers above the List Widget
 * when used with TwoWidgets
 */
class AlternatesListHeaderWidget : public TwoWidgets, public ActionListener
{
public:
  enum Buttons {
    AirFieldsOnly = 100,
  };

  ComputerSettings &settings_computer;

public:
  AlternatesListHeaderWidget(const DerivedInfo& calculated);
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Unprepare() override;
  void Show(const PixelRect &rc) override;
  virtual void Move(const PixelRect &rc) override;

  virtual PixelSize GetMinimumSize() const override {
    return PixelSize { 25u, Layout::GetMinimumControlHeight() / 2 };
  }
  virtual PixelSize GetMaximumSize() const override {
    return PixelSize { 25u, Layout::GetMaximumControlHeight() };
  }
  void CalculateLayout(const PixelRect &rc);

  /* virtual inhereted ActionListener */
  void OnAction(int id) override;

  void UpdateAirfieldsOnly(bool airfields_only);
  CheckBoxWidget &GetAirfieldsCheckbox();
};

#endif


