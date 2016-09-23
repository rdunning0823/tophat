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

#include "FilePickAndDownload.hpp"
#include "WidgetDialog.hpp"
#include "Message.hpp"
#include "Look/DialogLook.hpp"
#include "Look/GlobalFonts.hpp"
#include "Widget/Widget.hpp"
#include "Form/Frame.hpp"
#include "Form/Form.hpp"
#include "Form/DataField/Enum.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Canvas.hpp"
#include "Language/Language.hpp"
#include "OS/PathName.hpp"
#include "IO/FileLineReader.hpp"
#include "Util/ConvertString.hpp"
#include "Util/Macros.hpp"
#include "Repository/Parser.hpp"
#include "FilePickAndDownloadSettings.hpp"
#include "Interface.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Profile/Profile.hpp"
#include "Repository/FileType.hpp"

#ifdef ANDROID
#include "Android/NativeView.hpp"
#include "Android/Main.hpp"
#endif

#ifdef HAVE_DOWNLOAD_MANAGER
#include "Repository/Glue.hpp"
#include "ListPicker.hpp"
#include "Form/Button.hpp"
#include "Net/HTTP/DownloadManager.hpp"
#include "Event/Notify.hpp"
#include "Thread/Mutex.hpp"
#include "Event/Timer.hpp"

#include <map>
#include <set>
#include <vector>
#endif



#include <assert.h>
#include <windef.h> /* for MAX_PATH */

#ifdef HAVE_DOWNLOAD_MANAGER
static ManagedFilePickAndDownloadWidget *instance;
#endif

static bool
LocalPath(TCHAR *buffer, const AvailableFile &file)
{
  ACPToWideConverter base(file.GetName());
  if (!base.IsValid())
    return false;

  ::LocalPath(buffer, base);
  return true;
}

AvailableFile
ManagedFilePickAndDownloadWidget::GetResult()
{
  return the_file;
}

void
ManagedFilePickAndDownloadWidget::CreateButtons(WidgetDialog &dialog)
{
  search_button = dialog.AddButton(_("Continue"), *this, SEARCH_BUTTON);
  close_button = dialog.AddButton(_("Cancel"), *this, mrOK);
  parent_widget_dialog = &dialog;
}

bool
ManagedFilePickAndDownloadWidget::Save(bool &_changed)
{
  FilePickAndDownloadSettings &settings =
      CommonInterface::SetComputerSettings().file_pick_and_download;

  ACPToWideConverter area(file_filter.area);
  if (area.IsValid()) {
    settings.area_filter = area;
    Profile::Set(ProfileKeys::FilePickAndDownloadAreaFilter, settings.area_filter);
  }

  ACPToWideConverter subarea(file_filter.subarea);
  if (subarea.IsValid()) {
    settings.subarea_filter = subarea;
    Profile::Set(ProfileKeys::FilePickAndDownloadSubAreaFilter, settings.subarea_filter);
  }
  return true;
}

void
ManagedFilePickAndDownloadWidget::Show(const PixelRect &rc){
  RowFormWidget::Show(rc);
  SetFilterVisible(false);
}

