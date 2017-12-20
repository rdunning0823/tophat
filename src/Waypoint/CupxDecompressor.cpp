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
/*
 * CupxDecompressor.cpp
 *
 *  Created on: Nov 23, 2017
 *      Author: Ludovic Launer
 */

#include "CupxDecompressor.h"

#include "LogFile.hpp"
#include "LocalPath.hpp"
#include "OS/FileUtil.hpp"
#include "IO/ZipSource.hpp"
#include "Util/ConvertString.hpp"
#include <windef.h> /* for MAX_PATH */

#include <iostream>
#include <fstream>
#include <assert.h>

#include "Language/Language.hpp"
#include <vector>
#include "Util/tstring.hpp"


using namespace std;

#define CUPX_BASE_DIRECTORY_EXTENSION "_cupx";
#define CUPX_PICS_DIRECTORY_NAME "Pics/"
#define CUPX_POINTS_FILE_NAME "points.cup"
#define CUPX "cupx"

#define PICS_ZIPFILE_NAME "Pics.zip"
#define POINTS_ZIPFILE_NAME "points.zip"

struct ListItem
{
  string name;
  string path;
};


class CupxFileVisitor: public File::Visitor
{
  std::vector<ListItem> &list;

public:
  CupxFileVisitor(std::vector<ListItem> &_list):list(_list) {}

  void Visit(const TCHAR* path, const TCHAR* filename) {
    ListItem item;
    WideToUTF8Converter _filename(filename);
    WideToUTF8Converter _path(path);
    item.name = _filename;
    item.path = _path;
    list.push_back(item);
  }
};

CupxDecompressor::CupxDecompressor(const char* sourceZipFile)
{
  this->cupxFilename = string(sourceZipFile);
}

CupxDecompressor::~CupxDecompressor()
{
  // TODO Auto-generated destructor stub
}

/**
 * Decompress
 * Decompress the cupx file into a directory
 * The created diretory contains a Pics/ folder and a points.cup file
 */

void
CupxDecompressor::Decompress()
{
  // Get local XCSoarData path
  TCHAR _localPath[MAX_PATH];
  LocalPath(_localPath, _T("."));
  WideToUTF8Converter localPath(_localPath);

  // Create output directories
  this->CreateCupxOutputDirectories();

  // Split cupx into 2 zip files
  string outputDirectory = string(localPath);
  this->SplitToDisk(outputDirectory);

  // Unzip into directory
  // Unzip /Pics
  string picsZipFilename = this->GetCupxLocalPicsZipName();
  string picsDestinationDirectoryName = this->GetCupxLocalDirectoryName();
  this->unzipFileIntoDirectory(picsZipFilename, picsDestinationDirectoryName);

  // Unzip points.cup
  string pointsZipFilename = this->GetCupxLocalPointsZipName();
  string pointsDestinationDirectoryName = this->GetCupxLocalDirectoryName();
  this->unzipFileIntoDirectory(pointsZipFilename, pointsDestinationDirectoryName);

  // Delete .zip files
  UTF8ToWideConverter w_picsZipFilename(picsZipFilename.c_str());
  UTF8ToWideConverter w_pointsZipFilename(pointsZipFilename.c_str());
  File::Delete(w_picsZipFilename);
  File::Delete(w_pointsZipFilename);
}

/**
 * getOffset
 * Gets offset from the begining of the file to just before the zip last zip header.
 * @param *fp pointer to file where to look for the zip header
 * @return offset in file just before the last zip header
 */
long
CupxDecompressor::getOffset(FILE *fp)
{
  long offset = 0;
  long lastOffset = 0;
  char nextbyte;
  int index = 0, max = ZIP_DELIMITER_SIZE - 1;

  fread(&nextbyte, 1, sizeof(char), fp);
  offset++;

  while (fread(&nextbyte, 1, sizeof(char), fp) > 0) {
    offset++;
    if (nextbyte == zip_header[index]) {
      if (index == max) { // Delimitier found = matched the entire delimiter
        lastOffset = offset;
        index = 0;
      } else
        index++;  // Matching in progress
    } else
      index = 0; // Did not match
  }

  fclose(fp);
  return lastOffset - ZIP_DELIMITER_SIZE; // To get back to the header starts;
}

/**
 * SplitToDisk
 * Split the file into 2 zip files
 * @param outputDirectory directory where to store created files
 * @return true if splitting was successful
 */
