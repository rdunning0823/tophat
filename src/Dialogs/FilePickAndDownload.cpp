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

#include "FilePickAndDownload.hpp"
#include "WidgetDialog.hpp"
#include "Message.hpp"
#include "Look/DialogLook.hpp"
#include "Form/Widget.hpp"
#include "Form/Frame.hpp"
#include "Form/Form.hpp"
#include "Form/DataField/Enum.hpp"
#include "Screen/Layout.hpp"
#include "Language/Language.hpp"
#include "OS/PathName.hpp"
#include "IO/FileLineReader.hpp"
#include "Util/ConvertString.hpp"
#include "Repository/Parser.hpp"

#ifdef ANDROID
#include "Android/NativeView.hpp"
#include "Android/Main.hpp"
#endif

#ifdef HAVE_DOWNLOAD_MANAGER
#include "ListPicker.hpp"
#include "Form/Button.hpp"

#include <map>
#endif

#include <assert.h>

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
  close_button = dialog.AddButton(_("Cancel"), this, mrCancel);
  search_button = dialog.AddButton(_("Search"), this, SEARCH_BUTTON);
  parent_widget_dialog = &dialog;
}

void
ManagedFilePickAndDownloadWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  RowFormWidget::Prepare(parent, rc);
  const DialogLook &look = UIGlobals::GetDialogLook();
  font_height = look.list.font->GetHeight();

 UPixelScalar margin = Layout::Scale(2);
 UPixelScalar height = std::max(UPixelScalar(3 * margin + 2 * font_height),
                                Layout::GetMaximumControlHeight());

  PixelRect rc_status = rc;
  rc_status.left += Layout::Scale(2);
  rc_status.right -= Layout::Scale(2);
  rc_status.bottom = rc_status.top + height;

  WindowStyle style_frame;
  style_frame.Border();
  status_message = new WndFrame(*(ContainerWindow*)RowFormWidget::GetWindow(), look,
                                rc_status.left, rc_status.top,
                                rc_status.right - rc_status.left,
                                rc_status.bottom - rc_status.top,
                                style_frame);
  RowFormWidget::Add(status_message);
  status_message->SetFont(*look.list.font);
  status_message->SetCaption(_("Downloading list of files"));

  wp_area_filter = AddEnum(_("Continent or Country"),
                            _("The country or continent where you are flying"),
                            this);
  area_filter = (DataFieldEnum*)wp_area_filter->GetDataField();

  wp_subarea_filter = AddEnum(_("State"),
                              _("The state or country where you are flying"),
                              this);
  subarea_filter = (DataFieldEnum*)wp_subarea_filter->GetDataField();

  mutex.Lock();
  the_item.Set(_T(""), nullptr, false);
  mutex.Unlock();
  the_file.Clear();

  SetFilterVisible(false);
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
  mutex.Lock();
  repository_modified = false;
  repository_failed = false;
  mutex.Unlock();
#endif

  repository.Clear();

  TCHAR path[MAX_PATH];
  LocalPath(path, _T("repository"));
  FileLineReaderA reader(path);
  if (reader.error())
    return;

  ParseFileRepository(repository, reader);
  if (file_filter.type == AvailableFile::Type::MAP)
    EnhanceAreaNames();
}


