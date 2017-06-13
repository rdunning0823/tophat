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

#ifndef SCREEN_LABELARRANGER_HPP
#define SCREEN_LABELARRANGER_HPP

#include "Screen/Point.hpp"
#include "Util/StaticArray.hpp"
#include "Compiler.h"
#include "Math/fixed.hpp"
#include "Renderer/WaypointLabelList.hpp"
#include "Math/FastRotation.hpp"
#include "Time/PeriodClock.hpp"

/*********** FEATURES ****************/
/* use 8 discrete label position options instead of random positions */
#define DISCRETE_POSITION

/* jump to next cooling sate if 20% labs are accepted at current state */
#define ACCEPTED_PERCENT_LIMIT

/* if not using DISCRETE_POSITION, occasionally flip position 180 degrees */
#define ENABLE_HORIZONTAL_FLIP

/* displays stopwatch and stats at end of each run */
/* #define SHOW_RUN_STATS */

class Labeler {
  WaypointLabelList::LabelArray &lab;
  int w; // canvas width
  int h; // canvas height

  fixed max_move_x;
  fixed max_move_y;
  fixed max_angle;

  unsigned acc;
  unsigned acc_worse;
  unsigned rej;

  unsigned rot_acc;
  unsigned rot_acc_worse;
  unsigned rot_rej;

  unsigned mov_discrete_acc;
  unsigned mov_discrete_worse;
  unsigned mov_discrete_rej;

  // weights
  int w_len; // leader line length / distance weight
  int w_inter; // leader line intersection
  int w_lab2; // label-label overlap
  int w_lab_anc; // label-anchor overlap
  int w_orient; // orientation bias for discrete positions
  int w_off_page; // is start of label off page to left?
  bool allow_flip;  // allow the Move functions to flip the labels

  /* amount below which energy is considered zero and won't be moved again */
  int zero_energy_margin;

  uint32_t rand_seed;
  FastRotation fir;

  int max_label_width;
  int label_height;

  /*
   * Statistics and clocks.  These are only to ms so not very helpful
   * The stop_watch is much more accurate
   * */
  PeriodClock energy_clock;
  PeriodClock rotate_clock;
  PeriodClock move_clock;
  PeriodClock move_discrete_clock;
  PeriodClock intersect_clock;

  int energy_counter;
  int rotate_counter;
  int move_counter;
  int move_discrete_counter;
  int intersect_counter;

  int energy_loop_counter_potential;
  int energy_loop_counter_actual;

  /** the total unscaled energy of the system */
  int total_energy_raw;

public:
  Labeler(const Canvas &canvas, WaypointLabelList &list);
  void Start();

private:
  /**
   * sub method called by Energy
   * @param ener: energy object to be updated
   */
  void EnergyLoop1(unsigned index, unsigned i, const PixelRect& label1,
                   WaypointLabelList::EnergyMeasure &ener);

protected:
  /**
   * Updates Pos based on the old label position or position(0)
   * Updates energy based on that position.
   * results of the last calc for labels that were visible in the last run
   */
  void CalcInitialEnergyAndPosition();
  void SetMaxMove(unsigned label_height);

  /**
   * displays debug run statistics
   */
  void ShowRunStats(const TCHAR *text, unsigned move,
                    unsigned rotate, unsigned accepted);

  /**
   * @param index the index of the label to evaluate
   * returns energy for label position by comparing it to other labels
   * Assumes perimeter_size is set and Pos is set.
   */
  WaypointLabelList::EnergyMeasure Energy(unsigned index);
  void Clear();

  /**
   * Should the new solution be accepted based on delta energy and randomness
   * @param new_energy
   * @param old_energy
   * @param current_time. annealing time [1 down to 0]
   * @return true if new energy is better.  Else, randomly accept bad solutions
   * more earlier on.
   */
  bool Accept(WaypointLabelList::EnergyMeasure &new_energy,
              WaypointLabelList::EnergyMeasure &old_energy,
              fixed current_time);

  void ClearRandSeed() {
    rand_seed = 1;
  }
  /**
   * returns sequence of semi-random numbers [0..1]
   */
  fixed MyRand();

  /** returns the label position around anchor based on [0..7] position input */
  RasterPoint GetDiscreteLabelPosition(unsigned index, unsigned lab_pos);

#ifdef DISCRETE_POSITION
  /**
   * moves random label to a discretely relatively preset position around anchor
   * Accepts or rejects the move based on its relative energy, and annealing
   * temperature
   */
  unsigned MoveDiscrete(fixed current_time);
#else
  /**
   * Annealing move algorithm that uses vector direction push
   */
  unsigned Move(fixed current_time);
  unsigned Rotate(fixed current_time);
#endif

  bool Intersect(unsigned x1, unsigned x2, unsigned x3, unsigned x4,
                 unsigned y1, unsigned y2, unsigned y3, unsigned y4);
  fixed CoolingSchedule(fixed current_time, fixed init_time, unsigned num_sweeps);

};

#endif
