// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on vunify.cc by Jacques Gelinas
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
	This utility is used to unify (using hard links) two or more
	virtual servers.
	It compares the each vserver with the first one and for every
	common package (RPM, same version), it does a hard link on non
	configuration file. It turns the file immutable after that.
*/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "vserver.hh"
#include "util.h"

#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include "vutil.h"

using namespace std;

static bool undo = false;

static int  ext2flags = EXT2_IMMUTABLE_FILE_FL | EXT2_IMMUTABLE_LINK_FL;
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

#define OPTION_TEST		1024
#define OPTION_UNDO		1025
#define OPTION_DEBUG		1026
#define OPTION_NOFLAGS		1027
#define OPTION_IMMUTABLE	1028
#define OPTION_IMM_UNLINK	1029
#define OPTION_EXCLDIR		1030
#define OPTION_INCLDIR		1031

static struct option const
CMDLINE_OPTIONS[] = {
  { "help",                no_argument,       0, 'h' },
  { "version",             no_argument,       0, 'v' },
  { "test",                no_argument,       0, OPTION_TEST },
  { "undo",                no_argument,       0, OPTION_UNDO },
  { "debug",               no_argument,       0, OPTION_DEBUG },
  { "noflags",             no_argument,       0, OPTION_NOFLAGS },
  { "immutable",           no_argument,       0, OPTION_IMMUTABLE },
  { "immutable-mayunlink", no_argument,       0, OPTION_IMM_UNLINK },
  { "excldir",             required_argument, 0, OPTION_EXCLDIR },
  { "incldir",             required_argument, 0, OPTION_INCLDIR },
  { 0,0,0,0 }
};
  
  
static vector<EXCLDIR> excldirs;
static vector<EXCLDIR> incldirs;

static void
showHelp(std::ostream &out, char const *, int exit_code)
{
  out << "Usage: \n"
    "vunify [ options ] reference-server vservers ... -- packages\n"
    "vunify [ options ] reference-server vservers ... -- ALL\n"
    "\n"
    "--test: Show what will be done, do not do it.\n"
    "--undo: Put back the file in place, using copies from the\n"
    "        reference server.\n"
    "--debug: Prints some debugging messages.\n"
    "--noflags: Do not put any immutable flags on the file\n"
    "--immutable: Set the immutable_file bit on the files.\n"
    "--immutable-mayunlink: Sets the immutable_link flag on files.\n"
    "\n"
    "--excldir: None of the files under a given directory will be unified\n"
    "\tThe directory is expressed in absolute/logical form (relative\n"
    "\tto the vserver root (ex: /var/log)\n"
    "\n"
    "--incldir: All the files under a given directory will be unified\n"
    "\tThe directory is expressed in absolute/logical form (relative\n"
    "\tto the vserver root (ex: /var/log)\n"
    "\n"
    "By default, the immutable_file and	immutable_link flags are\n"
    "set on the files. So if you want no immutable flags, you must\n"
    "use --noflags. If you want a single flag, you must use\n"
    "--noflags first, then the --immutable or --immutable-mayunlink\n"
    "flag.\n";

  std::exit(exit_code);
}

static void
showVersion()
{
  std::cout << "vunify " VERSION " -- unifies files of vservers\n"
    "This program is part of " PACKAGE_STRING "\n\n"
    "Copyright (C) 2003 Enrico Scholz\n"
    VERSION_COPYRIGHT_DISCLAIMER;

  std::exit(0);
}

static bool vunify_inside (vector<EXCLDIR> &dirs, const char *path)
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

class PACKAGE_UNI: public Package{
public:
	list<string> files;		// Files to unify
							// This is loaded on demand
	PACKAGE_UNI(string &_name, string &_version)
		: Package(_name,_version)
	{
	}
	PACKAGE_UNI(const char *_name, const char *_version)
		: Package (_name,_version)
	{
	}
	PACKAGE_UNI(const string &line)
		: Package (line)
	{
	}
	// Load the file member of the package, but exclude configuration file
	void loadfiles(Vserver const &ref)
	{
		if (files.empty()){
			if (debug) cout << "Loading files for package " << name << endl;
			string namever;
			namever = name + '-' + version;
			FILE *fin = vutil_execdistcmd (K_UNIFILES,ref,namever.c_str());
			if (fin != NULL){
				char tmp[1000];
				while (fgets(tmp,sizeof(tmp)-1,fin)!=NULL){
					int last = strlen(tmp)-1;
					if (last >= 0 && tmp[last] == '\n') tmp[last] = '\0';
					bool must_unify = false;
					int type = 0;	// K_UNIFILES only report unify-able files
					if(type == 0 && !vunify_inside(excldirs,tmp)){
						must_unify = true;
					}else if(vunify_inside(incldirs,tmp)){
						must_unify = true;
					}
					if (must_unify){
						files.push_front (tmp);
					}else if (debug){
						cout << "Package " << name << " exclude " << tmp << endl;
					}
				}
			}
			if (debug) cout << "Done\n";
		}
	}
};


