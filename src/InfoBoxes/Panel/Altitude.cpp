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

#include "Altitude.hpp"
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
  ShowAlternativeUnits,
};

class AltitudeConfigPanel : public RowFormWidget, private DataFieldListener  {
public:
  AltitudeConfigPanel()
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
AltitudeConfigPanel::Show(const PixelRect &rc)
{
    RowFormWidget::Show(rc);
    RowFormWidget::GetControl(Spacer).SetVisible(false);
}

void
AltitudeConfigPanel::Move(const PixelRect &rc)
{
  RowFormWidget::Move(rc);
  form->Move(UIGlobals::GetMainWindow().GetClientRect());
}

void
AltitudeConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const InfoBoxSettings &settings_info_boxes = CommonInterface::GetUISettings().info_boxes;

  RowFormWidget::Prepare(parent, rc);

  assert(IsDefined());

  Add(_T("Spacer"));

  AddBoolean(_("Show feet and meters"),
             _("Display second set of units in the bottom of the Infobox"),
             settings_info_boxes.show_alternative_altitude_units, this);
}

void
AltitudeConfigPanel::OnModified(DataField &df)
{
  assert (form != nullptr);
  if (&df == &GetDataField(ShowAlternativeUnits))
      form->SetModalResult(mrOK);
}

bool
AltitudeConfigPanel::Save(bool &_changed)
{
  bool changed = false;
  InfoBoxSettings &settings_info_boxes = CommonInterface::SetUISettings().info_boxes;

  changed |=
      SaveValueEnum(ShowAlternativeUnits, ProfileKeys::ShowAlternateAltitudeUnits,
                    settings_info_boxes.show_alternative_altitude_units);

  _changed |= changed;

  return changed;
}

class AltitudePanel : public BaseAccessPanel {
public:

  AltitudePanel(unsigned _id, AltitudeConfigPanel *panel)
    :BaseAccessPanel(_id, panel) {}
  virtual void Hide();
};

void
AltitudePanel::Hide()
{
  AltitudeConfigPanel *altitude_config_panel =
      (AltitudeConfigPanel *)managed_widget.Get();
  assert(altitude_config_panel);
  bool changed;
  altitude_config_panel->Save(changed);

  BaseAccessPanel::Hide();
}

Widget *
LoadAltitudePanel(unsigned id)
{
  AltitudeConfigPanel *inner_panel = new AltitudeConfigPanel();
  AltitudePanel *panel = new AltitudePanel(id, inner_panel);
  inner_panel->SetForm(panel);
  return panel;
}
