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

#include <windef.h>
#include "StartupConfigPanel.hpp"
#include "Profile/Profile.hpp"
#include "Language/Language.hpp"
#include "Interface.hpp"
#include "Widget/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "LogFile.hpp"
#include "LocalPath.hpp"
#include "IO/TextWriter.hpp"
#include "OS/FileUtil.hpp"
#include "Startup/Settings.hpp"
#include "IO/FileLineReader.hpp"
#include "Util/StringUtil.hpp"

enum ControlIndex {
  TophatArguments,
};

class StartupConfigPanel final : public RowFormWidget {
public:
  StartupConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;
};

static const TCHAR *
ReadQuickStart()
{
  static TCHAR path[MAX_PATH];
  TCHAR *line;

  InitialiseDataPath();
  LocalPath(path, _T(TOPHAT_ARGUMENTS));
  FileLineReader *file = new FileLineReader(path);
  if (file == nullptr)
    return _T("");
  line = file->ReadLine();
  if (line == nullptr)
    return _T("");
  CopyASCII(path, line);
  delete file;
  return path;
}

static void
WriteQuickStart(TCHAR *string)
{
  static TCHAR path[MAX_PATH];
  LocalPath(path, _T(TOPHAT_ARGUMENTS));
  TextWriter *writer = new TextWriter(path);
  if (writer == nullptr)
    return;
  if (!writer->IsOpen()) {
    delete writer;
    return;
  }
  writer->FormatLine(_T("%s"), string);
  delete writer;
}

static void
DeleteQuickStart()
{
  static TCHAR path[MAX_PATH];
  LocalPath(path, _T(TOPHAT_ARGUMENTS));
  File::Delete(path);
}

void
StartupConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  StartupSettings *startup = new StartupSettings();

  startup->SetDefaults(ReadQuickStart());
  RowFormWidget::Prepare(parent, rc);
  AddText(_("tophat arguments"), nullptr, startup->tophat_arguments);

  AddMultiLine(_("Valid arguments are: -fly -simulator -quick.  [-quick] causes Top Hat to proceed to map screen."));
}

bool
StartupConfigPanel::Save(bool &changed)
{
  StartupSettings *startup = new StartupSettings();

  changed |= SaveValue(TophatArguments, ProfileKeys::TophatArguments,
                       startup->tophat_arguments.buffer(),
		       startup->tophat_arguments.CAPACITY);
  if (startup->tophat_arguments.length() != 0)
    WriteQuickStart(startup->tophat_arguments.buffer());
  else
    DeleteQuickStart();

  return true;
}

Widget *
CreateStartupConfigPanel()
{
  return new StartupConfigPanel();
}
