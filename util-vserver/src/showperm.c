// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on showperm.cc by Jacques Gelinas
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
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

int main (int argc, char *argv[])
{
	int ret = -1;
	if (argc < 2){
		fprintf (stderr,"showperm version %s\n",VERSION);
		fprintf (stderr,
			"showperm file or directory ...\n"
			"prints permission bits for files\n"
			"A very stripped down stat utility\n"
			);
	}else{
	        int i;
		ret = 0;
		for (i=1; i<argc; i++){
			struct stat st;
			if (lstat(argv[i],&st)==-1){
				fprintf (stderr,"can't lstat %s (%s)\n",argv[i]
					,strerror(errno));
				ret = -1;
			}else{
				printf ("%03o\n",(st.st_mode & 0777));
			}
		}
	}
	return ret;
}

