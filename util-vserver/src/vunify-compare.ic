// $Id$    --*- c -*--

// Copyright (C) 2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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


static bool
compareUnify(struct stat const *lhs, struct stat const *rhs)
{
  return (lhs->st_dev  ==rhs->st_dev  &&
	  lhs->st_ino  !=rhs->st_ino  &&
	  lhs->st_mode ==rhs->st_mode &&
	  lhs->st_uid  ==rhs->st_uid  &&
	  lhs->st_gid  ==rhs->st_gid  &&
	  lhs->st_size ==rhs->st_size &&
          lhs->st_mtime==rhs->st_mtime);

  // TODO: acl? time?
}

static bool
compareDeUnify(struct stat const *lhs, struct stat const *rhs)
{
  return (lhs->st_dev ==rhs->st_dev  &&
	  lhs->st_ino ==rhs->st_ino);
}
