// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on vbuild.cc by Jacques Gelinas
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
	This utility is used to build a new vserver using a reference vserver.
	It uses hard link whenever possible instead of duplicating files.
	Once done, it sets the immutable bits.
*/
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include <string>
#include <vector>
#include <list>
#include <set>
#include "vutil.h"

using namespace std;

struct EXCLDIR{
	string prefix;
	int len;
	EXCLDIR(const char *s)
	{
		prefix = s;
		prefix += '/';
		len = prefix.size();
	}
};
static vector<EXCLDIR> excldirs;


static int  ext2flags = EXT2_IMMUTABLE_FILE_FL | EXT2_IMMUTABLE_LINK_FL;
static struct {
	int nblink;
	int nbcopy;
	long size_copy;
	int nbdir;
	int nbsymlink;
	int nbspc;
} stats;


static void usage()
{
	cerr <<
		"vbuild version " << VERSION <<
		"\n\n"
		"vbuild [ options ] reference-server new-vservers\n"
		"\n"
		"--test: Show what will be done, do not do it.\n"
		"--debug: Prints some debugging messages.\n"
		"\n"
		"--excldir: None of the files under a given directory will be copied\n"
		"\tThe directory is expressed in absolute/logical form (relative\n"
		"\tto the vserver root (ex: /var/log)\n"
		"\n"
		"\n"
		"--noflags: Do not put any immutable flags on the file\n"
		"--immutable: Set the immutable_file bit on the files.\n"
		"--immutable-mayunlink: Sets the immutable_link flag on files.\n"
		"--stats: Produce statistics on the number of file linked\n"
		"         copied and so on.\n"
		"\n"
		"By default, the immutable_file and	immutable_link flags are\n"
		"set on the files. So if you want no immutable flags, you must\n"
		"use --noflags. If you want a single flag, you must use\n"
		"--noflags first, then the --immutable or --immutable-mayunlink\n"
		"flag.\n"
		;
}

/*
	Return true if a directory lies inside a directory set
*/
static bool vbuild_inside (vector<EXCLDIR> &dirs, const char *path)
{
	bool found = false;
	for (unsigned i=0; i<dirs.size(); i++){
		if (strncmp(dirs[i].prefix.c_str(),path,dirs[i].len)==0){
			found = true;
			break;
		}
	}
	return found;
}



