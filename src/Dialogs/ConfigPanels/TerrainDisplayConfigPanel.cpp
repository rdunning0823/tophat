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

#include "TerrainDisplayConfigPanel.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Profile/Profile.hpp"
#include "Form/Draw.hpp"
#include "Form/DataField/Listener.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Boolean.hpp"
#include "Language/Language.hpp"
#include "MapSettings.hpp"
#include "Terrain/TerrainRenderer.hpp"
#include "Projection/MapWindowProjection.hpp"
#include "Components.hpp"
#include "Interface.hpp"
#include "MapWindow/GlueMapWindow.hpp"
#include "Form/RowFormWidget.hpp"
#include "UIGlobals.hpp"

class WndOwnerDrawFrame;

enum ControlIndex {
  EnableTerrain,
  EnableTopography,
  TerrainColors,
  TerrainSlopeShading,
  TerrainContrast,
  TerrainBrightness,
  TerrainPreview,
};

class TerrainDisplayConfigPanel : public RowFormWidget, DataFieldListener {

protected:
  TerrainRendererSettings terrain_settings;

public:
  TerrainDisplayConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);
  void ShowTerrainControls();
  void OnPreviewPaint(Canvas &canvas);

protected:
  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df);
};

/** XXX this hack is needed because the form callbacks don't get a
    context pointer - please refactor! */
static TerrainDisplayConfigPanel *instance;

void
TerrainDisplayConfigPanel::ShowTerrainControls()
{
  bool show = terrain_settings.enable;
  SetRowVisible(TerrainColors, show);
  SetRowVisible(TerrainSlopeShading, show);
  SetRowVisible(TerrainContrast, show);
  SetRowVisible(TerrainBrightness, show);
  if (terrain != NULL)
    SetRowVisible(TerrainPreview, show);
}

static short
ByteToPercent(short byte)
{
  return (byte * 200 + 100) / 510;
}

static short
PercentToByte(short percent)
{
  return (percent * 510 + 255) / 200;
}

void
TerrainDisplayConfigPanel::OnModified(DataField &df)
{
  if (IsDataField(EnableTerrain, df)) {
    const DataFieldBoolean &dfb = (const DataFieldBoolean &)df;
    terrain_settings.enable = dfb.GetAsBoolean();
    ShowTerrainControls();
  } else {
    terrain_settings.slope_shading = (SlopeShading)
      GetValueInteger(TerrainSlopeShading);
    terrain_settings.contrast = PercentToByte(GetValueInteger(TerrainContrast));
    terrain_settings.brightness =
      PercentToByte(GetValueInteger(TerrainBrightness));
    terrain_settings.ramp = GetValueInteger(TerrainColors);

    // Invalidate terrain preview
    if (terrain != NULL)
      ((WndOwnerDrawFrame &)GetRow(TerrainPreview)).Invalidate();
  }
}

void
TerrainDisplayConfigPanel::OnPreviewPaint(Canvas &canvas)
{
  TerrainRenderer renderer(terrain);
  renderer.SetSettings(terrain_settings);

  const GlueMapWindow *map = UIGlobals::GetMap();
  if (map == NULL)
    return;

  MapWindowProjection projection = map->VisibleProjection();
  projection.SetScreenSize(canvas.get_width(), canvas.get_height());
  projection.SetScreenOrigin(canvas.get_width() / 2, canvas.get_height() / 2);

  Angle sun_azimuth(Angle::Degrees(fixed(-45)));
  if (terrain_settings.slope_shading == SlopeShading::SUN &&
      XCSoarInterface::Calculated().sun_data_available)
    sun_azimuth = XCSoarInterface::Calculated().sun_azimuth;

  renderer.Generate(projection, sun_azimuth);
  renderer.Draw(canvas, projection);
}

static void
OnPreviewPaint(gcc_unused WndOwnerDrawFrame *Sender,
               Canvas &canvas)
{
  instance->OnPreviewPaint(canvas);
}

