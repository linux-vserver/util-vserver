// $Id$    --*- c++ -*--

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on tests/chrootsafe.cc by Jacques Gelinas
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
	Test the chrootsafe command.
	Pass the path of a vserver as the only argument

	chrootsafe /vservers/test
*/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "compat.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "vserver.h"

int main (int argc, char *argv[])
{
	// This test must fail
	int fd = open ("/",O_RDONLY);
	if (fd != -1){
		if (vc_chrootsafe(argv[1])==-1){
			fprintf (stderr,"Ok, chrootsafe failed with one open directory errno=%s\n",strerror(errno));
		}else{
			fprintf (stderr,"Hum, chrootsafe succeed with one open directory\n");
			system ("/bin/sh");
		}
		close (fd);
	}
	// Now it should work
	if (vc_chrootsafe(argv[1])!=-1){
		fprintf (stderr,"Ok, chrootsafe worked\n");
		system ("/bin/sh");
	}else{
		fprintf (stderr,"chrootsafe failed errno=%s\n",strerror(errno));
	}
	return 0;
}	

