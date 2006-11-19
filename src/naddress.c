// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// Copyright (C) 2006 Daniel Hokka Zakrisson <daniel@hozac.com>
// based on chbind.cc by Jacques Gelinas
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

#include <lib/internal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ENSC_WRAPPERS_PREFIX	"naddress: "
#define ENSC_WRAPPERS_IO	1
#define ENSC_WRAPPERS_UNISTD	1
#define ENSC_WRAPPERS_VSERVER	1
#include "wrappers.h"

#define CMD_HELP	0x1000
#define CMD_VERSION	0x1001

#define CMD_SILENT	0x2000
#define CMD_NID		0x2001
#define CMD_ADD		0x2002
#define CMD_REMOVE	0x2003
#define CMD_IP		0x2010
#define CMD_BCAST	0x2011

int wrapper_exit_code = 255;


static struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
  { "silent",   no_argument,  0, CMD_SILENT },
  { "add",      no_argument,  0, CMD_ADD },
  { "remove",   no_argument,  0, CMD_REMOVE },
  { "nid",      required_argument, 0, CMD_NID },
  { "ip",       required_argument, 0, CMD_IP },
  { "bcast",    required_argument, 0, CMD_BCAST },
  { 0,0,0,0 }
};

struct vc_ips {
  struct vc_net_nx a;
  struct vc_ips *next;
};

struct Arguments {
  nid_t		nid;
  struct vc_ips	head;
  bool		is_silent;
  bool		do_add;
  bool		do_remove;
};

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [--silent] [--nid <nid>] [--ip <ip_num>[/<mask>]] [--bcast <broadcast>] [--] <commands> <args>*\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "naddress " VERSION " -- bind to an ip and execute a program\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003,2004 Enrico Scholz\n"
	    "Copyright (C) 2006 Daniel Hokka Zakrisson\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}

/*
	Check if a network device exist in /proc/net/dev.
	This is used because ifconfig_ioctl triggers modprobe if requesting
	information about non existant devices.

	Return != 0 if the device exist.
*/
static bool
existsDevice(char const *dev_raw)
{
  size_t	buf_size=8192;
  char		dev[strlen(dev_raw)+2];

  strcpy(dev, dev_raw);
  strcat(dev, ":");
  for (;;) {
    char	buf[buf_size];
    char *	pos;
    bool	too_small;
    int		fd=open("/proc/net/dev", O_RDONLY);
    
    if (fd==-1) return false;
    too_small = EreadAll(fd, buf, buf_size);
    close(fd);

    if (too_small) {
      buf_size *= 2;
      continue;
    }

    pos = strstr(buf, dev);
    return (pos && (pos==buf || pos[-1]==' ' || pos[-1]=='\n'));
  }
}

static int ifconfig_ioctl(
	int fd,
	const char *ifname,
	int cmd,
	struct ifreq *ifr)
{
	strcpy(ifr->ifr_name, ifname);
	return ioctl(fd, cmd, ifr);
}

/*
	Fetch the IP number of an interface from the kernel.
	Assume the device is already available in the kernel
	Return -1 if any error.
*/
int ifconfig_getaddr (
	const char *ifname,
	uint32_t *addr,
	uint32_t *mask,
	uint32_t *bcast)
{
	int ret = -1;
	if (existsDevice(ifname)){
		int skfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (skfd != -1){
			struct ifreq ifr;
			if (addr != NULL && ifconfig_ioctl(skfd,ifname,SIOCGIFADDR, &ifr) >= 0){
				struct sockaddr_in *sin = (struct sockaddr_in*)&ifr.ifr_addr;
				*addr = sin->sin_addr.s_addr;
				ret = 0;
			}
			if (mask != NULL && ifconfig_ioctl(skfd,ifname,SIOCGIFNETMASK, &ifr) >= 0){
				struct sockaddr_in *sin = (struct sockaddr_in*)&ifr.ifr_addr;
				*mask = sin->sin_addr.s_addr;
				ret = 0;
			}
			if (bcast != NULL && ifconfig_ioctl(skfd,ifname,SIOCGIFBRDADDR, &ifr) >= 0){
				struct sockaddr_in *sin = (struct sockaddr_in*)&ifr.ifr_addr;
				*bcast = sin->sin_addr.s_addr;
				ret = 0;
			}
			close (skfd);
		}
	}
	return ret;
}