void
ManagedFilePickAndDownloadWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);
  const DialogLook &look = UIGlobals::GetDialogLook();
  font_height = look.list.font->GetHeight();

  UPixelScalar margin = Layout::Scale(2);
  UPixelScalar height = std::max(unsigned(3 * margin + 2 * font_height),
                                 Layout::GetMaximumControlHeight());

  PixelRect rc_status = rc;
  rc_status.left += Layout::Scale(2);
  rc_status.right -= Layout::Scale(2);
  rc_status.bottom = rc_status.top + height;

  WindowStyle style_frame;
  style_frame.Border();

  status_message = new WndFrame((ContainerWindow&)RowFormWidget::GetWindow(), look,
                                rc_status,
                                style_frame);
  RowFormWidget::Add(status_message);
  status_message->SetCaption(_("Downloading list of files"));

  wp_area_filter = AddEnum(_("Continent or Country"),
                            _("The country or continent where you are flying"),
                            this);
  area_filter = (DataFieldEnum*)wp_area_filter->GetDataField();

  wp_subarea_filter = AddEnum(_("State"),
                              _("The state or country where you are flying"),
                              this);
  subarea_filter = (DataFieldEnum*)wp_subarea_filter->GetDataField();

  const FilePickAndDownloadSettings &settings =
      CommonInterface::GetComputerSettings().file_pick_and_download;

  WideToACPConverter base_area(settings.area_filter);
  file_filter.area = base_area;

  WideToACPConverter base_subarea(settings.subarea_filter);
  file_filter.subarea = base_subarea;

  LockMutex();
  the_item.Set(_T(""), nullptr, false);
  UnlockMutex();
  the_file.Clear();

  SetFilterVisible(false);
  assert(!IsFilterVisible());
  LoadRepositoryFile();
  RefreshForm();

#ifdef HAVE_DOWNLOAD_MANAGER
  if (Net::DownloadManager::IsAvailable()) {
    Net::DownloadManager::AddListener(*this);
    Net::DownloadManager::Enumerate(*this);

    Net::DownloadManager::Enqueue(GetRepositoryUri(file_filter.type), _T("repository"));
  }
#endif
}

void
ManagedFilePickAndDownloadWidget::Unprepare()
{
#ifdef HAVE_DOWNLOAD_MANAGER
  Timer::Cancel();

  if (Net::DownloadManager::IsAvailable())
    Net::DownloadManager::RemoveListener(*this);

  ClearNotification();
#endif

  //delete status_message;
}

void
ManagedFilePickAndDownloadWidget::LoadRepositoryFile()
{
#ifdef HAVE_DOWNLOAD_MANAGER
  LockMutex();
  repository_modified = false;
  repository_failed = false;
  UnlockMutex();
#endif

  repository.Clear();

  TCHAR path[MAX_PATH];
  LocalPath(path, _T("repository"));
  FileLineReaderA reader(path);
  if (reader.error())
    return;

  ParseFileRepository(repository, reader);
  if (file_filter.type == FileType::MAP)
    EnhanceAreaNames();
}

void
ManagedFilePickAndDownloadWidget::LockMutex()
{
#ifdef HAVE_DOWNLOAD_MANAGER
  mutex.Lock();
#endif
}

void
ManagedFilePickAndDownloadWidget::UnlockMutex()
{
#ifdef HAVE_DOWNLOAD_MANAGER
  mutex.Unlock();
#endif
}

