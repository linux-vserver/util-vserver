// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on parserpmdump.cc by Jacques Gelinas
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
	Litte utility to extract non config file from
	an rpm --dump command.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <alloca.h>


int main (int argc, char *argv[])
{
	int	*tblen = alloca(argc * sizeof(int));
	int	i;
	char tmp[1000];

	for (i=1; i<argc; i++) tblen[i] = strlen(argv[i]);
	while (fgets(tmp,sizeof(tmp)-1,stdin)!=NULL){
		int i;
		// Check if the file is in an excluded directory
		for (i=1; i<argc; i++){
			if (strncmp(argv[i],tmp,tblen[i])==0) break;
		}
		if (i == argc){
			// Ok no match
			int last = strlen(tmp)-1;
			mode_t	mode=-1;
			int type=-1;
			char *start = tmp;
			int	i;
			
			if (last >= 0 && tmp[last] == '\n') tmp[last] = '\0';

			for (i=0; i<8; i++){
				char *pt = start;
				while (*pt > ' ') pt++;
				if (*pt == ' ') *pt++ = '\0';
				if (i == 4){
					sscanf(start,"%o",&mode);
				}else if (i==7){
					type = atoi(start);
				}
				start = pt;
					
			}			
			if (S_ISREG(mode) && type == 0) printf ("%s\n",tmp);
		}
	}
	return 0;
}

