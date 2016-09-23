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

#include "PlaneDialogs.hpp"
#include "Dialogs/Message.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Widget/ListWidget.hpp"
#include "Form/Button.hpp"
#include "Screen/Canvas.hpp"
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
#include "Util/StringAPI.hxx"

#include <vector>
#include <assert.h>
#include <windef.h> /* for MAX_PATH */

struct ListItem
{
  StaticString<32> name;
  StaticString<MAX_PATH> path;

  bool operator<(const ListItem &i2) const {
    return _tcscmp(name, i2.name) < 0;
  }
};

class PlaneFileVisitor: public File::Visitor
{
  std::vector<ListItem> &list;

public:
  PlaneFileVisitor(std::vector<ListItem> &_list):list(_list) {}

  void Visit(const TCHAR* path, const TCHAR* filename) {
    ListItem item;
    item.name = filename;
    item.path = path;
    list.push_back(item);
  }
};

/* this macro exists in the WIN32 API */
#ifdef DELETE
#undef DELETE
#endif

class PlaneListWidget final
  : public ListWidget, private ActionListener {
  enum Buttons {
    NEW,
    EDIT,
    DELETE,
    LOAD,
  };

  WndForm *form;
  Button *edit_button, *delete_button, *load_button;

  std::vector<ListItem> list;

public:
  void CreateButtons(WidgetDialog &dialog);

private:
  void UpdateList();
  bool Load(unsigned i);
  bool LoadWithDialog(unsigned i);

  void LoadClicked();
  void NewClicked();
  void EditClicked();
  void DeleteClicked();
  bool IsAnyActive();
  void LoadFirstIfNoneActive();

public:
  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent,
                       const PixelRect &rc) override;
  virtual void Unprepare() override;

protected:
  /* virtual methods from ListItemRenderer */
  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned idx) override;

  /* virtual methods from ListCursorHandler */
  virtual bool CanActivateItem(unsigned index) const override {
    return true;
  }

  virtual void OnActivateItem(unsigned index) override;

private:
  /* virtual methods from class ActionListener */
  virtual void OnAction(int id) override;
};

gcc_pure
static UPixelScalar
GetRowHeight(const DialogLook &look)
{
  return look.list.font_bold->GetHeight() + Layout::Scale(6)
    + look.small_font.GetHeight();
}

/**
 * is any plane in the list active?
 * @return true if one of the planes in the list is active
 */
bool
PlaneListWidget::IsAnyActive()
{
  for (unsigned i = 0; i < list.size(); i++)
    if (Profile::GetPathIsEqual("PlanePath", list[i].path))
      return true;

  return false;
}

/**
 * if no planes in the list are active, it loads the first in the list
 */
void
PlaneListWidget::LoadFirstIfNoneActive()
{
  if (!IsAnyActive() && list.size() > 0)
    Load(0);
}

void
PlaneListWidget::UpdateList()
{
  list.clear();

  PlaneFileVisitor pfv(list);
  VisitDataFiles(_T("*.xcp"), pfv);

  unsigned len = list.size();

  if (len > 0)
    std::sort(list.begin(), list.end());

  ListControl &list_control = GetList();
  list_control.SetLength(len);
  list_control.Invalidate();

  const bool empty = list.empty();
  load_button->SetEnabled(!empty);
  edit_button->SetEnabled(!empty);
  delete_button->SetEnabled(!empty);

  LoadFirstIfNoneActive();
}

void
PlaneListWidget::CreateButtons(WidgetDialog &dialog)
{
  form = &dialog;

  dialog.AddButton(_("New"), *this, NEW);
  edit_button = dialog.AddButton(_("Edit"), *this, EDIT);
  delete_button = dialog.AddButton(_("Delete"), *this, DELETE);
  load_button = dialog.AddButton(_("Activate"), *this, LOAD);
}

void
PlaneListWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const DialogLook &look = UIGlobals::GetDialogLook();
  CreateList(parent, look, rc, GetRowHeight(look));
  UpdateList();
}

void
PlaneListWidget::Unprepare()
{
  DeleteWindow();
}

void
PlaneListWidget::OnPaintItem(Canvas &canvas, const PixelRect rc, unsigned i)
{
  assert(i < list.size());

  const DialogLook &look = UIGlobals::GetDialogLook();
  const Font &name_font = *look.list.font_bold;
  const Font &details_font = look.small_font;

  canvas.Select(name_font);

  Plane plane;
  if (PlaneGlue::ReadFile(plane, list[i].path)) {
    canvas.DrawClippedText(rc.left + Layout::FastScale(2),
                           rc.top + Layout::FastScale(2), rc,
                           plane.registration.c_str());

    if (Profile::GetPathIsEqual("PlanePath", list[i].path)) {
      StaticString<256> buffer;
      buffer.Format(_T("** %s **"), _("Active"));
      canvas.DrawClippedText(rc.right - Layout::FastScale(2) -
                             canvas.CalcTextSize(buffer).cx,
                             rc.top + Layout::FastScale(2), rc, buffer);
    }

    canvas.Select(details_font);

    StaticString<125> plane_desc;
    StaticString<5> slash(_T(" / "));
    if (plane.type.empty() || plane.competition_id.empty())
      slash.clear();
    plane_desc.Format(_T("%s%s%s"), plane.type.c_str(), slash.c_str(),
                      plane.competition_id.c_str());
    canvas.DrawClippedText(rc.left + Layout::FastScale(2),
                           rc.top + name_font.GetHeight() + Layout::FastScale(4),
                           rc, plane_desc.c_str());
  }
}

