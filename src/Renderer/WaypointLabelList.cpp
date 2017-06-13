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

#include "WaypointLabelList.hpp"
#include "Renderer/TextInBox.hpp"
#include "Util/StringAPI.hxx"
#include "Util/StringUtil.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Font.hpp"

#include <string.h>

#include <algorithm>

static constexpr PixelScalar WPCIRCLESIZE = 2;

int
WaypointLabelList::EnergyMeasure::ScaledEnergy() const
{
  return energy / energy_scale;
}

WaypointLabelList::EnergyMeasure::EnergyMeasure()
  :energy(-1), push_down(0), push_right(0), sum_overlap(-1),
  count_overlap(-1), energy_scale(Layout::Scale(1) * Layout::Scale(1)) {}


bool
WaypointLabelList::Label::Visible() const {
  return (energy.ScaledEnergy() < 2000 || inTask) &&
      !hidden;
}

bool
WaypointLabelList::Label::GetCornerForLine(RasterPoint &p) const
{
  switch (this->discrete_position) {
  case 3:
  case 4:
    p = (const RasterPoint){Pos.x, Pos.y + perimeter_size.cy};
    return true;
  case 7:
    p = (const RasterPoint){Pos.x, Pos.y};
    return true;
  }
  p = anchor;
  return false;
}

gcc_pure
static bool
MapWaypointLabelListCompare(const WaypointLabelList::Label &e1,
                            const WaypointLabelList::Label &e2)
{
  if (e1.inTask && !e2.inTask)
    return true;

  if (!e1.inTask && e2.inTask)
    return false;

  if (e1.isAirport && !e2.isAirport)
    return true;

  if (!e1.isAirport && e2.isAirport)
    return false;

  if (e1.isLandable && !e2.isLandable)
    return true;

  if (!e1.isLandable && e2.isLandable)
    return false;

  if (e1.isWatchedWaypoint && !e2.isWatchedWaypoint)
    return true;

  if (!e1.isWatchedWaypoint && e2.isWatchedWaypoint)
    return false;

  if (e1.AltArivalAGL > e2.AltArivalAGL)
    return true;

  if (e1.AltArivalAGL < e2.AltArivalAGL)
    return false;

  return false;
}

gcc_pure
static bool
MapWaypointLabelListCompareName(const WaypointLabelList::Label &e1,
                                const WaypointLabelList::Label &e2)
{
  int r = StringCollate(e1.Name, e2.Name, std::min(StringLength(e1.Name), StringLength(e2.Name)));
  if (r > 0)
    return false;
  if (r < 0)
    return true;

  return MapWaypointLabelListCompare(e1, e2);
}

gcc_pure
static bool
MapWaypointLabelListCompareYPosition(const WaypointLabelList::Label &e1,
                                     const WaypointLabelList::Label &e2)
{
  return e1.anchor.y < e2.anchor.y;
}

gcc_pure
static bool
MapWaypointLabelListCompareID(const WaypointLabelList::Label &e1,
                              const WaypointLabelList::Label &e2)
{
  return e1.wp_id < e2.wp_id;
}

void
WaypointLabelList::Add(unsigned wp_id,
                       const TCHAR *Name, PixelScalar X, PixelScalar Y,
                       RasterPoint _anchor,
                       unsigned _anchor_radius,
                       TextInBoxMode Mode, bool bold,
                       RoughAltitude AltArivalAGL, bool inTask,
                       bool isLandable, bool isAirport, bool isWatchedWaypoint)
{
  if ((X < - WPCIRCLESIZE)
      || X > PixelScalar(width + (WPCIRCLESIZE * 3))
      || (Y < - WPCIRCLESIZE)
      || Y > PixelScalar(height + WPCIRCLESIZE))
    return;

  if (labels.full())
    return;
  if (StringLength(Name) == 0)
    return;

  auto &l = labels.append();

  _tcscpy(l.Name, Name);
  l.Pos.x = X;
  l.Pos.y = Y;
  l.Pos_arranged = l.Pos;
  l.anchor = _anchor;
  l.anchor_radius = _anchor_radius;
  l.use_right_corner = false;
  l.discrete_position = 0;
  l.x_flipped = false;
  l.hidden = false;
  l.Mode = Mode;
  l.AltArivalAGL = AltArivalAGL;
  l.bold = bold;
  l.inTask = inTask;
  l.isLandable = isLandable;
  l.isAirport  = isAirport;
  l.isWatchedWaypoint = isWatchedWaypoint;
  l.wp_id = wp_id;
}

void
WaypointLabelList::Init(PixelScalar screen_width, PixelScalar screen_height)
{
  width = screen_width;
  height = screen_height;
}

