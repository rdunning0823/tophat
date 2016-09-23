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

#ifndef XCSOAR_FILE_DATA_FIELD_HPP
#define XCSOAR_FILE_DATA_FIELD_HPP

#include "Base.hpp"
#include "Repository/FileType.hpp"
#include "Util/StaticArray.hpp"
#include "Util/StaticString.hxx"

#include <utility>

/**
 * #DataField specialisation that supplies options as a list of
 * files matching a suffix.  First entry is always blank for null entry.
 * 
 */
class FileDataField final : public DataField {
  typedef StaticArray<StaticString<32>, 8> PatternList;

public:
  /** FileList item */
  struct Item {
    /** Filename */
    const TCHAR *filename;
    /** Path including Filename */
    TCHAR *path;
    /** filename without extension */
    TCHAR *filename_no_extension;

    Item():filename(nullptr), path(nullptr), filename_no_extension(nullptr) {}

    Item(Item &&src):filename(src.filename), path(src.path),
        filename_no_extension(src.filename_no_extension) {
      src.filename = src.filename_no_extension = src.path = nullptr;
    }

    Item(const Item &) = delete;

    ~Item();

    Item &operator=(Item &&src) {
      std::swap(filename, src.filename);
      std::swap(path, src.path);
      std::swap(filename_no_extension, src.filename_no_extension);
      return *this;
    }

    void Set(const TCHAR *_path);
  };

private:
#ifdef _WIN32_WCE
  static constexpr unsigned MAX_FILES = 100;
#else
  static constexpr unsigned MAX_FILES = 512;
#endif

  /** Index of the active file */
  unsigned int current_index;
  /** FileList item array */
  StaticArray<Item, MAX_FILES> files;

  FileType file_type;

  /**
   * Has the file list already been loaded?  This class tries to
   * postpone disk access for as long as possible, to reduce UI
   * latency.
   */
  bool loaded;

  /**
   * Set to true if Sort() has been called before the file list was
   * loaded.  It will trigger a call to Sort() after loading.
   */
  bool postponed_sort;

  /**
   * sort it in reverse order
   */
  bool sort_reverse;

  /**
   * Used to store the value while !loaded.
   */
  StaticString<512> postponed_value;

  /**
   * Stores the patterns while !loaded.
   */
  PatternList postponed_patterns;

  /**
   * display item at top of list to download internet files
   * the user is responsible for handling this selection
   */
  bool enable_file_download;

  /* don't display the file extension */
  bool hide_file_extension;

public:
  /**
   * Constructor of the FileDataField class
   * @param OnDataAccess
   */
  FileDataField(DataFieldListener *listener=nullptr);

  FileType GetFileType() const {
    return file_type;
  }

  void SetFileType(FileType _file_type) {
    file_type = _file_type;
  }

  /**
   * sort it reverse.  Must be called before sort occurs
   */
  void SetReverseSort() {
    sort_reverse = true;
  }

  /**
   * Adds a filename/filepath couple to the filelist
   * @param fname The filename
   * @param fpname The filepath
   */
  void AddFile(const TCHAR *fname, const TCHAR *fpname);

  /**
   * Adds an empty row to the filelist
   */
  void AddNull();

  /**
   * Returns the number of files in the list
   * @return The number of files in the list
   */
  gcc_pure
  unsigned GetNumFiles() const;

  gcc_pure
  int Find(const TCHAR *text) const;

  /**
   * Iterates through the file list and tries to find an item where the path
   * is equal to the given text, if found the selection is changed to
   * that item
   * @param text PathFile to search for
   */
  void Lookup(const TCHAR *text);

  /**
   * Force the value to the given path.  If the path is not in the
   * file list, add it.  This method does not check whether the file
   * really exists.
   */
  void ForceModify(const TCHAR *path);

  /**
   * Returns the PathFile of the currently selected item
   * @return The PathFile of the currently selected item
   */
  gcc_pure
  const TCHAR *GetPathFile() const;

  /**
   * Sets the selection to the given index
   * @param Value The array index to select
   */
  void Set(unsigned new_value);

  /** Sorts the filelist by filenames */
  void Sort();
  void ScanDirectoryTop(const TCHAR *filter);

  /**
   * Scan multiple shell patterns.  Each pattern is terminated by a
   * null byte, and the list ends with an empty pattern.
   */
  void ScanMultiplePatterns(const TCHAR *patterns);

  /** For use by other classes */
  gcc_pure
  unsigned size() const;

  gcc_pure
  const TCHAR *GetItem(unsigned index) const;

  /**
   * the label displayed in the list of internet download is enabled
   */
  gcc_pure
  const TCHAR* GetScanInternetLabel() const;

  /**
   * when enabled, inserts an item in the list
   * created by GetScanInternetLable()
   * The user is responsible for handling the action when selected
   */
  void EnableInternetDownload();

  void SetExtensionVisible(bool _visible);


  /* virtual methods from class DataField */
  void Inc() override;
  void Dec() override;
  int GetAsInteger() const override;
  const TCHAR *GetAsString() const override;
  const TCHAR *GetAsDisplayString() const override;
  void SetAsInteger(int value) override;
  ComboList CreateComboList(const TCHAR *reference) const override;

protected:
  void EnsureLoaded();

  /**
   * Hack for our "const" methods, to allow them to load on demand.
   */
  void EnsureLoadedDeconst() const {
    const_cast<FileDataField *>(this)->EnsureLoaded();
  }
};

#endif
