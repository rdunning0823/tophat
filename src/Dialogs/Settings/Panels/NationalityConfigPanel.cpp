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

#include "NationalityConfigPanel.hpp"
#include "Profile/Profile.hpp"
#include "Form/Button.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Listener.hpp"
#include "Dialogs/Dialogs.h"
#include "Util/StringUtil.hpp"
#include "Interface.hpp"
#include "Language/LanguageGlue.hpp"
#include "Asset.hpp"
#include "LocalPath.hpp"
#include "OS/FileUtil.hpp"
#include "OS/PathName.hpp"
#include "UtilsSettings.hpp"
#include "ConfigPanel.hpp"
#include "Language/Language.hpp"
#include "UIGlobals.hpp"
#include "Hardware/Vibrator.hpp"
#include "Engine/Task/TaskNationalities.hpp"
#include "Units/UnitsStore.hpp"

#include <windef.h> /* for MAXPATH */

enum ControlIndex {
#ifndef HAVE_NATIVE_GETTEXT
  LanguageFile,
#endif
  ContestNationality,
  UnitsPreset,
};

class NationalityConfigPanel final : public RowFormWidget, DataFieldListener {
  /**
   * did the user change the units with the UI?
   */
  bool units_changed;

public:
  NationalityConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;

  /**
   * checks for language change and reads the
   * language file if needed
   */
  void UpdateLanguage();
  void RefreshForm();

protected:
  void OnModified(DataField &df) override;
};

#ifndef HAVE_NATIVE_GETTEXT

class LanguageFileVisitor: public File::Visitor
{
private:
  DataFieldEnum &df;

public:
  LanguageFileVisitor(DataFieldEnum &_df): df(_df) {}

  void
  Visit(const TCHAR *path, const TCHAR *filename)
  {
    if (filename != NULL && !df.Exists(filename))
      df.addEnumText(filename);
  }
};

#endif

static constexpr StaticEnumChoice  task_rules_types[] = {
  { (unsigned)ContestNationalities::FAI, N_("FAI rules"), N_("Use FAI contest rules for flying contests.  Requires restart.") },
  { (unsigned)ContestNationalities::AMERICAN, N_("US rules"), N_("Use US contest rules for flying contests.  Requires restart.") },
  { 0 }
};

void
NationalityConfigPanel::OnModified(DataField &df)
{
  if (IsDataField(UnitsPreset, df))
    units_changed = true;
#ifndef HAVE_NATIVE_GETTEXT
  else if (IsDataField(LanguageFile, df)) {
    UpdateLanguage();
    RefreshForm();
  }
#endif
}

void
NationalityConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const ComputerSettings &settings_computer = CommonInterface::GetComputerSettings();
  const TaskBehaviour &task_behaviour = settings_computer.task;

  RowFormWidget::Prepare(parent, rc);

  WndProperty *wp;
#ifndef HAVE_NATIVE_GETTEXT
  wp = AddEnum(_("Language"),
               _("The text in Top Hat is displayed in the following languages.  "
                 "Select automatic to select the language based on your "
                 "device settings"), this);
  if (wp != NULL) {
    DataFieldEnum &df = *(DataFieldEnum *)wp->GetDataField();
    df.addEnumText(_("Automatic"));

#ifdef HAVE_BUILTIN_LANGUAGES
    for (const BuiltinLanguage *l = language_table;
         l->resource != NULL; ++l) {
      df.addEnumText(l->resource, l->name);
    }
#endif

    LanguageFileVisitor lfv(df);
    VisitDataFiles(_T("*.mo"), lfv);

    df.Sort(2);

    TCHAR value[MAX_PATH];
    if (!Profile::GetPath(ProfileKeys::LanguageFile, value))
      value[0] = _T('\0');

    // set "none" == ENGLISH t== "1" for legacy systems before we had an
    // English po file
    if (StringIsEqual(value, _T("none")))
      df.Set(1);
    else if (!StringIsEmpty(value) && !StringIsEqual(value, _T("auto"))) {
      const TCHAR *base = BaseName(value);
      if (base != NULL)
        df.Set(base);
    }
    wp->RefreshDisplay();
  }
