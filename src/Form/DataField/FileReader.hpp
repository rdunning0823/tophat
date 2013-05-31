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

#ifndef XCSOAR_DATA_FIELD_FILE_READER_HPP
#define XCSOAR_DATA_FIELD_FILE_READER_HPP

#include "Base.hpp"
#include "Util/NonCopyable.hpp"
#include "Util/StaticArray.hpp"
#include "Util/StaticString.hpp"

/**
 * #DataField specialisation that supplies options as a list of
 * files matching a suffix.  First entry is always blank for null entry.
 * 
 */
class DataFieldFileReader: public DataField
{
  typedef StaticArray<StaticString<32>, 8> PatternList;

public:
  /** FileList item */
  struct Item : private NonCopyable {
    /** Filename */
    const TCHAR *filename;
    /** Path including Filename */
    TCHAR *path;

    Item():filename(NULL), path(NULL) {}
    ~Item();
  };

private:
  /** Index of the active file */
  unsigned int mValue;
  /** FileList item array */
  StaticArray<Item, 100> files;

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

public:
  /**
   * Constructor of the DataFieldFileReader class
   * @param OnDataAccess
   */
  DataFieldFileReader(DataAccessCallback OnDataAccess);

  /** Move the selection up (+1) */
  void Inc();
  /** Move the selection down (-1) */
  void Dec();
  /**
   * Prepares the ComboList items
   * @return The number of items in the ComboList
   */
  virtual ComboList *CreateComboList() const;

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
  int GetNumFiles() const;

  /**
   * Returns the selection index in integer format
   * @return The selection index in integer format
   */
  gcc_pure
  virtual int GetAsInteger() const;

  /**
   * Returns the selection title (filename)
   * @return The selection title (filename)
   */
  gcc_pure
  virtual const TCHAR *GetAsDisplayString() const;

  /**
   * Returns the PathFile of the currently selected item
   * @return The PathFile of the currently selected item
   */
  gcc_pure
  virtual const TCHAR *GetAsString() const;

  /**
   * Iterates through the file list and tries to find an item where the path
   * is equal to the given text, if found the selection is changed to
   * that item
   * @param text PathFile to search for
   */
  void Lookup(const TCHAR* text);

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
  void Set(int Value);

  /**
   * @see Set()
   * @return The index that was set (min: 0 / max: nFiles)
   */
  virtual void SetAsInteger(int Value);

  /** Sorts the filelist by filenames */
  void Sort();
  void ScanDirectoryTop(const TCHAR *filter);

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

protected:
  void EnsureLoaded();

  /**
   * Hack for our "const" methods, to allow them to load on demand.
   */
  void EnsureLoadedDeconst() const {
    const_cast<DataFieldFileReader *>(this)->EnsureLoaded();
  }
};

#endif
