// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on ifspec.cc by Jacques Gelinas
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
	Prints the specs of a network device in shell like form

	ADDR=
	NETMASK=
	BCAST=
*/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

static void usage()
{
	fprintf (stderr,"ifspec version %s\n",VERSION);
	fprintf (stderr
		,"ifspec network-device [ ipaddr netmask broadcast ]\n"
		 "prints device specification in a shell usable way\n");
	exit (-1);
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

static unsigned long ip_cnv (const char *str)
{
	const char *start_str = str;
	unsigned tb[4];
	int no = 0;
	unsigned long ret;
	  
	memset (tb,-1,sizeof(tb));
	while (*str != '\0' && no < 4){
		if (isdigit(*str)){
			int val = atoi(str);
			if (val > 255) break;
			tb[no++] = val;
			while (isdigit(*str)) str++;
			if (*str == '.'){
				str++;
			}else{
				break;
			}
		}else{
			break;
		}
	}

	ret = (tb[0] << 24) | (tb[1]<<16) | (tb[2] << 8) | tb[3];
	if (no != 4 || *str != '\0'){
		fprintf (stderr,"Invalid IP number or netmask: %s\n",start_str);
		ret = 0xffffffff;
	}
	return ret;
}


/*
	Fetch the IP number of an interface from the kernel.
	Assume the device is already available in the kernel
	Return -1 if any error.
*/
int ifconfig_print (
	const char *ifname,
	const char *addrstr,
	const char *maskstr,
	const char *bcaststr)
{
	int ret = -1;
	int skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd != -1){
		struct ifreq ifr;
		struct {
			unsigned long addr;
			unsigned long mask;
		} solved;
		if (addrstr != NULL && addrstr[0] != '\0'){
			printf ("ADDR=%s\n",addrstr);
			solved.addr = ip_cnv (addrstr);
		}else if (ifconfig_ioctl(skfd,ifname,SIOCGIFADDR, &ifr) >= 0){
			struct sockaddr_in *sin = (struct sockaddr_in*)&ifr.ifr_addr;
			unsigned long addr = ntohl(sin->sin_addr.s_addr);
			printf ("ADDR=%lu.%lu.%lu.%lu\n"
				,(addr>>24)&0xff
				,(addr>>16)&0xff
				,(addr>>8)&0xff
				,addr&0xff);
			solved.addr = addr;
			ret = 0;
		}
		if (maskstr != NULL && maskstr[0] != '\0'){
			printf ("NETMASK=%s\n",maskstr);
			solved.mask = ip_cnv (maskstr);
		}else		if (ifconfig_ioctl(skfd,ifname,SIOCGIFNETMASK, &ifr) >= 0){
			struct sockaddr_in *sin = (struct sockaddr_in*)&ifr.ifr_addr;
			unsigned long addr = ntohl(sin->sin_addr.s_addr);
			printf ("NETMASK=%lu.%lu.%lu.%lu\n"
				,(addr>>24)&0xff
				,(addr>>16)&0xff
				,(addr>>8)&0xff
				,addr&0xff);
			solved.mask = addr;
			ret = 0;
		}
		if (bcaststr != NULL && bcaststr[0] != '\0'){
			printf ("BCAST=%s\n",bcaststr);
		}else if (ifconfig_ioctl(skfd,ifname,SIOCGIFBRDADDR, &ifr) >= 0){
			struct sockaddr_in *sin = (struct sockaddr_in*)&ifr.ifr_addr;
			unsigned long addr = ntohl(sin->sin_addr.s_addr);
			printf ("BCAST=%lu.%lu.%lu.%lu\n"
				,(addr>>24)&0xff
				,(addr>>16)&0xff
				,(addr>>8)&0xff
				,addr&0xff);
			ret = 0;
		}else{
			// Can't get it from the kernel, compute it from the IP
			// and the netmask
			unsigned long addr = (solved.addr & solved.mask)
				| ~solved.mask;
			printf ("BCAST=%lu.%lu.%lu.%lu\n"
				,(addr>>24)&0xff
				,(addr>>16)&0xff
				,(addr>>8)&0xff
				,addr&0xff);
			
		}
		close (skfd);
	}
	return ret;
}


int main (int argc, char *argv[])
{
	int ret = -1;
	if (argc < 2){
		usage();
	}else{
		const char *addrstr = argc >= 3 ? argv[2] : NULL;
		const char *maskstr = argc >= 4 ? argv[3] : NULL;
		const char *bcaststr = argc >= 5 ? argv[4] : NULL;
		ret = ifconfig_print (argv[1],addrstr,maskstr,bcaststr);
	}
	return ret;
}



