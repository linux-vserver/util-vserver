// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on reducecap.cc by Jacques Gelinas
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
#include "compat.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "linuxcaps.h"
#include "vserver.h"

#ifndef CAP_QUOTACTL
#  define CAP_QUOTACTL	29
#endif

extern int capget (struct __user_cap_header_struct *, struct __user_cap_data_struct *);
extern int capset (struct __user_cap_header_struct *, struct __user_cap_data_struct *);

static void usage()
{
	fprintf (stderr,"reducecap version %s\n",VERSION);
	fprintf (stderr,"reducecap [ options ] command argument\n");
	exit (-1);
}

static void reducecap_print(struct __user_cap_data_struct *user)
{
	static const char *tb[]={
		"CAP_CHOWN",
		"CAP_DAC_OVERRIDE",
		"CAP_DAC_READ_SEARCH",
		"CAP_FOWNER",
		"CAP_FSETID",
		"CAP_KILL",
		"CAP_SETGID",
		"CAP_SETUID",
		"CAP_SETPCAP",
		"CAP_LINUX_IMMUTABLE",
		"CAP_NET_BIND_SERVICE",
		"CAP_NET_BROADCAST",
		"CAP_NET_ADMIN",
		"CAP_NET_RAW",
		"CAP_IPC_LOCK",
		"CAP_IPC_OWNER",
		"CAP_SYS_MODULE",
		"CAP_SYS_RAWIO",
		"CAP_SYS_CHROOT",
		"CAP_SYS_PTRACE",
		"CAP_SYS_PACCT",
		"CAP_SYS_ADMIN",
		"CAP_SYS_BOOT",
		"CAP_SYS_NICE",
		"CAP_SYS_RESOURCE",
		"CAP_SYS_TIME",
		"CAP_SYS_TTY_CONFIG",
		"CAP_MKNOD",
		"CAP_LEASE",
		"CAP_QUOTACTL",
		NULL
	};
	int i;
	printf ("%22s %9s %9s %9s\n","Capability","Effective","Permitted"
		,"Inheritable");
	for (i=0; tb[i] != NULL; i++){
		int bit = (1 << i);
		printf ("%22s %9s %9s %9s\n"
			,tb[i]
			,(user->effective & bit) ? "X    " : " "
			,(user->permitted & bit) ? "X    " : " "
			,(user->inheritable & bit) ? "X    " : " ");
	}
}

static void reducecap_show()
{
	struct __user_cap_header_struct header;
	struct __user_cap_data_struct user;
	header.version = _LINUX_CAPABILITY_VERSION;
	header.pid = getpid();
	if (capget(&header,&user)==-1){
		perror ("capget");
	}else{
		reducecap_print (&user);
	}
}



