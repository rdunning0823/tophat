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

#include "LabelArranger.hpp"
#include "Renderer/WaypointLabelList.hpp"
#include "Screen/Canvas.hpp"
#include "Math/Angle.hpp"
#include "Screen/Layout.hpp"
#include "Util/StaticString.hxx"
#include "Util/ConvertString.hpp"
#ifdef SHOW_RUN_STATS
#include "Screen/StopWatch.hpp"
#include "LogFile.hpp"
#endif


Labeler::Labeler(const Canvas &canvas, WaypointLabelList &list)
:lab(list.GetLabels()), w(canvas.GetWidth()), h(canvas.GetHeight()),
                                                max_label_width(list.max_label_width)
{
  Clear();
}

void
Labeler::Clear()
{
  max_move_x = fixed(5);
  max_move_y = fixed(4);
  max_angle = fixed(60);
  acc = 0;
  acc_worse = 0;
  rej = 0;

  rot_acc = 0;
  rot_acc_worse = 0;
  rot_rej = 0;

  mov_discrete_acc = 0;
  mov_discrete_worse = 0;
  mov_discrete_rej = 0;

  w_len = 1;
  w_inter = 20;
  w_lab2 = 30;
  w_lab_anc = 40;
  w_orient = 3;
  w_off_page = 5;

  allow_flip = false;
  zero_energy_margin = 5;

  label_height = -1;

  energy_counter = 0;
  rotate_counter = 0;
  move_counter = 0;
  move_discrete_counter = 0;
  intersect_counter = 0;
  energy_loop_counter_potential = 0;
  energy_loop_counter_actual = 0;

  total_energy_raw = 0;

  ClearRandSeed();
}

// LCM pseudo-random number generator.
// Outputs numbers from 0..1 inclusive.
// With protection against identical sequences.
//   due to full 32-bit cycle but only returning 15 bits.
fixed
Labeler::MyRand()
{
  const static uint32_t a = 214013U;
  const static uint32_t c = 2531011U;
  // m is, of course, 2^32 since the values will wrap.

  rand_seed = rand_seed * a + c;
  fixed r = fixed((rand_seed >> 16) & 0x7FFF) / fixed(32767);
  return r;
}

inline void
Labeler::EnergyLoop1(unsigned index, unsigned i, const PixelRect& label1,
                     WaypointLabelList::EnergyMeasure &ener)

{
  assert(lab.size() > 0);
  assert(index < lab.size());
  assert(i < lab.size());

  const WaypointLabelList::Label &l = lab[index];
  const WaypointLabelList::Label &l2 = lab[i];


  // static for speed
  static PixelRect label2;
  static bool intersect;
  static int sum_overlap; // sum of all overlap w/ LAB
  static int count_overlap; // count of all items that overlap with LAB
  static int x_overlap;
  static int y_overlap;
  static int overlap_area;
  static int push_down;
  static int push_right;

  intersect = true;
  push_down = 0;
  push_right = 0;
  sum_overlap = 0;
  count_overlap = 0;

  if (i != index) {
    // penalty for intersection of leader lines

    intersect = Intersect(l.anchor.x, l.Pos.x,
                          l2.anchor.x, l2.Pos.x, l.anchor.y,
                          l.Pos.y, l2.anchor.y, l2.Pos.y);
    if (intersect) {
      ener.energy += w_inter * Layout::Scale(1);
    }

    if (std::abs(l2.Pos.y - l.Pos.y) < label_height) {
      // penalty for label-label overlap
      label2.left = l2.Pos.x;
      label2.top = l2.Pos.y;
      label2.right = l2.Pos.x + l2.perimeter_size.cx;
      label2.bottom = l2.Pos.y + l2.perimeter_size.cy;

      x_overlap = std::max(0, (int)std::min(label2.right, label1.right) - (int)std::max(label2.left, label1.left));
      y_overlap = std::max(0, (int)std::min(label2.bottom, label1.bottom) - (int)std::max(label2.top, label1.top));
      overlap_area = x_overlap * y_overlap;

      if (overlap_area > 0) {
        push_right = x_overlap * ((l.Pos.x > l2.Pos.x) ? 1 : -1);
        push_down = y_overlap * ((l.Pos.y > l2.Pos.y) ? 1 : -1);
        ener.push_right += push_right;
        ener.push_down += push_down;

        ener.energy += (overlap_area * w_lab2);
        sum_overlap += overlap_area;
        ++count_overlap;
      }
    }
  }

  push_right = push_down = 0;
  // penalty for label-anchor overlap
  PixelRect anchor((const PixelRect){
    l2.anchor.x - (int)l2.anchor_radius,
    l2.anchor.y - (int)l2.anchor_radius,
    l2.anchor.x + (int)l2.anchor_radius,
    l2.anchor.y + (int)l2.anchor_radius });

  x_overlap = std::max(0, (int)std::min(anchor.right, label1.right) - (int)std::max(anchor.left, label1.left));
  y_overlap = std::max(0, (int)std::min(anchor.bottom, label1.bottom) - (int)std::max(anchor.top, label1.top));
  overlap_area = x_overlap * y_overlap;

  if (overlap_area > 0) {
    push_down = y_overlap * (((l.Pos.y + l.perimeter_size.cy / 2) > l2.anchor.y) ? 1 : -1);
    push_right = x_overlap * (((l.Pos.x + l.perimeter_size.cx / 2) > l2.anchor.x) ? 1 : -1);
    ener.push_down += push_down;
    ener.push_right += push_right;

    ener.energy += (overlap_area * w_lab_anc);
    sum_overlap += overlap_area;
    ++count_overlap;
  }

  ener.sum_overlap += sum_overlap;
  ener.count_overlap += count_overlap;

  ++energy_loop_counter_actual;
}

