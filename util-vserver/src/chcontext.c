// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on chcontext.cc by Jacques Gelinas
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
	chcontext is a wrapper to user the new_s_context system call. It
	does little more than mapping command line option to the system call
	arguments.
*/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "compat.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "linuxcaps.h"
#include "vserver.h"

#ifndef CAP_QUOTACTL
#  define CAP_QUOTACTL	29
#endif

static void usage()
{
	fprintf (stderr,"chcontext version %s\n",VERSION);
	fprintf (stderr
		,"chcontext [ options ] command arguments ...\n"
		 "\n"
		 "chcontext allocate a new security context and executes\n"
		 "a command in that context.\n"
		 "By default, a new/unused context is allocated\n"
		 "\n"

		 "--cap CAP_NAME\n"
		 "\tAdd a capability from the command. This option may be\n"
		 "\trepeated several time.\n"
		 "\tSee /usr/include/linux/capability.h\n"
		 "\tIn general, this option is used with the --secure option\n"
		 "\t--secure removes most critical capabilities and --cap\n"
		 "\tadds specific ones.\n"
		 "\n"

		 "--cap !CAP_NAME\n"
		 "\tRemove a capability from the command. This option may be\n"
		 "\trepeated several time.\n"
		 "\tSee /usr/include/linux/capability.h\n"
		 "\n"
		 "--ctx num\n"
		 "\tSelect the context. On root in context 0 is allowed to\n"
		 "\tselect a specific context.\n"
		 "\tContext number 1 is special. It can see all processes\n"
		 "\tin any contexts, but can't kill them though.\n"
		 "\tOption --ctx may be repeated several times to specify up to 16 contexts.\n"

		 "--disconnect\n"
		 "\tStart the command in background and make the process\n"
		 "\ta child of process 1.\n"

		 "--domainname new_domainname\n"
		 "\tSet the domainname (NIS) in the new security context.\n"
		 "\tUse \"none\" to unset the domain name.\n"

		 "--flag\n"
		 "\tSet one flag in the new or current security context. The following\n"
		 "\tflags are supported. The option may be used several time.\n"
		 "\n"
		 "\tfakeinit: The new process will believe it is process number 1.\n"
		 "            Useful to run a real /sbin/init in a vserver.\n"
		 "\tlock: The new process is trapped and can't use chcontext anymore.\n"
		 "\tsched: The new process and its children will share a common \n"
		 "         execution priority.\n"
		 "\tnproc: Limit the number of process in the vserver according to\n"
		 "         ulimit setting. Normally, ulimit is a per user thing.\n"
		 "         With this flag, it becomes a per vserver thing.\n"
		 "\tprivate: No one can join this security context once created.\n"
		 "\tulimit: Apply the current ulimit to the whole context\n"

		 "--hostname new_hostname\n"
		 "\tSet the hostname in the new security context\n"
		 "\tThis is need because if you create a less privileged\n"
		 "\tsecurity context, it may be unable to change its hostname\n"

		 "--secure\n"
		 "\tRemove all the capabilities to make a virtual server trustable\n"

		 "--silent\n"
		 "\tDo not print the allocated context number.\n"
		 "\n"
		 "Information about context is found in /proc/self/status\n");
}


