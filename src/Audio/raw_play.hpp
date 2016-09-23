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

#ifndef RAW_PLAY_HPP
#define RAW_PLAY_HPP

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>
#include "LogFile.hpp"

#define PLAYBACK_RATE 16000
#define PLAYBACK_CHUNK 500000
#define PLAYBACK_CARD_NAME "default"

class RawPlayback
{
private:
  snd_pcm_t *handle;
  int playback_chunk(short *buff, int count);
  /// number of underruns during playback
  unsigned underrun_count;
public:
  RawPlayback()
  :underrun_count(0)
  {
    /* Open PCM device for playback. */
    int rc = snd_pcm_open(&handle, PLAYBACK_CARD_NAME,
		      SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
      LogFormat("can't open default sound device: %s\n",
		snd_strerror(rc));
      handle = NULL;
      return;
    }
  };
  ~RawPlayback()
  {
    if (handle)
      snd_pcm_close(handle);
  };
  static void setAlsaMasterVolume(int volume);

  /**
   * returns true if the AlsaMasterVolume functions on this device
   */
  static bool HasAlsaMasterVolume();

  /**
   * How many underruns occurred during the playback?
   * If more than about 10, then on a Linux machine with Pulse Audio installed
   * Pulse probably needs to be configured:
   *   %sudo nano /etc/pulse/default.pa:
   *      remove / comment out: module-udev-detect
   *      remove / comment out: module-detect
   *      add: load-module module-alsa-sink device=dmix
   *      uncomment: module-alsa-sink
   */
  unsigned GetUnderrunCount() {
    return underrun_count;
  }
  int playback_file(const char *name);
private:
  int playback_mem(short *buff, int count);
};
#endif /* RAW_PLAY_HPP */
