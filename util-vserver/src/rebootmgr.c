// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on rebootmgr.cc by Jacques Gelinas
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
	The reboot manager allow a virtual server administrator to request
	a complete restart of his vserver. This means that all services
	are terminated, all remaining processes are killed and then
	all services are started.

	This is done by issuing

		/usr/sbin/vserver vserver restart


	The rebootmgr installs a unix domain socket in each vservers
	and listen for the reboot messages. All other message are discarded.

	The unix domain socket is placed in /vservers/N/dev/reboot and is
	turned immutable.

	The vreboot utility is used to send the signal from the vserver
	environment.
*/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "compat.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <alloca.h>

static void usage()
{
	fprintf (stderr,"rebootmgr version %s\n",VERSION);
	fprintf (stderr,"\n");
	fprintf (stderr,"rebootmgr [--pidfile file ] vserver-name [ vserver-name ...]\n");
}

static int rebootmgr_opensocket (const char *vname)
{
	int ret = -1;
	char sockn[PATH_MAX];
	int fd =  socket (AF_UNIX,SOCK_STREAM,0);
	sprintf (sockn,"%s/%s/dev/reboot",VROOTDIR, vname);
	unlink (sockn);
	if (fd == -1){
		fprintf (stderr,"Can't create a unix domain socket (%s)\n"
				,strerror(errno));
	}else{
		struct sockaddr_un un;
		un.sun_family = AF_UNIX;
		strcpy (un.sun_path,sockn);
		if (bind(fd,(struct sockaddr*)&un,sizeof(un))==-1){
			fprintf (stderr,"Can't bind to file %s (%s)\n",sockn
				,strerror(errno));
		}else{
		        int code;
			chmod (sockn,0600);
			code = listen (fd,10);
			if (code == -1){
				fprintf (stderr,"Can't listen to file %s (%s)\n",sockn
					,strerror(errno));
			}else{
				ret = fd;
			}	
		}
	}
	return ret;
}

static int rebootmgr_process (int fd, const char *vname)
{
	int ret = -1;
	char buf[100];
	int len = read (fd,buf,sizeof(buf)-1);
	// fprintf (stderr,"process %d %s len %d\n",fd,vname,len);
	if (len > 0){
		buf[len] = '\0';
		if (strcmp(buf,"reboot\n")==0){
			char cmd[1000];
			syslog (LOG_NOTICE,"reboot vserver %s\n",vname);
			snprintf (cmd,sizeof(cmd)-1,"%s/vserver %s restart >>/var/log/boot.log 2>&1",SBINDIR, vname);
			system (cmd);
			ret = 0;
		}else if (strcmp(buf,"halt\n")==0){
			char cmd[1000];
			syslog (LOG_NOTICE,"halt vserver %s\n",vname);
			snprintf (cmd,sizeof(cmd)-1,"%s/vserver %s stop >>/var/log/boot.log 2>&1",SBINDIR, vname);
			system (cmd);
			ret = 0;
		}else{
			syslog (LOG_ERR,"Invalid request from vserver %s",vname);
		}
	}
	return ret;
}


int main (int argc, char *argv[])
{
	int ret = -1;
	if (argc < 2){
		usage();
	}else{
		int error = 0;
		int start = 1;
		int i;
		int *sockets = alloca(argc * sizeof(int));

		openlog ("rebootmgr",LOG_PID,LOG_DAEMON);
		for (i=0; i<argc; i++){
			const char *arg = argv[i];
			if (strcmp(arg,"--pidfile")==0){
				const char *pidfile = argv[i+1];
				FILE *fout = fopen (pidfile,"w");
				if (fout == NULL){
					fprintf (stderr,"Can't open pidfile %s (%s)\n"
						,pidfile,strerror(errno));

					__extension__
					syslog (LOG_ERR,"Can't open pidfile %s (%m)"
						,pidfile);
				}else{
					fprintf (fout,"%d\n",getpid());
					fclose (fout);
				}
				start = i+2;
				i++;
			}else if (strcmp(arg,"--")==0){
				start = i+1;
				break;
			}else if (arg[0] == '-'){
				fprintf (stderr,"Invalid argument %s\n",arg);
				syslog (LOG_ERR,"Invalid argument %s",arg);
			}
		}
		for (i=start; i<argc; i++){
			int fd = rebootmgr_opensocket (argv[i]);
			if (fd == -1){
				error = 1;
			}else{
				sockets[i] = fd;
			}
		}
		if (!error){
			int maxhandles = argc*2;
			struct {
				int handle;
				const char *vname;
			} handles[maxhandles];
			int nbhandles=0;
			while (1){
				int maxfd = 0;
				int i;
				int ok;
				
				fd_set fdin;
				FD_ZERO (&fdin);
				for (i=start; i<argc; i++){
					int fd = sockets[i];
					if (fd > maxfd) maxfd = fd;
					FD_SET (fd,&fdin);
				}
				for (i=0; i<nbhandles; i++){
					int fd = handles[i].handle;
					if (fd > maxfd) maxfd = fd;
					FD_SET (fd,&fdin);
				}
				ok = select (maxfd+1,&fdin,NULL,NULL,NULL);
				if (ok <= 0){
					break;
				}else{
				        int i;
					int dst = 0;

					for (i=start; i<argc; i++){
						int fd = sockets[i];
						if (FD_ISSET(fd,&fdin)){
							struct sockaddr_un unc;
							socklen_t len = sizeof(unc);
							unc.sun_family = AF_UNIX;
							fd = accept (fd,(struct sockaddr*)&unc,&len);
							if (fd != -1){
								if (nbhandles == maxhandles){
								        int j;
									// Overloaded, we close every handle
									syslog (LOG_ERR,"%d sockets opened: Overloaded\n",nbhandles);
									for (j=0; j<nbhandles; j++){
										close (handles[j].handle);
									}
									nbhandles = 0;
								}
								handles[nbhandles].handle = fd;
								handles[nbhandles].vname = argv[i];
								nbhandles++;
								// fprintf (stderr,"accept %d\n",nbhandles);
							}
						}
					}
					for (i=0; i<nbhandles; i++){
						int fd = handles[i].handle;
						if (FD_ISSET(fd,&fdin)){
							if (rebootmgr_process (fd,handles[i].vname)==-1){
								close (fd);
							}else{
								handles[dst++] = handles[i];
							}
						}else{
							handles[dst++] = handles[i];
						}
					}
					nbhandles = dst;
				}
			}
		}
	}
	return ret;
}


