// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on capchroot.cc by Jacques Gelinas
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
	This chroot command does very little. Once the chroot
	system call is executed, it (option) remove the CAP_SYS_CHROOT
	capability. Then it executes its argument
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "compat.h"

#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

#include "linuxcaps.h"
#include "vserver.h"

int main (int argc, char *argv[])
{
	if (argc < 3){
		fprintf (stderr,"capchroot version %s\n",VERSION);
		fprintf (stderr
			,"capchroot --nochroot directory [ --suid user ] command argument\n"
			 "\n"
			 "--nochroot remove the CAP_SYS_CHROOT capability\n"
			 "           after the chroot system call.\n"
			 "--suid switch to a different user (in the vserver context)\n"
			 "       before executing the command.\n");
	}else{
		const char *uid = NULL;
		bool nochroot = false;
		int dir;
		for (dir=1; dir<argc; dir++){
			const char *arg = argv[dir];
			if (arg[0] != '-' && arg[1] != '-'){
				break;
			}else if (strcmp(arg,"--nochroot")==0){
				nochroot = true;
			}else if (strcmp(arg,"--suid")==0){
				dir++;
				uid = argv[dir];
			}
			
		}
		// We resolve the UID before doing the chroot.
		// If we do the getpwnam after the chroot, we will end
		// up loading shared object from the vserver.
		// This is causing two kind of problem: Incompatibilities
		// and also a security flaw. The shared objects in the vserver
		// may be tweaked to get control of the root server ...
		getpwnam ("root");
		if (vc_chrootsafe (argv[dir]) == -1){
			fprintf (stderr,"Can't chroot to directory %s (%s)\n",argv[dir]
				,strerror(errno));
		}else{
			struct passwd *p = NULL;
			int cmd          = dir + 1;

			if (nochroot){
				vc_new_s_context (-2,1<<CAP_SYS_CHROOT,0);
			}

			if (uid != NULL && strcmp(uid,"root")!=0){
				p = getpwnam(uid);
				if (p == NULL){
					fprintf (stderr,"User not found: %s\n",uid);
					exit (-1);
				}
			}
			if (p != NULL) {
				setgroups (0,NULL);
				setgid(p->pw_gid);
				setuid(p->pw_uid);
			}
			if (cmd >= argc){
				fprintf (stderr,"capchroot: No command to execute, do nothing\n");
			}else{
				execvp (argv[cmd],argv+cmd);
				fprintf (stderr,"Can't execute %s (%s)\n",argv[cmd]
					,strerror(errno));
			}
		}
	}
	return -1;
}


