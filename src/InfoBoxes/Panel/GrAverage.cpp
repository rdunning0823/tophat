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

#include "GrAverage.hpp"
#include "Base.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Profile/Profile.hpp"
#include "Form/DataField/Enum.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Form/Form.hpp"
#include "Form/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "Screen/Layout.hpp"


enum ControlIndex {
  Spacer,
  AverEffTime,
};

class GrAverageConfigPanel : public RowFormWidget {
public:
  GrAverageConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);
};

void
GrAverageConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const ComputerSettings &settings_computer = XCSoarInterface::GetComputerSettings();

  RowFormWidget::Prepare(parent, rc);

  assert(IsDefined());

#ifdef ENABLE_OPENGL
  Window *window = new Window();
  ContainerWindow *panel = (ContainerWindow *)GetWindow();

  window->set(panel, InitialControlRect(Layout::Scale(40)));
  Add(window);
#else
  AddDummy();
#endif

  static constexpr StaticEnumChoice aver_eff_list[] = {
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
          aver_eff_list, settings_computer.average_eff_time);
}

bool
GrAverageConfigPanel::Save(bool &_changed, bool &_require_restart)
{
  bool changed = false, require_restart = false;

  ComputerSettings &settings_computer = XCSoarInterface::SetComputerSettings();

  changed |= require_restart |=
      SaveValueEnum(AverEffTime, ProfileKeys::AverEffTime, settings_computer.average_eff_time);

  _changed |= changed;
  _require_restart |= require_restart;

  return true;
}

class GrAveragePanel : public BaseAccessPanel {
public:

  GrAveragePanel(unsigned _id)
    :BaseAccessPanel(_id, new GrAverageConfigPanel()) {}
  virtual void Hide();
};

void
GrAveragePanel::Hide()
{
  GrAverageConfigPanel *gr_average_config_panel =
      (GrAverageConfigPanel *)managed_widget.Get();
  assert(gr_average_config_panel);
  bool changed, restart_required;
  gr_average_config_panel->Save(changed, restart_required);

  BaseAccessPanel::Hide();
}

Widget *
LoadGrAveragePanel(unsigned id)
{
  return new GrAveragePanel(id);
}
