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

#include "Parser.hpp"
#include "FileRepository.hpp"
#include "IO/LineReader.hpp"
#include "Util/StringUtil.hpp"
#include "Util/CharUtil.hpp"

static const char *
ParseLine(char *line)
{
  char *separator = strchr(line, '=');
  if (separator == NULL)
    /* malformed line */
    return NULL;

  char *p = separator;
  while (p > separator && IsWhitespaceOrNull(p[-1]))
    --p;

  if (p == line)
    /* empty name */
    return NULL;

  *p = 0;

  char *value = const_cast<char *>(TrimLeft(separator + 1));
  TrimRight(value);
  return value;
}

static bool
Commit(FileRepository &repository, AvailableFile &file)
{
  if (file.IsEmpty())
    return true;

  if (!file.IsValid())
    return false;

  repository.files.push_back(std::move(file));
  file.Clear();
  return true;
}

bool
ParseFileRepository(FileRepository &repository, NLineReader &reader)
{
  AvailableFile file;
  file.Clear();

  char *line;
  while ((line = reader.read()) != NULL) {
    line = const_cast<char *>(TrimLeft(line));
    if (*line == 0 || *line == '#')
      continue;

    const char *name = line, *value = ParseLine(line);
    if (value == NULL)
      return false;

    if (StringIsEqual(name, "name")) {
      if (file.display_name.empty())
        file.display_name = file.GetName();
      if (!Commit(repository, file))
        return false;

      file.name.assign(value);
    } else if (file.IsEmpty()) {
      /* ignore */
    } else if (StringIsEqual(name, "uri")) {
      file.uri.assign(value);
    } else if (StringIsEqual(name, "displayname")) {
      file.display_name.assign(value);
    } else if (StringIsEqual(name, "area")) {
      file.area = value;
    } else if (StringIsEqual(name, "subarea")) {
      file.subarea = value;
    } else if (StringIsEqual(name, "type")) {
      if (StringIsEqual(value, "airspace"))
        file.type = AvailableFile::Type::AIRSPACE;
      else if (StringIsEqual(value, "waypoint"))
        file.type = AvailableFile::Type::WAYPOINT;
      else if (StringIsEqual(value, "map"))
        file.type = AvailableFile::Type::MAP;
      else if (StringIsEqual(value, "flarmnet"))
        file.type = AvailableFile::Type::FLARMNET;
    }
  }

  return Commit(repository, file);
}
