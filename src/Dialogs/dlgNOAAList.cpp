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

#include "Dialogs/Weather.hpp"
#include "Dialogs/Dialogs.h"
#include "Dialogs/Message.hpp"
#include "Dialogs/JobDialog.hpp"
#include "Dialogs/CallBackTable.hpp"
#include "Language/Language.hpp"
#include "Weather/Features.hpp"

#ifdef HAVE_NOAA

#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Dialogs/TextEntry.hpp"
#include "Dialogs/XML.hpp"
#include "Form/Form.hpp"
#include "Form/List.hpp"
#include "Form/Button.hpp"
#include "Screen/Layout.hpp"
#include "Weather/NOAAGlue.hpp"
#include "Weather/NOAAStore.hpp"
#include "Weather/NOAAUpdater.hpp"
#include "Weather/METAR.hpp"
#include "Util/TrivialArray.hpp"
#include "Compiler.h"
#include "Renderer/NOAAListRenderer.hpp"

#include <stdio.h>

static WndForm *wf;
static ListControl *station_list;
static WndButton *add_button;
static WndButton *update_button;
static WndButton *remove_button;
static WndButton *details_button;

struct NOAAListItem
{
  StaticString<5> code;
  NOAAStore::iterator iterator;

  bool operator<(const NOAAListItem &i2) const {
    return _tcscmp(code, i2.code) == -1;
  }
};

static TrivialArray<NOAAListItem, 20> list;

static void
UpdateList()
{
  list.clear();

  for (auto i = noaa_store->begin(), end = noaa_store->end(); i != end; ++i) {
    NOAAListItem item;
    item.code = i->GetCodeT();
    item.iterator = i;
    list.push_back(item);
  }

  std::sort(list.begin(), list.end());

  station_list->SetLength(list.size());
  station_list->Invalidate();

  add_button->SetEnabled(!list.full());

  bool empty = list.empty();
  update_button->SetEnabled(!empty);
  remove_button->SetEnabled(!empty);
  details_button->SetEnabled(!empty);
}

static void
PaintListItem(Canvas &canvas, const PixelRect rc, unsigned index)
{
  assert(index < list.size());

  NOAAListRenderer::Draw(canvas, rc, *list[index].iterator,
                         UIGlobals::GetDialogLook());
}

static void
AddClicked(gcc_unused WndButton &button)
{
  TCHAR code[5] = _T("");
  if (!dlgTextEntryShowModal(*(SingleWindow *)button.GetRootOwner(),
                             code, 5, _("Airport ICAO code")))
    return;

  if (_tcslen(code) != 4) {
    ShowMessageBox(_("Please enter the FOUR letter code of the desired station."),
                _("Error"), MB_OK);
    return;
  }

  if (!NOAAStore::IsValidCode(code)) {
    ShowMessageBox(_("Please don't use special characters in the four letter code of the desired station."),
                  _("Error"), MB_OK);
    return;
  }

  NOAAStore::iterator i = noaa_store->AddStation(code);
  noaa_store->SaveToProfile();

  DialogJobRunner runner(wf->GetMainWindow(), wf->GetLook(),
                         _("Download"), true);

  NOAAUpdater::Update(*i, runner);

  UpdateList();
}

static void
UpdateClicked(gcc_unused WndButton &Sender)
{
  DialogJobRunner runner(wf->GetMainWindow(), wf->GetLook(),
                         _("Download"), true);
  NOAAUpdater::Update(*noaa_store, runner);
  UpdateList();
}

static void
RemoveClicked(gcc_unused WndButton &Sender)
{
  unsigned index = station_list->GetCursorIndex();
  assert(index < list.size());

  StaticString<256> tmp;
  tmp.Format(_("Do you want to remove station %s?"),
             list[index].code.c_str());

  if (ShowMessageBox(tmp, _("Remove"), MB_YESNO) == IDNO)
    return;

  noaa_store->erase(list[index].iterator);
  noaa_store->SaveToProfile();

  UpdateList();
}

static void
CloseClicked(gcc_unused WndButton &Sender)
{
  wf->SetModalResult(mrOK);
}

static void
OpenDetails(unsigned index)
{
  assert(index < list.size());
  dlgNOAADetailsShowModal(*(SingleWindow *)wf->GetRootOwner(),
                          list[index].iterator);
  UpdateList();
}

static void
DetailsClicked(gcc_unused WndButton &Sender)
{
  if (station_list->GetLength() > 0)
    OpenDetails(station_list->GetCursorIndex());
}

static void
ListItemSelected(unsigned index)
{
  OpenDetails(index);
}

static constexpr CallBackTableEntry CallBackTable[] = {
  DeclareCallBackEntry(PaintListItem),
  DeclareCallBackEntry(ListItemSelected),
  DeclareCallBackEntry(AddClicked),
  DeclareCallBackEntry(UpdateClicked),
  DeclareCallBackEntry(RemoveClicked),
  DeclareCallBackEntry(CloseClicked),
  DeclareCallBackEntry(DetailsClicked),
  DeclareCallBackEntry(NULL)
};

void
dlgNOAAListShowModal(SingleWindow &parent)
{
  wf = LoadDialog(CallBackTable, parent, Layout::landscape ?
                  _T("IDR_XML_NOAA_LIST_L") :
                  _T("IDR_XML_NOAA_LIST"));
  assert(wf != NULL);

  station_list = (ListControl *)wf->FindByName(_T("StationList"));
  assert(station_list != NULL);
  station_list->SetItemHeight(NOAAListRenderer::GetHeight(UIGlobals::GetDialogLook()));
  station_list->SetPaintItemCallback(PaintListItem);
  station_list->SetActivateCallback(ListItemSelected);

  add_button = (WndButton *)wf->FindByName(_T("AddButton"));
  assert(add_button != NULL);

  update_button = (WndButton *)wf->FindByName(_T("UpdateButton"));
  assert(update_button != NULL);

  remove_button = (WndButton *)wf->FindByName(_T("RemoveButton"));
  assert(remove_button != NULL);

  details_button = (WndButton *)wf->FindByName(_T("DetailsButton"));
  assert(details_button != NULL);

  UpdateList();

  wf->ShowModal();

  delete wf;
}

#else
void
dlgNOAAListShowModal(SingleWindow &parent)
{
  ShowMessageBox(_("This function is not available on your platform yet."),
              _("Error"), MB_OK);
}
#endif