static ostream & operator << (ostream &c, const PACKAGE_UNI &p)
{
	return c << p.name << "-" << p.version;
}

template<class T>
	void printit(T a){
		cout << "xx " << a << endl;
	}

template<class T>
	class printer{
		string title;
		public:
		printer (const char *_title): title(_title){}
		void operator()(T const &a){
			cout << title << " " << a << endl;
		}
	};


/*
	Load the list of all packages in a vserver
*/
static void vunify_loadallpkg (Vserver const &refserver, list<PACKAGE_UNI> &packages)
{
	FILE *fin = vutil_execdistcmd (K_PKGVERSION,refserver,NULL);
	if (fin != NULL){
		char line[1000];
		while (fgets(line,sizeof(line)-1,fin)!=NULL){
			// fprintf (stderr,"line: %s",line);
			int last = strlen(line)-1;
			if (last >= 0 && line[last] == '\n') line[last] = '\0';
			packages.push_back (PACKAGE_UNI(line));
		}
		pclose (fin);
	}
}

/*
	Object to unify a file
	The file is first removed, then a hard link is made  and then
	the immutable flag is done
*/
class file_unifier{
    Vserver const	&ref_server;
    Vserver const	&target_server;
    int &ret;

  public:
    file_unifier(Vserver const &_ref, Vserver const &_target, int &_ret)
      : ref_server(_ref),target_server(_target), ret(_ret)
    {}

    void operator()(const string &file)
    {
      std::string	refpath = ref_server.getVdir();
      refpath  	       += file;
	  
      std::string	dstpath = target_server.getVdir();
      dstpath              += file;
	  
      if (debug) cout << "Unify " << refpath << " -> " << dstpath << endl;
      struct stat st;
      if (stat(refpath.c_str(),&st)==-1){
	if (debug) cout << "File " << refpath << " does not exist, ignored\n";
      }else if (setext2flag(refpath.c_str(),false,ext2flags)==-1){
	ret = -1;
      }else if (vbuild_unlink(dstpath.c_str())==-1){
	ret = -1;
	cerr << "Can't delete file " << dstpath
	     << " (" << strerror(errno) << ")\n";
      }else{
	if (undo){
	  if (vbuild_file_copy(refpath.c_str(),dstpath.c_str(),st)==-1){
	    ret = -1;
	    cerr << "Can't copy file " << refpath << " to " << dstpath
		 << " (" << strerror(errno) << ")\n";
	  }
	}else{
	  if (vbuild_link(refpath.c_str(),dstpath.c_str())==-1){
	    ret = -1;
	    cerr << "Can't link file " << refpath << " to " << dstpath
		 << " (" << strerror(errno) << ")\n";
	  }
	}
	  // We put back the original immutable because other vservers
	  // may be unified on it.
	if (setext2flag(refpath.c_str(),true,ext2flags)==-1){
	  ret = -1;
	}
      }
    }
};

#if 0
// Check if two package have the same name (but potentially different version)
class same_name{
	PACKAGE_UNI &pkg;
public:
	same_name(PACKAGE_UNI &_pkg) : pkg(_pkg) {}
	bool operator()(const PACKAGE_UNI &p)
	{
		return pkg.name == p.name;
	}
};
#endif
  // Predicate to decide if a package must be unified
class package_unifier{
  public:
    Vserver const 	&ref_server;
    Vserver const 	&target_server;
    list<PACKAGE_UNI>	&target_packages;
    int			&ret;
    
