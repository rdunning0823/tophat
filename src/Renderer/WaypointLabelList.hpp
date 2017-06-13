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

#ifndef XCSOAR_WAYPOINT_LABEL_LIST_HPP
#define XCSOAR_WAYPOINT_LABEL_LIST_HPP

#include "Rough/RoughAltitude.hpp"
#include "Renderer/TextInBox.hpp"
#include "Screen/Point.hpp"
#include "Util/NonCopyable.hpp"
#include "Util/StaticArray.hpp"
#include "Sizes.h" /* for NAME_SIZE */

#include <tchar.h>

class Canvas;
class Font;

class WaypointLabelList : private NonCopyable {
public:

  /**
   * A struct to store information about the energy of the annealing system.
   * Larger energy means more overlaps and unfavorable position of the label
   */
  struct EnergyMeasure {
  public:
    int energy; /* label arranger raw energy, -1 == uninitialized */
    int push_down;
    int push_right;
    int sum_overlap; /* total overlap with anything, or -1 */
    int count_overlap; /* number of items that overlap with label or -1 */
    int energy_scale; /* screen scale squared */

    EnergyMeasure();
    void Clear() {
      energy = -1;
    }

    bool Initialized() {
      return energy > -1;
    }
    /** because energy is created based on pixel distances, the raw energy
     * is implicitly multiplied screen scale squared for all area measures of energy.
     * When linear calculations of energy are added, they must be multiplied
     * by Layout::Scale(1) before being added to raw energy.
     */
    int ScaledEnergy() const;
  };

  struct Label{
    TCHAR Name[NAME_SIZE+1];
    unsigned wp_id;  // or zero if undefined
    RasterPoint Pos;
    /* set by the arranging algorithm, used by the "Pan" non-calc merge */
    RasterPoint Pos_arranged;
    TextInBoxMode Mode;
    PixelSize perimeter_size;
    RasterPoint anchor;
    int anchor_radius;
    bool use_right_corner; /* the right corner is closer to the anchor than the */
    unsigned discrete_position; /* when using discrete position algorithm */
    EnergyMeasure energy;
    bool x_flipped; /* has position been flipped across y axis */
    RoughAltitude AltArivalAGL;
    bool inTask;
    bool isLandable;
    bool isAirport;
    bool isWatchedWaypoint;
    bool bold;
    bool hidden;
    bool Visible() const;
    /**
     * return. true if a line should be drawn from the anchor to the label
     * @param p.  sets to the endpoint of the line to be drawn
     */
    bool GetCornerForLine(RasterPoint &p) const;
  };

  typedef StaticArray<Label, 128u> LabelArray;

protected:
  UPixelScalar width, height;
  PixelRect bounds;

  LabelArray labels;

public:
  /** contains width of max label */
  unsigned max_label_width;

  /**
   * This version must not be used until SetSize() is called
   */
  WaypointLabelList()
    :width(0), height(0), max_label_width(0) {}

  /** sets the screen dimensions
   * must be called prior to Add()
   */
  void Init(PixelScalar screen_width, PixelScalar screen_height);

  void Add(unsigned wp_id, const TCHAR *Name, PixelScalar X, PixelScalar Y,
           RasterPoint _anchor,
           unsigned _anchor_radius,
           TextInBoxMode Mode, bool bold,
           RoughAltitude AltArivalAGL,
           bool inTask, bool isLandable, bool isAirport,
           bool isWatchedWaypoint);

  /**
   * sets the perimeter_size parameters and max_label_width
   * @param canvas the canvas where the TextInBox will be displayed
   * @return font and bold font for drawing labels
   */
  void SetSize(Canvas &canvas, const Font &font, const Font &font_bold);

  /**
   * Sort by attributes
   */
  void Sort();
  /**
   * sort first by name, then by other attributes
   */
  void SortName();

  /** sort labels by Y direction top to bottom */
  void SortY();

  /** sort labels by wp_id */
  void SortID();

  /**
   * merges the new labels into the labels_last list.
   * Adds new labels
   * Marks lables no longer visible as hidden
   * Updates label text
   * changes order of list, so need to sort afterwards
   * @param labels_new.
   * @param pre_calc.  Set to true if label Calc routine will be run on resulting labels
   * set this to true if Calc will be run on the label set
   */
  void MergeLists(WaypointLabelList &labels_new, bool pre_calc);

  /**
   * Removes duplicate names if location is same
   * Returns true if any items are removed
   */
  bool Dedupe();
  LabelArray &GetLabels() {
    return labels;
  }

  /**
   * erases the current list and copies the items from other
   * @param other.  the other list
   */
  void Set(const WaypointLabelList &other);

  /** removes all hidden labels */
  /** changes order of list, so need to sort afterwards */
  void RemoveHidden();

  /**
   * Do the two lists have the same waypoints?
   * The two lists must be first sorted in same order before they are compared
   *
   */
  //bool EqualWayoints(const WaypointLabelList &other);

  const Label *begin() const {
    return labels.begin();
  }

  const Label *end() const {
    return labels.end();
  }

  const Label &operator[](unsigned i) const {
    return labels[i];
  }
};

#endif
