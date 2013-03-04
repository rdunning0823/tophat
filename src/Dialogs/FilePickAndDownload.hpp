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

#ifndef XCSOAR_DIALOGS_FILE_PICK_AND_DOWNLOAD_HPP
#define XCSOAR_DIALOGS_FILE_PICK_AND_DOWNLOAD_HPP

#include "Repository/AvailableFile.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Form/DataField/Listener.hpp"
#include "Form/ActionListener.hpp"
#include "Screen/SingleWindow.hpp"
#include "Time/BrokenDateTime.hpp"
#include "LocalPath.hpp"
#include "Formatter/ByteSizeFormatter.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Repository/FileRepository.hpp"
#include "UIGlobals.hpp"
#include "OS/FileUtil.hpp"
#include "Net/Features.hpp"

#include <windef.h> /* for MAX_PATH */

#ifdef HAVE_DOWNLOAD_MANAGER
#include "Net/DownloadManager.hpp"
#include "Event/Notify.hpp"
#include "Event/Timer.hpp"
#include "Thread/Mutex.hpp"
#include "Dialogs/ListPicker.hpp"
#include "Form/List.hpp"
#include "Form/Button.hpp"

#include <map>
#include <set>
#include <vector>
#endif

class WndButton;
class WndFrame;
class WidgetDialog;

AvailableFile
ShowFilePickAndDownload(AvailableFile &file_filter);

/**
 * allows the user to pick a single file from the filtered repository and
 * download it.
 */
class ManagedFilePickAndDownloadWidget
  : public RowFormWidget,
#ifdef HAVE_DOWNLOAD_MANAGER
    private Timer, private Net::DownloadListener, private Notify,
#endif
    private ActionListener, ListItemRenderer
{

public:
  enum Controls {
    HEADER_TEXT,
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

  WndButton *close_button;
  WndFrame *status_message;

  FileRepository repository;

#ifdef HAVE_DOWNLOAD_MANAGER
  /**
   * This mutex protects the attributes repository_modified
   * and repository_failed.
   */
  mutable Mutex mutex;

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

  /**
   * pointer to the parent widget dialog so we can close it as needed
   */
  WidgetDialog *parent_widget_dialog;

public:
  ManagedFilePickAndDownloadWidget(AvailableFile& _file_filter)
    :RowFormWidget(UIGlobals::GetDialogLook()),
             file_filter(_file_filter),
             picker_state(PickerState::NOT_YET_SHOWN)
  {}

  void PromptAndAdd();

protected:
  void LoadRepositoryFile();
  void RefreshForm();
  void RefreshTheItem();

  /**
   * @param success. true if the files was successfully downloaded
   * Cancels the download manager and sets the the_item state accordingly
   * And closes the dialog
   */
  void Close(bool success);

public:
  /**
   * creates buttons with form as listener as necessary
   */
  void CreateButtons(WidgetDialog &dialog);
  /**
   * returns a copy of the struct with the file information
   * or cleared struct if error or cancel
   */
  AvailableFile GetResult();

  /**
   * return. string in singular describing filter type for files
   */
  const TCHAR * GetTypeFilterName() const;

  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();

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

#endif
