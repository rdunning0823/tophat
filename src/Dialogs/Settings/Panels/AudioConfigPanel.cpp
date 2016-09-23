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

#include "AudioConfigPanel.hpp"
#include "Profile/Profile.hpp"
#include "Language/Language.hpp"
#include "Interface.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Audio/Sound.hpp"
#include "Audio/Settings.hpp"

enum ControlIndex {
  ALSAVolumeControl,
  AudioTest
};

void
AudioConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);

  auto &sound_settings = CommonInterface::SetUISettings().sound;

  WndProperty *w = AddInteger(
      _("Sound volume"),
      _("Volume of sound card connected to your system. For KOBO it is usually UBS audio card. "
        "Volume value can be set between 5% and 100%"),
      _T("%d"), _T("%d"), 5, 100, 5, sound_settings.volume, this);

  if (w != nullptr) {
    w->SetEnabled(HasVolumeControl());
  }

  AddButton(_T("Test Sound"), *this, AudioTest);

  SetExpertRow(ALSAVolumeControl);
}

void
AudioConfigPanel::OnAction(int id) {
  if (id == AudioTest) {
    UpdateSoundConfig();
    PlayResource(_T("IDR_WAV_TRAFFIC_URGENT"));
  }
}

void
AudioConfigPanel::OnModified(DataField &df)
{
  if (IsDataField(ALSAVolumeControl, df)) {
    OnAction(AudioTest);
  }
}

void
AudioConfigPanel::UpdateSoundConfig() {
  SoundSettings tmp_settings = CommonInterface::SetUISettings().sound;

  tmp_settings.volume = GetValueInteger(ALSAVolumeControl);;
  ConfigureSoundDevice(tmp_settings);
}

bool
AudioConfigPanel::Save(bool &_changed)
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
CreateAudioConfigPanel()
{
  return new AudioConfigPanel();
}
