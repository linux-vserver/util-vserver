// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on vserver-stat.cc by Guillaum Dallaire and Jacques Gelinas
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
	vserver-stat help you to see all the active context currently in the kernel
	with some useful stat

	Changelog:

	2003-01-08 Jacques Gelinas: Shows vserver description
	2002-02-28 Jacques Gelinas: Use dynamic system call
	2002-06-05 Martial Rioux : fix memory output error
	2002-12-05 Martial Rioux : fix output glitch
	2001-11-29 added uptime/ctx stat
	2001-11-20 added vmsize, rss, stime and utime stat
*/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "compat.h"

#include "vserver.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <syscall.h>
#include <time.h>

#define PROC_DIR_NAME "/proc"
#define CTX_DIR_NAME "/var/run/vservers/"
#define CTX_NAME_MAX_LEN 50

struct ctx_list
{
	int ctx;
	int process_count;
	int VmSize_total;
	int VmRSS_total;
	long start_time_oldest;
	long stime_total, utime_total;
	char name[CTX_NAME_MAX_LEN];
	struct ctx_list *next;
} *my_ctx_list;

struct process_info
{
	long VmSize;		// number of pages of virtual memory
	long VmRSS;		// resident set size from /proc/#/stat
	long start_time;	// start time of process -- seconds since 1-1-70
	long stime, utime;	// kernel & user-mode CPU time accumulated by process
	long cstime, cutime;	// cumulative time of process and reaped children
	int s_context;
};

char *process_name;

void usage()
{
	fprintf(stderr, "%s: from vserver kit version %s\n", process_name, VERSION);
	fprintf(stderr, "(no argument needed)\n\n");
	fprintf(stderr, "Show informations about all the active context.\n\n");
	fprintf(stderr, "	CTX#		Context number\n");
	fprintf(stderr, "			#0 = root context\n");
	fprintf(stderr, "			#1 = monitoring context\n");
	fprintf(stderr, "	PROC QTY	Quantity of processes in each context\n");
	fprintf(stderr, "	VSZ		Number of pages of virtual memory\n");
	fprintf(stderr, "	RSS		Resident set size\n");
	fprintf(stderr, "	userTIME	User-mode CPU time accumulated\n");
	fprintf(stderr, "	sysTIME		Kernel-mode CPU time accumulated\n");
	fprintf(stderr, "	UPTIME		Uptime/context\n");
	fprintf(stderr, "	NAME		Virtual server name\n");
	fprintf(stderr, "\n");

}

// return uptime (in ms) from /proc/uptime
long get_uptime()
{
	int fd;
	double up;
	char buffer[64];

	// open the /proc/uptime file
	if ((fd = open("/proc/uptime", O_RDONLY, 0)) == -1)
		return 0;

	if (read(fd, buffer, sizeof(buffer)) < 1)
		return 0;

	close(fd);

	if (sscanf(buffer, "%lf", &up) < 1)
	{
		fprintf(stderr, "%s: bad data in /proc/uptime\n", process_name);
		return 0;
	}

	return up * 100;
}

// insert a new record to the list
struct ctx_list *insert_ctx(int ctx, struct ctx_list *next)
{
	struct ctx_list *new;

	new = (struct ctx_list *)malloc(sizeof(struct ctx_list));
	new->ctx = ctx;
	new->process_count = 0;
	new->VmSize_total = 0;
	new->VmRSS_total = 0;
	new->utime_total = 0;
	new->stime_total = 0;
	new->start_time_oldest = 0;
	new->next = next;
	new->name[0] = '\0';

	return new;
}

// find the ctx record with the ctx number
struct ctx_list *find_ctx(struct ctx_list *list, int ctx)
{
	// very simple search engine...
	while(list != NULL)
	{
		// find
		if (list->ctx == ctx)
		{
			return list;
		}
		list = list->next;
	}
	return NULL;
}

// compute the process info into the list
void add_ctx(struct ctx_list *list, struct process_info *process)
{
	list->process_count ++;
	list->VmSize_total += process->VmSize;
	list->VmRSS_total += process->VmRSS;
	list->utime_total += process->utime + process->cutime;
	list->stime_total += process->stime + process->cstime;

	if (list->start_time_oldest == 0) // first entry
		list->start_time_oldest = process->start_time;
	else
		if (list->start_time_oldest > process->start_time)
			list->start_time_oldest = process->start_time;
}

