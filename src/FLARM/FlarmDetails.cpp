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

#include "FLARM/FlarmDetails.hpp"
#include "FLARM/FlarmId.hpp"
#include "Util/StringUtil.hpp"
#include "Util/StaticString.hpp"
#include "Util/TrivialArray.hpp"
#include "LogFile.hpp"
#include "LocalPath.hpp"
#include "FLARM/FlarmNet.hpp"
#include "IO/DataFile.hpp"
#include "IO/TextWriter.hpp"

#include <assert.h>

struct FlarmIdNameCouple
{
  FlarmId id;

  StaticString<21> name;
};

static TrivialArray<FlarmIdNameCouple, 200> flarm_names;

void
FlarmDetails::Load()
{
  LogStartUp(_T("FlarmDetails::Load"));

  LoadSecondary();
  LoadFLARMnet();
}

void
FlarmDetails::LoadFLARMnet()
{
  NLineReader *reader = OpenDataTextFileA(_T("data.fln"));
  if (reader == NULL)
    return;

  unsigned num_records = FlarmNet::LoadFile(*reader);
  delete reader;

  if (num_records > 0)
    LogStartUp(_T("%u FLARMnet ids found"), num_records);
}

static void
LoadSecondaryFile(TLineReader &reader)
{
  TCHAR *line;
  while ((line = reader.read()) != NULL) {
    TCHAR *endptr;
    FlarmId id = FlarmId::Parse(line, &endptr);
    if (!id.IsDefined())
      /* ignore malformed records */
      continue;

    if (endptr > line && endptr[0] == _T('=') && endptr[1] != _T('\0')) {
      TCHAR *Name = endptr + 1;
      TrimRight(Name);
      if (!FlarmDetails::AddSecondaryItem(id, Name))
        break; // cant add anymore items !
    }
  }
}

void
FlarmDetails::LoadSecondary()
{
  LogStartUp(_T("OpenFLARMDetails"));

  // if (FLARM Details already there) delete them;
  if (!flarm_names.empty())
    flarm_names.clear();

  TLineReader *reader = OpenDataTextFile(_T("xcsoar-flarm.txt"));
  if (reader != NULL) {
    LoadSecondaryFile(*reader);
    delete reader;
  }
}

void
FlarmDetails::SaveSecondary()
{
  TextWriter *writer = CreateDataTextFile(_T("xcsoar-flarm.txt"));
  if (writer == NULL)
    return;

  TCHAR id[16];

  for (unsigned i = 0; i < flarm_names.size(); i++) {
    assert(flarm_names[i].id.IsDefined());

    writer->FormatLine(_T("%s=%s"),
                       flarm_names[i].id.Format(id),
                       flarm_names[i].name.c_str());
  }

  delete writer;
}

int
FlarmDetails::LookupSecondaryIndex(FlarmId id)
{
  for (unsigned i = 0; i < flarm_names.size(); i++) {
    assert(flarm_names[i].id.IsDefined());

    if (flarm_names[i].id == id)
      return i;
  }

  return -1;
}

int
FlarmDetails::LookupSecondaryIndex(const TCHAR *cn)
{
  for (unsigned i = 0; i < flarm_names.size(); i++) {
    assert(flarm_names[i].id.IsDefined());

    if (flarm_names[i].name.equals(cn))
      return i;
  }

  return -1;
}

const FlarmRecord *
FlarmDetails::LookupRecord(FlarmId id)
{
  // try to find flarm from FlarmNet.org File
  return FlarmNet::FindRecordById(id);
}

const TCHAR *
FlarmDetails::LookupCallsign(FlarmId id)
{
  // try to find flarm from userFile
  int index = LookupSecondaryIndex(id);
  if (index != -1)
    return flarm_names[index].name;

  // try to find flarm from FlarmNet.org File
  const FlarmRecord *record = FlarmNet::FindRecordById(id);
  if (record != NULL)
    return record->callsign;

  return NULL;
}

FlarmId
FlarmDetails::LookupId(const TCHAR *cn)
{
  // try to find flarm from userFile
  int index = LookupSecondaryIndex(cn);
  if (index != -1)
    return flarm_names[index].id;

  // try to find flarm from FlarmNet.org File
  const FlarmRecord *record = FlarmNet::FindFirstRecordByCallSign(cn);
  if (record != NULL)
    return record->GetId();

  FlarmId id;
  id.Clear();
  return id;
}

bool
FlarmDetails::AddSecondaryItem(FlarmId id, const TCHAR *name)
{
  if (!id.IsDefined())
    /* ignore malformed records */
    return false;

  int index = LookupSecondaryIndex(id);
  if (index != -1) {
    // modify existing record
    flarm_names[index].id = id;
    flarm_names[index].name = name;
    return true;
  }

  if (flarm_names.full())
    return false;

  // create new record
  FlarmIdNameCouple &item = flarm_names.append();
  item.id = id;
  item.name = name;

  return true;
}

unsigned
FlarmDetails::FindIdsByCallSign(const TCHAR *cn, FlarmId array[],
                                unsigned size)
{
  assert(cn != NULL);

  if (StringIsEmpty(cn))
    return 0;

  unsigned count = FlarmNet::FindIdsByCallSign(cn, array, size);

  for (unsigned i = 0; i < flarm_names.size() && count < size; i++) {
    if (flarm_names[i].name.equals(cn)) {
      assert(flarm_names[i].id.IsDefined());

      array[count] = flarm_names[i].id;
      count++;
    }
  }

  return count;
}
