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

#include "ProfileConfigPanel.hpp"
#include "Dialogs/ProfileListDialog.hpp"
#include "Language/Language.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Form/DataField/File.hpp"
#include "Interface.hpp"
#include "UIGlobals.hpp"
#include "OS/FileUtil.hpp"

enum ControlIndex {
  Profile,
};

class ProfileConfigPanel final : public RowFormWidget {
public:
  ProfileConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;
};


static bool
SelectProfileCallback(const TCHAR *caption, DataField &_df,
                      const TCHAR *help_text)
{
  FileDataField &df = (FileDataField &)_df;

  const auto path = SelectProfileDialog(df.GetPathFile(), _("Available profiles"));
  if (path.empty())
    return false;

  df.ForceModify(path.c_str());
  return true;
}

void
ProfileConfigPanel::Prepare(ContainerWindow &parent,
                            const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);

  /* scan all profile files */
  auto *dff = new FileDataField();
  dff->ScanDirectoryTop(_T("*.top"));
  dff->SetExtensionVisible(false);

  /* preselect the most recently used profile */
  unsigned best_index = 0;
  uint64_t best_timestamp = 0;
  unsigned length = dff->size();

  for (unsigned i = 0; i < length; ++i) {
    const TCHAR *path = dff->GetItem(i);
    uint64_t timestamp = File::GetLastModification(path);
    if (timestamp > best_timestamp) {
      best_timestamp = timestamp;
      best_index = i;
    }
  }
  dff->Set(best_index);

  WndProperty *pe = Add(_("Profile"), nullptr, dff);
  pe->SetEditCallback(SelectProfileCallback);

  AddMultiLine(_("Add and delete user profiles.  If more than one profile exists, Top Hat prompts for the profile during startup."));
}

bool
ProfileConfigPanel::Save(bool &changed)
{
  // this does not change anything to the current profile
  return false;
}

Widget *
CreateProfileConfigPanel()
{
  return new ProfileConfigPanel();
}
