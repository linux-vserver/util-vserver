// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on readlink.cc by Jacques Gelinas
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
	Copyright Jacques Gelinas jack@solucorp.qc.ca
	Distributed under the Gnu Public License, see the License file
	in this package.
*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

int main (int argc, char *argv[])
{
	int ret = -1;
	if (argc != 2){
		fprintf (stderr,"readlink symlink-file\n");
		fprintf (stderr,"Prints the contents of a symlink\n");
	}else{
		char buf[PATH_MAX];
		int len = readlink (argv[1],buf,sizeof(buf)-1);
		if (len > 0){
			buf[len] = '\0';
			printf ("%s\n",buf);
			ret = 0;
		}else{
			fprintf (stderr,"readlink failed for file %s (%s)\n"
				,argv[1],strerror(errno));
		}
	}
	return ret;
}

