// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on vutil.h by Jacques Gelinas
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#pragma interface
#ifndef VUTIL_H
#define VUTIL_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string>
#include <set>
#include <algorithm>
#include <iostream>
#include <list>

using namespace std;

extern int debug;
extern bool testmode;

// Patch to help compile this utility on unpatched kernel source
#ifndef EXT2_IMMUTABLE_FILE_FL
	#define EXT2_IMMUTABLE_FILE_FL	0x00000010
	#define EXT2_IMMUTABLE_LINK_FL	0x00008000
#endif


FILE *vutil_execdistcmd (const char *, const string &, const char *);
extern const char K_DUMPFILES[];
extern const char K_UNIFILES[];
extern const char K_PKGVERSION[];

class Package{
public:
	string name;
	string version;	// version + release
	Package(string &_name, string &_version)
		: name (_name), version(_version)
	{
	}
	Package(const char *_name, const char *_version)
		: name (_name), version(_version)
	{
	}
	Package(const string &line)
	{
		*this = line;
	}
	Package & operator = (const string &_line)
	{
		string line (_line);
		string::iterator pos = find (line.begin(),line.end(),'=');
		if (pos != line.end()){
			name = string(line.begin(),pos);
			version = string(pos + 1,line.end());
		}
		return *this;
	}
	Package (const Package &pkg)
	{
		name = pkg.name;
		version = pkg.version;
	}
	bool operator == (const Package &v) const
	{
		return name == v.name && version == v.version;
	}
	bool operator < (const Package &v) const
	{
		bool ret = false;
		if (name < v.name){
			ret = true;
		}else if (name == v.name && version < v.version){
			ret = true;
		}
		return ret;
	}
	// Load the file member of the package, but exclude configuration file
	void loadfiles(const string &ref, set<string> &files)
	{
		if (debug > 2) cout << "Loading files for package " << name << endl;
		string namever = name + '-' + version;
		FILE *fin = vutil_execdistcmd (K_UNIFILES,ref,namever.c_str());
		if (fin != NULL){
			char tmp[1000];
			while (fgets(tmp,sizeof(tmp)-1,fin)!=NULL){
				int last = strlen(tmp)-1;
				if (last >= 0 && tmp[last] == '\n') tmp[last] = '\0';
				files.insert (tmp);
			}
		}
		if (debug > 2) cout << "Done\n";
	}
	#if 0
	bool locate(const string &path)
	{
		return find (files.begin(),files.end(),path) != files.end();
	}
	#endif
};

// Check if two package have the same name (but potentially different version)
class same_name{
	const Package &pkg;
public:
	same_name(const Package &_pkg) : pkg(_pkg) {}
	bool operator()(const Package &p)
	{
		return pkg.name == p.name;
	}
};


#include "vutil.p"

#endif

