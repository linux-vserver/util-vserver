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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "vserver.h"
#include "util.h"
#include "internal.h"

#include <ensc_vector/vector.h>

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
#include <sys/param.h>

#define ENSC_WRAPPERS_DIRENT	1
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_UNISTD	1
#include "wrappers.h"

#define PROC_DIR_NAME "/proc"
#define CTX_DIR_NAME "/var/run/vservers/"
#define CTX_NAME_MAX_LEN 50

int	wrapper_exit_code = 1;

#ifndef AT_CLKTCK
#define AT_CLKTCK       17    /* frequency of times() */
#endif

static unsigned long	hertz=0x42;

struct XidData
{
    xid_t	xid;
    int		process_count;
    int		VmSize_total;
    int		VmRSS_total;
    uint64_t	start_time_oldest;
    uint64_t	stime_total, utime_total;
};

struct process_info
{
	long 		VmSize;		// number of pages of virtual memory
	long 		VmRSS;		// resident set size from /proc/#/stat
	uint64_t	start_time;	// start time of process -- milliseconds since 1-1-70
        uint64_t	stime, utime;	// kernel & user-mode CPU time accumulated by process
	uint64_t	cstime, cutime;	// cumulative time of process and reaped children
	xid_t		s_context;
};

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
  uint64_t	secs;
  uint32_t	msecs=0;

    // open the /proc/uptime file
  fd  = EopenD("/proc/uptime", O_RDONLY, 0);
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
  else {
    unsigned int	mult;
    switch (strlen(errptr+1)) {
      case 0	:  mult = 1000; break;
      case 1	:  mult = 100;  break;
      case 2	:  mult = 10;   break;
      case 3	:  mult = 1;    break;
      default	:  mult = 0;    break;
    }
    msecs = strtol(errptr+1, &errptr, 10) * mult;
  }

  if ((*errptr!='\0' && *errptr!=' ') || errptr==buffer) {
    WRITE_MSG(2, "Bad data in /proc/uptime\n");
    exit(1);
  }

  return secs*1000 + msecs;
}

static int
cmpData(void const *xid_v, void const *map_v)
{
  xid_t const * const			xid = xid_v;
  struct XidData const * const		map = map_v;
  int	res = *xid - map->xid;

  return res;
}

static void
registerXid(struct Vector *vec, struct process_info *process)
{
  struct XidData	*res;

  res = Vector_search(vec, &process->s_context, cmpData);
  if (res==0) {
    res = Vector_insert(vec, &process->s_context, cmpData);
    res->xid           = process->s_context;
    res->process_count = 0;
    res->VmSize_total  = 0;
    res->VmRSS_total   = 0;
    res->utime_total   = 0;
    res->stime_total   = 0;
    res->start_time_oldest = process->start_time;
  }

  ++res->process_count;
  res->VmSize_total += process->VmSize;
  res->VmRSS_total  += process->VmRSS;
  res->utime_total  += process->utime + process->cutime;
  res->stime_total  += process->stime + process->cstime;

  res->start_time_oldest = MIN(res->start_time_oldest, process->start_time);
}

static inline uint64_t
toMsec(uint64_t v)
{
  return v*1000llu/hertz;
}


// shamelessly stolen from procps...
static unsigned long
find_elf_note(unsigned long findme){
  unsigned long *ep = (unsigned long *)environ;
  while(*ep++);
  while(*ep){
    if(ep[0]==findme) return ep[1];
    ep+=2;
  }
  return (unsigned long)(-1);
}

static void initHertz()	__attribute__((__constructor__));

static void
initHertz()
{
  hertz = find_elf_note(AT_CLKTCK);
  if (hertz==(unsigned long)(-1))
    hertz = sysconf(_SC_CLK_TCK);
}