static int
convertAddress(const char *str, vc_net_nx_type *type, void *dst)
{
  int	ret;
  if (type) *type = vcNET_IPV4;
  ret = inet_pton(AF_INET, str, dst);
  if (ret==0) {
    if (type) *type = vcNET_IPV6;
    ret = inet_pton(AF_INET6, str, dst);
  }
  return ret > 0 ? 0 : -1;
}

static void
readIP(char const *str, struct vc_ips **ips)
{
  if (ifconfig_getaddr(str, &(*ips)->a.ip[0], &(*ips)->a.mask[0], NULL)==-1) {
    char		*pt;
    char		tmpopt[strlen(str)+1];
    uint32_t		*mask = (*ips)->a.mask;

    strcpy(tmpopt,str);
    pt = strchr(tmpopt,'/');
    if (pt)
      *pt++ = '\0';

    if (convertAddress(tmpopt, &(*ips)->a.type, (*ips)->a.ip) == -1) {
      WRITE_MSG(2, "Invalid IP number '");
      WRITE_STR(2, tmpopt);
      WRITE_MSG(2, "'\n");
      exit(wrapper_exit_code);
    }

    if (pt==0) {
      switch ((*ips)->a.type) {
	case vcNET_IPV4:
	  mask[0] = htonl(0xffffff00);
	  break;
	case vcNET_IPV6:
	  mask[0] = 64;
	  break;
	default: break;
      }
    }
    else {
      // Ok, we have a network size, not a netmask
      if (strchr(pt,'.')==0 && strchr(pt,':')==0) {
	int		sz = atoi(pt);
	switch ((*ips)->a.type) {
	  case vcNET_IPV4:
	    mask[0] = htonl((1 << sz) - 1);
	    break;
	  case vcNET_IPV6:
	    mask[0] = sz;
	    break;
	  default: break;
	}
      }
      else { 
	if (convertAddress(pt, NULL, &(*ips)->a.mask) == -1) {
	  WRITE_MSG(2, "Invalid netmask '");
	  WRITE_STR(2, pt);
	  WRITE_MSG(2, "'\n");
	  exit(wrapper_exit_code);
	}
      }
    }
  }
  else
    (*ips)->a.type = vcNET_IPV4;

  (*ips)->a.count = 1;
  (*ips)->next = calloc(1, sizeof(struct vc_ips));
  *ips = (*ips)->next;
}

static void
readBcast(char const *str, struct vc_ips **ips)
{
  uint32_t bcast;
  if (ifconfig_getaddr(str, NULL, NULL, &bcast)==-1){
    if (convertAddress(str, NULL, &bcast) == -1) {
      WRITE_MSG(2, "Invalid broadcast number '");
      WRITE_STR(2, optarg);
      WRITE_MSG(2, "'\n");
      exit(wrapper_exit_code);
    }
  }
  (*ips)->a.ip[0] = bcast;
  (*ips)->a.count = 1;
  (*ips)->a.type = vcNET_IPV4B;
  (*ips)->next = calloc(1, sizeof(struct vc_ips));
  *ips = (*ips)->next;
}

static void
tellAddress(struct vc_net_nx *addr, bool silent)
{
  char buf[41];
  if (silent)
    return;
  if (inet_ntop(addr->type == vcNET_IPV6 ? AF_INET6 : AF_INET,
		&addr->ip, buf, sizeof(buf)) == NULL) {
    WRITE_MSG(1, " <conversion failed>");
    return;
  }
  WRITE_MSG(1, " ");
  WRITE_STR(1, buf);
}

