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
 * CupxDecompressor.h
 *
 *  Created on: Nov 23, 2017
 *      Author: Ludovic Launer
 */

#ifndef CUPXDECOMPRESSOR_H_
#define CUPXDECOMPRESSOR_H_

#include <stdio.h>
#include <string>

static const int ZIP_DELIMITER_SIZE = 5;
static const char zip_header[ZIP_DELIMITER_SIZE] = { 0x50, 0x4B, 0x03, 0x04,
                                                     0x14 };

class CupxDecompressor
{
public:
  CupxDecompressor(const char* sourceZipFile);
  virtual
  ~CupxDecompressor();
  void
  Decompress();
  bool
  SplitToDisk(std::string outputDirectory = "");
  static bool
  Clean();
  std::string
  GetCupxFileNameWithoutExtension();
  bool
  IsCupxFile(std::string file_name);
  std::string
  GetCupxLocalDirectoryName();

private:
  std::string cupxFilename = "";
  long
  getOffset(FILE *fp);
  void
  CreateCupxOutputDirectories();
  void
  unzipFileIntoDirectory(std::string filename, std::string directoryName);
  std::string
  GetCupxLocalPicsDirectoryName();
  std::string
  GetCupxLocalPicsZipName();
  std::string
  GetCupxLocalPointsZipName();

};

#endif /* CUPXDECOMPRESSOR_H_ */