/*
 * Returns energy related to discrete position
 *            4  3  2     2 3 1
 *            5  A  1     2   1
 *            6  7  0     2 3 1
 */
static inline int
GetDiscreteLabelPositionEnergy(unsigned lab_pos, int w_orient)
{
  int e = 0;
  switch(lab_pos) {
  case 0:
    e = 0;
    break;
  case 1:
    e = 0;
    break;
  case 2:
    e = 0;
    break;
  case 3:
    e = 2;
    break;
  case 4:
    e = 1;
    break;
  case 5:
    e = 1;
    break;
  case 6:
    e = 1;
    break;
  case 7:
    e = 2;
    break;
  }

  return w_orient * Layout::Scale(e) * Layout::Scale(1);
}

/**
 * computes energy based on distance from label to anchor
 * @param l: the label
 * @param  bool &use_right_corner.  Set this if the anchor is closer
 * to the right side of the label than the left
 * @param w_len: weight for distance
 */
static inline int
GetDistanceEnergy(const WaypointLabelList::Label &l, bool &use_right_corner, int w_len)
{
  // compute distance energy
  int dx = std::abs(l.Pos.x - l.anchor.x);
  int dx_left = std::abs(l.Pos.x + l.perimeter_size.cx - l.anchor.x);

  dx = std::min(dx, dx_left);
  use_right_corner = dx != dx_left;
  if (dx < l.anchor_radius) {
    dx = 0;
  } else {
    dx -= l.anchor_radius;
  }

  int dx_flip = std::abs(l.Pos.x - l.perimeter_size.cx - l.anchor.x);
  if (dx_flip < l.anchor_radius) {
    dx_flip = 0;
  }

  int dy = std::abs(l.anchor.y - l.Pos.y);
  if (dy < l.anchor_radius) {
    dy = 0;
  }

  int dist = (int)(sqrt(dx * dx + dy * dy));
  int dist_flip = (int)(sqrt(dx_flip * dx_flip + dy * dy));
  dist = std::min(dist, dist_flip);

  /* delta energy is later dived by Scale(1)^2, and since dist has only
   * 1 dimension it needs to be multiplied by Scale(1) to be comparble
   */
  if (dist > 0)
    return dist * w_len * Layout::Scale(1);

  return 0;
}

