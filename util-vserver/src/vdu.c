// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on vdu.cc by Jacques Gelinas
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

__extension__ typedef long long		longlong;

static int vdu_onedir (char const *path, longlong *size)
{
	int ret = -1;
	int dirfd = open (path,O_RDONLY);	// A handle to speed up
												// chdir
	if (dirfd == -1){
		fprintf (stderr,"Can't open directory %s (%s)\n",path
			,strerror(errno));
	}else{
	        DIR *dir;

		fchdir (dirfd);
		dir = opendir (".");
		if (dir == NULL){
			fprintf (stderr,"Can't open (opendir) directory %s (%s)\n",path
				,strerror(errno));
		}else{
			struct stat dirst;
			struct dirent *ent;
			longlong dirsize = 0;

			ret = 0;
			lstat (".",&dirst);
			while ((ent=readdir(dir))!=NULL){
				struct stat st;
				if (lstat(ent->d_name,&st)==-1){
					fprintf (stderr,"Can't stat %s/%s (%s)\n",path
						,ent->d_name,strerror(errno));
					ret = -1;
					break;
				}else if (S_ISREG(st.st_mode)){
					if (st.st_nlink == 1){
						dirsize += st.st_size;
					}
				}else if (S_ISDIR(st.st_mode) && st.st_dev == dirst.st_dev){
					if (strcmp(ent->d_name,".")!=0
						&& strcmp(ent->d_name,"..")!=0){
					        char	*tmp = malloc(strlen(path) + strlen(ent->d_name) + 2);
						if (tmp==0) ret=-1;
						else {
						  strcpy(tmp, path);
						  strcat(tmp, "/");
						  strcat(tmp, ent->d_name);
						  ret = vdu_onedir(tmp,&dirsize);
						  free(tmp);
						  fchdir (dirfd);
						}
					}
				}
			}
			closedir (dir);
			*size += dirsize;
		}
		close (dirfd);
	}
	return ret;
}

int main (int argc, char *argv[])
{
	int ret = -1;
	if (argc < 2){
		fprintf (stderr,"vdu version %s\n",VERSION);
		fprintf (stderr,"vdu directory ...\n\n");
		fprintf (stderr
			,"Compute the size of a directory tree, ignoring files\n"
			 "with more than one link.\n");
	}else{
		int i;

		ret = 0;
		for (i=1; i<argc && ret != -1; i++){
			longlong size = 0;
			long ksize;
			
			ret = vdu_onedir (argv[i],&size);
			ksize = size >> 10;
			printf ("%s\t%ldK\n",argv[i],ksize);
		}
	}
	return ret;
}

