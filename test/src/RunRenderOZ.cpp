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

/*
 * This program demonstrates the OZRenderer library.  It
 * shows a list of shapes, and draws the selected shape on the right.
 *
 */

#include "Screen/SingleWindow.hpp"
#include "Screen/ButtonWindow.hpp"
#include "Screen/BufferCanvas.hpp"
#include "Screen/Init.hpp"
#include "Fonts.hpp"
#include "Look/DialogLook.hpp"
#include "Look/AirspaceLook.hpp"
#include "Look/TaskLook.hpp"
#include "Form/List.hpp"
#include "InfoBoxes/InfoBoxLayout.hpp"
#include "Renderer/OZRenderer.hpp"
#include "Engine/Task/ObservationZones/LineSectorZone.hpp"
#include "Engine/Task/ObservationZones/MatCylinderZone.hpp"
#include "Engine/Task/ObservationZones/FAISectorZone.hpp"
#include "Engine/Task/ObservationZones/KeyholeZone.hpp"
#include "Engine/Task/ObservationZones/BGAFixedCourseZone.hpp"
#include "Engine/Task/ObservationZones/BGAEnhancedOptionZone.hpp"
#include "Engine/Task/ObservationZones/BGAStartSectorZone.hpp"
#include "Engine/Task/ObservationZones/AnnularSectorZone.hpp"
#include "Engine/Task/ObservationZones/Boundary.hpp"
#include "Projection/Projection.hpp"
#include "Renderer/AirspaceRendererSettings.hpp"
#include "ResourceLoader.hpp"

enum {
  NUM_OZ_TYPES = 10,
};

static const TCHAR *const oz_type_names[NUM_OZ_TYPES] = {
  _T("Line"),
  _T("Cylinder"),
  _T("MAT Cylinder"),
  _T("Sector"),
  _T("FAI Sector"),
  _T("Keyhole"),
  _T("BGA Fixed Course"),
  _T("BGA Enhanced Option"),
  _T("BGA Start"),
  _T("Annular sector"),
};

static GeoPoint location(Angle::Degrees(fixed(7.7061111111111114)),
                         Angle::Degrees(fixed(51.051944444444445)));

static GeoPoint previous(Angle::Degrees(fixed(10.6)),
                         Angle::Degrees(fixed(49)));

static GeoPoint next(Angle::Degrees(fixed(10.2)),
                     Angle::Degrees(fixed(51.4)));

static AirspaceRendererSettings airspace_renderer_settings;

class OZWindow : public PaintWindow {
  OZRenderer roz;
  ObservationZonePoint *oz;
  Projection projection;

public:
  OZWindow(const TaskLook &task_look, const AirspaceLook &airspace_look)
    :roz(task_look, airspace_look, airspace_renderer_settings), oz(NULL) {
    projection.SetGeoLocation(location);
    set_shape(ObservationZonePoint::LINE);
  }

  ~OZWindow() {
    delete oz;
  }

  void set_shape(ObservationZonePoint::Shape shape) {
    if (oz != NULL && shape == oz->shape)
      return;

    delete oz;
    oz = NULL;

    fixed radius(10000);

    switch (shape) {
    case ObservationZonePoint::LINE:
      oz = new LineSectorZone(location, 2 * radius);
      break;

    case ObservationZonePoint::CYLINDER:
      oz = new CylinderZone(location, radius);
      break;

    case ObservationZonePoint::MAT_CYLINDER:
      oz = new MatCylinderZone(location);
      break;

    case ObservationZonePoint::SECTOR:
      oz = new SectorZone(location, radius,
                          Angle::Degrees(fixed(0)),
                          Angle::Degrees(fixed(70)));
      break;

    case ObservationZonePoint::ANNULAR_SECTOR:
      oz = new AnnularSectorZone(location, radius,
                                 Angle::Degrees(fixed(0)),
                                 Angle::Degrees(fixed(70)),
                                 radius*fixed_half);
      break;

    case ObservationZonePoint::FAI_SECTOR:
      oz = new FAISectorZone(location);
      break;

    case ObservationZonePoint::KEYHOLE:
      oz = new KeyholeZone(location);
      break;

    case ObservationZonePoint::BGAFIXEDCOURSE:
      oz = new BGAFixedCourseZone(location);
      break;

    case ObservationZonePoint::BGAENHANCEDOPTION:
      oz = new BGAEnhancedOptionZone(location);
      break;

    case ObservationZonePoint::BGA_START:
      oz = new BGAStartSectorZone(location);
      break;
    }

    if (oz != NULL)
      oz->SetLegs(&previous, &location, &next);

    Invalidate();
  }

protected:
  virtual void OnPaint(Canvas &canvas);

