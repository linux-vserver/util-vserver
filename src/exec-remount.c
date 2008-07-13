// $Id$    --*- c -*--

// Copyright (C) 2008 Daniel Hokka Zakrisson
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License.
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

#include "util.h"

#include <vserver.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mount.h>
#include <errno.h>

#define ENSC_WRAPPERS_PREFIX	"exec-remount: "
#define ENSC_WRAPPERS_MOUNT	1
#define ENSC_WRAPPERS_UNISTD	1
#include <wrappers.h>

#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001

int		wrapper_exit_code  =  255;

int main(int argc, char *argv[])
{
	int i = 1;
	if (vc_isSupported(vcFEATURE_PIDSPACE)) {
		/* FIXME: Get options from etc/mtab
		 *	  Get list of filesystems from argv
		 */
		if (umount("proc") == 0)
			Emount("proc", "proc", "proc", 0, NULL);
		if (umount("sys") == 0)
			Emount("sysfs", "sys", "sysfs", 0, NULL);
	}
	if (strcmp(argv[i], "--") == 0)
		i++;
	EexecvpD(argv[i], argv+i);
	/* NOTREACHED */
	return 1;
}
