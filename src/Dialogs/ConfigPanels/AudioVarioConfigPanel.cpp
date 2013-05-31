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

#include "AudioVarioConfigPanel.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Language/Language.hpp"
#include "Interface.hpp"
#include "Form/Form.hpp"
#include "Form/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "Audio/Features.hpp"
#include "Audio/VarioGlue.hpp"

enum ControlIndex {
  Enabled,
  Volume,
  DEAD_BAND_ENABLED,
  SPACER,
  MIN_FREQUENCY,
  ZERO_FREQUENCY,
  MAX_FREQUENCY,
};


class AudioVarioConfigPanel : public RowFormWidget {
public:
  AudioVarioConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);
};

void
AudioVarioConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);

  if (!AudioVarioGlue::HaveAudioVario())
    return;

  const auto &settings = CommonInterface::GetUISettings().sound.vario;

  AddBoolean(_("Audio vario"),
             _("Emulate the sound of an electronic vario."),
             settings.enabled);

  AddInteger(_("Volume"), NULL, _T("%u %%"), _T("%u"),
             0, 100, 1, settings.volume);

  AddBoolean(_("Enable Deadband"),
             _("Mute the audio output in when the current lift is in a certain "
               "range around zero"), settings.dead_band_enabled);

  AddSpacer();
  SetExpertRow(SPACER);

  AddInteger(_("Min. Frequency"),
             _("The tone frequency that is played at maximum sink rate."),
             _T("%u Hz"), _T("%u"),
             50, 3000, 50, settings.min_frequency);
  SetExpertRow(MIN_FREQUENCY);

  AddInteger(_("Zero Frequency"),
             _("The tone frequency that is played at zero climb rate."),
             _T("%u Hz"), _T("%u"),
             50, 3000, 50, settings.zero_frequency);
  SetExpertRow(ZERO_FREQUENCY);

  AddInteger(_("Max. Frequency"),
             _("The tone frequency that is played at maximum climb rate."),
             _T("%u Hz"), _T("%u"),
             50, 3000, 50, settings.max_frequency);
  SetExpertRow(MAX_FREQUENCY);

}

bool
AudioVarioConfigPanel::Save(bool &changed, bool &require_restart)
{
  if (!AudioVarioGlue::HaveAudioVario())
    return true;

  auto &settings = CommonInterface::SetUISettings().sound.vario;

  changed |= SaveValue(Enabled, ProfileKeys::SoundAudioVario,
                       settings.enabled);

  unsigned volume = settings.volume;
  if (SaveValue(Volume, ProfileKeys::SoundVolume, volume)) {
    settings.volume = volume;
    changed = true;
  }

  changed |= SaveValue(DEAD_BAND_ENABLED, ProfileKeys::VarioDeadBandEnabled,
                       settings.dead_band_enabled);

  changed |= SaveValue(MIN_FREQUENCY, ProfileKeys::VarioMinFrequency,
                       settings.min_frequency);

  changed |= SaveValue(ZERO_FREQUENCY, ProfileKeys::VarioZeroFrequency,
                       settings.zero_frequency);

  changed |= SaveValue(MAX_FREQUENCY, ProfileKeys::VarioMaxFrequency,
                       settings.max_frequency);

  return true;
}

Widget *
CreateAudioVarioConfigPanel()
{
  return new AudioVarioConfigPanel();
}