void
ManagedFilePickAndDownloadWidget::EnhanceAreaNames()
{
  std::map<std::string, AvailableFile > area_map;

  area_map["alps"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Alps"), AvailableFile::Type::UNKNOWN};
  area_map["na"] = AvailableFile {"", "", "", NarrowString<25ul>("Africa"), NarrowString<25ul>("Namibia"), AvailableFile::Type::UNKNOWN};
  area_map["za"] = AvailableFile {"", "", "", NarrowString<25ul>("Africa"), NarrowString<25ul>("South Africa"), AvailableFile::Type::UNKNOWN};
  area_map["au"] = AvailableFile {"", "", "", NarrowString<25ul>("Australia"), NarrowString<25ul>("Australia"), AvailableFile::Type::UNKNOWN};
  area_map["ca"] = AvailableFile {"", "", "", NarrowString<25ul>("Canada"), NarrowString<25ul>("Canada"), AvailableFile::Type::UNKNOWN};
  area_map["alps"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Alps"), AvailableFile::Type::UNKNOWN};
  area_map["cz_sk"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Czech Republic"), AvailableFile::Type::UNKNOWN};
  area_map["fi"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Finland"), AvailableFile::Type::UNKNOWN};
  area_map["fr"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("France"), AvailableFile::Type::UNKNOWN};
  area_map["de"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Germany"), AvailableFile::Type::UNKNOWN};
  area_map["hu"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Hungary"), AvailableFile::Type::UNKNOWN};
  area_map["ie"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Ireland"), AvailableFile::Type::UNKNOWN};
  area_map["il"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Israel"), AvailableFile::Type::UNKNOWN};
  area_map["it"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Italy"), AvailableFile::Type::UNKNOWN};
  area_map["benelux"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Netherlands"), AvailableFile::Type::UNKNOWN};
  area_map["nl"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Netherlands"), AvailableFile::Type::UNKNOWN};
  area_map["no"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Norway"), AvailableFile::Type::UNKNOWN};
  area_map["pl"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Poland"), AvailableFile::Type::UNKNOWN};
  area_map["pt"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Portugal"), AvailableFile::Type::UNKNOWN};
  area_map["es"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Spain"), AvailableFile::Type::UNKNOWN};
  area_map["se"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("Sweden"), AvailableFile::Type::UNKNOWN};
  area_map["uk"] = AvailableFile {"", "", "", NarrowString<25ul>("Europe"), NarrowString<25ul>("United Kingdom"), AvailableFile::Type::UNKNOWN};
  area_map["jp"] = AvailableFile {"", "", "", NarrowString<25ul>("Japan"), NarrowString<25ul>("Japan"), AvailableFile::Type::UNKNOWN};
  area_map["mx"] = AvailableFile {"", "", "", NarrowString<25ul>("Mexico"), NarrowString<25ul>("Central America"), AvailableFile::Type::UNKNOWN};
  area_map["nz"] = AvailableFile {"", "", "", NarrowString<25ul>("New Zealand"), NarrowString<25ul>("New Zealand"), AvailableFile::Type::UNKNOWN};
  area_map["ar"] = AvailableFile {"", "", "", NarrowString<25ul>("South America"), NarrowString<25ul>("Argentina"), AvailableFile::Type::UNKNOWN};
  area_map["br"] = AvailableFile {"", "", "", NarrowString<25ul>("South America"), NarrowString<25ul>("Brazil"), AvailableFile::Type::UNKNOWN};
  area_map["cl"] = AvailableFile {"", "", "", NarrowString<25ul>("South America"), NarrowString<25ul>("Chile"), AvailableFile::Type::UNKNOWN};
  area_map["co"] = AvailableFile {"", "", "", NarrowString<25ul>("South America"), NarrowString<25ul>("Colombia"), AvailableFile::Type::UNKNOWN};
  area_map["us"] = AvailableFile {"", "", "", NarrowString<25ul>("United States"), NarrowString<25ul>("United States"), AvailableFile::Type::UNKNOWN};

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
ManagedFilePickAndDownloadWidget::GetRepositoryUri(AvailableFile::Type type)
{
#define REPOSITORY_MAP_URI "http://download.xcsoar.org/repository"
//#define REPOSITORY_URI          "http://downloads.tophatsoaring.org/repository/repository_waypoint_tophat.txt"
#define REPOSITORY_WAYPOINT_URI "http://raspberryridgesheepfarm.com/tophat/repository/repository_waypoint_tophat.txt"
#define REPOSITORY_AIRSPACE_URI "http://raspberryridgesheepfarm.com/tophat/repository/repository_airspace_tophat.txt"


  switch (type) {
  case AvailableFile::Type::MAP:
  case AvailableFile::Type::FLARMNET:
  case AvailableFile::Type::UNKNOWN:
    return REPOSITORY_MAP_URI;

  case AvailableFile::Type::AIRSPACE:
    return REPOSITORY_AIRSPACE_URI;

  case AvailableFile::Type::WAYPOINT:
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
      mutex.Lock();

      DownloadStatus download_status = the_item.download_status;


      TCHAR path[MAX_PATH];
      LocalPath(path, file);
      the_item.Set(BaseName(path),
                   the_item.downloading ? &download_status : nullptr,
                   the_item.failed);
      mutex.Unlock();
      the_file = remote_file;
    }
  }

#ifdef HAVE_DOWNLOAD_MANAGER
  mutex.Lock();
  bool downloading = the_item.downloading;
  mutex.Unlock();
  if (downloading && !Timer::IsActive())
    Timer::Schedule(1000);
#endif
}

void
ManagedFilePickAndDownloadWidget::SetFilterVisible(bool visible)
{
  if (wp_area_filter == nullptr)
    return;

  wp_area_filter->SetVisible(visible);
  wp_subarea_filter->SetVisible(visible && subarea_filter->Count() > 1);
  search_button->SetVisible(visible);

  if (visible)
    status_message->SetCaption(_("Where will you be flying?"));
}

bool
ManagedFilePickAndDownloadWidget::IsFilterVisible()
{
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

  mutex.Lock();

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
    PixelSize space_size = font.TextSize(_T(" "));
    UPixelScalar text_width = name_size.cx + status_size.cx + space_size.cx;

    if (frame_width >= text_width) {
      unsigned spaces_needed = (frame_width - text_width) / space_size.cx;
      StaticString<100> spaces (_T("                                                                                                    "));
      if (spaces_needed < spaces.length())
        spaces.Truncate(spaces_needed);
      message.AppendFormat(_T("%s%s\n%s"),spaces.c_str(), status.c_str(),
                           the_item.size.c_str());
    } else
      message.AppendFormat(_T("\n%s %s"), status.c_str(), the_item.size.c_str());

    status_message->SetCaption(message.c_str());
  }

  mutex.Unlock();

}

#ifdef HAVE_DOWNLOAD_MANAGER

static const std::vector<AvailableFile> *add_list;

static void
OnPaintAddItem(Canvas &canvas, const PixelRect rc, unsigned i)
{

  assert(add_list != NULL);
  assert(i < add_list->size());

  const AvailableFile &file = (*add_list)[i];

  ACPToWideConverter name(file.GetDisplayName());
  if (name.IsValid())
    canvas.text(rc.left + Layout::Scale(2), rc.top + Layout::Scale(2), name);
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
  area_filter->Sort(0);
  area_filter->SetAsInteger(0);
  wp_area_filter->RefreshDisplay();
  OnModified(*area_filter);
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

  unsigned id = 0;
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
        (file_filter.type == AvailableFile::Type::UNKNOWN ||
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
  int i = ListPicker(UIGlobals::GetMainWindow(), _("Select a file"),
                     list.size(), 0, Layout::FastScale(18),
                     OnPaintAddItem);
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

  mutex.Lock();
  the_item.Set(base, nullptr, false);
  mutex.Unlock();
  picker_state = PickerState::ALREADY_SHOWN;
  Net::DownloadManager::Enqueue(remote_file.GetURI(), base);
#endif
}

void
ManagedFilePickAndDownloadWidget::Close(bool success)
{
#ifdef HAVE_DOWNLOAD_MANAGER
  assert(Net::DownloadManager::IsAvailable());

  mutex.Lock();
  const bool item_downloading = the_item.downloading;
  mutex.Unlock();

  if (!the_item.name.empty() && item_downloading)
    Net::DownloadManager::Cancel(the_item.name);
#endif
  if (!success)
    the_file.Clear();

  parent_widget_dialog->OnAction(mrCancel);
}

void
ManagedFilePickAndDownloadWidget::OnAction(int id)
{
  switch (id) {
  case mrCancel:
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
      subarea_filter->Set(0);
      wp_subarea_filter->RefreshDisplay();
    }
    wp_subarea_filter->SetVisible(subarea_filter->Count() > 1);
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

  mutex.Lock();
  bool downloading = the_item.downloading;
  mutex.Unlock();

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
    mutex.Lock();
    the_item.download_status = DownloadStatus{size, position};
    the_item.downloading = true;
    mutex.Unlock();
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

  mutex.Lock();

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

  mutex.Unlock();

  SendNotification();
}

void
ManagedFilePickAndDownloadWidget::OnNotification()
{
  mutex.Lock();
  bool repository_modified2 = repository_modified;
  repository_modified = false;
  const bool repository_failed2 = repository_failed;
  repository_failed = false;
  mutex.Unlock();

  if (repository_modified2) {
    assert(!IsFilterVisible());
    LoadRepositoryFile();
    BuildAreaFilter(repository);
    SetFilterVisible(true);
  }

  RefreshTheItem();
  RefreshForm();

  if (repository_failed2)
    ShowMessageBox(_("Failed to download the repository index."),
                   _("Error"), MB_OK);

  mutex.Lock();
  const bool item_downloading = the_item.downloading;
  const bool item_failed = the_item.failed;
  mutex.Unlock();

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
  case AvailableFile::Type::UNKNOWN:
    return _("all");
  case AvailableFile::Type::AIRSPACE:
    return _("airspace");
  case AvailableFile::Type::WAYPOINT:
    return _("waypoint");
  case AvailableFile::Type::MAP:
    return _("map");
  case AvailableFile::Type::FLARMNET:
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

  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();
  ButtonPanel::ButtonPanelPosition position = ButtonPanel::ButtonPanelPosition::Bottom;
  WidgetDialog dialog(title.c_str(),
                      rc, instance, nullptr, 0, position);

  instance->CreateButtons(dialog);
  dialog.ShowModal();

  return instance->GetResult();
}

#endif

/**
 * Displays a list of available files matching the area filter and type filter.
 * Downloads the file if selected.
 * @param file_filter.  Uses the area and type to filter the displayed files
 * @return. an AvailableFile struct of the downloaded file,
 * or cleared struct if none selected or if an error occurred during download
 */
AvailableFile
ShowFilePickAndDownload(AvailableFile &file_filter)
{
#ifdef HAVE_DOWNLOAD_MANAGER
  if (Net::DownloadManager::IsAvailable()) {
    return ShowFilePickAndDownload2(file_filter);
  }
#endif

  const TCHAR *message =
    _("The file manager is not available on this device.");
#ifdef ANDROID
  if (native_view->GetAPILevel() < 9)
    message = _("The file manager requires Android 2.3.");
#endif

  ShowMessageBox(message, _("File Manager"), MB_OK);

  AvailableFile t;
  t.Clear();
  return t;

}