void
ManagedFilePickAndDownloadWidget::EnhanceAreaNames()
{
  std::map<std::string, AvailableFile > area_map;

  area_map["alps"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Alps"), FileType::UNKNOWN};
  area_map["ar"] = AvailableFile {"", "", "", NarrowString<25ul>("South America"), NarrowString<25ul>("Argentina"), FileType::UNKNOWN};
  area_map["at"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Austria"), FileType::UNKNOWN};
  area_map["au"] = AvailableFile {"", "", "", NarrowString<25ul>("Australia"), NarrowString<25ul>("Australia"), FileType::UNKNOWN};
  area_map["be"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Belgium"), FileType::UNKNOWN};
  area_map["bg"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Bulgaria"), FileType::UNKNOWN};
  area_map["benelux"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Netherlands"), FileType::UNKNOWN};
  area_map["br"] = AvailableFile {"", "", "", NarrowString<25ul>("South America"), NarrowString<25ul>("Brazil"), FileType::UNKNOWN};
  area_map["ca"] = AvailableFile {"", "", "", NarrowString<25ul>("Canada"), NarrowString<25ul>("Canada"), FileType::UNKNOWN};
  area_map["ch"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Switzerland"), FileType::UNKNOWN};
  area_map["cl"] = AvailableFile {"", "", "", NarrowString<25ul>("South America"), NarrowString<25ul>("Chile"), FileType::UNKNOWN};
  area_map["co"] = AvailableFile {"", "", "", NarrowString<25ul>("South America"), NarrowString<25ul>("Colombia"), FileType::UNKNOWN};
  area_map["cz"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Czech Republic"), FileType::UNKNOWN};
  area_map["cz_sk"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Czech Republic"), FileType::UNKNOWN};
  area_map["de"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Germany"), FileType::UNKNOWN};
  area_map["dk"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Denmark"), FileType::UNKNOWN};
  area_map["es"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Spain"), FileType::UNKNOWN};
  area_map["eu"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Europe"), FileType::UNKNOWN};
  area_map["fi"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Finland"), FileType::UNKNOWN};
  area_map["fr"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("France"), FileType::UNKNOWN};
  area_map["hr"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Croatia"), FileType::UNKNOWN};
  area_map["hu"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Hungary"), FileType::UNKNOWN};
  area_map["ie"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Ireland"), FileType::UNKNOWN};
  area_map["il"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Israel"), FileType::UNKNOWN};
  area_map["it"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Italy"), FileType::UNKNOWN};
  area_map["jp"] = AvailableFile {"", "", "", NarrowString<25ul>("Japan"), NarrowString<25ul>("Japan"), FileType::UNKNOWN};
  area_map["lt"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Lithuania"), FileType::UNKNOWN};
  area_map["lv"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Latvia"), FileType::UNKNOWN};
  area_map["mx"] = AvailableFile {"", "", "", NarrowString<25ul>("Mexico"), NarrowString<25ul>("Central America"), FileType::UNKNOWN};
  area_map["na"] = AvailableFile {"", "", "", NarrowString<25ul>("Africa"), NarrowString<25ul>("Namibia"), FileType::UNKNOWN};
  area_map["nl"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Netherlands"), FileType::UNKNOWN};
  area_map["no"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Norway"), FileType::UNKNOWN};
  area_map["nz"] = AvailableFile {"", "", "", NarrowString<25ul>("New Zealand"), NarrowString<25ul>("New Zealand"), FileType::UNKNOWN};
  area_map["pl"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Poland"), FileType::UNKNOWN};
  area_map["po"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Poland"), FileType::UNKNOWN};
  area_map["pt"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Portugal"), FileType::UNKNOWN};
  area_map["se"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Sweden"), FileType::UNKNOWN};
  area_map["si"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Slovenia"), FileType::UNKNOWN};
  area_map["sk"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Slovakia"), FileType::UNKNOWN};
  area_map["uk"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("United Kingdom"), FileType::UNKNOWN};
  area_map["us"] = AvailableFile {"", "", "", NarrowString<25ul>("United States"), NarrowString<25ul>("United States"), FileType::UNKNOWN};
  area_map["za"] = AvailableFile {"", "", "", NarrowString<25ul>("Africa"), NarrowString<25ul>("South Africa"), FileType::UNKNOWN};

  for (auto i = repository.begin1(), end = repository.end1(); i != end; ++i) {
    AvailableFile &repo_file = *i;

    auto j = area_map.find(repo_file.area.c_str());
    if (j != area_map.end()) {
      AvailableFile area_file = j->second;
      repo_file.area = area_file.area;
      repo_file.subarea = area_file.subarea;
    }
  }
}

const char*
ManagedFilePickAndDownloadWidget::GetRepositoryUri(FileType type)
{
#define REPOSITORY_MAP_URI "http://download.xcsoar.org/repository"
//#define REPOSITORY_URI          "http://downloads.tophatsoaring.org/repository/repository_waypoint_tophat.txt"
#define REPOSITORY_WAYPOINT_URI "http://www.tophatsoaring.org/downloads/repository/repository_waypoint_tophat.txt"
#define REPOSITORY_AIRSPACE_URI "http://www.tophatsoaring.org/downloads/repository/repository_airspace_tophat.txt"


  switch (type) {
  case FileType::MAP:
  case FileType::FLARMNET:
  case FileType::UNKNOWN:
    return REPOSITORY_MAP_URI;

  case FileType::AIRSPACE:
    return REPOSITORY_AIRSPACE_URI;

  case FileType::WAYPOINT:
    return REPOSITORY_WAYPOINT_URI;

  }
  return REPOSITORY_MAP_URI;
}

void
ManagedFilePickAndDownloadWidget::RefreshTheItem()
{
  for (auto i = repository.begin(), end = repository.end(); i != end; ++i) {
    const auto &remote_file = *i;
    const AvailableFile &file = remote_file;
    WideToACPConverter item_name2(the_item.name);

    if (StringIsEqual(item_name2, file.name.c_str())) {
      LockMutex();

      DownloadStatus download_status = the_item.download_status;


      TCHAR path[MAX_PATH];
      LocalPath(path, file);
      the_item.Set(BaseName(path),
                   the_item.downloading ? &download_status : nullptr,
                   the_item.failed);
      UnlockMutex();
      the_file = remote_file;
    }
  }

#ifdef HAVE_DOWNLOAD_MANAGER
  LockMutex();
  bool downloading = the_item.downloading;
  UnlockMutex();
  if (downloading && !Timer::IsActive())
    Timer::Schedule(1000);
#endif
}

void
ManagedFilePickAndDownloadWidget::SetFilterVisible(bool visible)
{
  assert(wp_area_filter != nullptr);
  if (wp_area_filter == nullptr)
    return;

  wp_area_filter->SetVisible(visible);
  wp_subarea_filter->SetVisible(visible && subarea_filter->Count() > 2);
  search_button->SetVisible(visible);

  if (visible) {
    status_message->SetCaption(_("Where will you be flying?"));
    search_button->SetFocus();
  }
}

bool
ManagedFilePickAndDownloadWidget::IsFilterVisible()
{
  assert(wp_area_filter != nullptr);
  if (wp_area_filter == nullptr)
    return false;

  return wp_area_filter->IsVisible();
}

void
ManagedFilePickAndDownloadWidget::RefreshForm()
{
  const DialogLook &look = UIGlobals::GetDialogLook();
  const Font& font = *look.list.font;
  StaticString<512> message(_T(""));
  UPixelScalar frame_width = status_message->GetWidth();

  message.append(the_item.name.c_str());

  LockMutex();

  if (the_item.downloading || the_item.failed) {
    StaticString<64> status(_T(""));
    if (the_item.downloading) {

      if (the_item.download_status.position < 0) {
        status = _("Queued");

      } else if (the_item.download_status.size > 0) {
        status.Format(_T("%s (%u%%)"), _("Downloading"),
                      unsigned(the_item.download_status.position * 100
                               / the_item.download_status.size));
      } else {
        TCHAR size[32];
        FormatByteSize(size, ARRAY_SIZE(size), the_item.download_status.position);
        status.Format(_T("%s (%s)"), _("Downloading"), size);
      }
    } else if (the_item.failed) {
      status = _("Error");
    }
    PixelSize name_size = font.TextSize(message.c_str());
    PixelSize status_size = font.TextSize(status.c_str());
    PixelSize space_size = font.TextSize(_T("     "));
    UPixelScalar text_width = name_size.cx + status_size.cx + space_size.cx / 5;

    if (frame_width >= text_width) {
      unsigned spaces_needed = (frame_width - text_width) / space_size.cx / 5 - 1;
      StaticString<100> spaces (_T("                                                                                                    "));
      if (spaces_needed < spaces.length())
        spaces.Truncate(spaces_needed);
      message.AppendFormat(_T("%s%s\n%s"),spaces.c_str(), status.c_str(),
                           the_item.size.c_str());
    } else
      message.AppendFormat(_T("\n%s %s"), status.c_str(), the_item.size.c_str());

    status_message->SetCaption(message.c_str());
  }

  UnlockMutex();

}

#ifdef HAVE_DOWNLOAD_MANAGER

static const std::vector<AvailableFile> *add_list;

void
ManagedFilePickAndDownloadWidget::OnPaintItem(Canvas &canvas,
                                              const PixelRect rc,
                                              unsigned i)
{

  assert(add_list != NULL);
  assert(i < add_list->size());

  const AvailableFile &file = (*add_list)[i];

  ACPToWideConverter name(file.GetDisplayName());
  if (name.IsValid())
    canvas.DrawText(rc.left + Layout::Scale(2), rc.top + Layout::Scale(2), name);
}

#endif

void
ManagedFilePickAndDownloadWidget::BuildAreaFilter(FileRepository &repository)
{
  assert(area_filter->Count() == 0);

  std::map<std::string, int> area_map;
  area_vector.clear();
  for (auto i = repository.begin(), end = repository.end(); i != end; ++i) {
    const AvailableFile &remote_file = *i;
    std::string temp = remote_file.GetArea();
    if (area_map.find(temp) == area_map.end()) {
      area_map.insert(std::pair<std::string, int>(temp,0));
      area_vector.push_back(temp);
    }
  }

  unsigned id = 0;
  for (auto i = area_vector.begin(), end = area_vector.end(); i != end; ++i) {
    std::string t1 = (std::string)(*i);
    ACPToWideConverter base(t1.c_str());
    StaticString<100> t2(base);
    area_filter->AddChoice(id++, t2.c_str(), t2.c_str());
  }

  ACPToWideConverter base_area(file_filter.area);
  ACPToWideConverter base_subarea(file_filter.subarea);
  StaticString<50> subarea_original(base_subarea);

  area_filter->Sort(0);
  area_filter->SetAsString(base_area);
  wp_area_filter->RefreshDisplay();

  subarea_filter->SetAsString(subarea_original.c_str());
  wp_subarea_filter->RefreshDisplay();

  SetFilterVisible(true);

}

void
ManagedFilePickAndDownloadWidget::BuildSubAreaFilter(FileRepository &repository,
                                                     const char *area_filter)
{
  std::map<std::string, int> subarea_map;
  subarea_vector.clear();
  for (auto i = repository.begin(), end = repository.end(); i != end; ++i) {
    const AvailableFile &remote_file = *i;
    std::string area = remote_file.GetArea();

    if ((area_filter == nullptr) || (area == area_filter)) {
      std::string subarea = remote_file.GetSubArea();
      if (subarea_map.find(subarea) == subarea_map.end()) {
        subarea_map.insert(std::pair<std::string, int>(subarea,0));
        subarea_vector.push_back(subarea);
      }
    }
  }

  // sort vector instead of datafield so IDs of enums are in order
  sort(subarea_vector.begin(), subarea_vector.end(), [](std::string const& a,
       std::string const& b)
  {
      return a < b;
  });

  if (subarea_filter->Count() == 0)
    subarea_filter->AddChoice(0, _T(""), _T(""));
  else
    subarea_filter->replaceEnumText(0, _T(""));

  unsigned id = 1;
  for (auto i = subarea_vector.begin(), end = subarea_vector.end(); i != end; ++i) {
    std::string t1 = (std::string)(*i);
    ACPToWideConverter base(t1.c_str());
    StaticString<100> t2(base);
    if (id >= subarea_filter->Count())
      subarea_filter->AddChoice(id++, t2.c_str(), t2.c_str());
    else
      subarea_filter->replaceEnumText(id++, t2.c_str());
  }
  subarea_filter->Truncate(id);
  subarea_filter->SetAsInteger(0);
}

void
ManagedFilePickAndDownloadWidget::PromptAndAdd()
{
  picker_state = PickerState::VISIBLE;

#ifdef HAVE_DOWNLOAD_MANAGER
  assert(Net::DownloadManager::IsAvailable());

  std::vector<AvailableFile> list;
  for (auto i = repository.begin(), end = repository.end(); i != end; ++i) {
    const AvailableFile &remote_file = *i;
    if ((file_filter.area.empty() || (remote_file.area == file_filter.area)) &&
        (file_filter.subarea.empty() || (remote_file.subarea
            == file_filter.subarea)) &&
        (file_filter.type == FileType::UNKNOWN ||
            (file_filter.type == remote_file.type)))
      list.push_back(remote_file);
  }

  if (list.empty()) {
    picker_state = PickerState::INVALID;
    return;
  }

  sort(list.begin(), list.end(), [](AvailableFile const& a, AvailableFile const& b)
  {
      return a.display_name < b.display_name;
  });

  add_list = &list;
  int i = ListPicker(_("Select a file"),
                     list.size(), 0, Layout::GetMinimumControlHeight(),
                     *this);

  add_list = NULL;
  if (i < 0) {
    picker_state = PickerState::CANCELLED;
    return;
  }

  assert((unsigned)i < list.size());

  const AvailableFile &remote_file = list[i];
  ACPToWideConverter base(remote_file.GetName());
  if (!base.IsValid()) {
    picker_state = PickerState::INVALID;
    return;
  }

  LockMutex();
  the_item.Set(base, nullptr, false);
  UnlockMutex();
  picker_state = PickerState::ALREADY_SHOWN;
  Net::DownloadManager::Enqueue(remote_file.GetURI(), base);
#endif
}

void
ManagedFilePickAndDownloadWidget::Close(bool success)
{
#ifdef HAVE_DOWNLOAD_MANAGER
  assert(Net::DownloadManager::IsAvailable());

  LockMutex();
  const bool item_downloading = the_item.downloading;
  UnlockMutex();

  if (!the_item.name.empty() && item_downloading)
    Net::DownloadManager::Cancel(the_item.name);
#endif
  if (!success)
    the_file.Clear();

  parent_widget_dialog->OnAction(mrOK);
}

void
ManagedFilePickAndDownloadWidget::OnAction(int id)
{
  switch (id) {
  case mrOK:
    Close(false);
    break;

  case SEARCH_BUTTON:
    SetFilterVisible(false);
    PromptAndAdd();
    SetFilterVisible(picker_state == PickerState::CANCELLED);
    break;
  }
}

void
ManagedFilePickAndDownloadWidget::OnModified(DataField &df)
{
  if (IsDataField(AREA_FILTER, df)) {

    DataFieldEnum *dff = (DataFieldEnum*)&df;

    file_filter.area.clear();
    if (dff->GetAsDisplayString() != nullptr) {
      WideToACPConverter disp_string(dff->GetAsDisplayString());
      file_filter.area = disp_string;
    }

    BuildSubAreaFilter(repository, file_filter.area.c_str());
    if (subarea_filter->Count() > 0) {
      subarea_filter->Set((unsigned)0);
      wp_subarea_filter->RefreshDisplay();
    }
    wp_subarea_filter->SetVisible(subarea_filter->Count() > 2);
    OnModified(*subarea_filter);

  } else if (IsDataField(SUBAREA_FILTER, df)) {
    DataFieldEnum *dff = (DataFieldEnum*)&df;
    file_filter.subarea.clear();
    if (dff->GetAsDisplayString() != nullptr) {
      WideToACPConverter disp_string(dff->GetAsDisplayString());
      file_filter.subarea = disp_string;
    }
  }
}

#ifdef HAVE_DOWNLOAD_MANAGER


void
ManagedFilePickAndDownloadWidget::OnTimer()
{
  LockMutex();
  bool downloading = the_item.downloading;
  UnlockMutex();

  if (downloading) {
    Net::DownloadManager::Enumerate(*this);
    RefreshTheItem();
    RefreshForm();
    Timer::Schedule(1000);
  }
}

void
ManagedFilePickAndDownloadWidget::OnDownloadAdded(const TCHAR *path_relative,
                                                  int64_t size,
                                                  int64_t position)
{
  const TCHAR *name = BaseName(path_relative);
  if (name == NULL)
    return;

  WideToACPConverter name2(name);
  if (!name2.IsValid())
    return;

  if (the_item.name == name) {
    LockMutex();
    the_item.download_status = DownloadStatus{size, position};
    the_item.downloading = true;
    UnlockMutex();
  }

  SendNotification();
}

void
ManagedFilePickAndDownloadWidget::OnDownloadComplete(const TCHAR *path_relative,
                                                     bool success)
{
  const TCHAR *name = BaseName(path_relative);
  if (name == NULL)
    return;

  WideToACPConverter name2(name);
  if (!name2.IsValid())
    return;

  const std::string name3(name2);

  LockMutex();

  if (StringIsEqual(name2, "repository")) {
    repository_failed = !success;
    if (success)
      repository_modified = true;
  } else {
    if (StringIsEqual(name, the_item.name)) {
      the_item.failed = !success;
      the_item.downloading = false;
    }
  }

  UnlockMutex();

  SendNotification();
}

void
ManagedFilePickAndDownloadWidget::OnNotification()
{
  LockMutex();
  bool repository_modified2 = repository_modified;
  repository_modified = false;
  const bool repository_failed2 = repository_failed;
  repository_failed = false;
  UnlockMutex();

  if (repository_modified2) {
    LoadRepositoryFile();
    BuildAreaFilter(repository);
    SetFilterVisible(true);
  }

  RefreshTheItem();
  RefreshForm();

  if (repository_failed2)
    ShowMessageBox(_("Failed to download the repository index."),
                   _("Error"), MB_OK);

  LockMutex();
  const bool item_downloading = the_item.downloading;
  const bool item_failed = the_item.failed;
  UnlockMutex();

  if(item_failed)
    Close(false);

  if (!item_failed && !item_downloading &&
      picker_state == PickerState::ALREADY_SHOWN)
    Close(true);

  if (picker_state == PickerState::CANCELLED ||
      picker_state == PickerState::INVALID)
    Close(false);
}

const TCHAR *
ManagedFilePickAndDownloadWidget::GetTypeFilterName() const
{
  switch (file_filter.type) {
  case FileType::UNKNOWN:
    return _("all");
  case FileType::AIRSPACE:
    return _("airspace");
  case FileType::WAYPOINT:
    return _("waypoint");
  case FileType::MAP:
    return _("map");
  case FileType::FLARMNET:
    return _("FlarmNet ");
  }
  return _("");
}

static AvailableFile
ShowFilePickAndDownload2(AvailableFile &file_filter)
{

  instance = new ManagedFilePickAndDownloadWidget(file_filter);

  StaticString<50> title;
  title.Format(_T("%s %ss"), _("Download"), instance->GetTypeFilterName());

  ButtonPanel::ButtonPanelPosition position = ButtonPanel::ButtonPanelPosition::Bottom;
  WidgetDialog dialog(UIGlobals::GetDialogLook());

  dialog.CreateFull(UIGlobals::GetMainWindow(), title.c_str(), instance, nullptr, 0, position);

  instance->CreateButtons(dialog);
  dialog.ShowModal();

  return instance->GetResult();
}

#endif

AvailableFile
ShowFilePickAndDownload(AvailableFile &file_filter)
{
#ifdef HAVE_DOWNLOAD_MANAGER
  if (Net::DownloadManager::IsAvailable()) {
    return ShowFilePickAndDownload2(file_filter);
  }
#endif

  const TCHAR *message =
      _("Internet download is not available on this device.  Go to tophatsoaring.org/data%20files");
#ifdef ANDROID
  if (android_api_level < 9)
    message =
      _("Internet download is not available on this device.  Go to tophatsoaring.org/data%20files");
#endif

  ShowMessageBox(message, _("File Manager"), MB_OK);

  AvailableFile t;
  t.Clear();
  return t;

}
