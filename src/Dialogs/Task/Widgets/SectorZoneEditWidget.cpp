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

#include "SectorZoneEditWidget.hpp"
#include "Engine/Task/ObservationZones/SectorZone.hpp"
#include "Engine/Task/ObservationZones/SymmetricSectorZone.hpp"
#include "Engine/Task/ObservationZones/AnnularSectorZone.hpp"
#include "Language/Language.hpp"
#include "Formatter/UserUnits.hpp"
#include "Formatter/AngleFormatter.hpp"
#include "Util/StaticString.hxx"

enum Controls {
  RADIUS,
  START_RADIAL,
  END_RADIAL,
  INNER_RADIUS,
};

SectorZoneEditWidget::SectorZoneEditWidget(SectorZone &_oz)
  :ObservationZoneEditWidget(_oz) {}

void
SectorZoneEditWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  ObservationZoneEditWidget::Prepare(parent, rc);

  const auto shape = GetObject().GetShape();
  WndProperty* wp;
  StaticString<255> edit_label;

  wp = AddFloat(_("Radius"), _("Radius of the OZ sector."),
                _T("%.1f %s"), _T("%.1f"),
                fixed(0.1), fixed(200), fixed(1), true,
                UnitGroup::DISTANCE, GetObject().GetRadius(),
                this);
  edit_label.Format(_T("%s: %s"), _("Radius"), GetWaypointName());
  wp->SetEditingCaption(edit_label.c_str());

  if (shape == ObservationZone::Shape::SYMMETRIC_QUADRANT) {
    AddDummy();
    AddDummy();
  } else {
    wp = AddAngle(_("Start radial"), _("Start radial of the OZ area"),
                  GetObject().GetStartRadial(), 10, true,
                  this);
    edit_label.Format(_T("%s: %s"), _("Start radial"), GetWaypointName());
    wp->SetEditingCaption(edit_label.c_str());

    wp = AddAngle(_("Finish radial"), _("Finish radial of the OZ area"),
                  GetObject().GetEndRadial(), 10, true,
                  this);
    edit_label.Format(_T("%s: %s"), _("Finish radial"), GetWaypointName());
    wp->SetEditingCaption(edit_label.c_str());
  }

  if (shape == ObservationZonePoint::Shape::ANNULAR_SECTOR) {
    const AnnularSectorZone &annulus = (const AnnularSectorZone &)GetObject();

    wp = AddFloat(_("Inner radius"), _("Inner radius of the OZ sector."),
                  _T("%.1f %s"), _T("%.1f"),
                  fixed(0.1), fixed(100), fixed(1), true,
                  UnitGroup::DISTANCE, annulus.GetInnerRadius(),
                  this);
    edit_label.Format(_T("%s: %s"), _("Inner radius"), GetWaypointName());
    wp->SetEditingCaption(edit_label.c_str());
  }
}

const TCHAR*
SectorZoneEditWidget::GetOzSummary()
{
  const auto shape = GetObject().GetShape();
  StaticString<25> r1;
  FormatUserDistance(GetObject().GetRadius(), r1.buffer(), true, 1);
  oz_summary = r1;

  if (shape == ObservationZonePoint::Shape::ANNULAR_SECTOR) {
    StaticString<25>r2;
    const AnnularSectorZone &annulus = (const AnnularSectorZone &)GetObject();
    FormatUserDistance(annulus.GetInnerRadius(), r2.buffer(), true, 1);
    oz_summary.AppendFormat(_T(" / %s"), r2.c_str());
  }

  if (shape != ObservationZone::Shape::SYMMETRIC_QUADRANT) {
    StaticString<25>a1;
    StaticString<25>a2;
    FormatBearing(a1.buffer(), 25, GetObject().GetStartRadial());
    FormatBearing(a2.buffer(), 25, GetObject().GetEndRadial());
    oz_summary.AppendFormat(_T(", %s / %s"), a1.c_str(), a2.c_str());
  }
  return oz_summary.c_str();
}

bool
SectorZoneEditWidget::Save(bool &_changed)
{
  const auto shape = GetObject().GetShape();
  bool changed = false;

  fixed radius = GetObject().GetRadius();
  if (SaveValue(RADIUS, UnitGroup::DISTANCE, radius)) {
    GetObject().SetRadius(radius);
    changed = true;
  }

  if (shape == ObservationZone::Shape::SYMMETRIC_QUADRANT) {
  } else {
    Angle radial = GetObject().GetStartRadial();
    if (SaveValue(START_RADIAL, radial)) {
      GetObject().SetStartRadial(radial);
      changed = true;
    }

    radial = GetObject().GetEndRadial();
    if (SaveValue(END_RADIAL, radial)) {
      GetObject().SetEndRadial(radial);
      changed = true;
    }
  }

  if (GetObject().GetShape() == ObservationZonePoint::Shape::ANNULAR_SECTOR) {
    AnnularSectorZone &annulus = (AnnularSectorZone &)GetObject();

    radius = annulus.GetInnerRadius();
    if (SaveValue(INNER_RADIUS, UnitGroup::DISTANCE, radius)) {
      annulus.SetInnerRadius(radius);
      changed = true;
    }
  }

  _changed |= changed;
  return true;
}
