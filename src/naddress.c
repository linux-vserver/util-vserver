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
#define CMD_SET		0x2004
#define CMD_IP		0x2010
#define CMD_BCAST	0x2011
#define CMD_MASK	0x2012
#define CMD_RANGE	0x2013
#define CMD_LBACK	0x2014

int wrapper_exit_code = 255;


static struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
  { "silent",   no_argument,  0, CMD_SILENT },
  { "add",      no_argument,  0, CMD_ADD },
  { "remove",   no_argument,  0, CMD_REMOVE },
  { "set",      no_argument,  0, CMD_SET },
  { "nid",      required_argument, 0, CMD_NID },
  { "ip",       required_argument, 0, CMD_IP },
  { "mask",     required_argument, 0, CMD_MASK },
  { "range",    required_argument, 0, CMD_RANGE },
  { "bcast",    required_argument, 0, CMD_BCAST },
  { "lback",    required_argument, 0, CMD_LBACK },
  { 0,0,0,0 }
};

struct vc_ips {
  struct vc_net_addr a;
  struct vc_ips *next;
};

struct Arguments {
  nid_t		nid;
  struct vc_ips	head;
  bool		is_silent;
  bool		do_add;
  bool		do_remove;
  bool		do_set;
};

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " (--add|--remove|--set) [--silent] [--nid <nid>]\n"
	    "    [--ip <ip_num>[/<mask>]] [--bcast <broadcast>] [--] <commands> <args>*\n\n"
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
convertAddress(const char *str, uint16_t *type, void *dst)
{
  int	ret;
  if (type) *type = VC_NXA_TYPE_IPV4;
  ret = inet_pton(AF_INET, str, dst);
  if (ret==0) {
    if (type) *type = VC_NXA_TYPE_IPV6;
    ret = inet_pton(AF_INET6, str, dst);
  }
  return ret > 0 ? 0 : -1;
}

static void
ipv6PrefixToMask(struct in6_addr *mask, int prefix)
{
  int i;
  mask->s6_addr32[0] = mask->s6_addr32[1] = mask->s6_addr32[2] = mask->s6_addr32[3] = 0;
  for (i = 0; (i << 3) < prefix; i++) {
    mask->s6_addr[i] = 0xff;
  }
  if ((i << 3) > prefix)
    mask->s6_addr[i-1] = ~((1 << (prefix & 0x07)) - 1);
}

static int
maskToPrefix(void *data, int limit)
{
  uint8_t *mask = data;
  int prefix;
  for (prefix = 0; prefix < limit && mask[prefix >> 3] & (1 << (prefix & 0x07)); prefix++)
    ;
  return prefix;
}

static int
parseIPFormat(char const *str_c, struct vc_ips **ips,
	     char const *format)
{
  size_t len = strlen(str_c);
  char const *fc;
  char str[len + 1], *ptr = str;
  int ret = 0;

  strcpy(str, str_c);

  /* XXX: condition at the bottom */
  for (fc = format; ; fc += 2) {
    char *sep;
    void *dst;
    unsigned long limit = 0;

    switch (*fc) {
      case '1': dst = &(*ips)->a.s.ip;		break;
      case '2': dst = &(*ips)->a.s.ip2;		break;
      case 'm':	dst = &(*ips)->a.s.mask;	break;
      default:					goto out;
    }

    if (len == 0)
      goto out;
    if ((sep = memchr(ptr, *(fc + 1), len)) == NULL)
      sep = ptr + len;
    *sep = '\0';

    /* This is ugly, and means that m cannot be first */
    switch ((*ips)->a.vna_type) {
      case VC_NXA_TYPE_IPV4: limit =  32;	break;
      case VC_NXA_TYPE_IPV6: limit = 128;	break;
    }

    /* This is required due to the dain-bramage that is inet_pton in dietlibc.
     * Essentially any number will be parsed as a valid IPv4 address...
     */
    if (*fc == 'm' && strchr(ptr, ':') == NULL && strchr(ptr, '.') == NULL) {
      /* This is a prefix, not a netmask */
      unsigned long	sz;

      if (!isNumberUnsigned(ptr, &sz, true) || sz > limit) {
	ret = -1;
	goto out;
      }

      (*ips)->a.vna_prefix = sz;
      switch ((*ips)->a.vna_type) {
	case VC_NXA_TYPE_IPV4:
	  (*ips)->a.vna_v4_mask.s_addr = htonl(~((1 << (32 - sz)) - 1));
	  break;
	case VC_NXA_TYPE_IPV6:
	  ipv6PrefixToMask(&(*ips)->a.vna_v6_mask, sz);
	  break;
      }
    }
    else {
      if (convertAddress(ptr, &(*ips)->a.vna_type, dst) == -1) {
	ret = -1;
	goto out;
      }
      else if (*fc == 'm') {
	/* Got a mask, set the prefix */
	(*ips)->a.vna_prefix = maskToPrefix(&(*ips)->a.s.mask, limit);
      }
    }

    ret++;
    len -= (sep - ptr);
    ptr = sep + 1;

    if (*(fc + 1) == '\0')
      break;
  }

out:
  return ret;
}

