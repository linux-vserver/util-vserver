// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on tests/testipc.cc by Jacques Gelinas
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
	Test to see isolation of the various IPC resources
	between security context
*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

int main (int argc, char *argv[])
{
	int ret = -1;
	if (argc < 2){
		fprintf (stderr,
			"testipc createshm\n"
			);
	}else if(strcmp(argv[1],"createshm")==0){
		int id = shmget (1,1024,IPC_CREAT|0666);
		if (id == -1){
			fprintf (stderr,"shmget failed (%s)\n",strerror(errno));
		}else{
			void *pt = shmat (id,NULL,0);
			printf ("shmget id %d\n",id);
			if (pt == NULL){
				fprintf (stderr,"can't shmat to id %d (%s)\n",id,strerror(errno));
			}else{
				char tmp[100];
				int  ok;
				strcpy ((char*)pt,"original string");

				printf ("Letting a sub-program attach to this memory\n");
				sprintf (tmp,"./testipc accessshm %d",id);
				ok = system (tmp);
				printf ("\tSub-program returned %d\n",ok);

				printf ("\tThe segment now hold :%s:\n",(char*)pt);
				shmdt (pt);

				printf ("A sub-program in another context can't attach\n");
				sprintf (tmp,"/usr/sbin/chcontext ./testipc accessshm %d",id);
				ok = system (tmp);
				printf ("\tSub-program returned %d\n",ok);

				printf ("Executing a sub-shell\n");
				system ("/bin/sh");
			}
			printf ("Delete the share memory segment\n");
			if (shmctl (id,IPC_RMID,NULL)==-1){
				fprintf (stderr,"shmctl failed (%s)\n",strerror(errno));
			}else{
				ret = 0;
			}
		}
	}else if(strcmp(argv[1],"accessshm")==0){
		int id = atoi(argv[2]);
		void *pt = shmat (id,NULL,0);
		if (pt == (void*)-1){
			fprintf (stderr,"can't shmat to id %d (%s)\n",id,strerror(errno));
		}else{
			printf ("\tWriting hello in share memory\n");
			strcpy ((char*)pt,"hello");
			ret = 0;
		}
	}else if(strcmp(argv[1],"createsem")==0){
		int id = semget (1,1,IPC_CREAT|0666);
		if (id == -1){
			fprintf (stderr,"semget failed (%s)\n",strerror(errno));
		}else{
			char tmp[100];
			int  ok;
			printf ("semget id %d\n",id);

			printf ("Letting a sub-program play with this semaphore\n");
			sprintf (tmp,"./testipc accesssem %d",id);
			ok = system (tmp);
			printf ("\tSub-program returned %d\n",ok);

			printf ("A sub-program in another context can't use the semaphore\n");
			sprintf (tmp,"/usr/sbin/chcontext ./testipc accesssem %d",id);
			ok = system (tmp);
			printf ("\tSub-program returned %d\n",ok);

			printf ("Executing a sub-shell\n");
			system ("/bin/sh");

			printf ("Delete the semaphore\n");
			if (semctl (id,0,IPC_RMID,NULL)==-1){
				fprintf (stderr,"semctl failed (%s)\n",strerror(errno));
			}else{
				ret = 0;
			}
		}
	}else if(strcmp(argv[1],"accesssem")==0){
		int id = atoi(argv[2]);
		struct sembuf ops[]={
			{0,0,0}
		};
		if (semop (id,ops,1) == -1){
			fprintf (stderr,"can't semop with id %d (%s)\n",id,strerror(errno));
		}else{
			ret = 0;
		}
	}
	return ret;
}
		

