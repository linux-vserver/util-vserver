// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on setctxlimit.cc by Jacques Gelinas
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
	Set the global per context limit of a resource (memory, file handle).
	This utility can do it either for the current context or a selected
	one.

	It uses the same options as ulimit, when possible
*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/resource.h>

#include "vserver.h"

static void set_ctxlim (int res, long lim, const char *msg)
{
	if (call_set_ctxlimit(res,lim)==-1){
		fprintf (stderr,"Error setting limit \"%s\": %s\n"
			,msg,strerror(errno));
		exit (-1);
	}
}

static void usage()
{
		fprintf (stderr,"setctxlimit version %s\n",VERSION);
		fprintf (stderr
			,"setctxlimit [ --ctx context ] limits\n"
			 "\n"
			 "-n nax opened files\n");
}

int main (int argc, char *argv[])
{
	int ret = -1;
	if (argc < 2){
		usage();
	}else{
	        int i;
		ret = 0;
		for (i=1; i<argc; i++){
			const char *arg = argv[i];
			const char *narg = argv[i+1];
			int val;
			
			if (narg == NULL) narg = "";
			val = atoi(narg);
			if (strcmp(arg,"--help")==0){
				usage();
			}else if (strcmp(arg,"--ctx")==0){
				int tb[1];

				tb[0] = val;
				if (call_new_s_context (1,tb,0,0)==-1){
					fprintf (stderr,"Can't select context %d (%s)\n"
						,val,strerror(errno));
					exit (-1);
				}
			}else if (strcmp (arg,"-n")==0){
				set_ctxlim (RLIMIT_NOFILE,val,"Number of opened files");
			}
		}
	}
	return ret;	
}

