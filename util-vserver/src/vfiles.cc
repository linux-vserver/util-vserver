// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on vfiles.cc by Jacques Gelinas
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

/*
	This utility is used to extract the list of non unified files in
	a vserver.
*/
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include <string>
#include <list>
#include <set>
#include "vutil.h"

using namespace std;

static bool ignorelink = false;


static void usage()
{
	cerr <<
		"vfiles version " << VERSION <<
		"\n\n"
		"vfiles [ options ] reference-server vserver\n"
		"\n"
		"--debug: Prints some debugging messages.\n"
		"--ignorelink: Do not print symbolic links (they are never unified)\n"
		"\n"
		;
}


static int vfiles_walk (
	string absdir,
	dev_t dev,			// We stay on the same volume
	string logical_dir,
	set<string> &files)
{
	int ret = -1;
	if (debug > 0) printf ("Entering directory %s\n",logical_dir.c_str());
	DIR *dir = opendir (absdir.c_str());
	if (dir == NULL){
		fprintf (stderr,"Can't open directory %s (%s)\n",absdir.c_str()
			,strerror(errno));
	}else{
		struct dirent *ent;
		ret = 0;
		while (ret == 0 && (ent=readdir(dir))!=NULL){
			if (strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0){
				continue;
			}
			string file = absdir + "/" + ent->d_name;
			struct stat st;
			if (vutil_lstat(file,st) == -1){
				ret = -1;
			}else if (st.st_dev != dev){
				if (debug > 0) printf ("Ignore sub-directory %s\n",file.c_str());
			}else{
				if (S_ISDIR(st.st_mode)){
					ret |= vfiles_walk (file,dev
							,logical_dir + "/" + ent->d_name,files);
				}else if (S_ISLNK(st.st_mode)){
					if (!ignorelink) printf ("%s\n",file.c_str());	
				}else if (S_ISBLK(st.st_mode)
					|| S_ISCHR(st.st_mode)
					|| S_ISFIFO(st.st_mode)){
					printf ("%s\n",file.c_str());	
				}else if (S_ISSOCK(st.st_mode)){
					// Do nothing
				}else{
					// Ok, this is a file. We either copy it or do a link
					string logical_file = logical_dir + "/" + ent->d_name;
					if (files.find (logical_file)==files.end()){
						printf ("%s\n",file.c_str());	
					}
				}
			}
		}
		closedir(dir);
	}
	return ret;
}

int main (int argc, char *argv[])
{
	int ret = -1;
	int i;
	for (i=1; i<argc; i++){
		const char *arg = argv[i];
		//const char *opt = argv[i+1];
		if (strcmp(arg,"--debug")==0){
			debug++;
		}else if (strcmp(arg,"--ignorelink")==0){
			ignorelink=true;
		}else{
			break;
		}
	}
	if (i!=argc-2){
		usage();
	}else{
		string refserv = argv[i++];
		string newserv = argv[i];
		list<PACKAGE> packages;
		// Load the files which are not configuration files from
		// the packages
		vutil_loadallpkg (refserv,packages);
		set<string> files;
		for (list<PACKAGE>::iterator it=packages.begin(); it!=packages.end(); it++){
			(*it).loadfiles(refserv,files);
		}
		struct stat st;
		if (vutil_lstat(newserv,st)!=-1){
			// Now, we do a recursive walk of newserv and prints
			// all files not unifiable
			ret = vfiles_walk (newserv,st.st_dev,"",files);
		}
	}
	return ret;
}