inline WaypointLabelList::EnergyMeasure
Labeler::Energy(unsigned index)
{
  assert(lab.size() > 0);
  assert(index < lab.size());
  const WaypointLabelList::Label &l = lab[index];

  energy_clock.Update();

  WaypointLabelList::EnergyMeasure ener;
  ener.energy = 0; // initialize
  ener.sum_overlap = 0; // this is set only by loops below
  ener.count_overlap = 0; // this is set only by loops below

#ifdef DISCRETE_POSITION
  ener.energy += GetDiscreteLabelPositionEnergy(l.discrete_position, w_orient);
#else
  ener.energy += GetDistanceEnergy(l, lab[index].use_right_corner, w_len);
#endif

  //off page to left energy
  if (l.Pos.x < 0) {
    // TODO increase weight if label will be forced to be on page by TextInBox (text_mode.move_in_view)
    int off_page_energy = -1 * l.Pos.x * w_off_page * Layout::Scale(1); // scale energy b/c only one screen dimension
    if (l.Mode.move_in_view)
      off_page_energy *= 5;
    ener.energy += off_page_energy;

    int page_overlap = -l.Pos.x * label_height;
    ener.push_right = page_overlap;
    ++ener.count_overlap;
    ener.sum_overlap += page_overlap;

  } else  if (l.perimeter_size.cx + l.Pos.x > w) {
    //off page to right energy
    int right = l.perimeter_size.cx + l.Pos.x;
    int off_page_energy = (right - w) * w_off_page * Layout::Scale(1); // scale energy b/c only one screen dimension
    ener.energy += off_page_energy;

    int page_overlap = (right - w) * label_height;
    ener.push_right = -page_overlap;
    ++ener.count_overlap;
    ener.sum_overlap += page_overlap;
  }

  const PixelRect label1((const PixelRect)
      { l.Pos.x, l.Pos.y,
        l.Pos.x + l.perimeter_size.cx,
        l.Pos.y + l.perimeter_size.cy });

  const int height_filter = label_height * 2;
  const int width_filter = max_label_width + 2 * l.anchor_radius;

  int top_y_threshhold = std::max(0, (int)(l.anchor.y - height_filter));
  int bottom_y_threshhold = std::max(0, (int)(l.anchor.y + height_filter));
  unsigned lab_size = lab.size();

  // loop y descending (inclusive of self)
  for (unsigned i = index;; --i) {
    ++energy_loop_counter_potential;
    if (lab[i].anchor.y < top_y_threshhold) {
      break;
    }
    if (std::abs(lab[i].anchor.x - l.anchor.x) < width_filter) {
      EnergyLoop1(index, i, label1, ener);
    }
    if (i == 0)
      break;
  }

  // repeat, y ascending
  if ((index + 1) < lab_size) {
    for (unsigned i = index + 1; i < lab_size; ++i) {
      ++energy_loop_counter_potential;
      if (lab[i].anchor.y > bottom_y_threshhold) {
        break;
      }
      if (std::abs(lab[i].anchor.x - l.anchor.x) < width_filter) {
        EnergyLoop1(index, i, label1, ener);
      }
    }
  }
  energy_counter += energy_clock.Elapsed();
  return ener;
};

inline bool
Labeler::Accept(WaypointLabelList::EnergyMeasure &new_energy,
                WaypointLabelList::EnergyMeasure &old_energy, fixed current_time)
{
  if (new_energy.energy < old_energy.energy)
    return true;
  else
    return MyRand() <
        fixed(std::exp(float(fixed(old_energy.ScaledEnergy() - new_energy.ScaledEnergy()) / current_time)));
}