  virtual void OnResize(UPixelScalar width, UPixelScalar height) {
    PaintWindow::OnResize(width, height);
    projection.SetScale(fixed(width) / 21000);
    projection.SetScreenOrigin(width / 2, height / 2);
  }
};

void
OZWindow::OnPaint(Canvas &canvas)
{
  canvas.ClearWhite();
  if (oz == NULL)
    return;

  const int offset = 0;

  roz.Draw(canvas, OZRenderer::LAYER_SHADE, projection, *oz, offset);
  roz.Draw(canvas, OZRenderer::LAYER_INACTIVE, projection, *oz, offset);
  roz.Draw(canvas, OZRenderer::LAYER_ACTIVE, projection, *oz, offset);

  /* debugging for ObservationZone::GetBoundary() */
  Pen pen(1, COLOR_RED);
  canvas.Select(pen);
  const OZBoundary boundary = oz->GetBoundary();
  for (auto i = boundary.begin(), end = boundary.end(); i != end; ++i) {
    RasterPoint p = projection.GeoToScreen(*i);
    canvas.DrawLine(p.x - 3, p.y - 3, p.x + 3, p.y + 3);
    canvas.DrawLine(p.x + 3, p.y - 3, p.x - 3, p.y + 3);
  }
}

static void
paint_oz_type_name(Canvas &canvas, const PixelRect rc, unsigned idx)
{
  assert(idx < NUM_OZ_TYPES);

  canvas.text(rc.left + 2, rc.top + 2, oz_type_names[idx]);
}

static OZWindow *oz_window;

static void
oz_type_cursor_callback(unsigned idx)
{
  assert(idx < NUM_OZ_TYPES);

  oz_window->set_shape((ObservationZonePoint::Shape)idx);
}

class TestWindow : public SingleWindow {
  ButtonWindow close_button;
  ListControl *type_list;
  OZWindow oz;

  enum {
    ID_START = 100,
    ID_CLOSE
  };

public:
  TestWindow(const TaskLook &task_look, const AirspaceLook &airspace_look)
    :type_list(NULL), oz(task_look, airspace_look) {}
  ~TestWindow() {
    delete type_list;
  }

#ifdef USE_GDI
  static bool register_class(HINSTANCE hInstance) {
    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = Window::WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = 0;
    wc.lpszClassName = _T("RunRenderOZ");

    return RegisterClass(&wc);
  }
#endif /* USE_GDI */

  void set(const DialogLook &look, PixelRect _rc) {
    SingleWindow::set(_T("RunRenderOZ"), _T("RunRenderOZ"), _rc);

    const PixelRect rc = GetClientRect();

    WindowStyle with_border;
    with_border.Border();

    oz.set(*this, rc.right / 2, 0, rc.right - (rc.right / 2), rc.bottom,
           with_border);
    oz_window = &oz;

    const PixelRect list_rc = {
      0, 0, PixelScalar(rc.right / 2), PixelScalar(rc.bottom - 30),
    };

    type_list = new ListControl(*this, look, list_rc,
                                with_border, 25);
    type_list->SetPaintItemCallback(paint_oz_type_name);
    type_list->SetCursorCallback(oz_type_cursor_callback);
    type_list->SetLength(NUM_OZ_TYPES);

    PixelRect button_rc = rc;
    button_rc.right = (rc.left + rc.right) / 2;
    button_rc.top = button_rc.bottom - 30;
    close_button.set(*this, _T("Close"), ID_CLOSE, button_rc);

    oz.set_shape(ObservationZonePoint::LINE);

    type_list->SetFocus();
  }

protected:
  virtual bool OnCommand(unsigned id, unsigned code) {
    switch (id) {
    case ID_CLOSE:
      Close();
      return true;
    }

    return SingleWindow::OnCommand(id, code);
  }
};

#ifndef WIN32
int main(int argc, char **argv)
#else
int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
#ifdef _WIN32_WCE
        LPWSTR lpCmdLine,
#else
        LPSTR lpCmdLine2,
#endif
        int nCmdShow)
#endif
{
  ScreenGlobalInit screen_init;

  airspace_renderer_settings.SetDefaults();

#ifdef USE_GDI
  ResourceLoader::Init(hInstance);
  TestWindow::register_class(hInstance);
#endif

  InitialiseFonts();
  DialogLook *look = new DialogLook();
  look->Initialise(bold_font, normal_font, small_font, bold_font, bold_font);

  TaskLook *task_look = new TaskLook();
  task_look->Initialise();

  AirspaceLook *airspace_look = new AirspaceLook();
  airspace_look->Initialise(airspace_renderer_settings);

  TestWindow window(*task_look, *airspace_look);
  window.set(*look, PixelRect{0, 0, 480, 480});

  window.Show();
  window.RunEventLoop();

  delete airspace_look;
  delete task_look;
  delete look;
  DeinitialiseFonts();

  return 0;
}
