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
#include "Net/HTTP/Features.hpp"
#include "Form/List.hpp"

#include <boost/intrusive/list.hpp>
#include <boost/intrusive/set.hpp>
#include <windef.h> /* for MAX_PATH */
#ifdef HAVE_DOWNLOAD_MANAGER

#include "Net/HTTP/DownloadManager.hpp"
#include "Event/Notify.hpp"
#include "Event/Timer.hpp"
#include "Thread/Mutex.hpp"
#include "Dialogs/ListPicker.hpp"
#include "Form/Button.hpp"

#include <map>
#include <set>
#include <vector>
#endif

class Button;
class WndFrame;
class WidgetDialog;
class DataFieldEnum;

/**
 * Displays a list of available files matching the area filter and type filter.
 * Downloads the file if selected.
 * @param file_filter.  Uses the area and type to filter the displayed files
 * @return. an AvailableFile struct of the downloaded file,
 * or cleared struct if none selected or if an error occurred during download
 */
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
    private ActionListener, ListItemRenderer, DataFieldListener
{

public:
  enum Controls {
    HEADER_TEXT = 0,
    AREA_FILTER,
    SUBAREA_FILTER,
  };

  enum Buttons {
    SEARCH_BUTTON = 100,
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
        FormatByteSize(size.buffer(), size.CAPACITY,
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

  Button *close_button;
  Button *search_button;
  WndFrame *status_message;
  DataFieldEnum *area_filter;
  DataFieldEnum *subarea_filter;
  /**
   * pointer to the property behind the area_filter datafield
   */
  WndProperty *wp_area_filter;
  WndProperty *wp_subarea_filter;

  FileRepository repository;

#ifdef HAVE_DOWNLOAD_MANAGER
  /**
   * This mutex protects the attributes repository_modified, repository_failed,
   * the_item.download_status, the_item.downloading and
   * the_item.failed properies.
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
   * a vector holding the list of areas for the repository
   */
  std::vector<std::string> area_vector;

  /**
   * a vector holding the list of areas for the repository or a
   * subset of the repository if an optional area filter exists
   */
  std::vector<std::string> subarea_vector;

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

  /**
   * loads the area_vector property with a sorted list of areas
   * of all the items in the repository
   * @param repository
   */
  void BuildAreaFilter(FileRepository &repository);

  /**
   * loads subarea_vector property with a sorted list of sub areas
   * of items in the repository based on the area_filter
   * @param repository
   * @param area_filter. the filter used on the repository
   */
  void BuildSubAreaFilter(FileRepository &repository, const char *area_filter);

protected:
  void LoadRepositoryFile();

  /**
   * replaces all the cryptic area names from the XCSoar repository items
   * with readable ones
   * Should be called before the filters are built from the repository
   */
  void EnhanceAreaNames();
  void RefreshForm();
  void RefreshTheItem();

  /**
   * shows or hides the Filter page
   */
  void SetFilterVisible(bool visible);

  /**
   * @return. true if the Filter page is visible
   */
  bool IsFilterVisible();

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

  /**
   * @param type.  type of file
   * @return. URI of repository for specified type of file
   */
  const char* GetRepositoryUri(FileType type);

  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Unprepare() override;
  virtual void Show(const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;

  /* virtual methods from class ActionListener */
  virtual void OnAction(int id) override;

  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df) override;

  /**
   * locks and unlocks the mutex on the repository and downloading status
   */
  void LockMutex();
  void UnlockMutex();

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