    package_unifier(Vserver const &_ref,
		    Vserver const &_target,
		    list<PACKAGE_UNI> &_target_packages,
		    int &_ret) :
      ref_server(_ref),target_server(_target),
      target_packages(_target_packages), ret(_ret)
    {}
    void operator()(PACKAGE_UNI &pkg)
    {
      if (find(target_packages.begin(),target_packages.end(),pkg)
	  !=target_packages.end()){
	  // Ok, the package is also in the target vserver
	cout << "Unify pkg " << pkg << " from " << ref_server << " to "
	     << target_server << endl;

	if (!testmode || debug){
	  pkg.loadfiles(ref_server);
	  for_each (pkg.files.begin(),pkg.files.end()
		    ,file_unifier(ref_server,target_server,ret));
	}
      }else if (testmode){
	  // The package is missing, in test mode we provide more information
	if (find_if(target_packages.begin(),target_packages.end(),same_name(pkg))
	    !=target_packages.end()){
	  cout << pkg << " exist in server " << target_server << " not unified\n";
	}else{
	  cout << pkg << " does not exist in server " << target_server << endl;
	}
      }
    }
};

// For each vserver, find the common packages and unify them
class server_unifier{
public:
    list<PACKAGE_UNI>	&ref_packages;
    Vserver const	&ref_server;
    int			&ret;
    
    server_unifier(Vserver const &_ref_server, list<PACKAGE_UNI> &_packages, int &_ret)
      : ref_packages(_packages),ref_server(_ref_server), ret(_ret)
    {}
    
    void operator()(Vserver const &serv)
    {
      list<PACKAGE_UNI> pkgs;
      vunify_loadallpkg (serv,pkgs);
      for_each(ref_packages.begin(),ref_packages.end()
	       ,package_unifier(ref_server,serv,pkgs,ret));
    }
};

class deleteif{
public:
	char **argv0,**argvn;
	deleteif(char **_argv0, char **_argvn): argv0(_argv0),argvn(_argvn){}
	bool operator()(const PACKAGE_UNI &pkg)
	{
		bool found = false;
		for (char **pt = argv0; pt < argvn; pt++){
			if (pkg.name == *pt){
				found = true;
				break;
			}
		}
		return !found;
	}
};

int main (int argc, char *argv[])
{
  while (1) {
    int		c = getopt_long(argc, argv, "hv", CMDLINE_OPTIONS, 0);
    switch (c) {
      case 'h'			:  showHelp(std::cout, argv[0], 0); break;
      case 'v'			:  showVersion();                   break;
      case OPTION_TEST		:  testmode = true; break;
      case OPTION_UNDO		:  undo     = true; break;
      case OPTION_DEBUG		:  ++debug;         break;
      case OPTION_NOFLAGS	:  ext2flags = 0;   break;
      case OPTION_IMMUTABLE	:  ext2flags |= EXT2_IMMUTABLE_FILE_FL; break;
      case OPTION_IMM_UNLINK	:  ext2flags |= EXT2_IMMUTABLE_LINK_FL; break;
      case OPTION_EXCLDIR	:  excldirs.push_back(EXCLDIR(optarg)); break;
      case OPTION_INCLDIR	:  incldirs.push_back(EXCLDIR(optarg)); break;
      default			:
	std::cerr << "Try '" << argv[0] << " --help' for more information." << std::endl;
	return EXIT_FAILURE;
    }
  }

  if (optind+1>=argc) {
    std::cerr << "No vserver specified" << std::endl;
    return EXIT_FAILURE;
  }

  Vserver			refserv(argv[optind++]);
  std::vector<Vserver>		vservers;
  while (optind<argc && strcmp(argv[optind], "--")!=0)
    vservers.push_back(Vserver(argv[optind++]));

  ++optind;
  if (optind>=argc) {
    std::cerr << "No packages specified" << std::endl;
    return EXIT_FAILURE;
  }

  for_each (vservers.begin(),vservers.end(),printer<Vserver>("vservers"));

  std::list<PACKAGE_UNI>	packages;
  vunify_loadallpkg(refserv, packages);
  
  if (optind!=argc-1 || strcmp(argv[optind], "ALL")!=0)
    packages.remove_if(deleteif(argv+optind, argv+argc));

  int		ret = EXIT_SUCCESS;
  umask(0);
  for_each(vservers.begin(), vservers.end(), server_unifier(refserv,packages,ret));

  return ret;
}
