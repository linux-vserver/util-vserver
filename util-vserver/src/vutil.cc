// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on vutil.cc by Jacques Gelinas
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

#pragma implementation
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include "vutil.h"
#include <sys/ioctl.h>
#include <linux/ext2_fs.h>

bool testmode;
int debug;

int file_copy (const char *src, const char *dst, struct stat &st)
{
	int ret = -1;
	FILE *fin = fopen (src,"r");
	if (fin == NULL){
		fprintf (stderr,"Can't open file %s (%s)\n",src,strerror(errno));
	}else{
		FILE *fout = fopen (dst,"w");
		if (fout == NULL){
			fprintf (stderr,"Can't open file %s (%s)\n",dst,strerror(errno));
		}else{
			char buf[8192];
			int len;
			while ((len=fread(buf,1,sizeof(buf),fin))>0){
				fwrite (buf,1,len,fout);
			}
			fflush (fout);
			ret = 0;
			if (fchown (fileno(fout),st.st_uid,st.st_gid)==-1){
				fprintf (stderr,"Can't chown file %s (%s)\n"
					,dst,strerror(errno));
				ret = -1;
			}else if (fchmod (fileno(fout),st.st_mode)==-1){
				fprintf (stderr,"Can't chmod file %s (%s)\n"
					,dst,strerror(errno));
				ret = -1;
			}
			fclose(fout);
			struct utimbuf timbuf;
			timbuf.modtime = st.st_mtime;
			timbuf.actime = st.st_atime;
			if (utime (dst,&timbuf)==-1){
				fprintf (stderr,"Can't set time stamp on file %s (%s)\n"
					,dst,strerror(errno));
			}
		}
		fclose (fin);
	}
	return ret;
}

/*
	Set the immutable flag on a file
*/
int setext2flag (const char *fname, bool set, int ext2flags)
{
	int ret = -1;
	if (testmode){
		ret = 0;
	}else{
		int fd = open (fname,O_RDONLY);
		if (fd == -1){
			fprintf (stderr,"Can't open file %s (%s)\n",fname 
				,strerror(errno));
		}else{
			int flags = set ? ext2flags : 0;
			ret = ioctl (fd,EXT2_IOC_SETFLAGS,&flags);
			close (fd);
			if (ret == -1){
				fprintf (stderr,"Can't %s immutable flag on file %s (^s)\n"
					,(set ? "set" : "unset")
					,fname
					,strerror(errno));
			}
		}
	}
	return ret;
}

int vbuild_mkdir (const char *path, mode_t mode)
{
	int ret = -1;
	if (testmode){
		printf ("mkdir %s; chmod %o %s\n",path,mode,path);
		ret = 0;
	}else{
		ret = mkdir (path,mode);
		if (ret == -1 && errno == EEXIST){
			struct stat st;
			if (lstat(path,&st)!=-1	&& S_ISDIR(st.st_mode)){
				ret = chmod (path,mode);
			}
		}
	}
	return ret;
}

int vbuild_mknod(const char *path, mode_t mode, dev_t dev)
{
	int ret = -1;
	if (testmode){
		printf ("mknod %s %o %02x:%02x\n",path,mode,major(dev),minor(dev));
		ret = 0;
	}else{
		ret = mknod (path,mode,dev);
		if (ret == -1 && errno == EEXIST){
			struct stat st;
			lstat(path,&st);
			if (lstat(path,&st)!=-1
				&& (st.st_mode & S_IFMT) == (mode & S_IFMT)
				&& st.st_rdev == dev){
				ret = chmod (path,mode);
			}
		}
	}
	return ret;
}
int vbuild_symlink(const char *src, const char *dst)
{
	int ret = -1;
	if (testmode){
		printf ("ln -s %s %s\n",src,dst);
		ret = 0;
	}else{
		ret = symlink (src,dst);
	}
	return ret;
}

int vbuild_link(const char *src, const char *dst)
{
	int ret = -1;
	if (testmode){
		printf ("ln %s %s\n",src,dst);
		ret = 0;
	}else{
		ret = link (src,dst);
	}
	return ret;
}

int vbuild_unlink(const char *path)
{
	int ret = -1;
	if (testmode){
		printf ("unlink %s\n",path);
		ret = 0;
	}else{
		ret = unlink (path);
	}
	return ret;
}

int vbuild_chown(const char *path, uid_t uid, gid_t gid)
{
	int ret = -1;
	if (testmode){
		printf ("chown %d.%d %s\n",uid,gid,path);
		ret = 0;
	}else{
		ret = chown (path,uid,gid);
	}
	return ret;
}

int vbuild_file_copy(
	const char *src,
	const char *dst,
	struct stat &st)
{
	int ret = -1;
	if (testmode){
		printf ("cp -a %s %s\n",src,dst);
		ret = 0;
	}else{
		ret = file_copy (src,dst,st);
	}
	return ret;
}

/*
	Load the list of all packages in a vserver
*/
void vutil_loadallpkg (string &refserver, list<PACKAGE> &packages)
{
	FILE *fin = vutil_execdistcmd (K_PKGVERSION,refserver,NULL);
	if (fin != NULL){
		char line[1000];
		while (fgets(line,sizeof(line)-1,fin)!=NULL){
			int last = strlen(line)-1;
			if (last >= 0 && line[last] == '\n') line[last] = '\0';
			packages.push_back (PACKAGE(line));
		}
		pclose (fin);
	}
}

int vutil_lstat (string path, struct stat &st)
{
	int ret = 0;
	if (lstat(path.c_str(),&st) == -1){
		fprintf (stderr,"Can't lstat file %s (%s)\n"
			,path.c_str(),strerror(errno));
		ret = -1;
	}
	return ret;
}

const char K_PKGVERSION[]="pkgversion";
const char K_DUMPFILES[]="dumpfiles";
const char K_UNIFILES[]="unifiles";

FILE *vutil_execdistcmd (const char *key, const string &vserver, const char *args)
{
	string cmd = PKGLIBDIR "/distrib-info ";
	cmd += vserver;
	cmd += " ";
	cmd += key;
	if (args != NULL){
		cmd += " ";
		cmd += args;
	}
	FILE *ret = popen (cmd.c_str(),"r");
	if (ret == NULL){
		fprintf (stderr,"Can't execute command %s\n",cmd.c_str());
	}else{
		#if 0
		char buf[1000];
		while (fgets(buf,sizeof(buf)-1,fin)!=NULL){
			int last = strlen(buf)-1;
			if (last >= 0) buf[last] = '\0';
			ret = buf;
			break;
		}
		pclose (fin);
		#endif
	}
	return ret;
}