#endif /* !HAVE_NATIVE_GETTEXT */


  AddEnum(_("Task rules"),
          _("Fly contest with US contest rules or with FAI contest rules"),
          task_rules_types,
          (unsigned)task_behaviour.contest_nationality);

  wp = AddEnum(_("Units"), _("Your nationality's units."), this);
  DataFieldEnum &df = *(DataFieldEnum *)wp->GetDataField();

  unsigned len = Units::Store::Count();
  UnitSetting current_dlg_set = CommonInterface::SetUISettings().format.units;

  for (unsigned i = 0; i < len; i++)
    df.addEnumText(Units::Store::GetName(i), i);

  unsigned index = Units::Store::EqualsPresetUnits(current_dlg_set);
  bool has_custom_units = (index == 0);
  if (has_custom_units) {
    // adjust index b/c EqualsPresetUnits assumes "Custom" is always at
    // start of list
    df.addEnumText(_("Custom"), (unsigned)len, _("My customized set of units."));
    index = len;
  } else
    --index;

  LoadValueEnum(UnitsPreset, index);

  units_changed = false;
}

void
NationalityConfigPanel::RefreshForm()
{
#ifndef HAVE_NATIVE_GETTEXT
  WndProperty &wp_language = GetControl(LanguageFile);
  wp_language.SetCaption(_("Language"));
#endif

  WndProperty &wp_task_rules = GetControl(ContestNationality);
  wp_task_rules.SetCaption(_("Task rules"));

  WndProperty &wp_units = GetControl(UnitsPreset);
  wp_units.SetCaption(_("Units"));
}

void
NationalityConfigPanel::UpdateLanguage()
{
#ifndef HAVE_NATIVE_GETTEXT
  WndProperty *wp = (WndProperty *)&GetControl(LanguageFile);
  if (wp != NULL) {
    DataFieldEnum &df = *(DataFieldEnum *)wp->GetDataField();

    TCHAR old_value[MAX_PATH];
    if (!Profile::GetPath(ProfileKeys::LanguageFile, old_value))
      old_value[0] = _T('\0');

    const TCHAR *old_base = BaseName(old_value);
    if (old_base == NULL)
      old_base = old_value;

    TCHAR buffer[MAX_PATH];
    const TCHAR *new_value, *new_base;

    switch (df.GetValue()) {
    case 0:
      new_value = new_base = _T("auto");
      break;

    default:
      _tcscpy(buffer, df.GetAsString());
      ContractLocalPath(buffer);
      new_value = buffer;
      new_base = BaseName(new_value);
      if (new_base == NULL)
        new_base = new_value;
      break;
    }
    if (_tcscmp(old_value, new_value) != 0 &&
        _tcscmp(old_base, new_base) != 0) {
      Profile::Set(ProfileKeys::LanguageFile, new_value);
      ReadLanguageFile();
    }
  }
#endif
}

bool
NationalityConfigPanel::Save(bool &_changed)
{
  ComputerSettings &settings_computer = CommonInterface::SetComputerSettings();
  TaskBehaviour &task_behaviour = settings_computer.task;

  bool changed = false;;


  require_restart |= SaveValueEnum(ContestNationality,
                                   ProfileKeys::ContestNationality,
                                   task_behaviour.contest_nationality);

  DataFieldEnum* dfe = (DataFieldEnum*)
      RowFormWidget::GetControl(UnitsPreset).GetDataField();
  unsigned the_unit = (unsigned)dfe->GetAsInteger();
  // don't change anytyhing if we started with custom units and ended with them
  if (the_unit != Units::Store::Count()) {
    if (units_changed) {

      UnitSetting preset_units = Units::Store::Read(the_unit);
      UnitSetting &config = CommonInterface::SetUISettings().format.units;
      config = preset_units;

      config.wind_speed_unit = config.speed_unit; // Mapping the wind speed to the speed unit
      Profile::Set(ProfileKeys::SpeedUnitsValue,
                   (int)config.speed_unit);

      Profile::Set(ProfileKeys::DistanceUnitsValue,
                   (int)config.distance_unit);

      Profile::Set(ProfileKeys::LiftUnitsValue,
                   (int)config.vertical_speed_unit);

      Profile::Set(ProfileKeys::AltitudeUnitsValue,
                   (int)config.altitude_unit);

      Profile::Set(ProfileKeys::TemperatureUnitsValue,
                   (int)config.temperature_unit);

      Profile::Set(ProfileKeys::TaskSpeedUnitsValue,
                   (int)config.task_speed_unit);

      Profile::Set(ProfileKeys::PressureUnitsValue,
                   (int)config.pressure_unit);

      Profile::Set(ProfileKeys::WingLoadingUnitValue,
                   (int)config.wing_loading_unit);

      Profile::Set(ProfileKeys::MassUnitValue,
                   (int)config.mass_unit);

    }
  }

  _changed |= changed;
  return true;
}

Widget *
CreateNationalityConfigPanel()
{
  return new NationalityConfigPanel();
}
