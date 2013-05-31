/* Copyright_License {

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

#include "Profile/Profile.hpp"
#include "IO/FileLineReader.hpp"
#include "TestUtil.hpp"

static void
TestMap()
{
  Profile::Clear();

  {
    int value;
    ok1(!Profile::Exists(_T("key1")));
    ok1(!Profile::Get(_T("key1"), value));
    Profile::Set(_T("key1"), 4);
    ok1(Profile::Exists(_T("key1")));
    ok1(Profile::Get(_T("key1"), value));
    ok1(value == 4);
  }

  {
    short value;
    ok1(!Profile::Get(_T("key2"), value));
    Profile::Set(_T("key2"), 123);
    ok1(Profile::Get(_T("key2"), value));
    ok1(value == 123);
  }

  {
    unsigned value;
    ok1(!Profile::Get(_T("key3"), value));
    Profile::Set(_T("key3"), -42);
    ok1(Profile::Get(_T("key3"), value));
    ok1(value == -42u);
  }

  {
    bool value;
    ok1(!Profile::Get(_T("key4"), value));
    Profile::Set(_T("key4"), true);
    ok1(Profile::Get(_T("key4"), value));
    ok1(value);
    Profile::Set(_T("key4"), false);
    ok1(Profile::Get(_T("key4"), value));
    ok1(!value);
  }

  {
    fixed value;
    ok1(!Profile::Get(_T("key5"), value));
    Profile::Set(_T("key5"), fixed(1.337));
    ok1(Profile::Get(_T("key5"), value));
    ok1(equals(value, 1.337));
  }
}

static void
TestWriter()
{
  Profile::Clear();
  Profile::Set(_T("key1"), 4);
  Profile::Set(_T("key2"), _T("value2"));

  Profile::SaveFile(_T("output/TestProfileWriter.top"));

  FileLineReader reader(_T("output/TestProfileWriter.top"));
  if (reader.error()) {
    skip(3, 0, "read error");
    return;
  }

  unsigned count = 0;
  bool found1 = false, found2 = false;

  TCHAR *line;
  while ((line = reader.read()) != NULL) {
    if (_tcscmp(line, _T("key1=\"4\"")) == 0)
      found1 = true;
    if (_tcscmp(line, _T("key2=\"value2\"")) == 0)
      found2 = true;

    count++;
  }

  ok1(count == 2);
  ok1(found1);
  ok1(found2);
}

static void
TestReader()
{
  Profile::Clear();
  Profile::LoadFile(_T("test/data/TestProfileReader.top"));

  {
    int value;
    ok1(Profile::Exists(_T("key1")));
    ok1(Profile::Get(_T("key1"), value));
    ok1(value == 1);
  }

  {
    StaticString<32> value;
    ok1(Profile::Exists(_T("key2")));
    ok1(Profile::Get(_T("key2"), value));
    ok1(value == _T("value"));
  }

  {
    int value;
    ok1(Profile::Exists(_T("key3")));
    ok1(Profile::Get(_T("key3"), value));
    ok1(value == 5);
  }
}

int main(int argc, char **argv)
{
  plan_tests(31);

  TestMap();
  TestWriter();
  TestReader();

  return exit_status();
}