int main (int argc, char *argv[])
{
	int ret = -1;
	int i;
	int nbctx = 0;
	int ctxs[16];
	int disconnect = 0;
	int silent = 0;
	int flags = 0;
	unsigned remove_cap = 0;
	unsigned add_cap = 0;
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
	const char *hostname=NULL, *domainname=NULL;

	for (i=1; i<argc; i++){
		const char *arg = argv[i];
		const char *opt = argv[i+1];
		if (strcmp(arg,"--ctx")==0){
			if (nbctx >= 16){
				fprintf (stderr,"Too many context, max 16, ignored.\n");
			}else{
				ctxs[nbctx++] = atoi(opt);
			}
			i++;
		}else if (strcmp(arg,"--disconnect")==0){
			disconnect = 1;
		}else if (strcmp(arg,"--silent")==0){
			silent = 1;
		}else if (strcmp(arg,"--flag")==0){
			if (strcmp(opt,"lock")==0){
				flags |= 1;
			}else if (strcmp(opt,"sched")==0){
				flags |= 2;
			}else if (strcmp(opt,"nproc")==0){
				flags |= 4;
			}else if (strcmp(opt,"private")==0){
				flags |= 8;
			}else if (strcmp(opt,"fakeinit")==0){
				flags |= 16;
			}else if (strcmp(opt,"hideinfo")==0){
				flags |= 32;
			}else if (strcmp(opt,"ulimit")==0){
				flags |= 64;
			}else{
				fprintf (stderr,"Unknown flag %s\n",opt);
			}
			i++;
		}else if (strcmp(arg,"--cap")==0){
			static struct {
				const char *option;
				int bit;
			}tbcap[]={
				// The following capabilities are normally available
				// to vservers administrator, but are place for
				// completeness
				{"CAP_CHOWN",CAP_CHOWN},
				{"CAP_DAC_OVERRIDE",CAP_DAC_OVERRIDE},
				{"CAP_DAC_READ_SEARCH",CAP_DAC_READ_SEARCH},
				{"CAP_FOWNER",CAP_FOWNER},
				{"CAP_FSETID",CAP_FSETID},
				{"CAP_KILL",CAP_KILL},
				{"CAP_SETGID",CAP_SETGID},
				{"CAP_SETUID",CAP_SETUID},
				{"CAP_SETPCAP",CAP_SETPCAP},
				{"CAP_SYS_TTY_CONFIG",CAP_SYS_TTY_CONFIG},
				{"CAP_LEASE",CAP_LEASE},
				{"CAP_SYS_CHROOT",CAP_SYS_CHROOT},

				// Those capabilities are not normally available
				// to vservers because they are not needed and
				// may represent a security risk
				{"CAP_LINUX_IMMUTABLE",CAP_LINUX_IMMUTABLE},
				{"CAP_NET_BIND_SERVICE",CAP_NET_BIND_SERVICE},
				{"CAP_NET_BROADCAST",CAP_NET_BROADCAST},
				{"CAP_NET_ADMIN",	CAP_NET_ADMIN},
				{"CAP_NET_RAW",	CAP_NET_RAW},
				{"CAP_IPC_LOCK",	CAP_IPC_LOCK},
				{"CAP_IPC_OWNER",	CAP_IPC_OWNER},
				{"CAP_SYS_MODULE",CAP_SYS_MODULE},
				{"CAP_SYS_RAWIO",	CAP_SYS_RAWIO},
				{"CAP_SYS_PACCT",	CAP_SYS_PACCT},
				{"CAP_SYS_ADMIN",	CAP_SYS_ADMIN},
				{"CAP_SYS_BOOT",	CAP_SYS_BOOT},
				{"CAP_SYS_NICE",	CAP_SYS_NICE},
				{"CAP_SYS_RESOURCE",CAP_SYS_RESOURCE},
				{"CAP_SYS_TIME",	CAP_SYS_TIME},
				{"CAP_MKNOD",		CAP_MKNOD},
				{"CAP_QUOTACTL",        CAP_QUOTACTL},
				{NULL,0}
			};
			int j;
			unsigned *cap = &add_cap;
			if (opt[0] == '!'){
				cap = &remove_cap;
				opt++;
			}
			for (j=0; tbcap[j].option != NULL; j++){
				if (strcasecmp(tbcap[j].option,opt)==0){
					*cap |= (1<<tbcap[j].bit);
					break;
				}
			}
			if (tbcap[j].option == NULL){
				fprintf (stderr,"Unknown capability %s\n",opt);
			}
			i++;
		}else if (strcmp(arg,"--secure")==0){
			remove_cap |= secure;
		}else if (strcmp(arg,"--hostname")==0){
			hostname = opt;
			i++;
		}else if (strcmp(arg,"--domainname")==0){
			if (opt != NULL && strcmp(opt,"none")==0) opt = "";
			domainname = opt;
			i++;
		}else{
			break;
		}
	}
	if (i == argc){
		usage();
	}else if (argv[i][0] == '-'){
		usage();
	}else{
		/*
			We must fork early because fakeinit set the current
			process as the special init process
		*/
		if (disconnect == 0 || fork()==0){
		        int newctx;
			int xflags = flags & 16;

			if (nbctx == 0) ctxs[nbctx++] = -1;
			newctx = vc_new_s_context(ctxs[0],0,flags&~16);
			if (newctx != -1){
				if (hostname != NULL){
					if (sethostname (hostname,strlen(hostname))==-1){
						fprintf (stderr,"Can't set the host name (%s)\n"
							,strerror(errno));
					}else if (!silent){
						printf ("Host name is now %s\n",hostname);
					}
				}
				if (domainname != NULL){
					setdomainname (domainname,strlen(domainname));
					if (!silent){
						printf ("Domain name is now %s\n",domainname);
					}
				}
				remove_cap &= (~add_cap);
				if (remove_cap!=0 || xflags!=0)
				        vc_new_s_context (-2,remove_cap,xflags);
				if (!silent){
					printf ("New security context is %d\n"
						,ctxs[0] == -1 ? newctx : ctxs[0]);
				}
				execvp (argv[i],argv+i);
				fprintf (stderr,"Can't exec %s (%s)\n",argv[i]
					,strerror(errno));
			}else{
				perror ("Can't set the new security context\n");
			}
			if (disconnect != 0) _exit(0);
		}
	}
	return ret;
}