inline RasterPoint
Labeler::GetDiscreteLabelPosition(unsigned index, unsigned lab_pos)
{
  WaypointLabelList::Label &l = lab[index];
  int lab_width = l.perimeter_size.cx;
  int lab_height = l.perimeter_size.cy;
  int radius = l.anchor_radius;
  int lab_height_half = lab_height / 2;

  /*
   *            4  3  2
   *            5  A  1
   *            6  7  0
   */
  RasterPoint p_offset((const RasterPoint){0,0});
  switch (lab_pos) {
  case 0:
    p_offset = RasterPoint {radius, radius};
  break;
  case 1:
    p_offset = RasterPoint {radius, 0};
  break;
  case 2:
    p_offset = RasterPoint {radius, -radius};
  break;
  case 3:
    p_offset = RasterPoint {(-lab_width / 2), -lab_height_half - radius};
  break;
  case 4:
    p_offset = RasterPoint {-(lab_width - radius), -radius};
  break;
  case 5:
    p_offset = RasterPoint {-(lab_width - radius), -radius};
  break;
  case 6:
    p_offset = RasterPoint {-lab_width - radius, 0};
  break;
  case 7:
    p_offset = RasterPoint {(-lab_width / 2), lab_height_half + radius};
    break;
  }

  p_offset.y -= lab_height_half; // offset because label is drawn below point
  p_offset.x += Layout::Scale(2);

  l.discrete_position = lab_pos;
  return l.anchor + p_offset;
}

#ifdef DISCRETE_POSITION
unsigned
Labeler::MoveDiscrete(fixed current_time)
{
  move_discrete_clock.Update();

  unsigned i = std::floor(double(MyRand() * fixed(lab.size())));
  assert(lab.size() > 0);
  assert(i < lab.size());
  WaypointLabelList::Label &l = lab[i];

  if (l.energy.sum_overlap == 0) {
    return 0;
  }

  // save old coordinates
  int x_old = l.Pos.x;
  int y_old = l.Pos.y;
  unsigned lab_pos_old = l.discrete_position;

  WaypointLabelList::EnergyMeasure old_energy = l.energy;
  old_energy = Energy(i);

  unsigned lab_pos = std::floor(double(MyRand() * fixed(8))); // 0..7
  l.Pos = GetDiscreteLabelPosition(i, lab_pos);

  WaypointLabelList::EnergyMeasure new_energy = Energy(i);

  bool accept = Accept(new_energy, old_energy, current_time);
  if (accept) {

    ++mov_discrete_acc;
    if (new_energy.energy > old_energy.energy)
      ++mov_discrete_worse;

    l.energy = new_energy;
    total_energy_raw -= old_energy.energy;
    total_energy_raw += l.energy.energy;

    l.Pos_arranged = l.Pos;


  } else {
    // move back to old coordinates
    l.Pos.x = x_old;
    l.Pos.y = y_old;
    l.discrete_position = lab_pos_old;
    mov_discrete_rej += 1;
  }

  move_discrete_counter += move_discrete_clock.Elapsed();
  return accept ? 1 : 0;
}
#else
unsigned
Labeler::Move(fixed current_time)
{
  move_clock.Update();

  // select a random label
  unsigned i = std::floor(double(MyRand() * fixed(lab.size())));
  assert(lab.size() > 0);
  assert(i < lab.size());
  WaypointLabelList::Label &l = lab[i];

  if (l.energy.Initialized() && l.energy.ScaledEnergy() < zero_energy_margin) {
    return 0;
  }

  // save old coordinates
  int x_old = l.Pos.x;
  int y_old = l.Pos.y;

  WaypointLabelList::EnergyMeasure old_energy = l.energy;
  if (true /*old_energy < 0*/)
    old_energy = Energy(i);

  // random translation
  int dx = 0;
  int dy = 0;

  while (dy == 0) {
    dy = int((MyRand() - fixed(0.5)) * max_move_y);

  }
  if (old_energy.push_down == 0) {
    dy /= 2;
  } else {
    dy *= ((dy * old_energy.push_down > 0) ? 1 : -1);
  }
  l.Pos.y += dy;

  // flip sign of dx to align with push pressure

  if (allow_flip &&
      !l.x_flipped &&
      old_energy.count_overlap > 2 &&
      old_energy.push_right < 0) {
    l.Pos.x = l.anchor.x - l.anchor_radius - l.perimeter_size.cx - Layout::Scale(3);
    if (l.Pos.x < 0)
      l.Pos.x /= 2;

    dx = l.Pos.x - x_old;
    l.x_flipped = true; // only try to flip once

  } else {
    while (dx == 0) {
      dx = int((MyRand() - fixed(0.5)) * max_move_x);
    }
    if (old_energy.push_right == 0) {
      dx /= 2;
    } else {
      dx *= ((dx * old_energy.push_right > 0) ? 1 : -1);
    }

    l.Pos.x += dx;
  }

  WaypointLabelList::EnergyMeasure new_energy = Energy(i);
  bool accept = Accept(new_energy, old_energy, current_time);
  if (accept) {
    ++acc;
    if (new_energy.energy > old_energy.energy)
      ++acc_worse;
    l.energy = new_energy;
    total_energy_raw -= old_energy.energy;
    total_energy_raw += l.energy.energy;

    l.Pos_arranged = l.Pos;
  } else {
    // move back to old coordinates
    l.Pos.x = x_old;
    l.Pos.y = y_old;
    if (!l.energy.Initialized())
      l.energy = old_energy; // sets the initialized flag
    ++rej;
  }

  move_counter += move_clock.Elapsed();
  return accept ? 1 : 0;
}

