/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Look/Fonts.hpp"
#include "Widget/Widget.hpp"
#include "Form/Frame.hpp"
#include "Form/Form.hpp"
#include "Screen/Layout.hpp"
#include "Screen/SingleWindow.hpp"
#include "Screen/Canvas.hpp"
#include "Language/Language.hpp"
#include "LocalPath.hpp"
#include "OS/FileUtil.hpp"
#include "OS/PathName.hpp"
#include "IO/FileLineReader.hpp"
#include "Formatter/ByteSizeFormatter.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Net/Features.hpp"
#include "Util/ConvertString.hpp"
#include "Util/Macros.hpp"
#include "Repository/FileRepository.hpp"
#include "Repository/Parser.hpp"
#include "Time/BrokenDateTime.hpp"


#ifdef ANDROID
#include "Android/NativeView.hpp"
#include "Android/Main.hpp"
#endif

#ifdef HAVE_DOWNLOAD_MANAGER
#include "Dialogs/ListPicker.hpp"
#include "Form/List.hpp"
#include "Form/Button.hpp"
#include "Net/DownloadManager.hpp"
#include "Thread/Mutex.hpp"
#include "Event/Timer.hpp"
#include "Event/Notify.hpp"

#include <map>
#include <set>
#include <vector>
#endif

#include <assert.h>
#include <windef.h> /* for MAX_PATH */

#define REPOSITORY_URI "http://download.xcsoar.org/repository"

static WindowStyle
GetDialogStyle()
{
  WindowStyle style;
  style.Hide();
  style.ControlParent();
  return style;
}

static bool
LocalPath(TCHAR *buffer, const AvailableFile &file)
{
  ACPToWideConverter base(file.GetName());
  if (!base.IsValid())
    return false;

  ::LocalPath(buffer, base);
  return true;
}

/**
 * allows the user to pick a single file from the filtered repository and
 * download it.
 */
class ManagedFilePickAndDownloadWidget
  : public NullWidget, public WndForm
#ifdef HAVE_DOWNLOAD_MANAGER
    , private Timer, private Net::DownloadListener, private Notify, ListItemRenderer