bool
CupxDecompressor::SplitToDisk(string outputDirectory)
{
  // --- Get offset to define where to split ---
  FILE *fp;
  assert((fp = fopen(this->cupxFilename.c_str(), "rb")));
  long offset = getOffset(fp);

  if (offset == 0)
    return false;

  // --- Split file ---
  char* buffer1;
  char* buffer2;
  long size;
  long cutOffLocaltion;

  ifstream infile(this->cupxFilename.c_str(), ifstream::binary);

  // Destination files
  string picsZipFilename =this->GetCupxLocalPicsZipName();
  string pointsZipFilename = this->GetCupxLocalPointsZipName();

  ofstream outfile(picsZipFilename, ofstream::binary);
  ofstream outfile2(pointsZipFilename, ofstream::binary);

  // get size of file
  infile.seekg(0, ifstream::end);
  size = infile.tellg();
  infile.seekg(0);
  cutOffLocaltion = static_cast<int>(offset);

  // allocate memory for file content
  buffer1 = new char[offset];
  buffer2 = new char[size - offset];

  // read content of infile
  infile.read(buffer1, cutOffLocaltion);
  infile.seekg(cutOffLocaltion);
  // read content of infile
  infile.read(buffer2, size - cutOffLocaltion);

  // write to outfile
  outfile.write(buffer1, cutOffLocaltion);
  outfile.close();

  outfile2.write(buffer2, size - cutOffLocaltion);
  outfile2.close();

  infile.close();

  // release dynamically-allocated memory
  delete[] buffer1;
  delete[] buffer2;

  return true;
}

/**
 * CreateCupxOutputDirectories
 * Create diretories needed to extract cupx file onto disk
 */
void
CupxDecompressor::CreateCupxOutputDirectories()
{
  // Create root directory
  std::string cupxLocalDirectoryName = this->GetCupxLocalDirectoryName();
  UTF8ToWideConverter wide_cupxLocalDirectoryName(cupxLocalDirectoryName.c_str());
  Directory::Create(wide_cupxLocalDirectoryName);

  // Create Pics directory
  string picsDirectoryName = this->GetCupxLocalPicsDirectoryName();
  UTF8ToWideConverter wide_picsDirectoryNamee(picsDirectoryName.c_str());
  Directory::Create(wide_picsDirectoryNamee);
}

/**
 * Clean
 * Deletes the Pics directory and the points.cup file
 * @return true if directory or file were deleted
 */
bool
CupxDecompressor::Clean()
{
  // TODO: This needs to go through all directories in the "local" data directory and delete directories called xxx_cupx
  bool deleted = false;

  std::string directoryTodelete = string("/home/rob/.xcsoar/small_cupx");
  UTF8ToWideConverter wide_wide_directoryToDelete (directoryTodelete.c_str());
  Directory::Delete(wide_wide_directoryToDelete);

  return deleted;
}

/**
 * IsCupxFile
 * Finds out if the file is a .cupx file based on file extension
 * @return the filename without the extension if it's a .cupx file. Empty string otherwise
 */
string
CupxDecompressor::GetCupxFileNameWithoutExtension()
{
  std::string filename = "";
  std::string::size_type idx;

  idx = this->cupxFilename.rfind('.');

  if(idx != std::string::npos)
  {
      std::string extension = this->cupxFilename.substr(idx+1);
      filename = this->cupxFilename.substr(0, idx);
      bool is_cupx= (extension==string(CUPX))?true:false;
      return (is_cupx)?filename:"";
  }
  return filename;
}

bool
CupxDecompressor::IsCupxFile(std::string file_name)
{
  string filename = this->GetCupxFileNameWithoutExtension();
  bool is_cupx = (filename.empty())?false:true;
  return is_cupx;
}

std::string
CupxDecompressor::GetCupxLocalDirectoryName()
{
  string filename = this->GetCupxFileNameWithoutExtension();
  string directoryName = filename + CUPX_BASE_DIRECTORY_EXTENSION;

  return directoryName;
}

std::string
CupxDecompressor::GetCupxLocalPicsDirectoryName()
{
  string directoryName = this->GetCupxLocalDirectoryName();
  string pointsZipName = directoryName + "/" + CUPX_PICS_DIRECTORY_NAME;
  return pointsZipName;
}

std::string
CupxDecompressor::GetCupxLocalPicsZipName()
{
  string directoryName = this->GetCupxLocalDirectoryName();
  string picsZipName = directoryName + "/" + PICS_ZIPFILE_NAME;
  return picsZipName;
}

std::string
CupxDecompressor::GetCupxLocalPointsZipName()
{
  string directoryName = this->GetCupxLocalDirectoryName();
  string pointsZipName = directoryName + "/" +POINTS_ZIPFILE_NAME;
  return pointsZipName;
}

void
CupxDecompressor::unzipFileIntoDirectory(std::string filename, std::string directoryName)
{
    ZipSource zipSource = ZipSource(filename.c_str(), true);
    zipSource.Unzip(directoryName.c_str());
}