static void
readIP(char const *str, struct vc_ips **ips, uint16_t type)
{
  if (ifconfig_getaddr(str, &(*ips)->a.vna_v4_ip.s_addr, &(*ips)->a.vna_v4_mask.s_addr, NULL)==-1) {
    if (parseIPFormat(str, ips, "1/m") < 1) {
      WRITE_MSG(2, "Invalid IP number '");
      WRITE_STR(2, str);
      WRITE_MSG(2, "'\n");
      exit(wrapper_exit_code);
    }
  }
  else
    (*ips)->a.vna_type = VC_NXA_TYPE_IPV4;
  (*ips)->a.vna_type |= type;

  (*ips)->next = calloc(1, sizeof(struct vc_ips));
  *ips = (*ips)->next;
}

static void
readBcast(char const *str, struct vc_ips **ips)
{
  uint32_t bcast;
  if (ifconfig_getaddr(str, NULL, NULL, &bcast)==-1){
    if (inet_pton(AF_INET, str, &bcast) < 0) {
      WRITE_MSG(2, "Invalid broadcast number '");
      WRITE_STR(2, optarg);
      WRITE_MSG(2, "'\n");
      exit(wrapper_exit_code);
    }
  }
  (*ips)->a.vna_v4_ip.s_addr = bcast;
  (*ips)->a.vna_type = VC_NXA_TYPE_IPV4 | VC_NXA_MOD_BCAST | VC_NXA_TYPE_ADDR;
  (*ips)->next = calloc(1, sizeof(struct vc_ips));
  *ips = (*ips)->next;
}

static void
readRange(char const *str, struct vc_ips **ips)
{
  if (parseIPFormat(str, ips, "1-2/m") < 2) {
    WRITE_MSG(2, "Invalid range '");
    WRITE_STR(2, str);
    WRITE_MSG(2, "'\n");
    exit(wrapper_exit_code);
  }
  (*ips)->a.vna_type |= VC_NXA_TYPE_RANGE;
  (*ips)->next = calloc(1, sizeof(struct vc_ips));
  *ips = (*ips)->next;
}

static void
tellAddress(struct vc_net_addr *addr, bool silent)
{
  char buf[41];
  int af;
  void *address;
  if (silent)
    return;
  switch (addr->vna_type & (VC_NXA_TYPE_IPV4 | VC_NXA_TYPE_IPV6)) {
    case VC_NXA_TYPE_IPV4:
      af = AF_INET;
      address = &addr->vna_v4_ip.s_addr;
      break;
    case VC_NXA_TYPE_IPV6:
      af = AF_INET6;
      address = addr->vna_v6_ip.s6_addr32;
      break;
    default:
      WRITE_MSG(1, " <unknown address type>");
      return;
  }
  if (inet_ntop(af, address, buf, sizeof(buf)) == NULL) {
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

  if (args->do_set) {
    struct vc_net_addr remove = { .vna_type = VC_NXA_TYPE_ANY };
    if (vc_net_remove(args->nid, &remove) == -1) {
      perror(ENSC_WRAPPERS_PREFIX "vc_net_remove()");
      exit(wrapper_exit_code);
    }
  }

  if (args->do_add || args->do_set) {
    if (!args->is_silent)
      WRITE_MSG(1, "Adding");
    for (ips = &args->head; ips->next; ips = ips->next) {
      tellAddress(&ips->a, args->is_silent);
      if (vc_net_add(args->nid, &ips->a) == -1) {
	if (!args->is_silent)
	  WRITE_MSG(1, "\n");
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
      if (vc_net_remove(args->nid, &ips->a) == -1) {
	if (!args->is_silent)
	  WRITE_MSG(1, "\n");
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
    .do_set	= false,
    .head	= { .next = NULL },
  };
  struct vc_ips *ips = &args.head;
  
  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP	:  showHelp(1, argv[0], 0);
      case CMD_VERSION	:  showVersion();
      case CMD_SILENT	:  args.is_silent = true; break;
      case CMD_NID	:  args.nid       = Evc_nidopt2nid(optarg,true); break;
      case CMD_ADD	:  args.do_add    = true; break;
      case CMD_REMOVE	:  args.do_remove = true; break;
      case CMD_SET	:  args.do_set    = true; break;
      case CMD_IP	:  readIP(optarg, &ips, VC_NXA_TYPE_ADDR);  break;
      case CMD_MASK	:  readIP(optarg, &ips, VC_NXA_TYPE_MASK);  break;
      case CMD_RANGE	:  readRange(optarg, &ips); break;
      case CMD_BCAST	:  readBcast(optarg, &ips); break;
      case CMD_LBACK	:  readIP(optarg, &ips, VC_NXA_TYPE_ADDR | VC_NXA_MOD_LBACK); break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help' for more information.\n");
	exit(wrapper_exit_code);
	break;
    }
  }

  if (args.nid == VC_NOCTX) args.nid = Evc_get_task_nid(0);

  if (!args.do_add && !args.do_remove && !args.do_set) {
    WRITE_MSG(2, "No operation specified; try '--help' for more information\n");
    exit(wrapper_exit_code);
  }
  else if (((args.do_add ? 1 : 0) + (args.do_remove ? 1 : 0) + (args.do_set ? 1 : 0)) > 1) {
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
