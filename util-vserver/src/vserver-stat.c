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

#include "vserver.h"
#include "util.h"
#include "internal.h"
#include "wrappers.h"

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
#include <stdbool.h>

#define PROC_DIR_NAME "/proc"
#define CTX_DIR_NAME "/var/run/vservers/"
#define CTX_NAME_MAX_LEN 50

int	wrapper_exit_code = 1;

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

struct ArgInfo {
    enum { tpUNSET, tpCTX, tpPID }	type;
    xid_t		ctx;
    pid_t		pid;
    unsigned int	interval;
    bool		shutdown;
    bool		omit_init;
    size_t		argc;
    char * const *	argv;
};

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    "\n"
	    "Show informations about all the active context.\n\n"
	    "	CTX#		Context number\n"
	    "			#0 = root context\n"
	    "			#1 = monitoring context\n"
	    "	PROC QTY	Quantity of processes in each context\n"
	    "	VSZ		Number of pages of virtual memory\n"
	    "	RSS		Resident set size\n"
	    "	userTIME	User-mode CPU time accumulated\n"
	    "	sysTIME		Kernel-mode CPU time accumulated\n"
	    "	UPTIME		Uptime/context\n"
	    "	NAME		Virtual server name\n"
	    "\n");
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vserver-stat " VERSION " -- show virtual context statistics\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}


// return uptime (in ms) from /proc/uptime
static uint64_t
getUptime()
{
  int		fd;
  char		buffer[64];
  char *	errptr;
  size_t	len;
  time_t	secs;
  uint32_t	msecs=0;

    // open the /proc/uptime file
  fd  = Eopen("/proc/uptime", O_RDONLY, 0);
  len = Eread(fd, buffer, sizeof buffer);

  if (len==sizeof(buffer)) {
    WRITE_MSG(2, "Too much data in /proc/uptime; aborting...\n");
    exit(1);
  }
  Eclose(fd);

  while (len>0 && buffer[len-1]=='\n') --len;
  buffer[len] = '\0';

  secs = strtol(buffer, &errptr, 10);
  if (*errptr!='.') errptr = buffer;
  else              msecs = strtol(errptr+1, &errptr, 10);

  if ((*errptr!='\0' && *errptr!=' ') || errptr==buffer) {
    WRITE_MSG(2, "Bad data in /proc/uptime\n");
    exit(1);
  }

  return secs*100 + msecs;
}

// insert a new record to the list
static struct ctx_list *
insert_ctx(int ctx, struct ctx_list *next)
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

// compute the process info into the list
static void
add_ctx(struct ctx_list *list, struct process_info *process)
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
static void
count_ctx(struct ctx_list *list, struct process_info *process)
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
static void
free_ctx(struct ctx_list *list)
{
	struct ctx_list *prev;

	for(;list != NULL; list = prev)
	{
		prev = list->next;		
		free(list);
	}
}


// open the process's status file to get the ctx number, and other stat
struct process_info *get_process_info(char *pid)
{
  int 		fd;
  char		buffer[1024];
  char		*p;
  size_t		idx, l=strlen(pid);
  static struct process_info process;

  process.s_context = vc_X_getctx(atoi(pid));

  
  memcpy(buffer,     "/proc/", 6); idx  = 6;
  memcpy(buffer+idx, pid,      l); idx += l;
  memcpy(buffer+idx, "/stat",  6);
	
    // open the /proc/#/stat file
  if ((fd = open(buffer, O_RDONLY, 0)) == -1)
    return NULL;
    // put the file in a buffer
  if (read(fd, buffer, sizeof(buffer)) < 1)
    return NULL;

  close(fd);

  p   = strchr(buffer, ')');		// go after the PID (process_name)
  for (idx = 0; idx<12 && *p!='\0'; ++p)
    if ((*p)==' ') ++idx;

  process.utime  = strtol(p,   &p, 10);
  process.stime  = strtol(p+1, &p, 10);
  process.cutime = strtol(p+1, &p, 10);
  process.cstime = strtol(p+1, &p, 10);

  for (idx = 0; idx<5 && *p!='\0'; ++p)
    if ((*p)==' ') ++idx;

  process.start_time = strtol(p,   &p, 10);
  process.VmSize     = strtol(p+1, &p, 10);
  process.VmRSS      = strtol(p+1, &p, 10);
	
  return &process;
}



static size_t
fillUintZero(char *buf, unsigned long val, size_t cnt)
{
  size_t	l;
  
  l = utilvserver_fmt_ulong(buf, val);
  if (l<cnt) {
    memmove(buf+cnt-l, buf, l);
    memset(buf, '0', cnt-l);
  }
  buf[cnt] = '\0';

  return cnt;
}