void
WaypointLabelList::SetSize(Canvas &canvas, const Font &font, const Font &font_bold)
{
  max_label_width = 0;
  for (auto &ll : labels) {
    canvas.Select(ll.bold ? font_bold : font);
    PixelSize text_size;
    ll.perimeter_size = TextInBoxGetSize(canvas, ll.Name, ll.Mode, text_size);
    max_label_width = std::max(max_label_width, (unsigned)ll.perimeter_size.cx);
  }
}

void
WaypointLabelList::Sort()
{
  std::sort(labels.begin(), labels.end(),
            MapWaypointLabelListCompare);
}

void
WaypointLabelList::Set(const WaypointLabelList &other)
{
  // remove if other list is smaller than this one
  while (labels.size() > other.labels.size()) {
    labels.remove(labels.size() - 1);
  }

  // ensure each items is identical
  for (unsigned i = 0; i < other.labels.size(); ++i) {
    if (i >= labels.size()) {
      // that task is larger than this
      labels.append(other.labels[i]);
    } else {
      // that task point is changed
      labels[i] = other.labels[i];
    }
  }
}

void
WaypointLabelList::RemoveHidden()
{
  if (labels.size() == 0)
    return;

  for (int i = labels.size() - 1; i >= 0; --i)
    if (labels[i].hidden)
      labels.remove(i);
}

void
WaypointLabelList::SortName()
{
  std::sort(labels.begin(), labels.end(),
            MapWaypointLabelListCompareName);
}

void
WaypointLabelList::SortY()
{
  std::sort(labels.begin(), labels.end(),
            MapWaypointLabelListCompareYPosition);
}

void
WaypointLabelList::SortID()
{
  std::sort(labels.begin(), labels.end(),
            MapWaypointLabelListCompareID);
}

void
WaypointLabelList::MergeLists(WaypointLabelList &labels_new, bool pre_calc)
{
  SortID();
  labels_new.SortID();
  WaypointLabelList labels_merged;

  unsigned old_size = labels.size();
  unsigned i = 0;
  unsigned j = 0;

  while (i < old_size && j < labels_new.labels.size()) {

    WaypointLabelList::Label &lab = labels[i];
    WaypointLabelList::Label &lab_new = labels_new.labels[j];

    // update text, mode, bold, etc.
    // leave perimeter_size - it will be recalculated
    // if pre_cal, update anchor, else update label.Pos for drag/pan
    if (lab.wp_id == lab_new.wp_id) {
      //TEMP DEBUG CODE:
      CopyString(lab.Name, lab_new.Name, NAME_SIZE);
      lab.Mode = lab_new.Mode;
      lab.AltArivalAGL = lab_new.AltArivalAGL;
      lab.bold = lab_new.bold;
      lab.inTask = lab_new.inTask;
      lab.isAirport = lab_new.isAirport;
      lab.isLandable = lab_new.isLandable;
      lab.isWatchedWaypoint = lab_new.isWatchedWaypoint;
      lab.hidden = false;

      if (pre_calc) {
        lab.anchor = lab_new.anchor;
      } else {
        // shift lab position by delta between new/old anchors
        lab.Pos = lab.Pos_arranged - lab.anchor + lab_new.anchor;
      }
      ++i;
      ++j;

    } else if (lab.wp_id > lab_new.wp_id) {
      // add to list
      if (!labels.full()) {
        labels.append(lab_new);
      }
      ++j;

    } else {
      // lab.wp_id < lab_new.wp_id
      // no longer there, so mark has hidden
      lab.hidden = true;
      ++i;
    }
  }
  while(i < old_size) {
    labels[i].hidden = true;
    i++;
  }
  for (unsigned j1 = j; j1 < labels_new.labels.size(); ++j1) {
    if (!labels.full()) {
      labels.append(labels_new[j1]);
    }
  }

  if (pre_calc || labels.full())
    RemoveHidden();
}

bool
WaypointLabelList::Dedupe()
{
  if (labels.size() < 2)
    return false;

  SortName();
  Label *current = nullptr;
  Label *last = nullptr;
  unsigned removed = 0;

  for (unsigned i = 0; i < labels.size(); ++i) {
    current = &labels[i];

    if (last != nullptr) {
      while (i < labels.size()
          && last->anchor.x == current->anchor.x
          && last->anchor.y == current->anchor.y
          && StringIsEqual(
              last->Name, current->Name,
              std::min(StringLength(last->Name), StringLength(current->Name)))) {
        labels.remove(i);
        ++removed;
      }
    }
    last = current;
  }
  return removed > 0;
}
