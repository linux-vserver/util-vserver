// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on showattr.cc by Jacques Gelinas
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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ext2_fs.h>

// Patch to help compile this utility on unpatched kernel source
#ifndef EXT2_IMMUTABLE_FILE_FL
	#define EXT2_IMMUTABLE_FILE_FL	0x00000010
	#define EXT2_IMMUTABLE_LINK_FL	0x00008000
#endif

/*
	Get the extended attributes of a file
*/
static int getext2flags (const char *fname, long *flags)
{
	int ret = -1;
	int fd = open (fname,O_RDONLY);
	if (fd == -1){
		fprintf (stderr,"Can't open file %s (%s)\n",fname,strerror(errno));
	}else{
		*flags = 0;
		ret = ioctl (fd,EXT2_IOC_GETFLAGS,flags);
		close (fd);
		if (ret == -1){
			fprintf (stderr,"Can't get ext2 flags on file %s (%s)\n"
				,fname,strerror(errno));
		}
	}
	return ret;
}

/*
	Set the extended attributes of a file
*/
static int setext2flags (const char *fname, long flags)
{
	int ret = -1;
	int fd = open (fname,O_RDONLY);
	if (fd == -1){
		fprintf (stderr,"Can't open file %s (%s)\n",fname,strerror(errno));
	}else{
		ret = ioctl (fd,EXT2_IOC_SETFLAGS,&flags);
		close (fd);
		if (ret == -1){
			fprintf (stderr,"Can't set ext2 flags on file %s (%s)\n"
				,fname,strerror(errno));
		}
	}
	return ret;
}


int main (int argc, char *argv[])
{
	int ret = -1;
	if (argc <= 1){
		fprintf (stderr
			,"showattr file ...\n"
			 "\n"
			 "Presents extended file attribute.\n"
			 "\n"
			 "setattr --immutable --immulink file ...\n"
			 "\n"
			 "Sets the extended file attributes.\n"
			 "\n"
			 "These utilities exist as an interim until lsattr and\n"
			 "chattr are updated.\n"
			);
	}else if (strstr(argv[0],"showattr")!=NULL){
	        int i;
		for (i=1; i<argc; i++){
			long flags;
			ret = getext2flags (argv[i],&flags);
			if (ret == -1){
				break;
			}else{
				printf ("%s\t%08lx\n",argv[i],flags);
			}
		}
	}else if (strstr(argv[0],"setattr")!=NULL){
		long flags = 0;
		int  i;
		ret = 0;
		for (i=1; i<argc; i++){
			const char *arg = argv[i];
			if (strncmp(arg,"--",2)==0){
				if (strcmp(arg,"--immutable")==0){
					flags |= EXT2_IMMUTABLE_FILE_FL;
				}else if (strcmp(arg,"--immulink")==0){
					flags |= EXT2_IMMUTABLE_LINK_FL;
				}else{
					fprintf (stderr,"Invalid option %s\n",arg);
					ret = -1;
					break;
				}
			}else{
				ret = setext2flags (arg,flags);
				if (ret == -1){
					break;
				}
			}
		}
	}
	return ret;
}

