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


#ifndef HH_UTIL_VSERVER_SRC_VSERVSER_HH
#define HH_UTIL_VSERVER_SRC_VSERVSER_HH

#include <string>
#include <ostream>

class Vserver
{
  public:
    Vserver(std::string const &name);

    std::string const &		getConfDir() const   { return conf_dir_; }
    std::string const &		getVdir() const      { return vdir_; }
    std::string const &		getRPMDbPath() const { return rpmdb_path_; }
    std::string const &		getName() const      { return name_; }

  private:
    std::string		conf_dir_;
    std::string		vdir_;
    std::string		rpmdb_path_;
    std::string		name_;
};

std::ostream &	operator << (std::ostream &lhs, Vserver const &rhs);


#endif	//  HH_UTIL_VSERVER_SRC_VSERVSER_HH