static bool
LoadFile(const TCHAR *path)
{
  ComputerSettings &settings = CommonInterface::SetComputerSettings();

  if (!PlaneGlue::ReadFile(settings.plane, path))
    return false;

  Profile::SetPath("PlanePath", path);
  PlaneGlue::Synchronize(settings.plane, settings,
                         settings.polar.glide_polar_task);
  if (protected_task_manager != NULL)
    protected_task_manager->SetGlidePolar(settings.polar.glide_polar_task);

  return true;
}

bool
PlaneListWidget::Load(unsigned i)
{
  assert(i < list.size());

  return LoadFile(list[i].path);
}

bool
PlaneListWidget::LoadWithDialog(unsigned i)
{
  assert(i < list.size());
  StaticString<256> tmp;
  tmp.Format(_("Activate plane \"%s\"?"),
             list[i].name.c_str());

  if (ShowMessageBox(tmp, _(" "), MB_YESNO) == IDYES) {

    const TCHAR *title;
    StaticString<256> text;

    bool result = Load(i);
    if (!result) {
      title = _("Error");
      text.Format(_("Activating plane \"%s\" failed!"),
                  list[i].name.c_str());
      ShowMessageBox(text, title, MB_OK);
    }
    GetList().Invalidate();
    return result;
  }
  return false;
}

inline void
PlaneListWidget::LoadClicked()
{
  LoadWithDialog(GetList().GetCursorIndex());
}

inline void
PlaneListWidget::NewClicked()
{
  Plane plane = CommonInterface::GetComputerSettings().plane;

  while (dlgPlaneDetailsShowModal(plane)) {
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
      tmp.Format(_("Plane \"%s\" already exists. "
                   "Overwrite it?"),
                   plane.registration.c_str());
      if (ShowMessageBox(tmp, _("Overwrite"), MB_YESNO) != IDYES)
        continue;
    }

    PlaneGlue::WriteFile(plane, path);
    UpdateList();
    break;
  }
}

inline void
PlaneListWidget::EditClicked()
{
  assert(GetList().GetCursorIndex() < list.size());

  const unsigned index = GetList().GetCursorIndex();
  const TCHAR *old_path = list[index].path;
  const TCHAR *old_filename = list[index].name;

  Plane plane;
  PlaneGlue::ReadFile(plane, old_path);

  while (dlgPlaneDetailsShowModal(plane)) {
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
        tmp.Format(_("Plane \"%s\" already exists. "
                     "Overwrite it?"),
                     plane.registration.c_str());
        if (ShowMessageBox(tmp, _("Overwrite"), MB_YESNO) != IDYES)
          continue;
      }

      File::Delete(old_path);
      PlaneGlue::WriteFile(plane, path);
      if (Profile::GetPathIsEqual("PlanePath", old_path)) {
        list[index].path = path;
        list[index].name = filename;
        Load(index);
      }
    } else {
      PlaneGlue::WriteFile(plane, old_path);
      if (Profile::GetPathIsEqual("PlanePath", old_path))
        Load(index);
    }

    UpdateList();
    break;
  }
}

inline void
PlaneListWidget::DeleteClicked()
{
  assert(GetList().GetCursorIndex() < list.size());

  StaticString<256> tmp;
  StaticString<256> tmp_name(list[GetList().GetCursorIndex()].name.c_str());
  if (tmp_name.length() > 4)
    tmp_name.Truncate(tmp_name.length() - 4);

  tmp.Format(_("Delete plane \"%s\"?"),
             tmp_name.c_str());
  if (ShowMessageBox(tmp, _("Delete"), MB_YESNO) != IDYES)
    return;

  File::Delete(list[GetList().GetCursorIndex()].path);
  UpdateList();
}

void
PlaneListWidget::OnAction(int id)
{
  switch ((Buttons)id) {
  case NEW:
    NewClicked();
    break;

  case EDIT:
    EditClicked();
    break;

  case DELETE:
    DeleteClicked();
    break;

  case LOAD:
    LoadClicked();
    break;
  }
}

void
PlaneListWidget::OnActivateItem(unsigned i)
{
  LoadWithDialog(i);
}

void
dlgPlanesShowModal()
{
  PlaneListWidget widget;
  WidgetDialog dialog(UIGlobals::GetDialogLook());
  dialog.CreateFull(UIGlobals::GetMainWindow(), _("Planes"), &widget);
  dialog.AddSymbolButton(_T("_X"), mrOK);
  widget.CreateButtons(dialog);

  dialog.ShowModal();
  dialog.StealWidget();
}