#endif
{
  enum Buttons {
    CLOSE,
  };

  struct DownloadStatus {
    int64_t size, position;
  };

  struct FileItem {
    StaticString<64u> name;
    StaticString<32u> size;
    StaticString<32u> last_modified;

    bool downloading, failed, completed;

    DownloadStatus download_status;

    void Set(const TCHAR *_name, const DownloadStatus *_download_status,
             bool _failed) {
      name = _name;

      TCHAR path[MAX_PATH];
      LocalPath(path, name);

      if (File::Exists(path)) {
        FormatByteSize(size.buffer(), size.MAX_SIZE,
                       File::GetSize(path));
#ifdef HAVE_POSIX
        FormatISO8601(last_modified.buffer(),
                      BrokenDateTime::FromUnixTimeUTC(File::GetLastModification(path)));
#else
        // XXX implement
        last_modified.clear();
#endif
      } else {
        size.clear();
        last_modified.clear();
      }

      downloading = _download_status != NULL;
      if (downloading)
        download_status = *_download_status;

      failed = _failed;
    }
  };

  UPixelScalar font_height;

#ifdef HAVE_DOWNLOAD_MANAGER
  WndButton *close_button;
  WndFrame *status_message;

#endif

  FileRepository repository;

#ifdef HAVE_DOWNLOAD_MANAGER
  /**
   * This mutex protects the attributes "downloads" and
   * "repository_modified".
   */
  mutable Mutex mutex;

  /**
   * The list of file names (base names) that are currently being
   * downloaded.
   */
  std::map<std::string, DownloadStatus> downloads;

  /**
   * Each item in this set is a failed download.
   */
  std::set<std::string> failures;

  /**
   * Was the repository file modified, and needs to be reloaded by
   * LoadRepositoryFile()?
   */
  bool repository_modified;

  /**
   * Has the repository file download failed?
   */
  bool repository_failed;
#endif


  /**
   * collect the data to be returned of the file selected and downloaded
   */
  AvailableFile the_file;

  FileItem the_item;

  /**
   * uses the type and area properties to filter the files shown for selection
   */
  AvailableFile& file_filter;

  enum PickerState {
    VISIBLE,
    NOT_YET_SHOWN,
    ALREADY_SHOWN,
    CANCELLED,
    /**
     * the selected name is invalid
     */
    INVALID,
  };
  /**
   * is the picker visible, and has it been shown
   */
  PickerState picker_state;

  /**
   * has the repository been loaded
   */
  bool repository_loaded;

public:
  ManagedFilePickAndDownloadWidget(AvailableFile& _file_filter)
    :WndForm(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
             UIGlobals::GetMainWindow().GetClientRect(),
             _T(""), GetDialogStyle()),
             file_filter(_file_filter),
             picker_state(PickerState::NOT_YET_SHOWN)
  {}

  void PromptAndAdd();

protected:
  gcc_pure
  bool IsDownloading(const char *name) const {
#ifdef HAVE_DOWNLOAD_MANAGER
    ScopeLock protect(mutex);
    return downloads.find(name) != downloads.end();
#else
    return false;
#endif
  }

  gcc_pure
  bool IsDownloading(const AvailableFile &file) const {
    return IsDownloading(file.GetName());
  }

  gcc_pure
  bool IsDownloading(const char *name, DownloadStatus &status_r) const {
#ifdef HAVE_DOWNLOAD_MANAGER
    ScopeLock protect(mutex);
    auto i = downloads.find(name);
    if (i == downloads.end())
      return false;

    status_r = i->second;
    return true;
#else
    return false;
#endif
  }

  gcc_pure
  bool IsDownloading(const AvailableFile &file,
                     DownloadStatus &status_r) const {
    return IsDownloading(file.GetName(), status_r);
  }

  gcc_pure
  bool HasFailed(const char *name) const {
#ifdef HAVE_DOWNLOAD_MANAGER
    ScopeLock protect(mutex);
    return failures.find(name) != failures.end();
#else
    return false;
#endif
  }

  gcc_pure
  bool HasFailed(const AvailableFile &file) const {
    return HasFailed(file.GetName());
  }

  gcc_pure
  int FindItem(const TCHAR *name) const;

  void LoadRepositoryFile();
  void RefreshForm();
  void RefreshTheItem();

  /**
   * @param success. true if the files was successfully downloaded
   */
  void Close(bool success);

public:
  /**
   * returns a copy of the struct with the file information
   * or cleared struct if error or cancel
   */
  AvailableFile GetResult();

  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Unprepare() override;
  virtual void Hide() {};
  virtual void Show(const PixelRect &rc) {};

  /* virtual methods from class ActionListener */
  virtual void OnAction(int id) override;

#ifdef HAVE_DOWNLOAD_MANAGER
  /* virtual methods from class Timer */
  virtual void OnTimer() override;

  /* virtual methods from class Net::DownloadListener */
  virtual void OnDownloadAdded(const TCHAR *path_relative,
                               int64_t size, int64_t position) override;
  virtual void OnDownloadComplete(const TCHAR *path_relative, bool success) override;

  /* virtual methods from class Notify */
  virtual void OnNotification() override;

  void OnPaintItem(Canvas &canvas, const PixelRect rc, unsigned i) override;

#endif
};

AvailableFile
ManagedFilePickAndDownloadWidget::GetResult()
{
  return the_file;
}

