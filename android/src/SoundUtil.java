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

package org.tophat;

import java.util.HashMap;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.content.Context;
import java.util.Queue;
import java.util.LinkedList;
import android.util.Log;

public class SoundUtil {
  private static final String TAG = "TopHat";
  private static HashMap<String, Integer> resources = new HashMap();

  static {
    resources.put("IDR_FAIL", R.raw.fail);
    resources.put("IDR_INSERT", R.raw.insert);
    resources.put("IDR_REMOVE", R.raw.remove);
    resources.put("IDR_WAV_BEEP_BWEEP", R.raw.beep_bweep);
    resources.put("IDR_WAV_BEEP_CLEAR", R.raw.beep_clear);
    resources.put("IDR_WAV_BEEP_DRIP", R.raw.beep_drip);
    resources.put("IDR_WAV_TRAFFIC_LOW", R.raw.traffic_low);
    resources.put("IDR_WAV_TRAFFIC_IMPORTANT", R.raw.traffic_important);
    resources.put("IDR_WAV_TRAFFIC_URGENT", R.raw.traffic_urgent);
    resources.put("IDR_WAV_ABOVE", R.raw.above);
    resources.put("IDR_WAV_BELOW", R.raw.below);
    resources.put("IDR_WAV_ONE_OCLOCK", R.raw.one_oclock);
    resources.put("IDR_WAV_TWO_OCLOCK", R.raw.two_oclock);
    resources.put("IDR_WAV_THREE_OCLOCK", R.raw.three_oclock);
    resources.put("IDR_WAV_FOUR_OCLOCK", R.raw.four_oclock);
    resources.put("IDR_WAV_FIVE_OCLOCK", R.raw.five_oclock);
    resources.put("IDR_WAV_SIX_OCLOCK", R.raw.six_oclock);
    resources.put("IDR_WAV_SEVEN_OCLOCK", R.raw.seven_oclock);
    resources.put("IDR_WAV_EIGHT_OCLOCK", R.raw.eight_oclock);
    resources.put("IDR_WAV_NINE_OCLOCK", R.raw.nine_oclock);
    resources.put("IDR_WAV_TEN_OCLOCK", R.raw.ten_oclock);
    resources.put("IDR_WAV_ELEVEN_OCLOCK", R.raw.eleven_oclock);
    resources.put("IDR_WAV_TWELVE_OCLOCK", R.raw.twelve_oclock);
    resources.put("IDR_WAV_AIRSPACE", R.raw.airspace);
  }
  private static MediaPlayer instance;

  static Boolean isPlaying = false;
  static Context _context;

  static Queue<Integer> queuedSounds = new LinkedList<Integer>();

  private static Boolean deQueueAndPlay() {
    synchronized (queuedSounds) {
      if (isPlaying) {
        return true;
      }

      Integer id = queuedSounds.poll();
      if (id == null) {
        return false;
      }

      isPlaying = true;
      isPlaying = playNow(_context, id);
    }
    return true;
  }

  /**
   * queues the sound and triggers play of the queue
   */
  public static boolean play(Context context, String name) {

    Integer id = resources.get(name);
    if (id == null) {
      Log.w(TAG, "SoundUtil::play error:  Resource not found: " + name);
      return false;
    }
    synchronized (queuedSounds) {
      queuedSounds.offer(id);
    }
    _context = context;

    return deQueueAndPlay();
  }

  private static boolean playNow(Context context, Integer id) {
    if (id == null) {
      Log.w(TAG, "SoundUtil::PlayNow error: id null");
      return false;
    }

    MediaPlayer mp = MediaPlayer.create(context, id);
    if (mp == null) {
      Log.w(TAG, "SoundUtil::PlayNow error.  Could not create Media Player. id: " + id);
      return false;
    }

    mp.setOnCompletionListener(new OnCompletionListener() {
	  @Override
	  public void onCompletion(MediaPlayer mp) {
		mp.release();
		mp = null;
		isPlaying = false;
        deQueueAndPlay();
      }
	});

    mp.start();
    return true;
  }
}
