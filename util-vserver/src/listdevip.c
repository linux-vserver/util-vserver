// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on listdevip.cc by Jacques Gelinas
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
	Print the list of all network (IP) devices. Print the IP
	in fact, including all aliases.
*/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>


static int ifconfig_ioctl(
	int fd,
	const char *ifname,
	int cmd,
	struct ifreq *ifr)
{
	strcpy(ifr->ifr_name, ifname);
	return ioctl(fd, cmd,ifr);
}


static int devlist_read2_2()
{
	int ret = -1;
	int skfd = socket (AF_INET,SOCK_DGRAM,0);
	if (skfd < 0) {
		perror ("socket");
	}else{
		struct ifconf ifc;
		int numreqs = 30;
		ifc.ifc_buf = NULL;
		ret = 0;
		while (1) {
			ifc.ifc_len = sizeof(struct ifreq) * numreqs;
			ifc.ifc_buf = (char*)realloc(ifc.ifc_buf, ifc.ifc_len);

			if (ioctl(skfd, SIOCGIFCONF, &ifc) < 0) {
				perror("SIOCGIFCONF");
				ret = -1;
				break;
			}
			if (ifc.ifc_len == (int)sizeof(struct ifreq) * numreqs) {
				/* assume it overflowed and try again */
				numreqs += 10;
				continue;
			}
			break;
		}
		if (ret == 0){
			struct ifreq *ifr = ifc.ifc_req;
			int		n;
			for (n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq)) {
				struct sockaddr_in *sin = (struct sockaddr_in*)&ifr->ifr_addr;
				unsigned long addr = ntohl(sin->sin_addr.s_addr);
				unsigned long mask = 0xffffff00;
				struct ifreq ifmask;
				if (ifconfig_ioctl(skfd,ifr->ifr_name,SIOCGIFNETMASK, &ifmask) >= 0){
					struct sockaddr_in *sin = (struct sockaddr_in*)&ifmask.ifr_addr;
					mask = ntohl(sin->sin_addr.s_addr);
				}

				printf ("%lu.%lu.%lu.%lu/%lu.%lu.%lu.%lu\n"
					,(addr>>24)&0xff
					,(addr>>16)&0xff
					,(addr>>8)&0xff
					,addr&0xff
					,(mask>>24)&0xff
					,(mask>>16)&0xff
					,(mask>>8)&0xff
					,mask&0xff);
				ifr++;
			}
		}
		free(ifc.ifc_buf);
	}
	return ret;
}

int main (int UNUSED argc, char UNUSED *argv[])
{
	devlist_read2_2();
	return 0;
}

