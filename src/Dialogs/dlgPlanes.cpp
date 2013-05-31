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

#include "Dialogs/Planes.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "Dialogs/XML.hpp"
#include "Dialogs/Message.hpp"
#include "Form/Form.hpp"
#include "Form/List.hpp"
#include "Form/Button.hpp"
#include "Screen/Layout.hpp"
#include "Plane/Plane.hpp"
#include "Plane/PlaneGlue.hpp"
#include "Plane/PlaneFileGlue.hpp"
#include "OS/FileUtil.hpp"
#include "OS/PathName.hpp"
#include "Compatibility/path.h"
#include "LocalPath.hpp"
#include "Components.hpp"
#include "Profile/Profile.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "LogFile.hpp"

#include <vector>
#include <assert.h>
#include <windef.h> /* for MAX_PATH */

static bool Load(unsigned i);

static WndForm *dialog = NULL;
static ListControl *plane_list = NULL;

struct ListItem
{
  StaticString<32> name;
  StaticString<MAX_PATH> path;

  bool operator<(const ListItem &i2) const {
    return _tcscmp(name, i2.name) < 0;
  }
};

static std::vector<ListItem> list;

class PlaneFileVisitor: public File::Visitor
{
  void Visit(const TCHAR* path, const TCHAR* filename) {
    ListItem item;
    item.name = filename;
    item.path = path;
    list.push_back(item);
  }
};

gcc_pure
static UPixelScalar
GetRowHeight(const DialogLook &look)
{
  return look.list.font->GetHeight() + Layout::Scale(6)
    + look.small_font->GetHeight();
}

/**
 * is any plane in the list active?
 * @return true if one of the planes in the list is active
 */
static bool
IsAnyActive()
{
  for (unsigned i = 0; i < list.size(); i++)
    if (Profile::GetPathIsEqual(_T("PlanePath"), list[i].path))
      return true;

  return false;
}

/**
 * if no planes in the list are active, it loads the first in the list
 */
static void
LoadFirstIfNoneActive()
{
  if (!IsAnyActive() && list.size() > 0)
    Load(0);
}

static void
UpdateList()
{
  list.clear();

  PlaneFileVisitor pfv;
  VisitDataFiles(_T("*.xcp"), pfv);

  unsigned len = list.size();

  if (len > 0)
    std::sort(list.begin(), list.end());

  plane_list->SetLength(len);
  plane_list->Invalidate();

  WndButton* b = (WndButton*)dialog->FindByName(_T("LoadButton"));
  assert(b != NULL);
  b->SetEnabled(len > 0);

  b = (WndButton*)dialog->FindByName(_T("EditButton"));
  assert(b != NULL);
  b->SetEnabled(len > 0);

  b = (WndButton*)dialog->FindByName(_T("DeleteButton"));
  assert(b != NULL);
  b->SetEnabled(len > 0);

  LoadFirstIfNoneActive();
}

static void
OnPlaneListPaint(Canvas &canvas, const PixelRect rc, unsigned i)
{
  assert(i < list.size());

  const DialogLook &look = UIGlobals::GetDialogLook();
  const Font &name_font = *look.list.font;
  const Font &details_font = *look.small_font;

  canvas.Select(name_font);

  canvas.text_clipped(rc.left + Layout::FastScale(2),
                      rc.top + Layout::FastScale(2), rc, list[i].name);

  if (Profile::GetPathIsEqual(_T("PlanePath"), list[i].path)) {
    StaticString<256> buffer;
    buffer.Format(_T("** %s **"), _("Active"));
    canvas.text_clipped(rc.right - Layout::FastScale(2) -
                        canvas.CalcTextSize(buffer).cx,
                        rc.top + Layout::FastScale(2), rc, buffer);
  } else

  canvas.Select(details_font);

  canvas.text_clipped(rc.left + Layout::FastScale(2),
                      rc.top + name_font.GetHeight() + Layout::FastScale(4),
                      rc, list[i].path);
}

static bool
Load(unsigned i)
{
  assert(i < list.size());

  ComputerSettings &settings = CommonInterface::SetComputerSettings();

  if (!PlaneGlue::ReadFile(settings.plane, list[i].path))
    return false;

  Profile::SetPath(_T("PlanePath"), list[i].path);
  PlaneGlue::Synchronize(settings.plane, settings,
                         settings.polar.glide_polar_task);
  if (protected_task_manager != NULL)
    protected_task_manager->SetGlidePolar(settings.polar.glide_polar_task);

  return true;
}

static bool
LoadWithDialog(unsigned i)
{
  assert(i < list.size());

  StaticString<256> tmp;
  tmp.Format(_("Do you want to activate plane profile \"%s\"?"),
             list[i].name.c_str());

  if (ShowMessageBox(tmp, _(" "), MB_YESNO) == IDYES) {

    const TCHAR *title;
    StaticString<256> text;

    bool result = Load(i);
    if (!result) {
      title = _("Error");
      text.Format(_("Activating plane profile \"%s\" failed!"),
                  list[i].name.c_str());
      ShowMessageBox(text, title, MB_OK);
    }
    return result;
  }
  return false;
}