static void
shortenMem(char *buf, unsigned long val)
{
  char const *	SUFFIXES[] = { " ", "K", "M", "G", "T", "+" };
  char		tmp[16];
  char const *	suffix = "+";
  size_t	i, l;
  unsigned int	mod = 0;

  for (i=0; i<6; ++i) {
    if (val<1000) {
      suffix = SUFFIXES[i];
      break;
    }
    mod   = 10*(val & 1023)/1024;
    val >>= 10;
  }

  if (val >9999) val=9999;
  if (val>=1000) mod=0;

  l = utilvserver_fmt_ulong(tmp, val);
  if (mod!=0) {
    tmp[l++] = '.';
    l += utilvserver_fmt_ulong(tmp+l, mod);
  }
  i = 7-l-strlen(suffix);
  
  memcpy(buf+i,   tmp, l);
  memcpy(buf+i+l, suffix, strlen(suffix));
}

static void
shortenTime(char *buf, uint64_t t)
{
  char		tmp[32];
  char		*ptr = tmp;

  unsigned long	hh, mm, ss, ms;

  ms = t % 100;
  t /= 100;

  ss = t%60;
  t /= 60;
  mm = t%60;
  t /= 60;
  hh = t%60;
  t /= 24;

  if (t>0) {
    ptr   += utilvserver_fmt_ulong(ptr, t);
    *ptr++ = 'd';
    ptr   += fillUintZero(ptr, hh, 2);
    *ptr++ = 'h';
    ptr   += fillUintZero(ptr, mm, 2);
  }
  else if (hh>0) {
    ptr   += utilvserver_fmt_ulong(ptr, hh);
    *ptr++ = 'h';
    ptr   += fillUintZero(ptr, mm, 2);
    *ptr++ = 'm';
    ptr   += fillUintZero(ptr, ss, 2);    
  }
  else {
    ptr   += utilvserver_fmt_ulong(ptr, mm);
    *ptr++ = 'm';
    ptr   += fillUintZero(ptr, ss, 2);
    *ptr++ = 's';
    ptr   += fillUintZero(ptr, ms, 2);
  }

  *ptr = ' ';
  memcpy(buf+10-(ptr-tmp), tmp, ptr-tmp);
}

void showContexts(struct ctx_list *list)
{
  uint64_t	uptime = getUptime();

  WRITE_MSG(1, "CTX   PROC    VSZ    RSS  userTIME   sysTIME    UPTIME NAME\n");
  while (list!=0) {
    char	buf[512];
    char	tmp[32];
    size_t	l;

    memset(buf, ' ', sizeof(buf));
    l = utilvserver_fmt_ulong(buf, list->ctx);
    l = utilvserver_fmt_ulong(tmp, list->process_count);
    memcpy(buf+10-l, tmp, l);

    shortenMem (buf+10, list->VmSize_total);
    shortenMem (buf+17, list->VmRSS_total);
    shortenTime(buf+24, list->utime_total);
    shortenTime(buf+34, list->stime_total);
    shortenTime(buf+44, uptime - list->start_time_oldest);

    switch (list->ctx) {
      case 0		:  strncpy(buf+55, "root server",       20); break;
      case 1		:  strncpy(buf+55, "monitoring server", 20); break;
      default		: {
	char *	name     = 0;
	char *	cfgpath  = 0;
	vcCfgStyle	cfgstyle = vcCFG_AUTO;

	if ((cfgpath = vc_getVserverByCtx(list->ctx, &cfgstyle, 0))!=0 &&
	    (name    = vc_getVserverName(cfgpath, cfgstyle))!=0)
	  strncpy(buf+55, name, 20);
	free(name);
	free(cfgpath);
      }
    }

    write(1, buf, 80);
    write(1, "\n", 1);

    list = list->next;
  }
}

int main(int argc, char **argv)
{
  DIR *proc_dir;
  struct dirent *dir_entry;
  pid_t my_pid;

    // for error msg
  process_name = argv[0];

  if (argc==2) {
    if (strcmp(argv[1], "--help")   ==0) showHelp(1, argv[0], 0);
    if (strcmp(argv[1], "--version")==0) showVersion();
  }
  if (argc!=1) {
    WRITE_MSG(2, "Unknown parameter, use '--help' for more information\n");
    return EXIT_FAILURE;
  }

    // do not include own stat
  my_pid = getpid();

#if 1
    // try to switch in context 1
  if (vc_new_s_context(1,0, 0) < 0)
  {
    perror("vc_new_s_context(#1,...)");
    return EXIT_FAILURE;
  }
#endif
	
    // create the fist...
  my_ctx_list = insert_ctx(0, NULL);
    // init with the default name for the context 0
  strncpy(my_ctx_list->name, "root server", CTX_NAME_MAX_LEN);

    // open the /proc dir
  if ((proc_dir = opendir(PROC_DIR_NAME)) == NULL) {
    perror("opendir()");
    return EXIT_FAILURE;
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
  showContexts(my_ctx_list);

    // free the ctx_list
  free_ctx(my_ctx_list);

  return 0;
}