void
TerrainDisplayConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  instance = this;

  RowFormWidget::Prepare(parent, rc);

  const MapSettings &settings_map = CommonInterface::GetMapSettings();
  const TerrainRendererSettings &terrain = settings_map.terrain;

  AddBoolean(_("Terrain display"),
             _("Draw a digital elevation terrain on the map."),
             terrain.enable);
  GetDataField(EnableTerrain).SetListener(this);

  AddBoolean(_("Topography display"),
             _("Draw topographical features (roads, rivers, lakes etc.) on the map."),
             settings_map.topography_enabled);

  static constexpr StaticEnumChoice terrain_ramp_list[] = {
    { 0, N_("Low lands"), },
    { 1, N_("Mountainous"), },
    { 2, N_("Imhof 7"), },
    { 3, N_("Imhof 4"), },
    { 4, N_("Imhof 12"), },
    { 5, N_("Imhof Atlas"), },
    { 6, N_("ICAO"), },
    { 7, N_("Grey"), },
    { 0 }
  };

  AddEnum(_("Terrain colors"),
          _("Defines the color ramp used in terrain rendering."),
          terrain_ramp_list, terrain.ramp);
  GetDataField(TerrainColors).SetListener(this);

  static constexpr StaticEnumChoice slope_shading_list[] = {
    { (unsigned)SlopeShading::OFF, N_("Off"), },
    { (unsigned)SlopeShading::FIXED, N_("Fixed"), },
    { (unsigned)SlopeShading::SUN, N_("Sun"), },
    { (unsigned)SlopeShading::WIND, N_("Wind"), },
    { 0 }
  };

  AddEnum(_("Slope shading"),
          _("The terrain can be shaded among slopes to indicate either wind direction, sun position or a fixed shading from north-east."),
          slope_shading_list, (unsigned)terrain.slope_shading);
  GetDataField(TerrainSlopeShading).SetListener(this);
  SetExpertRow(TerrainSlopeShading);

  AddInteger(_("Terrain contrast"),
             _("Defines the amount of Phong shading in the terrain rendering.  Use large values to emphasise terrain slope, smaller values if flying in steep mountains."),
             _T("%d %%"), _T("%d %%"), 0, 100, 5,
             ByteToPercent(terrain.contrast));
  GetDataField(TerrainContrast).SetListener(this);
  SetExpertRow(TerrainContrast);

  AddInteger(_("Terrain brightness"),
             _("Defines the brightness (whiteness) of the terrain rendering.  This controls the average illumination of the terrain."),
             _T("%d %%"), _T("%d %%"), 0, 100, 5,
             ByteToPercent(terrain.brightness));
  GetDataField(TerrainBrightness).SetListener(this);
  SetExpertRow(TerrainBrightness);

  if (::terrain != NULL) {
    WindowStyle style;
    style.Border();
    AddRemaining(new WndOwnerDrawFrame(*(ContainerWindow *)GetWindow(),
                                       {0, 0, 100, 100},
                                       style,
                                       ::OnPreviewPaint));
  }

  terrain_settings = terrain;
  ShowTerrainControls();
}

bool
TerrainDisplayConfigPanel::Save(bool &_changed, bool &_require_restart)
{
  MapSettings &settings_map = CommonInterface::SetMapSettings();

  bool changed = false, require_restart = false;
  changed = (settings_map.terrain != terrain_settings);

  settings_map.terrain = terrain_settings;
  Profile::Set(ProfileKeys::DrawTerrain, terrain_settings.enable);
  Profile::Set(ProfileKeys::TerrainContrast, terrain_settings.contrast);
  Profile::Set(ProfileKeys::TerrainBrightness, terrain_settings.brightness);
  Profile::Set(ProfileKeys::TerrainRamp, terrain_settings.ramp);
  Profile::SetEnum(ProfileKeys::SlopeShadingType, terrain_settings.slope_shading);

  changed |= SaveValue(EnableTopography, ProfileKeys::DrawTopography,
                       settings_map.topography_enabled);

  _changed |= changed;
  _require_restart |= require_restart;

  return true;
}

Widget *
CreateTerrainDisplayConfigPanel()
{
  return new TerrainDisplayConfigPanel();
}
