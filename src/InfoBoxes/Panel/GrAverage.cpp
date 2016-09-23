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

#include "GrAverage.hpp"
#include "Base.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Profile/Profile.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Listener.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Form/Form.hpp"
#include "Widget/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "Screen/Layout.hpp"
#include "Screen/SingleWindow.hpp"

enum ControlIndex {
  Spacer,
  AverEffTime,
};

class GrAverageConfigPanel : public RowFormWidget, private DataFieldListener  {
public:
  GrAverageConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()), form(nullptr) {}
  /**
   * the parent form
   */
  WndForm *form;

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;
  void Show(const PixelRect &rc) override;
  virtual void Move(const PixelRect &rc) override;
  void SetForm(WndForm *_form) {
    assert(_form != nullptr);
    form = _form;
  }

protected:
  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df) override;
};

void
GrAverageConfigPanel::Show(const PixelRect &rc)
{
    RowFormWidget::Show(rc);
    RowFormWidget::GetControl(Spacer).SetVisible(false);
}

void
GrAverageConfigPanel::Move(const PixelRect &rc)
{
  RowFormWidget::Move(rc);
  form->Move(UIGlobals::GetMainWindow().GetClientRect());
}

void
GrAverageConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const ComputerSettings &settings_computer = CommonInterface::GetComputerSettings();

  RowFormWidget::Prepare(parent, rc);

  assert(IsDefined());

  Add(_T("Spacer"));

  static constexpr StaticEnumChoice aver_eff_list[] = {
    { ae7seconds, _T("7 s"), N_("For paragliders going to the speed-bar or brakes.") },
    { ae15seconds, _T("15 s"), N_("Suggested period for paragliders.") },
    { ae30seconds, _T("30 s") },
    { ae60seconds, _T("60 s") },
    { ae90seconds, _T("90 s"), N_("Suggested period for gliders.") },
    { ae2minutes, _T("2 min") },
    { ae3minutes, _T("3 min") },
    { 0 }
  };

  AddEnum(_("GR average period"),
          _("Number of seconds over which the Average glide ratio is calculated. "
              "Normally for gliders a good value is 90-120 seconds, and for paragliders 15 seconds."),
          aver_eff_list, settings_computer.average_eff_time, this);
}

void
GrAverageConfigPanel::OnModified(DataField &df)
{
  assert (form != nullptr);
  if (&df == &GetDataField(AverEffTime))
      form->SetModalResult(mrOK);
}

bool
GrAverageConfigPanel::Save(bool &_changed)
{
  bool changed = false;

  ComputerSettings &settings_computer = CommonInterface::SetComputerSettings();

  changed |=
      SaveValueEnum(AverEffTime, ProfileKeys::AverEffTime, settings_computer.average_eff_time);

  _changed |= changed;

  return true;
}

class GrAveragePanel : public BaseAccessPanel {
public:

  GrAveragePanel(unsigned _id, GrAverageConfigPanel *panel)
    :BaseAccessPanel(_id, panel) {}
  virtual void Hide();
};

void
GrAveragePanel::Hide()
{
  GrAverageConfigPanel *gr_average_config_panel =
      (GrAverageConfigPanel *)managed_widget.Get();
  assert(gr_average_config_panel);
  bool changed;
  gr_average_config_panel->Save(changed);

  BaseAccessPanel::Hide();
}

Widget *
LoadGrAveragePanel(unsigned id)
{
  GrAverageConfigPanel *inner_panel = new GrAverageConfigPanel();
  GrAveragePanel *panel = new GrAveragePanel(id, inner_panel);
  inner_panel->SetForm(panel);
  return panel;
}
