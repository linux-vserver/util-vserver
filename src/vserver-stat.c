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
#include "pathconfig.h"

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
#include <getopt.h>
#include <sys/param.h>
#include <sys/resource.h>

#define ENSC_WRAPPERS_DIRENT	1
#define ENSC_WRAPPERS_VSERVER	1
#define ENSC_WRAPPERS_FCNTL	1
#define ENSC_WRAPPERS_UNISTD	1
#include "wrappers.h"

#define PROC_DIR_NAME "/proc"
#define PROC_VIRT_DIR_NAME "/proc/virtual"
#define CTX_DIR_NAME "/var/run/vservers/"
#define CTX_NAME_MAX_LEN 50

int	wrapper_exit_code = 1;

#ifndef AT_CLKTCK
#define AT_CLKTCK       17    /* frequency of times() */
#endif

static unsigned long	hertz   =0x42;
static unsigned long	pagesize=0x42;

struct XidData
{
    xid_t		xid;
    unsigned int	process_count;
    uint64_t		VmSize_total;
    uint64_t		VmRSS_total;
    uint64_t		start_time_oldest;
    uint64_t		stime_total, utime_total;

    vcCfgStyle		cfgstyle;
    char const *	name;
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

#define CMD_HELP		0x1000
#define CMD_VERSION		0x1001

struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
  { "sort",     required_argument, 0, 'O' },
  {0,0,0,0}
};

