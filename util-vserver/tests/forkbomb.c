// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on tests/forkbomb.cc by Jacques Gelinas
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

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

typedef enum {MODE_SLEEP,MODE_LOOP,MODE_FORK, MODE_FORKSHELL} MODE;


static void forkbomb_userfork (MODE mode)
{
	pid_t pid = fork();
	if (pid==-1){
		fprintf (stderr,"Fork failed (%s)\n",strerror(errno));
	}else if (pid == 0){
		if (mode == MODE_SLEEP){
			sleep(20);
		}else if (mode == MODE_LOOP){
			int k=0;
			while (1) k++;
		}else if (mode == MODE_FORKSHELL){
			system ("/bin/false");
		}
		_exit (0);
	}
}


int main (int argc, char *argv[])
{
	if (argc != 4){
		fprintf (stderr,"formboom N M mode\n"
			"where N is the number of process to start\n"
			"and M is the number of user to start\n"
			"Each user will try to start N process\n"
			"\n"
			"mode is:\n"
			"    sleep: Each process sleeps for 20 seconds and exits\n"
			"    loop:  Each process loops forever\n"
			"    fork:  Each process exits immediatly and is restarted\n"
			"           by the parent\n"
			"    forkshell: Each process runs /bin/false in a shell and\n"
			"           exits, then the parent start a new one\n"
			);
	}else{
		MODE mode;
		if (strcmp(argv[3],"sleep")==0){
			mode = MODE_SLEEP;
		}else if (strcmp(argv[3],"loop")==0){
			mode = MODE_LOOP;
		}else if (strcmp(argv[3],"fork")==0){
			mode = MODE_FORK;
		}else if (strcmp(argv[3],"forkshell")==0){
			mode = MODE_FORKSHELL;
		}else{
			fprintf (stderr,"Invalid mode\n");
			exit (-1);
		}
		for (int i=0; i<atoi(argv[2]); i++){
			if (fork()==0){
				if (setuid (i+1)==-1){
					fprintf (stderr,"Can't setuid to uid %d (%s)\n",i+1
						,strerror(errno));
				}else{
					for (int j=0; j<atoi(argv[1]); j++){
						forkbomb_userfork (mode);
					}
					if (mode == MODE_FORK || mode == MODE_FORKSHELL){
						// Ok, all processes are started, in MODE_FORK
						// we create a new one all the time
						int status;
						while (wait(&status)!=-1) forkbomb_userfork(mode);
					}
				}
				_exit (0);
			}
		}
		system ("ps ax | wc -l");
		printf ("All the process are running now\n");
		printf ("Exit to end all processes\n");
		system ("/bin/sh");
		system ("killall forkbomb");
	}
	return 0;
}

