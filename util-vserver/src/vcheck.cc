// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on vcheck.cc by Jacques Gelinas
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
	This utility is used to compare two vservers. One is known to
	be clean and the other is potentially corrupted (cracked). The
	goal of this program is to run the rpm verify command, but using
	the RPM database of the first vserver.
*/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include <string>
#include <vector>
#include <list>
#include <set>
#include "vutil.h"

using namespace std;


static void usage()
{
	cerr <<
		"vcheck version " << VERSION <<
		"\n\n"
		"vcheck [ options ] reference-server chk-vservers\n"
		"\n"
		"--diffpkgs: Shows which package differ.\n"
		"            + means the package only exists in chk-server.\n"
		"            - means the package does not exist in chk-server.\n"
		"            ! means the servers have different version.\n"
		"\n"
		"--verify: Execute an RPM verify on common packages.\n"
		"--debug: Turn on some (useless) debugging messages.\n"
		;
}

typedef list<PACKAGE> PACKAGES;

/*
	Delete a directory silently
*/
static int vcheck_deldir (const string &path)
{
	int ret = -1;
	struct stat st;
	if (lstat(path.c_str(),&st)==-1){
		ret = 0;
	}else{
		if (!S_ISDIR(st.st_mode)){
			fprintf (stderr,"%s already exist and is not a directory\n"
				,path.c_str());
			exit (-1);
		}else{
			DIR *d = opendir (path.c_str());
			if (d != NULL){
				struct dirent *ent;
				ret = 0;
				while ((ent=readdir(d))!=NULL){
					if (strcmp(ent->d_name,".")!=0
						&& strcmp(ent->d_name,"..")!=0){
						string tmp = path + "/" + ent->d_name;
						if (unlink(tmp.c_str())==-1){
							fprintf (stderr,"Can't delete file %s (%s)\n",tmp.c_str()
								,strerror(errno));
							ret = -1;
							break;
						}
					}
				}
				closedir (d);
				rmdir (path.c_str());
			}
		}
	}
	return ret;
}
		

static int vcheck_copydb (const string &refserv, const string &path)
{
	int ret = -1;
	string refpath = refserv + "/var/lib/rpm";
	DIR *d = opendir (refpath.c_str());
	if (d == NULL){
		fprintf (stderr,"Can't open directory %s (%s)\n",refpath.c_str()
			,strerror(errno));
	}else{
		ret = 0;
		struct dirent *ent;
		while ((ent=readdir(d))!=NULL){
			if (strcmp(ent->d_name,".")!=0
				&& strcmp(ent->d_name,"..")!=0){
				string srcpath = refpath + "/" + ent->d_name;
				const char *spath = srcpath.c_str();
				struct stat st;
				if (stat(spath,&st)!=-1){
					string dstpath = path + "/" + ent->d_name;
					if (file_copy (spath,dstpath.c_str(),st) == -1){
						ret = -1;
						break;
					}
				}else{
					ret = -1;
					fprintf (stderr,"Can't stat %s (%s)\n",spath,strerror(errno));
					break;
				}
			}
		}
		closedir (d);
	}
	return ret;
}

class cmp_name{
public:
	int operator()(const PACKAGE &p1, const PACKAGE &p2){
		return strcmp(p1.name.c_str(),p2.name.c_str());
	}
};


int main (int argc, char *argv[])
{
	int ret = -1;
	bool diffpkg = false;
	bool verify = false;
	int i;
	for (i=1; i<argc; i++){
		const char *arg = argv[i];
		//const char *opt = argv[i+1];
		if (strcmp(arg,"--diffpkg")==0){
			diffpkg = true;
		}else if (strcmp(arg,"--verify")==0){
			verify = true;
		}else if (strcmp(arg,"--debug")==0){
			debug++;
		}else{
			break;
		}
	}
	if (i!=argc-2){
		usage();
	}else{
		string refserv = argv[i++];
		string chkserv = argv[i];
		PACKAGES refpkgs,chkpkgs;
		// Load the package list from both vservers
		vutil_loadallpkg (refserv,refpkgs);
		vutil_loadallpkg (chkserv,chkpkgs);
		PACKAGES common, differ, added, removed;
		// Find which package are different, missing and added
		// to chkserv
		for (PACKAGES::iterator it=refpkgs.begin(); it!=refpkgs.end(); it++){
			PACKAGES::iterator f = find_if(chkpkgs.begin(),chkpkgs.end(),same_name(*it));
			if (f == chkpkgs.end()){
				removed.push_back (*it);
			}else if (f->version != it->version){
				differ.push_back (*it);
			}else{
				common.push_back (*it);
			}
		}
		for (list<PACKAGE>::iterator it=chkpkgs.begin(); it!=chkpkgs.end(); it++){
			list<PACKAGE>::iterator f = find_if(refpkgs.begin(),refpkgs.end(),same_name(*it));
			if (f == refpkgs.end()){
				added.push_back (*it);
			}
		}
		differ.sort ();
		added.sort();
		removed.sort();
		common.sort ();
		bool something = false;
		if (diffpkg){
			for (PACKAGES::iterator it=removed.begin(); it!=removed.end(); it++){
				printf ("- %s\n",it->name.c_str());
			}
			for (PACKAGES::iterator it=added.begin(); it!=added.end(); it++){
				printf ("+ %s\n",it->name.c_str());
			}
			for (PACKAGES::iterator it=differ.begin(); it!=differ.end(); it++){
				printf ("! %s\n",it->name.c_str());
			}
			something = true;
		}
		if (verify){
			// We copy the rpm database from the reference vserver to
			// the target vserver
			string dbpath = chkserv + "/tmp/vcheck.db";
			vcheck_deldir (dbpath);
			if (mkdir (dbpath.c_str(),0)==-1){
				fprintf (stderr,"Can't create directory %s (%s)\n"
					,dbpath.c_str(),strerror(errno));
			}else if (vcheck_copydb (refserv,dbpath) != -1){
				// We only compare the common package
				string cmd = "rpm --dbpath /tmp/vcheck.db --root " + chkserv + " -V";
				for (PACKAGES::iterator it=common.begin(); it!=common.end(); it++){
					//printf ("compare %s\n",it->name.c_str());
					cmd += " " + it->name;
				}
				if (debug) printf ("CMD: %s\n",cmd.c_str());
				system (cmd.c_str());
			}
			vcheck_deldir (dbpath);
			something = true;
		}
		if (!something){
			fprintf (stderr,"Nothing to do !!!\n\n");
			usage();
		}
	}
	return ret;
}



