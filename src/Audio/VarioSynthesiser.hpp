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

#ifndef XCSOAR_AUDIO_VARIO_SYNTHESISER_HPP
#define XCSOAR_AUDIO_VARIO_SYNTHESISER_HPP

#include "ToneSynthesiser.hpp"
#include "Math/fixed.hpp"
#include "Compiler.h"

/**
 * This class generates vario sound.
 */
class VarioSynthesiser : public ToneSynthesiser {
  /**
   * The number of audible samples in each period.
   */
  size_t audible_count;

  /**
   * The number of silent samples in each period.  If this is zero,
   * then no silence will be generated (continuous tone).
   */
  size_t silence_count;

  /**
   * The number of audible/silence samples remaining in the current
   * period.  These two attributes will be reset to the according
   * _count value when both reach zero.
   */
  size_t audible_remaining, silence_remaining;

  bool dead_band_enabled;

  /**
   * The tone frequency for #min_vario.
   */
  unsigned min_frequency;

  /**
   * The tone frequency for stationary altitude.
   */
  unsigned zero_frequency;

  /**
   * The tone frequency for #max_vario.
   */
  unsigned max_frequency;

  /**
   * The minimum silence+audible period for #max_vario.
   */
  unsigned min_period_ms;

  /**
   * The maximum silence+audible period for #min_vario.
   */
  unsigned max_period_ms;

public:
  VarioSynthesiser()
    :audible_count(0), silence_count(1),
     audible_remaining(0), silence_remaining(0),
     dead_band_enabled(false),
     min_frequency(200), zero_frequency(500), max_frequency(1500),
     min_period_ms(150), max_period_ms(600) {}

  /**
   * Update the vario value.  This calculates a new tone frequency and
   * a new "silence" rate (for positive vario values).
   *
   * @param vario the current vario value [m/s]
   */
  void SetVario(unsigned sample_rate, fixed vario);

  /**
   * Produce silence from now on.
   */
  void SetSilence();

  /**
   * Enable/disable the dead band silence
   */
  void SetDeadBand(bool enabled) {
    dead_band_enabled = enabled;
  }

  /**
   * Set the base frequencies for minimum, zero and maximum lift
   */
  void SetFrequencies(unsigned min, unsigned zero, unsigned max) {
    min_frequency = min;
    zero_frequency = zero;
    max_frequency = max;
  }

  /**
   * Set the time periods for minimum and maximum lift
   */
  void SetPeriods(unsigned min, unsigned max) {
    min_period_ms = min;
    max_period_ms = max;
  }

  /* methods from class PCMSynthesiser */
  virtual void Synthesise(int16_t *buffer, size_t n);

private:
  /**
   * Convert a vario value to a tone frequency.
   *
   * @param ivario the current vario value [cm/s]
   */
  gcc_const
  unsigned VarioToFrequency(int ivario);
};

#endif
