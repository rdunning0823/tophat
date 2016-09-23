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

#include "Audio/Sound.hpp"
#include "Audio/Features.hpp"

#ifdef ANDROID
#include "Android/Main.hpp"
#include "Android/SoundUtil.hpp"
#include "Android/Context.hpp"
#else
#include "SoundQueue.hpp"
#endif

#if defined(WIN32) && !defined(GNAV)
#include "ResourceLoader.hpp"
#include <windows.h>
#include <mmsystem.h>
#endif

#if defined(HAVE_RAW_PLAY)
#include "Audio/raw_play.hpp"
#include "Util/StringUtil.hpp"
#include "Util/StaticString.hxx"
#include <windef.h>

const char *beep_id_str = "IDR_";
const char *wav_str = "WAV_";
#endif

#if !defined(ANDROID)
bool PlayResourceNow(const SoundQueue::SoundName resource_name)
{
#if defined(WIN32) && !defined(GNAV)
  if (_tcsstr(resource_name.c_str(), TEXT(".wav")))
    return sndPlaySound(resource_name.c_str(), SND_SYNC | SND_NODEFAULT);

  ResourceLoader::Data data = ResourceLoader::Load(resource_name.c_str(), _T("WAVE"));
  return !data.IsNull() &&
         sndPlaySound((LPCTSTR)data.data,
                      SND_MEMORY | SND_SYNC | SND_NODEFAULT);
#else
#if defined(HAVE_RAW_PLAY)
  // Linux, Kobo
  SoundQueue::SoundName resource_name2(resource_name);
  const TCHAR *resource_name_pointer = resource_name2.buffer();
  StaticString<MAX_PATH> raw_file_name;
  raw_file_name = _T("/opt/tophat/share/sounds/");

  if (strncmp(resource_name_pointer, beep_id_str, strlen(beep_id_str)) == 0) {
    resource_name_pointer += strlen(beep_id_str);
    if (strncmp(resource_name_pointer, wav_str, strlen(wav_str)) == 0)
      resource_name_pointer += strlen(wav_str);
  }
  StaticString<64> lower_resource_name;
  CopyASCIILower(lower_resource_name.buffer(), resource_name_pointer);

  raw_file_name.append(lower_resource_name.c_str());
  raw_file_name.append(".raw");

  RawPlayback *raw_playback = new RawPlayback();
  int ret = raw_playback->playback_file(raw_file_name);
  delete raw_playback;
  if (ret < 0)
    return false;

  return true;
#endif
  return false;
#endif
}
#endif

bool
PlayResource(const TCHAR *resource_name)
{
#ifdef ANDROID
  return SoundUtil::Play(Java::GetEnv(), context->Get(), resource_name);
#else
  SoundQueue::SoundName sound_name(resource_name);
  SoundQueue::Enqueue(sound_name);
  return true;
#endif
}

void ConfigureSoundDevice(const SoundSettings &sound_settings)
{
#ifdef HAVE_RAW_PLAY
  RawPlayback::setAlsaMasterVolume(sound_settings.volume);
#endif
}

bool HasVolumeControl()
{
#ifdef HAVE_RAW_PLAY
  return RawPlayback::HasAlsaMasterVolume();
#else
  return false;
#endif
}
