/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2016 The XCSoar Project
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

#include "UnixSystemConfigPanel.hpp"
#include "Profile/Profile.hpp"
#include "Language/Language.hpp"
#include "Interface.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Audio/Sound.hpp"

#if defined(KOBO) || !(defined(WIN32) || defined(GNAV) || defined(ANDROID))
enum ControlIndex {
  ALSAVolumeControl
};

void
UnixSystemConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);

  auto &sound_settings = CommonInterface::SetUISettings().sound;

  AddInteger(_("Sound volume"),
             _("Volume of sound card connected to your system. For KOBO it is usually UBS audio card. "
                 "Volume value can be set between 5% and 100%"),
                 _T("%d"), _T("%d"), 5, 100, 5, sound_settings.volume);

  SetExpertRow(ALSAVolumeControl);
}

bool
UnixSystemConfigPanel::Save(bool &_changed)
{
  bool changed = false;
  int new_volume = GetValueInteger(ALSAVolumeControl);
  auto &sound_settings = CommonInterface::SetUISettings().sound;

  if (new_volume != sound_settings.volume) {
    Profile::Set(ProfileKeys::SystemSoundVolume, new_volume);
    changed = true;
    sound_settings.volume = new_volume;
    ConfigureSoundDevice(sound_settings);
  }

  _changed |= changed;

  return true;
}

Widget *
CreateUnixSystemConfigPanel()
{
  return new UnixSystemConfigPanel();
}
#endif