// open the process's status file to get the ctx number, and other stat
struct process_info *
get_process_info(char *pid)
{
  int 				fd;
  char				buffer[1024];
  char				*p;
  size_t			idx, l=strlen(pid);
  static struct process_info	process;

#if 1
  process.s_context = vc_get_task_xid(atoi(pid));
#else
#  warning Compiling in debug-code
  process.s_context = random()%6;
#endif

  if (process.s_context==VC_NOCTX) {
    int		err=errno;
    WRITE_MSG(2, "vc_get_task_xid(");
    WRITE_STR(2, pid);
    WRITE_MSG(2, "): ");
    WRITE_STR(2, strerror(err));
    WRITE_MSG(2, "\n");

    return 0;
  }
  
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

  process.utime  = toMsec(strtol(p,   &p, 10));
  process.stime  = toMsec(strtol(p+1, &p, 10));
  process.cutime = toMsec(strtol(p+1, &p, 10));
  process.cstime = toMsec(strtol(p+1, &p, 10));

  for (idx = 0; idx<5 && *p!='\0'; ++p)
    if ((*p)==' ') ++idx;

  process.start_time = toMsec(strtol(p,   &p, 10));
  process.VmSize     = strtol(p+1, &p, 10);
  process.VmRSS      = strtol(p+1, &p, 10);

  //printf("pid=%s, start_time=%llu\n", pid, process.start_time);
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

  ms = t % 1000;
  t /= 1000;

  ss = t%60;
  t /= 60;
  mm = t%60;
  t /= 60;
  hh = t%60;
  t /= 24;

  if (t>999*999) {
    memcpy(ptr, "INVALID", 7);
    ptr   += 7;
  }
  else if (t>999) {
    ptr   += utilvserver_fmt_ulong(ptr, t/365);
    *ptr++ = 'y';
    ptr   += fillUintZero(ptr, t%365, 2);
    *ptr++ = 'd';
    ptr   += fillUintZero(ptr, hh, 2);
  }    
  else if (t>0) {
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

static void
showContexts(struct Vector *vec)
{
  uint64_t			uptime  = getUptime();
  struct XidData const *	ptr     = Vector_begin_const(vec);
  struct XidData const * const	end_ptr = Vector_end_const(vec);
  

  WRITE_MSG(1, "CTX   PROC    VSZ    RSS  userTIME   sysTIME    UPTIME NAME\n");
  for (; ptr<end_ptr; ++ptr) {
    char	buf[sizeof(xid_t)*3 + 512];
    char	tmp[sizeof(int)*3 + 2];
    size_t	l;

    memset(buf, ' ', sizeof(buf));
    l = utilvserver_fmt_long(buf, ptr->xid);
    l = utilvserver_fmt_long(tmp, ptr->process_count);
    memcpy(buf+10-l, tmp, l);

    shortenMem (buf+10, ptr->VmSize_total);
    shortenMem (buf+17, ptr->VmRSS_total);
    shortenTime(buf+24, ptr->utime_total);
    shortenTime(buf+34, ptr->stime_total);
    //printf("%llu, %llu\n", uptime, ptr->start_time_oldest);
    shortenTime(buf+44, uptime - ptr->start_time_oldest);

    switch (ptr->xid) {
      case 0		:  strcpy(buf+55, "root server"      ); break;
      case 1		:  strcpy(buf+55, "monitoring server"); break;
      default		: {
	char *		name     = 0;
	char *		cfgpath  = 0;
	vcCfgStyle	cfgstyle = vcCFG_AUTO;

	if ((cfgpath = vc_getVserverByCtx(ptr->xid, &cfgstyle, 0))==0 ||
	    (name    = vc_getVserverName(cfgpath, cfgstyle))==0) {
	  name     = strdup("");
	  cfgstyle = vcCFG_NONE;
	}
	
	switch (cfgstyle) {
	  case vcCFG_LEGACY	: {
	    size_t	len = MIN(strlen(name), 18);
	    buf[55]     = '[';
	    memcpy(buf+56,     name, len);
	    memcpy(buf+56+len, "]",  2);
	    break;
	  }
	  default		: {
	    size_t	len = MIN(strlen(name), 20);
	    memcpy(buf+55, name, len);
	    buf[55+len] = '\0';
	    break;
	  }
	}
	
	free(name);
	free(cfgpath);
      }
    }

    Vwrite(1, buf, strlen(buf));
    Vwrite(1, "\n", 1);
  }
}

int main(int argc, char **argv)
{
  DIR *			proc_dir;
  struct dirent*	dir_entry;
  pid_t			my_pid;
  struct Vector		xid_data;
  char const *		errptr;

  if (argc==2) {
    if (strcmp(argv[1], "--help")   ==0) showHelp(1, argv[0], 0);
    if (strcmp(argv[1], "--version")==0) showVersion();
  }
  if (argc!=1) {
    WRITE_MSG(2, "Unknown parameter, use '--help' for more information\n");
    return EXIT_FAILURE;
  }

  if (hertz==0x42) initHertz();
  
  //printf("hertz=%lu\n", hertz);
    // do not include own stat
  my_pid = getpid();

  if (!switchToWatchXid(&errptr)) {
    perror(errptr);
    exit(1);
  }

  if (access("/proc/uptime",R_OK)==-1 && errno==ENOENT)
    WRITE_MSG(2,
	      "WARNING: can not access /proc/uptime. Usually, this is caused by\n"
	      "         procfs-security. Please read the FAQ for more details\n"
	      "         http://www.linux-vserver.org/index.php?page=Linux-Vserver+FAQ\n");

  Vector_init(&xid_data, sizeof(struct XidData));

  Echdir(PROC_DIR_NAME);
  proc_dir = Eopendir(".");
  while ((dir_entry = readdir(proc_dir)) != NULL)
  {
      // select only process file
    if (!isdigit(*dir_entry->d_name))
      continue;

    if (atoi(dir_entry->d_name) != my_pid) {
      struct process_info *	info = get_process_info(dir_entry->d_name);
      if (info)
	registerXid(&xid_data, info);
    }
  }
  closedir(proc_dir);

    // output the ctx_list	
  showContexts(&xid_data);

#ifndef NDEBUG
  Vector_free(&xid_data);
#endif
  
  return 0;
}
