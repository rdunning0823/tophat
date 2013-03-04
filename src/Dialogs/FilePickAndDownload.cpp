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
#endif

#include <assert.h>

#define REPOSITORY_URI "http://download.xcsoar.org/repository"

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


  the_item.Set(_T(""), nullptr, false);
  the_file.Clear();

  LoadRepositoryFile();
  RefreshForm();

#ifdef HAVE_DOWNLOAD_MANAGER
  if (Net::DownloadManager::IsAvailable()) {
    Net::DownloadManager::AddListener(*this);
    Net::DownloadManager::Enumerate(*this);

    Net::DownloadManager::Enqueue(REPOSITORY_URI, _T("repository"));
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
}

void
ManagedFilePickAndDownloadWidget::RefreshTheItem()
{
  for (auto i = repository.begin(), end = repository.end(); i != end; ++i) {
    const auto &remote_file = *i;
    const AvailableFile &file = remote_file;
    WideToACPConverter item_name2(the_item.name);

    if (StringIsEqual(item_name2, file.name.c_str())) {

      DownloadStatus download_status = the_item.download_status;


      TCHAR path[MAX_PATH];
      LocalPath(path, file);
      the_item.Set(BaseName(path),
                   the_item.downloading ? &download_status : nullptr,
                   the_item.failed);

      the_file = remote_file;
    }
  }

#ifdef HAVE_DOWNLOAD_MANAGER
  if (the_item.downloading && !Timer::IsActive())
    Timer::Schedule(1000);
#endif
}

void
ManagedFilePickAndDownloadWidget::RefreshForm()
{
  const DialogLook &look = UIGlobals::GetDialogLook();
  const Font& font = *look.list.font;
  StaticString<512> message(_T(""));
  UPixelScalar frame_width = status_message->GetWidth();


  message.append(the_item.name.c_str());

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
      spaces.Truncate(spaces_needed);
      message.AppendFormat(_T("%s%s\n%s"),spaces.c_str(), status.c_str(),
                           the_item.size.c_str());
    } else
      message.AppendFormat(_T("\n%s %s"), status.c_str(), the_item.size.c_str());

    status_message->SetCaption(message.c_str());
  }
}

#ifdef HAVE_DOWNLOAD_MANAGER

static const std::vector<AvailableFile> *add_list;

static void
OnPaintAddItem(Canvas &canvas, const PixelRect rc, unsigned i)
{

  assert(add_list != NULL);
  assert(i < add_list->size());

  const AvailableFile &file = (*add_list)[i];

  ACPToWideConverter name(file.GetName());
  if (name.IsValid())
    canvas.text(rc.left + Layout::Scale(2), rc.top + Layout::Scale(2), name);
}

#endif

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
        (file_filter.type == AvailableFile::Type::UNKNOWN ||
            (file_filter.type == remote_file.type)))
      list.push_back(remote_file);
  }

  if (list.empty()) {
    picker_state = PickerState::INVALID;
    return;
  }
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

  the_item.Set(base, nullptr, false);
  picker_state = PickerState::ALREADY_SHOWN;
  Net::DownloadManager::Enqueue(remote_file.GetURI(), base);
#endif
}

void
ManagedFilePickAndDownloadWidget::Close(bool success)
{
#ifdef HAVE_DOWNLOAD_MANAGER
  assert(Net::DownloadManager::IsAvailable());

  if (!the_item.name.empty() && the_item.downloading)
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
    parent_widget_dialog->OnAction(mrCancel);
    break;
  }
}

#ifdef HAVE_DOWNLOAD_MANAGER


void
ManagedFilePickAndDownloadWidget::OnTimer()
{
  if (the_item.downloading) {
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
    the_item.download_status = DownloadStatus{size, position};
    the_item.downloading = true;
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
    LoadRepositoryFile();

      switch (picker_state) {
      case NOT_YET_SHOWN:
        PromptAndAdd();
        break;
      case VISIBLE:
        break;

      case ALREADY_SHOWN:
        assert(false);
        break;
      case INVALID:
      case CANCELLED:
        break;
    }
  }

  RefreshTheItem();
  RefreshForm();

  if (repository_failed2)
    ShowMessageBox(_("Failed to download the repository index."),
                   _("Error"), MB_OK);

  if(the_item.failed)
    Close(false);

  if (!the_item.failed && !the_item.downloading &&
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
