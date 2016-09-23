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

#ifndef XCSOAR_MAP_WIDGET_OVERLAYS_HPP
#define XCSOAR_MAP_WIDGET_OVERLAYS_HPP

#include "Util/StaticArray.hpp"
#include "Screen/Point.hpp"

class Widget;
class ContainerWindow;

/**
 * A class that contains and partially manages a groups of widgets that are
 * displayed on the map concurrently.
 * All widgets must be added to the collection should be not be Initialised.
 * It calls Leave(), Hide(), Unprepare for all it's widgets in it's destructor
 *
 */
class MapWidgetOverlays {

private:
  enum
  {
    MAX_ITEMS = 32,
  };

  StaticArray<Widget*, MAX_ITEMS> widget_list;
protected:

public:
  ~MapWidgetOverlays();

  /**
   * returns true if any widgets in the collection are visible
   */
  bool IsVisible();

  /**
   * Shows or hides each of the widgets based on these parameters
   * @rc. the rc of the full screen map window
   * @is_panning.  is the map in panning mode
   * @is_main_window_widget. is the mainWindow's widget non-NULL
   * @is_map. is the map non-NULL
   * @is_top_widget the top widget visible.
   */
  void UpdateVisibility(const PixelRect &rc_full_screen, bool is_panning,
                        bool is_main_window_widget, bool is_map,
                        bool is_full_screen, bool is_top_widget);

  /**
   * Adds an Uninitialised widget to the list of widgets
   * @param rc_map the rect of the map window with respect to the main window
   */
  void Add(Widget *widget, const PixelRect &rc_map);


  /** Initialises all the widgets
   * @param rc Rect of the Map window
   */
  void Initialise(ContainerWindow &parent, const PixelRect &rc);

  /** prepares all the widgets
   * @param rc Rect of the Map window
   */
  void Prepare(ContainerWindow &parent, const PixelRect &rc);

  /**
   * Unprepares all the widgets
   */
  void Unprepare();

  /**
   * shows all the widgets
   * @param rc Rect of the Map window
   */
  void Show(const PixelRect &rc);

  /**
   * returns false if any of the widgets returns false
   */
  bool Leave();

  /**
   * hides all the widgets
   */
  void Hide();

  /**
   * resizes all the overlays to fit in the map window
   * Each widget must know how to size and position itself
   * based on the Rect of the map
   * Skips if not visible
   * @param rc Rect of the Map window
   */
  void Move(PixelRect rc_map);

  /**
   * What is the max height from top of screen used of all widgets in the collection
   */
  UPixelScalar HeightFromTop();

  /**
   * What is the max height from bottom left of screen used of
   * all widgets in the collection
   */
  UPixelScalar HeightFromBottomLeft();

  /**
   * What is the max height from bottom right of screen used of
   * all widgets in the collection
   */
  UPixelScalar HeightFromBottomRight();

  /**
   * What is the max height from bottom of screen used of
   * all widgets in the collection (max of left and right)
   */
  UPixelScalar HeightFromBottomMax();

};

#endif
