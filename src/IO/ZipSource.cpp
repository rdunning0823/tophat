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

#include "ZipSource.hpp"

#include "LogFile.hpp"
#include "OS/FileUtil.hpp"
#include <fstream>
#include <iostream>
#include <stdio.h>


#ifdef _UNICODE
#include <windows.h>
#endif

using namespace std;

ZipSource::ZipSource(struct zzip_dir *dir, const char *path)
{
  file = zzip_open_rb(dir, path);
  this->zzipDir = nullptr;
}

ZipSource::ZipSource(const char *path)
{
  file = zzip_fopen(path, "rb");
  this->zzipDir = nullptr;
}

/*
 * @param path path to zzip_file or zzip_directory
 * @param isZipArchiveDirectory true if the path represents a zzip_directory
 */
ZipSource::ZipSource(const char* path, bool isZipArchiveDirectory)
{
  // zzip_file
 if (!isZipArchiveDirectory)
 {
   this->zzipDir = nullptr;
   file = zzip_fopen(path, "rb");
 }
 // zzip_dir;
 else
 {
   file = nullptr;
   this->zzipDir = zzip_dir_open(path, 0);
 }
}

#ifdef _UNICODE
ZipSource::ZipSource(const TCHAR *path)
  :file(nullptr)
{
  char narrow_path[4096];

  int length = WideCharToMultiByte(CP_ACP, 0, path, -1,
                                   narrow_path, sizeof(narrow_path), nullptr, nullptr);
  if (length == 0)
    return;

  file = zzip_fopen(narrow_path, "rb");
}
#endif

ZipSource::~ZipSource()
{
  if (file != nullptr)
    zzip_fclose(file);
}

long
ZipSource::GetSize() const
{
  ZZIP_STAT st;
  return zzip_file_stat(file, &st) >= 0
    ? (long)st.st_size
    : -1l;
}

unsigned
ZipSource::Read(char *p, unsigned n)
{
  zzip_ssize_t nbytes = zzip_read(file, p, n);
  return nbytes >= 0
    ? (unsigned)nbytes
    : 0;
}

void
ZipSource::Unzip(const char* targetDirectory)
{
  ZZIP_DIR* dir  = this->zzipDir;

    if (dir) {
      ZZIP_DIRENT dirent;
      while (zzip_dir_read(dir, &dirent)) {
        /* show info for first file */
        LogFormat("%s %i/%i", dirent.d_name, dirent.d_csize, dirent.st_size);

        // Build output file location
        string picFilename = string(targetDirectory) + "/";
        picFilename += dirent.d_name;
        LogFormat("Extracting: %s", picFilename.c_str());

        ZZIP_FILE* fp = zzip_file_open(dir, dirent.d_name, 0);
        if (fp) {
          int fileSize = dirent.st_size;
          char buf[fileSize];
          zzip_ssize_t len = zzip_file_read(fp, buf, fileSize);
          if (len) {
            ofstream outfile(picFilename.c_str(), ofstream::out | ofstream::binary);
            outfile.write(buf, fileSize);
            outfile.flush();
            outfile.close();
          }
          zzip_file_close(fp);
        }

      }

      zzip_dir_close(dir);
    }
}
