// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#define ENSC_WRAPPERS_IO	1
#define ENSC_WRAPPERS_UNISTD	1
#include "wrappers.h"

#define CMD_HELP	0x1000
#define CMD_VERSION	0x1001

#define CMD_SILENT	0x2000
#define CMD_IP		0x2001
#define CMD_BCAST	0x2002

int wrapper_exit_code = 255;


static struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, CMD_HELP },
  { "version",  no_argument,  0, CMD_VERSION },
  { "silent",   no_argument,  0, CMD_SILENT },
  { "ip",       required_argument, 0, CMD_IP },
  { "bcast",    required_argument, 0, CMD_BCAST },
  { 0,0,0,0 }
};

static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:\n  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " [--silent] [--ip <ip_num>[/<mask>]] [--bcast <broadcast>] [--] <commands> <args>*\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");

  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "chbind " VERSION " -- bind to an ip and execute a program\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2003,2004 Enrico Scholz\n"
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
	return ioctl(fd, cmd,ifr);
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
		*addr = 0;
		*bcast = 0xffffffff;
		if (skfd != -1){
			struct ifreq ifr;
			if (ifconfig_ioctl(skfd,ifname,SIOCGIFADDR, &ifr) >= 0){
				struct sockaddr_in *sin = (struct sockaddr_in*)&ifr.ifr_addr;
				*addr = sin->sin_addr.s_addr;
				ret = 0;
			}
			if (ifconfig_ioctl(skfd,ifname,SIOCGIFNETMASK, &ifr) >= 0){
				struct sockaddr_in *sin = (struct sockaddr_in*)&ifr.ifr_addr;
				*mask = sin->sin_addr.s_addr;
				ret = 0;
			}
			if (ifconfig_ioctl(skfd,ifname,SIOCGIFBRDADDR, &ifr) >= 0){
				struct sockaddr_in *sin = (struct sockaddr_in*)&ifr.ifr_addr;
				*bcast = sin->sin_addr.s_addr;
				ret = 0;
			}
			close (skfd);
		}
	}
	return ret;
}

static void
readIP(char const *str, struct vc_ip_mask_pair *ip, uint32_t *bcast)
{
  if (ifconfig_getaddr(str, &ip->ip, &ip->mask, bcast)==-1) {
    char		*pt;
    char		tmpopt[strlen(str)+1];
    struct hostent	*h;

    strcpy(tmpopt,str);
    pt = strchr(tmpopt,'/');
    
    if (pt==0)
      ip->mask = ntohl(0xffffff00);
    else {
      *pt++ = '\0';

      // Ok, we have a network size, not a netmask
      if (strchr(pt,'.')==0) {
	int		sz = atoi(pt);
	;
	for (ip->mask = 0; sz>0; --sz) {
	  ip->mask >>= 1;
	  ip->mask  |= 0x80000000;
	}
	ip->mask = ntohl(ip->mask);
      }
      else { 
	struct hostent *h = gethostbyname (pt);
	if (h==0) {
	  WRITE_MSG(2, "Invalid netmask '");
	  WRITE_STR(2, pt);
	  WRITE_MSG(2, "'\n");
	  exit(wrapper_exit_code);
	}

	memcpy (&ip->mask, h->h_addr, sizeof(ip->mask));
      }
    }

    h = gethostbyname (tmpopt);
    if (h==0) {
      WRITE_MSG(2, "Invalid IP number or host name '");
      WRITE_STR(2, tmpopt);
      WRITE_MSG(2, "'\n");
      exit(wrapper_exit_code);
    }

    memcpy (&ip->ip, h->h_addr,sizeof(ip->ip));
  }
}

static void
readBcast(char const *str, uint32_t *bcast)
{
  uint32_t	tmp;
  if (ifconfig_getaddr(str, &tmp, &tmp, bcast)==-1){
    struct hostent *h = gethostbyname (str);
    if (h==0){
      WRITE_MSG(2, "Invalid broadcast number '");
      WRITE_STR(2, optarg);
      WRITE_MSG(2, "'\n");
      exit(wrapper_exit_code);
    }
    memcpy (bcast, h->h_addr,sizeof(*bcast));
  }
}

int main (int argc, char *argv[])
{
  bool				is_silent = false;
  struct vc_ip_mask_pair	ips[16];
  size_t			nbaddrs = 0;
  uint32_t			bcast = 0xffffffff;
  
  while (1) {
    int		c = getopt_long(argc, argv, "+", CMDLINE_OPTIONS, 0);
    if (c==-1) break;

    switch (c) {
      case CMD_HELP		:  showHelp(1, argv[0], 0);
      case CMD_VERSION		:  showVersion();
      case CMD_SILENT		:  is_silent = true; break;
      case CMD_BCAST		:  readBcast(optarg, &bcast); break;
      case CMD_IP		:
	if (nbaddrs>=16) {
	  WRITE_MSG(2, "Too many IP numbers, max 16\n");
	  exit(wrapper_exit_code);
	}
	readIP(optarg, ips+nbaddrs, &bcast);
	++nbaddrs;
	break;
      default		:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	exit(wrapper_exit_code);
	break;
    }
  }

  if (optind==argc) {
    WRITE_MSG(2, "No command given; try '--help' for more information\n");
    exit(wrapper_exit_code);
  }
  

  if (vc_set_ipv4root(bcast,nbaddrs,ips)!=0) {
    perror("vc_set_ipv4root()");
    exit(wrapper_exit_code);
  }

  if (!is_silent) {
    size_t		i;
    
    WRITE_MSG(1, "ipv4root is now");
    for (i=0; i<nbaddrs; ++i) {
      WRITE_MSG(1, " ");
      WRITE_STR(1, inet_ntoa(*reinterpret_cast(struct in_addr *)(&ips[i].ip)));
    }
    WRITE_MSG(1, "\n");
  }

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
  
  readIP("lo", &ip, &bcast);
  assert(ip.ip==ntohl(0x7f000001) && ip.mask==ntohl(0xff000000) && bcast==ntohl(0x7fffffff));
}
#endif