void
ManagedFilePickAndDownloadWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  SetCaption(_("Download files from internet"));

  const DialogLook &look = UIGlobals::GetDialogLook();
  UPixelScalar margin = Layout::Scale(2);
  font_height = look.list.font->GetHeight();

  UPixelScalar height = std::max(unsigned(3 * margin + 2 * font_height),
                                 Layout::GetMaximumControlHeight());

  PixelRect rc_status = rc;
  rc_status.left += Layout::Scale(2);
  rc_status.right -= Layout::Scale(2);
  rc_status.top = height;
  rc_status.bottom = rc_status.top + height;

  WindowStyle style_frame;
  style_frame.Border();
  status_message = new WndFrame(GetClientAreaWindow(), look,
                                rc_status,
                                style_frame);
  status_message->SetFont(*look.list.font);
  status_message->SetCaption(_("Downloading list of files"));

  PixelRect rc_close = rc;
  rc_close.left += Layout::Scale(2);
  rc_close.right -= Layout::Scale(2);
  rc_close.top = (rc_close.top + rc_close.bottom) / 2;
  rc_close.bottom = rc_close.top + Layout::Scale(35);

  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();
  close_button = new WndButton(GetClientAreaWindow(), look, _T("Cancel"),
                               rc_close,
                               button_style, *this, CLOSE);

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

  delete status_message;
  delete close_button;
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
  bool download_active = false;
  for (auto i = repository.begin(), end = repository.end(); i != end; ++i) {
    const auto &remote_file = *i;
    const AvailableFile &file = remote_file;
    WideToACPConverter item_name2(the_item.name);

    if (StringIsEqual(item_name2, file.name.c_str())) {

      DownloadStatus download_status;
      const bool is_downloading = IsDownloading(file, download_status);

      TCHAR path[MAX_PATH];
      LocalPath(path, file);
      download_active |= is_downloading;
      the_item.Set(BaseName(path),
                   is_downloading ? &download_status : nullptr,
                   HasFailed(file));

      the_file = remote_file;
    }
  }

#ifdef HAVE_DOWNLOAD_MANAGER
  if (download_active && !Timer::IsActive())
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

void
ManagedFilePickAndDownloadWidget::OnPaintItem(Canvas &canvas,
                                                 const PixelRect rc,
                                                 unsigned i)
{

  assert(add_list != NULL);
  assert(i < add_list->size());

  const AvailableFile &file = (*add_list)[i];

  ACPToWideConverter name(file.GetName());
  if (name.IsValid())
    canvas.DrawText(rc.left + Layout::Scale(2), rc.top + Layout::Scale(2), name);
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
  int i = ListPicker(_("Select a file"),
                     list.size(), 0, Layout::FastScale(18),
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

  if (!the_item.name.empty())
    Net::DownloadManager::Cancel(the_item.name);
#endif
  if (!success)
    the_file.Clear();

  WndForm::SetModalResult(mrOK);
}

void
ManagedFilePickAndDownloadWidget::OnAction(int id)
{
  switch (id) {

  case CLOSE:
    Close(false);
    break;
  }
}

#ifdef HAVE_DOWNLOAD_MANAGER


void
ManagedFilePickAndDownloadWidget::OnTimer()
{
  mutex.Lock();
  const bool download_active = !downloads.empty();
  mutex.Unlock();

  if (download_active) {
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

  const std::string name3(name2);

  mutex.Lock();
  downloads[name3] = DownloadStatus{size, position};
  failures.erase(name3);
  mutex.Unlock();

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

  downloads.erase(name3);

  if (StringIsEqual(name2, "repository")) {
    repository_failed = !success;
    if (success)
      repository_modified = true;
  } else {
    if (!success)
      failures.insert(name3);

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

static AvailableFile
ShowFilePickAndDownload2(AvailableFile &file_filter)
{
  ManagedFilePickAndDownloadWidget widget(file_filter);

  ContainerWindow &w = UIGlobals::GetMainWindow();
  widget.Initialise(w, w.GetClientRect());
  widget.Prepare(w, w.GetClientRect());
  widget.ShowModal();
  widget.Hide();
  widget.Unprepare();

  AvailableFile result = widget.GetResult();
  return result;
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