static int vbuild_copy (
	string refserv,
	string newserv,
	dev_t dev,			// We stay on the same volume
	string logical_dir,
	set<string> &files)
{
	int ret = -1;
	if (debug > 0) printf ("Copying directory %s\n",logical_dir.c_str());
	DIR *dir = opendir (refserv.c_str());
	if (dir == NULL){
		fprintf (stderr,"Can't open directory %s (%s)\n",refserv.c_str()
			,strerror(errno));
	}else{
		logical_dir += "/";
		bool copy_files = !vbuild_inside(excldirs,logical_dir.c_str());
		struct dirent *ent;
		ret = 0;
		while (ret == 0 && (ent=readdir(dir))!=NULL){
			if (strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0){
				continue;
			}
			string file = refserv + "/" + ent->d_name;
			struct stat st;
			if (vutil_lstat(file.c_str(),st) == -1){
				ret = -1;
			}else if (st.st_dev != dev){
				if (debug > 0) printf ("Ignore sub-directory %s\n",file.c_str());
			}else{
				string newfile = newserv + "/" + ent->d_name;
				if (S_ISDIR(st.st_mode)){
					if (vbuild_mkdir (newfile.c_str(),st.st_mode)==-1){
						fprintf (stderr,"Can't mkdir %s (%s)\n"
							,newfile.c_str(),strerror(errno));
						ret = -1;
					}else{
						stats.nbdir++;
						if (vbuild_chown(newfile.c_str(),st.st_uid,st.st_gid)==-1){
							fprintf (stderr,"Can't chown %s (%s)\n"
								,newfile.c_str(),strerror(errno));
							ret = -1;
						}
						ret |= vbuild_copy (file,newfile,dev
							,logical_dir + ent->d_name,files);
					}
				}else if (S_ISLNK(st.st_mode)){
					char path[PATH_MAX];
					int len = readlink(file.c_str(),path,sizeof(path)-1);
					if (len < 0){
						fprintf (stderr,"Can't readlink %s (%s)\n"
							,file.c_str(),strerror(errno));
						ret = -1;
					}else{
						path[len] = '\0';
						stats.nbsymlink++;
						if (vbuild_symlink (path,newfile.c_str())==-1){
							fprintf (stderr,"Can't symlink %s to %s (%s)\n",
								newfile.c_str(),path,strerror(errno));
						}
					}
				}else if (S_ISBLK(st.st_mode)
					|| S_ISCHR(st.st_mode)
					|| S_ISFIFO(st.st_mode)){
					stats.nbspc++;
					if (vbuild_mknod (newfile.c_str(),st.st_mode,st.st_rdev)==-1){
						fprintf (stderr,"Can't mknod %s (%s)\n"
							,newfile.c_str(),strerror(errno));
						ret = -1;
					}
				}else if (S_ISSOCK(st.st_mode)){
					// Do nothing
				}else if (copy_files){
					// Ok, this is a file. We either copy it or do a link
					string logical_file = logical_dir + ent->d_name;
					if (files.find (logical_file)==files.end()){
						if (debug > 1) printf ("Copying file %s\n",file.c_str());
						if (vbuild_file_copy (file.c_str(),newfile.c_str(),st)==-1){
							fprintf (stderr,"Can't copy %s to %s (%s)\n",
								file.c_str(),newfile.c_str(),strerror(errno));
							ret = -1;
						}else{
							stats.size_copy += st.st_size;
							stats.nbcopy++;
						}
					}else{
						if (debug > 2) printf ("Linking file %s\n",file.c_str());
						setext2flag (file.c_str(),false,ext2flags);
						stats.nblink++;
						if (vbuild_link (file.c_str(),newfile.c_str())==-1){
							fprintf (stderr,"Can't link %s to %s (%s)\n",
								file.c_str(),newfile.c_str(),strerror(errno));
							ret = -1;
						}
						setext2flag (file.c_str(),true,ext2flags);
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
	bool statistics = false;
	int i;
	for (i=1; i<argc; i++){
		const char *arg = argv[i];
		//const char *opt = argv[i+1];
		if (strcmp(arg,"--test")==0){
			testmode = true;
		}else if (strcmp(arg,"--debug")==0){
			debug++;
		}else if (strcmp(arg,"--stats")==0){
			statistics = true;
		}else if (strcmp(arg,"--noflags")==0){
			ext2flags = 0;
		}else if (strcmp(arg,"--immutable")==0){
			ext2flags |= EXT2_IMMUTABLE_FILE_FL;
		}else if (strcmp(arg,"--immutable-mayunlink")==0){
			ext2flags |= EXT2_IMMUTABLE_LINK_FL;
		}else if (strcmp(arg,"--excldir")==0){
			i++;
			excldirs.push_back (EXCLDIR(argv[i]));
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
		// Now, we do a recursive copy of refserv into newserv
		umask (0);
		mkdir (newserv.c_str(),0755);
		// Check if it is on the same volume
		struct stat refst,newst;
		if (vutil_lstat(refserv,refst)!=-1
			&& vutil_lstat(newserv,newst)!=1){
			if (refst.st_dev != newst.st_dev){
				fprintf (stderr,"Can't vbuild %s because it is not on the same volume as %s\n"
					,newserv.c_str(),refserv.c_str());
			}else{
				stats.nbdir = stats.nblink = stats.nbcopy = stats.nbsymlink = 0;
				stats.nbspc = 0;
				stats.size_copy = 0;
				ret = vbuild_copy (refserv,newserv,refst.st_dev,"",files);
				if (statistics){
					printf ("Directory created: %d\n",stats.nbdir);
					printf ("Files copied     : %d\n",stats.nbcopy);
					printf ("Bytes copied     : %ld\n",stats.size_copy);
					printf ("Files linked     : %d\n",stats.nblink);
					printf ("Files symlinked  : %d\n",stats.nbsymlink);
					printf ("Special files    : %d\n",stats.nbspc);
				}
			}
		}
	}
	return ret;
}


