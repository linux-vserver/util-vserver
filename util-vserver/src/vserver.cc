// $Id$    --*- c++ -*--

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "vserver.hh"
#include "pathconfig.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <iostream>

static bool
dirExists(std::string const &path)
{
  struct stat	st;
  return (stat(path.c_str(), &st)!=-1 &&
	  S_ISDIR(st.st_mode));
}

Vserver::Vserver(std::string const &str) :
  conf_dir_(str), vdir_(str), name_(str)
{
  vdir_       += "/.vdir";
  
  if (!dirExists(vdir_)) {
    conf_dir_  = CONFDIR;
    conf_dir_ += str;
  }

  rpmdb_path_  = conf_dir_;
  rpmdb_path_ += "/apps/pkgmgmt/rpmstate";

  if (!dirExists(rpmdb_path_)) {
    rpmdb_path_  = conf_dir_;
    rpmdb_path_ += "/apps/pkgmgmt/base/rpm/state";
  }

  if (!dirExists(rpmdb_path_)) {
    rpmdb_path_  = vdir_;
    rpmdb_path_ += "/var/lib/rpm";
  }

  std::string	tmp = conf_dir_;
  tmp  += ".name";

  std::ifstream		name_file(tmp.c_str());

  if (name_file.good()) getline(name_file, name_);
}

std::ostream &
operator << (std::ostream &lhs, Vserver const &rhs)
{
  return lhs << rhs.getName();
}
