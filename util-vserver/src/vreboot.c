// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on vreboot.cc by Jacques Gelinas
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
	Used to send a reboot message to the reboot manager. It opens /dev/reboot
	and write "reboot\n".
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdbool.h>

/*
	Connect to a unix domain socket
*/
static int vreboot_connect (const char *sockpath, bool showerror)
{
	int ret = -1;
	int fd =  socket (AF_UNIX,SOCK_STREAM,0);
	if (fd == -1){
		if (showerror) perror("socket client");
	}else{
		struct sockaddr_un un;
		int s;
		
		un.sun_family = AF_UNIX;
		strcpy (un.sun_path,sockpath);
		s = connect(fd,(struct sockaddr*)&un,sizeof(un));
		if (s == -1){
			if (showerror) fprintf (stderr,"connect %s (%s)\n"
				,sockpath,strerror(errno));
		}else{
			ret = fd;
		}
	}
	return ret;
}

static void usage()
{
	fprintf (stderr,"vreboot version %s\n",VERSION);
	fprintf (stderr,"\n");
	fprintf (stderr,"vreboot [ --socket path ]\n");
	fprintf (stderr,"vhalt   [ --socket path ]\n");
	fprintf (stderr,"vreboot request a reboot or a halt of a virtual server\n");
}

int main (int argc, char *argv[])
{
	int ret = -1;
	int i;
	const char *sockpath = "/dev/reboot";
	for (i=1; i<argc; i++){
		const char *arg = argv[i];
		const char *opt = argv[i+1];
		if (strcmp(arg,"--socket")==0){
			sockpath = opt;
			i++;
		}else if (strcmp(arg,"--help")==0){
			break;

		}else{
			fprintf (stderr,"Invalid option %s\n",arg);
			break;
		}
	}
	if (argc != i){
		usage();
	}else{
		int fd = vreboot_connect (sockpath,true);
		if (fd != -1){
			if (strstr(argv[0],"halt")!=NULL){
				write (fd,"halt\n",5);
			}else{
				write (fd,"reboot\n",7);
			}
			close (fd);
			ret = 0;
		}
	}
	return ret;
}

