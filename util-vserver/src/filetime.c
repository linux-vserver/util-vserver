// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on filetime.cc by Jacques Gelinas
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
#include <time.h>
#include <sys/stat.h>

int main (int argc, char *argv[])
{
	int ret = -1;
	if (argc != 2){
		fprintf (stderr
			,"filetime version %s\n"
			 "filetime file\n"
			 "\n"
			 "Prints the age of a file\n"
			 "(how long since it was created or modified)\n"
			,VERSION);
	}else{
		struct stat st;
		if (stat(argv[1],&st)==-1){
			fprintf (stderr,"Can't stat file %s (%s)\n",argv[1]
				,strerror(errno));
		}else{
			time_t now = time(NULL);
			time_t since = now - st.st_mtime;
			int days = since / (24*60*60);
			int today = since % (24*60*60);
			int hours = today / (60*60);
			int minutes = (today % (60*60)) / 60;
			if (days > 0){
				printf ("%d days ",days);
			}
			printf ("%02d:%02d\n",hours,minutes);
			ret = 0;
		}
	}
	return ret;
}