static void
LoadClicked(gcc_unused WndButton &button)
{
  LoadWithDialog(plane_list->GetCursorIndex());
}

static void
CloseClicked(gcc_unused WndButton &button)
{
  dialog->SetModalResult(mrCancel);
}

static void
NewClicked(gcc_unused WndButton &button)
{
  Plane plane = CommonInterface::GetComputerSettings().plane;

  while (dlgPlaneDetailsShowModal(*(SingleWindow*)dialog->GetRootOwner(), plane)) {
    if (plane.registration.empty()) {
      ShowMessageBox(_("Please enter the registration of the plane!"),
                  _("Error"), MB_OK);
      continue;
    }

    StaticString<42> filename(plane.registration);
    filename += _T(".xcp");

    StaticString<MAX_PATH> path;
    LocalPath(path.buffer(), filename);

    if (File::Exists(path)) {
      StaticString<256> tmp;
      tmp.Format(_("A plane profile \"%s\" already exists. "
                   "Do you want to overwrite it?"),
                   filename.c_str());
      if (ShowMessageBox(tmp, _("Overwrite"), MB_YESNO) != IDYES)
        continue;
    }

    PlaneGlue::WriteFile(plane, path);
    UpdateList();
    break;
  }
}

static void
EditClicked(gcc_unused WndButton &button)
{
  assert(plane_list->GetCursorIndex() < list.size());

  const unsigned index = plane_list->GetCursorIndex();
  const TCHAR *old_path = list[index].path;
  const TCHAR *old_filename = list[index].name;

  Plane plane;
  PlaneGlue::ReadFile(plane, old_path);

  while (dlgPlaneDetailsShowModal(*(SingleWindow*)dialog->GetRootOwner(), plane)) {
    if (plane.registration.empty()) {
      ShowMessageBox(_("Please enter the registration of the plane!"),
                  _("Error"), MB_OK);
      continue;
    }

    StaticString<42> filename(plane.registration);
    filename += _T(".xcp");

    if (filename != old_filename) {
      StaticString<MAX_PATH> path;
      DirName(old_path, path.buffer());
      path += _T(DIR_SEPARATOR_S);
      path += filename;

      if (File::Exists(path)) {
        StaticString<256> tmp;
        tmp.Format(_("A plane profile \"%s\" already exists. "
                     "Do you want to overwrite it?"),
                     filename.c_str());
        if (ShowMessageBox(tmp, _("Overwrite"), MB_YESNO) != IDYES)
          continue;
      }

      File::Delete(old_path);
      PlaneGlue::WriteFile(plane, path);
      if (Profile::GetPathIsEqual(_T("PlanePath"), old_path)) {
        list[index].path = path;
        list[index].name = filename;
        Load(index);
      }
    } else {
      PlaneGlue::WriteFile(plane, old_path);
      if (Profile::GetPathIsEqual(_T("PlanePath"), old_path))
        Load(index);
    }

    UpdateList();
    break;
  }
}

static void
DeleteClicked(gcc_unused WndButton &button)
{
  assert(plane_list->GetCursorIndex() < list.size());

  StaticString<256> tmp;
  tmp.Format(_("Do you really want to delete plane profile \"%s\"?"),
             list[plane_list->GetCursorIndex()].name.c_str());
  if (ShowMessageBox(tmp, _("Delete"), MB_YESNO) != IDYES)
    return;

  File::Delete(list[plane_list->GetCursorIndex()].path);
  UpdateList();
}

static void
ListItemSelected(unsigned i)
{
  LoadWithDialog(i);
}

static constexpr CallBackTableEntry CallBackTable[] = {
   DeclareCallBackEntry(LoadClicked),
   DeclareCallBackEntry(CloseClicked),
   DeclareCallBackEntry(NewClicked),
   DeclareCallBackEntry(EditClicked),
   DeclareCallBackEntry(DeleteClicked),
   DeclareCallBackEntry(NULL)
};

void
dlgPlanesShowModal(SingleWindow &parent)
{
  dialog = LoadDialog(CallBackTable, parent,
                      Layout::landscape ?
                      _T("IDR_XML_PLANES_L") : _T("IDR_XML_PLANES"));
  assert(dialog != NULL);

  UPixelScalar item_height = GetRowHeight(UIGlobals::GetDialogLook());

  plane_list = (ListControl*)dialog->FindByName(_T("List"));
  assert(plane_list != NULL);
  plane_list->SetItemHeight(item_height);
  plane_list->SetPaintItemCallback(OnPlaneListPaint);
  plane_list->SetActivateCallback(ListItemSelected);

  UpdateList();
  dialog->ShowModal();

  delete dialog;
}