unsigned
Labeler::Rotate(fixed current_time)
{
  // Monte Carlo rotation move

  rotate_clock.Update();
  // select a random label
  unsigned i = (unsigned)std::floor(double(MyRand() * fixed(lab.size())));
  assert(lab.size() > 0);
  assert(i < lab.size());
  WaypointLabelList::Label &l = lab[i];

  if (l.energy.sum_overlap == 0) {
    return 0;
  }
  // save old coordinates
  int x_old = l.Pos.x;
  int y_old = l.Pos.y;

  WaypointLabelList::EnergyMeasure old_energy = l.energy;
  if (true /*old_energy < 0*/)
    old_energy = Energy(i);

  // random angle
  Angle angle(Angle::Degrees((MyRand() - fixed(0.5)) * max_angle));
  fir.SetAngle(angle);

  Point2D<fixed> p1(fixed(l.Pos.x - l.anchor.x),
                     fixed(l.Pos.y - l.anchor.y));
  p1 = fir.Rotate(p1);

  // translate label back
  l.Pos.x =  int(p1.x) + l.anchor.x;
  l.Pos.y = int(p1.y) + l.anchor.y;

  WaypointLabelList::EnergyMeasure new_energy = Energy(i);

  bool accept = Accept(new_energy, old_energy, current_time);
  if (accept) {
    ++rot_acc;
    if (new_energy.energy > old_energy.energy)
      ++rot_acc_worse;
    l.energy = new_energy;
    total_energy_raw -= old_energy.energy;
    total_energy_raw += l.energy.energy;

    l.Pos_arranged = l.Pos;

  } else {
    // move back to old coordinates
    l.Pos.x = x_old;
    l.Pos.y = y_old;
    ++rot_rej;
  }
  rotate_counter += rotate_clock.Elapsed();
  return accept ? 1 : 0;
}
#endif

bool
Labeler::Intersect(unsigned x1, unsigned x2, unsigned x3, unsigned x4,
                   unsigned y1, unsigned y2, unsigned y3, unsigned y4)
{
// returns true if two lines intersect, else false
// from http://paulbourke.net/geometry/lineline2d/

  fixed mua, mub;
  int denom, numera, numerb;
  intersect_clock.Update();

  denom = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);
  numera = (x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3);
  numerb = (x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3);

  /* Is the intersection along the the segments */
  if (denom == 0) {
    intersect_counter += intersect_clock.Elapsed();
    return false;
  }

  mua = fixed(numera / denom);
  mub = fixed(numerb / denom);

  if (!(negative(mua) || mua > fixed(1) || negative(mub) || mub > fixed(1))) {
    return true;
  }
  return false;
}

fixed
Labeler::CoolingSchedule(fixed current_time, fixed init_time, unsigned num_sweeps)
{
// linear cooling
  return (current_time - (init_time / fixed(num_sweeps)));
}

void
Labeler::SetMaxMove(unsigned list_size)
{
  if (list_size > 0)
    label_height = lab[0].perimeter_size.cy;
  if (label_height < 8)
    label_height = 8;

  max_move_x = fixed(label_height * 1.5 );
  max_move_y = max_move_x;
}