// increment the count number in the ctx record using ctx number
void count_ctx(struct ctx_list *list, struct process_info *process)
{
	struct ctx_list *prev = list;

	if (process == NULL) return;

	// search
	while(list != NULL)
	{
		// find
		if (list->ctx == process->s_context)
		{
			add_ctx(list, process);
			return;
		}
		// insert between
		if (list->ctx > process->s_context)
		{
			prev->next = insert_ctx(process->s_context, list);
			add_ctx(prev->next, process);
			return;
		}
		// ++
		prev = list;
		list = list->next;
	}
	// add at the end
	prev->next = insert_ctx(process->s_context, NULL);
	add_ctx(prev->next, process);
}

// free mem
void free_ctx(struct ctx_list *list)
{
	struct ctx_list *prev;

	for(;list != NULL; list = prev)
	{
		prev = list->next;		
		free(list);
	}
}

/*
	Read the vserver description
*/
static void read_description(
	const char *name,		// Vserver name
	char descrip[1000])
{
	char conf[PATH_MAX];
	FILE *fin;
	descrip[0] = '\0';
	snprintf (conf,sizeof(conf)-1,"/etc/vservers/%s.conf",name);
	fin = fopen (conf,"r");
	if (fin != NULL){
		char line[1000];
		while (fgets(line,sizeof(line)-1,fin)!=NULL){
			if (line[0] == '#'){
				char *pt = line+1;
				while (isspace(*pt)) pt++;
				if (strncmp(pt,"Description:",12)==0){
					int last;
					pt += 12;
					while (isspace(*pt)) pt++;
					strcpy (descrip,pt);
					last = strlen(descrip)-1;
					if (last >=0 && descrip[last] == '\n'){
						descrip[last] = '\0';
					}
				}
			}
		}
		fclose (fin);
	}
}

// show the ctx_list with name from /var/run/servers/*.ctx
void show_ctx(struct ctx_list *list)
{
	// fill the ctx_list using the /var/run/servers/*.ctx file(s)
	 __extension__ int bind_ctx_name(struct ctx_list *list)
	{
		// fetch the context number in /var/run/vservers/'filename'
		int fetch_ctx_number(char *filename)
		{
			int fd;
			int ctx;
			char buf[25];

			// open file
			if ((fd = open(filename, O_RDONLY, 0)) == -1)
				return -1;
			// put the file in a small buffer
			if (read(fd, buf, sizeof(buf)) < 1)
				return -1;

			close(fd);

			sscanf(buf, "S_CONTEXT=%d", &ctx);
			return ctx;
		}

		/* begin bind_ctx_name */

		DIR *ctx_dir;
		struct dirent *dir_entry;
		char *p;
		char ctx_name[CTX_NAME_MAX_LEN];
		struct ctx_list *ctx;
		int ctx_number;

		// open the /var/run/vservers directory
		if ((ctx_dir = opendir(CTX_DIR_NAME)) == NULL)
		{
			fprintf(stderr, "%s: in openning %s: %s\n", process_name, CTX_DIR_NAME, strerror(errno));
			return -1;
		}
	
		chdir(CTX_DIR_NAME);
		while ((dir_entry = readdir(ctx_dir)) != NULL)
		{
			strncpy(ctx_name, dir_entry->d_name, sizeof(ctx_name));
			p = strstr(ctx_name, ".ctx");
			if (p != NULL) // make sure that it is a .ctx file..
			{
				*p = '\0'; // remove the .ctx in the file name
				if ((ctx_number = fetch_ctx_number(dir_entry->d_name)) > 1)
				{
					if ((ctx = find_ctx(list, ctx_number)) != NULL)
						strncpy(ctx->name, ctx_name, CTX_NAME_MAX_LEN);
				}
			}
			// else fprintf(stderr, "invalid file %s in %s\n", dir_entry->d_name, CTX_DIR_NAME);
		}
		closedir(ctx_dir);	
		return 0;
	}

	 __extension__ char *convert_time(unsigned t, char *str)
	{
		unsigned hh, mm, ss, ms;

		ms = t % 100;
		t /= 100;

		ss = t%60;
		t /= 60;
		mm = t%60;
		t /= 60;
		hh = t%60;
 		t /= 24;

		if (t > 0)	// day > 0
		{
	  			snprintf(str, 25, "%3.ud%02uh%02u", t, (hh%12) ? hh%12 : 12, mm);
		}
		else
		{
			if (hh > 0) // hour > 0
	  			snprintf(str, 25, " %2.uh%02um%02u", hh, mm, ss);
			else
			{
	  			snprintf(str, 25, " %2.um%02u.%02u", mm, ss, ms);
			}
		}
		return str;
	}

	 __extension__ char *convert_mem(unsigned long total, char *str)
	{
		// Byte
		if (total < 1024)
		{
			snprintf(str, 25, "%luB", total);
			return str;
		}

		total >>= 10; // kByte
		if (total < 1024)
		{
			snprintf(str, 25, "%lukB", total);
			return str;
		}

		total >>= 10; // MByte
		if (total < 1024)
		{
			snprintf(str, 25, "%luMB", total);
			return str;
		}

		total >>= 10; // GByte
		if (total < 1024)
		{
			snprintf(str, 25, "%luGB", total);
			return str;
		}
		total >>= 10; // TByte
		snprintf(str, 25, "%luTB", total);
		return str;
	}

	/* begin show_ctx */
	char utime[25], stime[25], ctx_uptime[25];
	char vmsize[25], vmrss[25];
	long uptime = get_uptime();

	// now we have all the active context, fetch the name
	// from /var/run/vservers/*.ctx
	bind_ctx_name(list);

	printf("CTX  PROC    VSZ    RSS  userTIME   sysTIME    UPTIME NAME     DESCRIPTION\n");
	while(list != NULL)
	{
		char descrip[1000];
		if (list->ctx == 1)
			strncpy(list->name, "monitoring server", CTX_NAME_MAX_LEN);

		read_description (list->name,descrip);

		printf("%-4d %4d %6s %6s %9s %9s %9s %-8s %s\n", list->ctx, list->process_count,
			convert_mem(list->VmSize_total, vmsize), convert_mem(list->VmRSS_total, vmrss),
			convert_time(list->utime_total, utime), convert_time(list->stime_total, stime), convert_time(uptime - list->start_time_oldest, ctx_uptime)
			, list->name,descrip);
		list = list->next;
	}
}

