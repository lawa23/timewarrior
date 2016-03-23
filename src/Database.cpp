////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2015 - 2016, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <Database.h>
#include <FS.h>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <ctime>

////////////////////////////////////////////////////////////////////////////////
void Database::initialize (const std::string& location)
{
  _location = location;
  _current = currentDataFile ();

  // Because the data files have names YYYY-MM.data, sorting them by name also
  // sorts by the intervals within.
  Directory d (_location);
  auto files = d.list ();
  std::sort (files.begin (), files.end ());

  // Ensure the list always contains the name of the current file, even if it
  // does not exist.
  if (std::find (files.begin (), files.end (), _current) == files.end ())
    files.push_back (_current);

  for (auto& file : files)
  {
    // If it looks like a data file.
    if (file.find (".data") == file.length () - 5)
    {
      Datafile df;
      df.initialize (file);
      _files.push_back (df);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Database::commit ()
{
  if (_dirty)
  {
    for (auto& file : _files)
      file.commit ();

    _dirty = false;
  }
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Database::files () const
{
  std::vector <std::string> all;
  for (auto& file : _files)
    all.push_back (file.name ());

  return all;
}

////////////////////////////////////////////////////////////////////////////////
Interval Database::getLatestInterval ()
{
  return _files.back ().getLatestInterval ();
}

////////////////////////////////////////////////////////////////////////////////
std::vector <Interval> Database::getAllIntervals ()
{
  std::vector <Interval> all;
  for (auto& file : _files)
  {
    auto i = file.getAllIntervals ();
    all.insert (all.end (),
                std::make_move_iterator (i.begin ()),
                std::make_move_iterator (i.end ()));
  }

  return all;
}

////////////////////////////////////////////////////////////////////////////////
void Database::addExclusion (const std::string& exclusion)
{
  _files.back ().addExclusion (exclusion);
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
void Database::addInterval (const Interval& interval)
{
  _files.back ().addInterval (interval);
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
void Database::modifyInterval (const Interval& interval)
{
  _files.back ().modifyInterval (interval);
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
std::string Database::dump () const
{
  std::stringstream out;
  out << "Database\n";
  for (const auto& file : _files)
    out << "  Datafile: " << file.name ()
        << (file.name () == _current ? " (current)" : "")
        << "\n";

  out << "  Dirty: " << (_dirty ? "true" : "false") << "\n";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string Database::currentDataFile () const
{
  time_t current;
  time (&current);
  struct tm* t = gmtime (&current);

  std::stringstream out;
  out << _location
      << '/'
      << std::setw (4) << std::setfill ('0') << (t->tm_year + 1900)
      << '-'
      << std::setw (2) << std::setfill ('0') << (t->tm_mon + 1)
      << ".data";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