int main (int argc, char *argv[])
{
	int ret = -1;
	unsigned long remove = 0;
	int show = 0;
	int flags = 0;
	unsigned long secure = (1<<CAP_LINUX_IMMUTABLE)
		|(1<<CAP_NET_BROADCAST)
		|(1<<CAP_NET_ADMIN)
		|(1<<CAP_NET_RAW)
		|(1<<CAP_IPC_LOCK)
		|(1<<CAP_IPC_OWNER)
		|(1<<CAP_SYS_MODULE)
		|(1<<CAP_SYS_RAWIO)
		|(1<<CAP_SYS_PACCT)
		|(1<<CAP_SYS_ADMIN)
		|(1<<CAP_SYS_BOOT)
		|(1<<CAP_SYS_NICE)
		|(1<<CAP_SYS_RESOURCE)
		|(1<<CAP_SYS_TIME)
		|(1<<CAP_MKNOD)
	        |(1<<CAP_QUOTACTL);
	int i;
	for (i=1; i<argc; i++){
		const char *arg = argv[i];
		const char *opt = argv[i+1];
		if (strcmp(arg,"--secure")==0){
			remove = secure;
		}else if (strcmp(arg,"--show")==0){
			show = 1;
		}else if (strcmp(arg,"--flag")==0){
			if (strcmp(opt,"lock")==0){
				flags |= 1;
			}else if (strcmp(opt,"sched")==0){
				flags |= 2;
			}else if (strcmp(opt,"nproc")==0){
				flags |= 4;
			}else if (strcmp(opt,"private")==0){
				flags |= 8;
			}else if (strcmp(opt,"hideinfo")==0){
				flags |= 32;
			}else{
				fprintf (stderr,"Unknown flag %s\n",opt);
			}
			i++;
		}else if (arg[0] == '-' && arg[1] == '-'){
			static struct {
				const char *option;
				int bit;
			}tbcap[]={
				// The following capabilities are normally available
				// to vservers administrator, but are place for
				// completeness
				{"CHOWN",CAP_CHOWN},
				{"DAC_OVERRIDE",CAP_DAC_OVERRIDE},
				{"DAC_READ_SEARCH",CAP_DAC_READ_SEARCH},
				{"FOWNER",CAP_FOWNER},
				{"FSETID",CAP_FSETID},
				{"KILL",CAP_KILL},
				{"SETGID",CAP_SETGID},
				{"SETUID",CAP_SETUID},
				{"SETPCAP",CAP_SETPCAP},
				{"SYS_TTY_CONFIG",CAP_SYS_TTY_CONFIG},
				{"LEASE",CAP_LEASE},
				{"SYS_CHROOT",CAP_SYS_CHROOT},

				// Those capabilities are not normally available
				// to vservers because they are not needed and
				// may represent a security risk
				{"LINUX_IMMUTABLE",CAP_LINUX_IMMUTABLE},
				{"NET_BIND_SERVICE",CAP_NET_BIND_SERVICE},
				{"NET_BROADCAST",CAP_NET_BROADCAST},
				{"NET_ADMIN",	CAP_NET_ADMIN},
				{"NET_RAW",	CAP_NET_RAW},
				{"IPC_LOCK",	CAP_IPC_LOCK},
				{"IPC_OWNER",	CAP_IPC_OWNER},
				{"SYS_MODULE",CAP_SYS_MODULE},
				{"SYS_RAWIO",	CAP_SYS_RAWIO},
				{"SYS_PACCT",	CAP_SYS_PACCT},
				{"SYS_ADMIN",	CAP_SYS_ADMIN},
				{"SYS_BOOT",	CAP_SYS_BOOT},
				{"SYS_NICE",	CAP_SYS_NICE},
				{"SYS_RESOURCE",CAP_SYS_RESOURCE},
				{"SYS_TIME",	CAP_SYS_TIME},
				{"MKNOD",		CAP_MKNOD},
				{"QUOTACTL",          CAP_QUOTACTL},
				{NULL,0}
			};
			int j;
			arg += 2;
			if (*arg=='\0') {
			  ++i;
			  break;
			}
			if (strncmp(arg, "CAP_", 4)==0) arg += 4;
			for (j=0; tbcap[j].option != NULL; j++){
				if (strcasecmp(tbcap[j].option,arg)==0){
					remove |= (1<<tbcap[j].bit);
					break;
				}
			}
			if (tbcap[j].option == NULL){
				usage();
			}
		}else{
			break;
		}
	}
	if (i == argc){
		if (show){
			reducecap_show();
		}else{
			usage();
		}
	}else{
		struct __user_cap_header_struct header;
		struct __user_cap_data_struct user;
		header.version = _LINUX_CAPABILITY_VERSION;
		header.pid = 0;
		if (capget(&header,&user)==-1){
			perror ("capget");
		}else{
			if (show){
				reducecap_print (&user);
			}
			if (vc_new_s_context(-2,remove,flags)==-1){
				perror ("new_s_context -2");
			}else{
				fprintf (stderr,"Executing\n");
				execvp (argv[i],argv+i);
				fprintf (stderr,"Can't execute command %s\n",argv[i]);
			}
		}
	}
	return ret;
}