// open the process's status file to get the ctx number, and other stat
struct process_info *get_process_info(char *pid)
{
	int fd;
	char buffer[1024];
	char *p;
	static struct process_info process;

	// open the proc/#/status file
	snprintf(buffer, sizeof(buffer),  "/proc/%s/status", pid);
	if ((fd = open(buffer, O_RDONLY, 0)) == -1)
		return NULL;
	// put the file in a buffer
	if (read(fd, buffer, sizeof(buffer)) < 1)
		return NULL;

	close(fd);

	// find the s_context entry
	if ((p = strstr(buffer, "s_context:")) == NULL)
		return NULL;

	sscanf(p, "s_context: %d", &process.s_context);

	// open the /proc/#/stat file
	snprintf(buffer, sizeof(buffer),  "/proc/%s/stat", pid);
	if ((fd = open(buffer, O_RDONLY, 0)) == -1)
		return NULL;
	// put the file in a buffer
	if (read(fd, buffer, sizeof(buffer)) < 1)
		return NULL;

	close(fd);

	p = strchr(buffer, ')');		// go after the PID (process_name)
	sscanf(p + 2,
		"%*s "
		"%*s %*s %*s %*s %*s "
		"%*s %*s %*s %*s %*s %ld %ld "
		"%ld %ld %*s %*s %*s %*s "
		"%ld %ld "
		"%ld ", &process.utime, &process.stime,
			&process.cutime, &process.cstime,
			&process.start_time,
 			&process.VmSize, &process.VmRSS);

	return &process;
}

int main(int argc, char **argv)
{
	DIR *proc_dir;
	struct dirent *dir_entry;
	pid_t my_pid;

	// for error msg
	process_name = argv[0];

	if (argc > 1)
	{
		usage();
		return 0;
	}

	// do not include own stat
	my_pid = getpid();

	// try to switch in context 1
	if (vc_new_s_context(1,0, 0) < 0)
	{
		fprintf(stderr, "%s: unable to switch in context security #1\n", process_name);
		return -1;
	}

	// create the fist...
	my_ctx_list = insert_ctx(0, NULL);
	// init with the default name for the context 0
	strncpy(my_ctx_list->name, "root server", CTX_NAME_MAX_LEN);

	// open the /proc dir
	if ((proc_dir = opendir(PROC_DIR_NAME)) == NULL)
	{
		fprintf(stderr, "%s: %s\n", process_name, strerror(errno));
		return -1;
	}
	
	chdir(PROC_DIR_NAME);
	while ((dir_entry = readdir(proc_dir)) != NULL)
	{
		// select only process file
		if (!isdigit(*dir_entry->d_name))
			continue;

		if (atoi(dir_entry->d_name) != my_pid)
			count_ctx(my_ctx_list, get_process_info(dir_entry->d_name));
		
	}
	closedir(proc_dir);

	// output the ctx_list	
	show_ctx(my_ctx_list);

	// free the ctx_list
	free_ctx(my_ctx_list);

	return 0;
}
