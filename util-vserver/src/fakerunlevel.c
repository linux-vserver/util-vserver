// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on fakerunlevel.cc by Jacques Gelinas
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
	This program add a RUNLEVEL record in a utmp file.
	This is used when a vserver lack a private init process
	so runlevel properly report the fake runlevel.
*/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <utmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static void usage()
{
	fprintf (stderr,"fakerunlevel version %s\n",VERSION);
	fprintf (stderr
		,"\n"
		 "fakerunlevel runlevel utmp_file\n"
		 "\n"
		 "Put a runlevel record in file utmp_file\n");
}

int main (int argc, char *argv[])
{
	if (argc != 3){
		usage();
	}else{
		int runlevel = atoi(argv[1]);
		const char *fname = argv[2];
		if (runlevel < 1 || runlevel > 5){
			usage();
		}else{
			// Make sure the file exist
			FILE *fout = fopen (fname,"a");
			if (fout == NULL){
				fprintf (stderr,"Can't open file %s (%s)\n",fname
					,strerror(errno));
			}else{
				struct utmp ut;

				fclose (fout);
				utmpname (fname);
				setutent();
				memset (&ut,0,sizeof(ut));
				ut.ut_type = RUN_LVL;
				ut.ut_pid = ('#' << 8) + runlevel+'0';
				pututline (&ut);
				endutent();
			}
		}
	}

	return 0;
}