void
Labeler::CalcInitialEnergyAndPosition()
{
  const unsigned labs = lab.size();
  total_energy_raw = 0;
  for (unsigned i = 0; i < labs; ++i) {
    WaypointLabelList::Label &l = lab[i];
#ifdef DISCRETE_POSITION
    l.Pos = l.Pos_arranged = GetDiscreteLabelPosition(i, l.discrete_position);
#else
    if (!l.energy.Initialized()) {
      l.Pos = l.Pos_arranged = GetDiscreteLabelPosition(i, 0); // new labels
    }
#endif
    l.energy = Energy(i);
    total_energy_raw += l.energy.energy;
  }
}

void
Labeler::Start()
{
// main simulated annealing function
  unsigned labs = lab.size();
#ifdef SHOW_RUN_STATS
  ScreenStopWatch start_sw;
  StaticString<100>stop_watch_text;
#endif
  SetMaxMove(labs);

  /* the larger this is, the more probable a worse solutions are accepted */
  fixed init_time = fixed(200);
  fixed current_time = init_time;

  unsigned move = 0;
  unsigned rotate = 0;
  unsigned accepted;

#ifdef ENABLE_HORIZONTAL_FLIP
  allow_flip = true;
#endif
#ifdef DISCRETE_POSITION
  unsigned num_sweeps = 10;
  const bool no_rotate = true;
#else
  unsigned num_sweeps = 6;
  const bool no_rotate = false;
#endif
#ifdef ACCEPTED_PERCENT_LIMIT
  num_sweeps = num_sweeps * 1.2;
#endif

  CalcInitialEnergyAndPosition();

  for (unsigned i = 0; i < num_sweeps; i++) {
#ifdef SHOW_RUN_STATS
    stop_watch_text.Format(_T("num_sweeps:%i i:%i tot_e_scaled:%i e/lab:%i"), num_sweeps, i,
             total_energy_raw / Layout::Scale(1) / Layout::Scale(1),
             total_energy_raw / Layout::Scale(1) / Layout::Scale(1) / (labs > 0 ? labs : 1));
#ifdef _UNICODE
    start_sw.Mark(ConvertWideToUTF8(stop_watch_text.c_str()));
#else
    start_sw.Mark(stop_watch_text.c_str());
#endif
#endif

    accepted = 0;
    for (unsigned j = 0; j < labs; j++) {
      if (no_rotate || (MyRand()) < fixed(0.5)) {
        ++move;
#ifdef DISCRETE_POSITION
        accepted += MoveDiscrete(current_time);
#else
        accepted += Move(current_time);
      } else {
        accepted += Rotate(current_time);
        ++rotate;
#endif
      }
#ifdef ACCEPTED_PERCENT_LIMIT
      if ((labs > 15) && (accepted > labs / 5)) {
        ShowRunStats(_T("                  "), move, rotate, accepted);
        break;
      }
#endif
    }

    current_time = CoolingSchedule(current_time, init_time, num_sweeps);
  }

#ifdef SHOW_RUN_STATS
  ShowRunStats(_T("Labeler::START EXIT"), move, rotate, accepted);
  start_sw.Finish();
#endif
}

void
Labeler::ShowRunStats(const TCHAR *text, unsigned move, unsigned rotate, unsigned accepted)
{
#ifdef SHOW_RUN_STATS
  LogFormat(_T("%s tot_e:%i e/l:%i acc:%u acc_worse:%u rej:%u | disc_acc:%i, disc_wrse:%i, disc_rej:%i | rot_acc:%u r_acc_worse:%u r_rej:%u"),
          text, total_energy_raw / Layout::Scale(1) / Layout::Scale(1), total_energy_raw / Layout::Scale(1) / Layout::Scale(1) / lab.size(),
          acc, acc_worse, rej,
          mov_discrete_acc, mov_discrete_worse, mov_discrete_rej,
          rot_acc, rot_acc_worse, rot_rej);
  LogFormat(_T("                     move:%u rot:%u | e_clock:%i rot_clock:%i move_clock:%i mdisc_clock:%i"),
            move, rotate,
            energy_counter, rotate_counter, move_counter, move_discrete_counter);
#endif
}