static void
showHelp(char const *cmd)
{
  WRITE_MSG(1, "Usage:  ");
  WRITE_STR(1, cmd);
  WRITE_MSG(1,
	    "\n"
	    "Show information about all active contexts.\n\n"
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
  exit(0);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vserver-stat " VERSION " -- show virtual context statistics\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003,2005 Enrico Scholz\n"
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

static inline uint64_t
toMsec(uint64_t v)
{
  return v*1000llu/hertz;
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

static void
registerXidVstat(struct Vector *vec, unsigned long xid_l)
{
  xid_t			xid = (xid_t) xid_l;
  struct XidData	*res;
  struct vc_rlimit_stat	limit[3];
  struct vc_virt_stat	vstat;
  struct vc_sched_info	sched;
  int			cpu;

  res = Vector_search(vec, &xid, cmpData);
  if (res!=0) {
    WRITE_MSG(2, "Duplicate xid found?!\n");
    return;
  }
  if (vc_virt_stat(xid, &vstat) == -1) {
    perror("vc_virt_stat()");
    return;
  }
  if (vc_rlimit_stat(xid, RLIMIT_NPROC, &limit[0]) == -1) {
    perror("vc_rlimit_stat(RLIMIT_NRPOC)");
    return;
  }
  if (vc_rlimit_stat(xid, RLIMIT_AS, &limit[1]) == -1) {
    perror("vc_rlimit_stat(RLIMIT_AS)");
    return;
  }
  if (vc_rlimit_stat(xid, RLIMIT_RSS, &limit[2]) == -1) {
    perror("vc_rlimit_stat(RLIMIT_RSS)");
    return;
  }

  res			= Vector_insert(vec, &xid, cmpData);
  res->xid		= xid;

  res->process_count	= limit[0].value;
  res->VmSize_total	= limit[1].value * pagesize;
  res->VmRSS_total	= limit[2].value;
  res->start_time_oldest= getUptime() - vstat.uptime/1000000;

  res->utime_total	= 0;
  res->stime_total	= 0;
  // XXX: arbitrary CPU limit.
  for (cpu = 0; cpu < 1024; cpu++) {
    sched.cpu_id = cpu;
    sched.bucket_id = 0;
    if (vc_sched_info(xid, &sched) == -1)
      break;

    res->utime_total	+= sched.user_msec;
    res->stime_total	+= sched.sys_msec;
  }
}

static void
registerXidCgroups(struct Vector *vec, struct process_info *process)
{
  xid_t				xid = (xid_t) process->s_context;
  struct XidData		*res;

  switch (vc_getXIDType(xid)) {
    case vcTYPE_STATIC:
    case vcTYPE_DYNAMIC:
      break;
    default:
      return;
  }

  res = Vector_search(vec, &xid, cmpData);
  if (res == 0) {
    struct vc_rlimit_stat	limit;
    struct vc_virt_stat		vstat;
    struct vc_sched_info	sched;
    int				cpu;
    char			vhi_name[65],
				filename[128],
				cgroup[129],
				buf[30];
    int				fd;
    ssize_t			cgroup_len;
    unsigned long long		rss = 0;
    char			*endptr;
    size_t			len;
    uint64_t			stime_total, utime_total;


    if (vc_virt_stat(xid, &vstat) == -1) {
      perror("vc_virt_stat()");
      return;
    }
    if (vc_rlimit_stat(xid, RLIMIT_NPROC, &limit) == -1) {
      perror("vc_rlimit_stat(RLIMIT_NRPOC)");
      return;
    }
    if (vc_get_vhi_name(xid, vcVHI_CONTEXT, vhi_name, sizeof(vhi_name)) == -1) {
      perror("vc_get_vhi_name(CONTEXT)");
      return;
    }

    if ((fd = open(DEFAULTCONFDIR "/cgroup/mnt", O_RDONLY)) == -1) {
      strcpy(cgroup, "/dev/cgroup/");
      cgroup_len = sizeof("/dev/cgroup");
    }
    else {
      cgroup_len = read(fd, cgroup, sizeof(cgroup));
      if (cgroup_len == -1) {
        perror("read(cgroup/mnt)");
        return;
      }
      close(fd);
      cgroup[cgroup_len] = '/';
      cgroup_len += 1;
      cgroup[cgroup_len] = 0;
    }

    if ((fd = open(DEFAULTCONFDIR "/cgroup/base", O_RDONLY)) != -1) {
      len = read(fd, cgroup + cgroup_len, sizeof(cgroup) - cgroup_len);
      if (len == -1) {
        perror("read(cgroup/base)");
        return;
      }
      close(fd);
      cgroup_len += len;
      if (cgroup[cgroup_len - 1] != '/') {
        cgroup[cgroup_len] = '/';
        cgroup_len += 1;
      }
      cgroup[cgroup_len] = 0;
    }

    len = strlen(vhi_name);
    if ((len + sizeof("/cgroup/name")) >= sizeof(filename)) {
      WRITE_MSG(2, "too long context name: ");
      WRITE_STR(2, vhi_name);
      WRITE_MSG(2, "\n");
      return;
    }
    strcpy(filename, vhi_name);
    strcpy(filename + len, "/cgroup/name");

    if ((fd = open(filename, O_RDONLY)) == -1) {
      char *dir = strrchr(vhi_name, '/');
      if (dir == NULL) {
        WRITE_MSG(2, "invalid context name: ");
        WRITE_STR(2, dir);
        WRITE_MSG(2, "\n");
        return;
      }
      len = strlen(dir);
      if ((len + cgroup_len) >= sizeof(cgroup)) {
        WRITE_MSG(2, "cgroup name too long: ");
        WRITE_STR(2, dir);
        WRITE_MSG(2, "\n");
        return;
      }
      strcpy(cgroup + cgroup_len, dir);
      cgroup_len += len;
    }
    else {
      ssize_t ret;
      ret = read(fd, cgroup + cgroup_len, sizeof(cgroup) - cgroup_len);
      if (ret == -1) {
        perror("read(cgroup/name)");
        return;
      }
      cgroup_len += ret;
      close(fd);
    }

    if ((cgroup_len + sizeof("/memory.usage_in_bytes")) > sizeof(filename)) {
      WRITE_MSG(2, "cgroup name too long: ");
      WRITE_STR(2, cgroup);
      WRITE_MSG(2, "\n");
      return;
    }
    strcpy(filename, cgroup);
    strcpy(filename + cgroup_len, "/memory.usage_in_bytes");

    if ((fd = open(filename, O_RDONLY)) == -1)
      perror("open(memory.usage_in_bytes)");
    else {
      if (read(fd, buf, sizeof(buf)) == -1) {
	perror("read(memory.usage_in_bytes)");
	return;
      }
      close(fd);
      if ((rss = strtoull(buf, &endptr, 0)) == ULLONG_MAX ||
	  (*endptr != '\n' && *endptr != '\0')) {
	perror("strtoull(memory.usage_in_bytes)");
	return;
      }
    }

    strcpy(filename, cgroup);
    strcpy(filename + cgroup_len, "/cpuacct.stat");

    if ((fd = open(filename, O_RDONLY)) == -1) {
      utime_total	= 0;
      stime_total	= 0;
      // XXX: arbitrary CPU limit.
      for (cpu = 0; cpu < 1024; cpu++) {
	sched.cpu_id = cpu;
	sched.bucket_id = 0;
	if (vc_sched_info(xid, &sched) == -1)
	  break;

	utime_total	+= sched.user_msec;
	stime_total	+= sched.sys_msec;
      }
    }
    else {
      if (read(fd, buf, sizeof(buf)) == -1) {
	perror("read(cpuacct.stat)");
	return;
      }
      close(fd);

      if (sscanf(buf, "user %llu\nsystem %llu\n", &utime_total, &stime_total) != 2) {
	perror("sscanf(cpuacct.stat)");
	return;
      }
    }

    res			= Vector_insert(vec, &xid, cmpData);
    res->xid		= xid;

    res->process_count	= limit.value;
    res->VmRSS_total	= rss / 4096;
    res->start_time_oldest= getUptime() - vstat.uptime/1000000;

    res->utime_total	= toMsec(utime_total);
    res->stime_total	= toMsec(stime_total);
  }
  
  res->VmSize_total	+= process->VmSize;
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

static void initHertz()	   __attribute__((__constructor__));
static void initPageSize() __attribute__((__constructor__));

static void
initHertz()
{
  hertz = find_elf_note(AT_CLKTCK);
  if (hertz==(unsigned long)(-1))
    hertz = sysconf(_SC_CLK_TCK);
}

static void
initPageSize()
{
  pagesize = sysconf(_SC_PAGESIZE);
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
  hh = t%24;
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

static char *
formatName(char *dst, vcCfgStyle style, char const *name)
{
  size_t		len;
  
  if (name==0) name = "";
  len = strlen(name);

  switch (style) {
    case vcCFG_LEGACY	:
      len    = MIN(len, 18);
      *dst++ = '[';
      memcpy(dst, name, len);
      dst   += len;
      *dst++ = ']';
      break;

    default		:
      len    = MIN(len, 20);
      memcpy(dst, name, len);
      dst   += len;
      break;
  }

  return dst;
}

static void
showContexts(struct Vector const *vec)
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
    shortenMem (buf+17, ptr->VmRSS_total*pagesize);
    shortenTime(buf+24, ptr->utime_total);
    shortenTime(buf+34, ptr->stime_total);
    //printf("%llu, %llu\n", uptime, ptr->start_time_oldest);
    shortenTime(buf+44, uptime - ptr->start_time_oldest);

    formatName(buf+55, ptr->cfgstyle, ptr->name)[0] = '\0';

    Vwrite(1, buf, strlen(buf));
    Vwrite(1, "\n", 1);
  }
}

static void
fillName(void *obj_v, void UNUSED * a)
{
  struct XidData *	obj = obj_v;

  switch (obj->xid) {
    case 0		:
      obj->cfgstyle = vcCFG_NONE;
      obj->name     = strdup("root server");
      break;

    case 1		:
      obj->cfgstyle = vcCFG_NONE;
      obj->name     = strdup("monitoring server");
      break;

    default		: {
      char *		cfgpath;

      obj->cfgstyle  = vcCFG_AUTO;

      if ((cfgpath   = vc_getVserverByCtx(obj->xid, &obj->cfgstyle, 0))==0 ||
	  (obj->name = vc_getVserverName(cfgpath, obj->cfgstyle))==0) {
	obj->name     = 0;
	obj->cfgstyle = vcCFG_NONE;
      }

      free(cfgpath);

      break;
    }
  }
}

static void UNUSED
freeXidData(void *obj_v, void UNUSED * a)
{
  struct XidData *	obj = obj_v;

  free(const_cast(char *)(obj->name));
}

int main(int argc, char **argv)
{
  DIR *			proc_dir;
  struct dirent*	dir_entry;
  pid_t			my_pid;
  struct Vector		xid_data;
  char const *		errptr;

  while (1) {
    int		c = getopt_long(argc, argv, "+O:", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(argv[0]);
      case CMD_VERSION	:  showVersion();
      case 'O'		:  break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	return EXIT_FAILURE;
	break;
    }
  }
    
  if (optind!=argc) {
    WRITE_MSG(2, "Unknown parameter, use '--help' for more information\n");
    return EXIT_FAILURE;
  }

  if (hertz==0x42)    initHertz();
  if (pagesize==0x42) initPageSize();
  
  Vector_init(&xid_data, sizeof(struct XidData));

  if (vc_isSupported(vcFEATURE_VSTAT)) {
    unsigned long xid;
    Echdir(PROC_VIRT_DIR_NAME);
    proc_dir = Eopendir(".");
    while ((dir_entry = readdir(proc_dir)) != NULL) {
      if (!isNumberUnsigned(dir_entry->d_name, &xid, false))
	continue;

      registerXidVstat(&xid_data, xid);
    }
    closedir(proc_dir);
  }
  else {
    void (*handler)(struct Vector *vec, struct process_info *process);

    my_pid = getpid();

    if (!switchToWatchXid(&errptr)) {
      perror(errptr);
      exit(1);
    }

    if (access("/proc/uptime",R_OK)==-1 && errno==ENOENT)
      WRITE_MSG(2,
	      "WARNING: can not access /proc/uptime. Usually, this is caused by\n"
	      "         procfs-security. Please read the FAQ for more details\n"
	      "         http://linux-vserver.org/Proc-Security\n");

    if (vc_isSupported(vcFEATURE_MEMCG))
      handler = registerXidCgroups;
    else
      handler = registerXid;

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
	  handler(&xid_data, info);
      }
    }
    closedir(proc_dir);
  }

  Vector_foreach(&xid_data, fillName, 0);

    // output the ctx_list	
  showContexts(&xid_data);

#ifndef NDEBUG
  Vector_foreach(&xid_data, freeXidData, 0);
  Vector_free(&xid_data);
#endif
  
  return 0;
}