static inline void
doit(struct Arguments *args)
{
  struct vc_ips *ips;
  if (args->do_add) {
    if (!args->is_silent)
      WRITE_MSG(1, "Adding");
    for (ips = &args->head; ips->next; ips = ips->next) {
      tellAddress(&ips->a, args->is_silent);
      if (vc_net_add(args->nid, &ips->a) != (int)ips->a.count) {
	perror(ENSC_WRAPPERS_PREFIX "vc_net_add()");
	exit(wrapper_exit_code);
      }
    }
    if (!args->is_silent)
      WRITE_MSG(1, "\n");
  }
  else if (args->do_remove) {
    if (!args->is_silent)
      WRITE_MSG(1, "Removing");
    for (ips = &args->head; ips->next; ips = ips->next) {
      tellAddress(&ips->a, args->is_silent);
      if (vc_net_remove(args->nid, &ips->a) != (int)ips->a.count) {
	perror(ENSC_WRAPPERS_PREFIX "vc_net_remove()");
	exit(wrapper_exit_code);
      }
    }
    if (!args->is_silent)
      WRITE_MSG(1, "\n");
  }
}

int main (int argc, char *argv[])
{
  struct Arguments args = {
    .nid	= VC_NOCTX,
    .is_silent	= false,
    .do_add	= false,
    .do_remove	= false,
    .head	= { .next = NULL },
  };
  struct vc_ips *ips = &args.head;
  
  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP		:  showHelp(1, argv[0], 0);
      case CMD_VERSION		:  showVersion();
      case CMD_SILENT		:  args.is_silent = true; break;
      case CMD_BCAST		:  readBcast(optarg, &ips); break;
      case CMD_NID		:  args.nid = Evc_nidopt2nid(optarg,true); break;
      case CMD_ADD		:  args.do_add = true; break;
      case CMD_REMOVE		:  args.do_remove = true; break;
      case CMD_IP		:  readIP(optarg, &ips); break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	exit(wrapper_exit_code);
	break;
    }
  }

  if (args.nid == VC_NOCTX) args.nid = Evc_get_task_nid(0);

  if (!args.do_add && !args.do_remove) {
    WRITE_MSG(2, "No operation specified; try '--help' for more information\n");
    exit(wrapper_exit_code);
  }
  else if (args.do_add && args.do_remove) {
    WRITE_MSG(2, "Multiple operations specified; try '--help' for more information\n");
    exit(wrapper_exit_code);
  }

  doit(&args);

  if (optind != argc)
    Eexecvp (argv[optind],argv+optind);
  return EXIT_SUCCESS;
}

#ifdef ENSC_TESTSUITE
#include <assert.h>

void test()
{
  struct vc_ip_mask_pair	ip;
  uint32_t			bcast;

  bcast = 0;
  readIP("1.2.3.4", &ip, &bcast);
  assert(ip.ip==ntohl(0x01020304) && ip.mask==ntohl(0xffffff00) && bcast==0);

  readIP("1.2.3.4/8", &ip, &bcast);
  assert(ip.ip==ntohl(0x01020304) && ip.mask==ntohl(0xff000000) && bcast==0);

  readIP("1.2.3.4/255.255.0.0", &ip, &bcast);
  assert(ip.ip==ntohl(0x01020304) && ip.mask==ntohl(0xffff0000) && bcast==0);

  readIP("localhost", &ip, &bcast);
  assert(ip.ip==ntohl(0x7f000001) && ip.mask==ntohl(0xffffff00) && bcast==0);

#if 0
  if (ifconfig_getaddr("lo", &tmp, &tmp, &tmp)!=-1) {
    readIP("lo", &ip, &bcast);
    assert(ip.ip==ntohl(0x7f000001) && ip.mask==ntohl(0xff000000) && bcast==ntohl(0x7fffffff));
  }
#endif
}
#endif
