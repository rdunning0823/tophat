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

#include "InterfaceConfigPanel.hpp"
#include "Profile/Profile.hpp"
#include "Form/Button.hpp"
#include "Form/RowFormWidget.hpp"
#include "Form/DataField/Enum.hpp"
#include "Dialogs/Dialogs.h"
#include "Util/StringUtil.hpp"
#include "Interface.hpp"
#include "Language/LanguageGlue.hpp"
#include "Asset.hpp"
#include "LocalPath.hpp"
#include "OS/FileUtil.hpp"
#include "OS/PathName.hpp"
#include "Protection.hpp"
#include "ConfigPanel.hpp"
#include "Language/Language.hpp"
#include "UIGlobals.hpp"
#include "Hardware/Vibrator.hpp"

#include <windef.h> /* for MAX_PATH */

enum ControlIndex {
#ifdef HAVE_BLANK
  AutoBlank,
#endif
  InputFile,
#ifndef HAVE_NATIVE_GETTEXT
  LanguageFile,
#endif
  StatusFile,
  MenuTimeout,
  HapticFeedback
};

class InterfaceConfigPanel : public RowFormWidget {
public:
  InterfaceConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()), buttonFonts(0) {}

  WndButton *buttonFonts;

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);
  virtual void Show(const PixelRect &rc);
  virtual void Hide();
};

static void
OnFonts(gcc_unused WndButton &button)
{
  dlgConfigFontsShowModal();
}

void
InterfaceConfigPanel::Show(const PixelRect &rc)
{
  buttonFonts->set_text(_("Fonts"));
  buttonFonts->SetOnClickNotify(OnFonts);
  buttonFonts->Show();

  RowFormWidget::Show(rc);
}

void
InterfaceConfigPanel::Hide()
{
  buttonFonts->Hide();

  RowFormWidget::Hide();
}

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

void
InterfaceConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const UISettings &settings = CommonInterface::GetUISettings();

  RowFormWidget::Prepare(parent, rc);

  buttonFonts = ConfigPanel::GetExtraButton(1);
  assert(buttonFonts);

#ifdef HAVE_BLANK
  AddBoolean(_("Auto. blank"),
             _("This determines whether to blank the display after a long period of inactivity "
                 "when operating on internal battery power."),
             settings.display.enable_auto_blank);
#endif

  AddFileReader(_("Events"),
                _("The Input Events file defines the menu system and how XCSoar responds to "
                    "button presses and events from external devices."),
                ProfileKeys::InputFile, _T("*.xci\0"));
  SetExpertRow(InputFile);

#ifndef HAVE_NATIVE_GETTEXT
  WndProperty *wp;
  wp = AddEnum(_("Language"),
               _("The language options selects translations for English texts to other "
                   "languages. Select English for a native interface or Automatic to localise "
                   "XCSoar according to the system settings."));
  if (wp != NULL) {
    DataFieldEnum &df = *(DataFieldEnum *)wp->GetDataField();
    df.addEnumText(_("Automatic"));
    df.addEnumText(_("English"));

#ifdef HAVE_BUILTIN_LANGUAGES
    for (const struct builtin_language *l = language_table;
         l->resource != NULL; ++l)
      df.addEnumText(l->resource);
#endif

    LanguageFileVisitor lfv(df);
    VisitDataFiles(_T("*.mo"), lfv);

    df.Sort(2);

    TCHAR value[MAX_PATH];
    if (!Profile::GetPath(ProfileKeys::LanguageFile, value))
      value[0] = _T('\0');

    if (_tcscmp(value, _T("none")) == 0)
      df.Set(1);
    else if (!StringIsEmpty(value) && _tcscmp(value, _T("auto")) != 0) {
      const TCHAR *base = BaseName(value);
      if (base != NULL)
        df.Set(base);
    }
    wp->RefreshDisplay();
  }
#endif /* !HAVE_NATIVE_GETTEXT */

  AddFileReader(_("Status message"),
                _("The status file can be used to define sounds to be played when certain "
                    "events occur, and how long various status messages will appear on screen."),
                ProfileKeys::StatusFile, _T("*.xcs\0"));
  SetExpertRow(StatusFile);

  AddTime(_("Menu timeout"),
          _("This determines how long menus will appear on screen if the user does not make any button "
            "presses or interacts with the computer."),
          1, 60, 1, settings.menu_timeout / 2);
  SetExpertRow(MenuTimeout);

  #ifdef HAVE_VIBRATOR
  static constexpr StaticEnumChoice haptic_feedback_list[] = {
    { (unsigned) UISettings::HapticFeedback::Default, N_("OS settings") },
    { (unsigned) UISettings::HapticFeedback::Off, N_("Off") },
    { (unsigned) UISettings::HapticFeedback::On, N_("On") },
    { 0 }
  };

  wp = AddEnum(_("Haptic feedback"),
               _("Determines if haptic feedback like vibration is used."),
               haptic_feedback_list, (unsigned)settings.haptic_feedback);
  SetExpertRow(HapticFeedback);
#endif /* HAVE_VIBRATOR */
}

bool
InterfaceConfigPanel::Save(bool &_changed, bool &_require_restart)
{
  UISettings &settings = CommonInterface::SetUISettings();
  bool changed = false, require_restart = false;;

#ifdef HAVE_BLANK
  changed |= SaveValue(AutoBlank, ProfileKeys::AutoBlank,
                       settings.display.enable_auto_blank);
#endif

  require_restart |= changed |= SaveValueFileReader(InputFile, ProfileKeys::InputFile);

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

    switch (df.GetAsInteger()) {
    case 0:
      new_value = new_base = _T("auto");
      break;

    case 1:
      new_value = new_base = _T("none");
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
      LanguageChanged = changed = true;
    }
  }
#endif

  require_restart |= changed |= SaveValueFileReader(StatusFile, ProfileKeys::StatusFile);

  unsigned menu_timeout = GetValueInteger(MenuTimeout) * 2;
  if (settings.menu_timeout != menu_timeout) {
    settings.menu_timeout = menu_timeout;
    Profile::Set(ProfileKeys::MenuTimeout, menu_timeout);
    changed = true;
  }

#ifdef HAVE_VIBRATOR
  changed |= SaveValueEnum(HapticFeedback, ProfileKeys::HapticFeedback, settings.haptic_feedback);
#endif

  _changed |= changed;
  _require_restart |= require_restart;
  return true;
}

Widget *
CreateInterfaceConfigPanel()
{
  return new InterfaceConfigPanel();
}
