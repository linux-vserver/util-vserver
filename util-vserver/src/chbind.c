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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <errno.h>

#include "vserver.h"

static void usage()
{
	fprintf (stderr,"chbind version %s\n",VERSION);
	fprintf (stderr,"chbind [ --silent ] [ --ip ip_num[/mask] ] [ --bcast broadcast ] command argument\n");
	exit (-1);
}

/*
	Check if a network device exist in /proc/net/dev.
	This is used because ifconfig_ioctl triggers modprobe if requesting
	information about non existant devices.

	Return != 0 if the device exist.
*/
static int chbind_devexist (const char *dev)
{
	int ret = 0;
	FILE *fin = fopen ("/proc/net/dev","r");
	if (fin != NULL){
		int len = strlen(dev);
		char buf[1000];
		fgets(buf,sizeof(buf)-1,fin);	// Skip one line
		while (fgets(buf,sizeof(buf)-1,fin)!=NULL){
			const char *pt = strstr(buf,dev);
			if (pt != NULL && pt[len] == ':'){
				ret = 1;
				break;
			}
		}
		fclose (fin);
	}
	return ret;
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
	unsigned long *addr,
	unsigned long *mask,
	unsigned long *bcast)
{
	int ret = -1;
	if (chbind_devexist(ifname)){
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




int main (int argc, char *argv[])
{
	int ret = -1;
	int silent = 0;
	int i;
	unsigned long addrs[16],masks[16];
	int nbaddrs = 0;
	unsigned long bcast = 0xffffffff;
	for (i=1; i<argc; i++){
		const char *arg = argv[i];
		const char *opt = argv[i+1];
		if (strcmp(arg,"--ip")==0){
			unsigned long addr,mask;
			if (nbaddrs == 16){
				fprintf (stderr,"Too many IP numbers, max 16, ignored\n");

			}else if (ifconfig_getaddr(opt,&addr,&mask,&bcast)==-1){
				unsigned long mask = 0x00ffffff;
				const char *pt = strchr(opt,'/');
				char tmpopt[strlen(opt)+1];
				struct hostent *h;
				
				if (pt != NULL){
					strcpy (tmpopt,opt);
					tmpopt[pt-opt] = '\0';
					opt = tmpopt;
					pt++;
					if (strchr(pt,'.')==NULL){
						// Ok, we have a network size, not a netmask
						int size = atoi(pt);
						int i;
						mask = 0;
						for (i=0; i<size; i++){
							mask = mask >> 1;
							mask |= 0x80000000;
						}
						mask = ntohl(mask);
					}else{
						struct hostent *h = gethostbyname (pt);
						if (h != NULL){
							memcpy (&mask,h->h_addr,sizeof(mask));
						}else{
							fprintf (stderr,"Invalid netmask: %s\n",pt);
							usage();
						}
					}
							
				}

				h = gethostbyname (opt);
				if (h == NULL){
					fprintf (stderr,"Invalid IP number or host name: %s\n",opt);
					usage();
				}else{
					memcpy (&addr,h->h_addr,sizeof(addr));
					masks[nbaddrs] = mask;
					addrs[nbaddrs++] = addr;
				}
			}else{
				masks[nbaddrs] = mask;
				addrs[nbaddrs++] = addr;
			}
			i++;
		}else if (strcmp(arg,"--bcast")==0){
			unsigned long tmp;
			if (ifconfig_getaddr(opt,&tmp,&tmp,&bcast)==-1){
				struct hostent *h = gethostbyname (opt);
				if (h == NULL){
					fprintf (stderr,"Invalid broadcast number: %s\n",opt);
					usage();
				}else{
					memcpy (&bcast,h->h_addr,sizeof(bcast));
				}
			}
			i++;
		}else if (strcmp(arg,"--silent")==0){
			silent = 1;
		}else{
			break;
		}
	}
	if (i == argc){
		usage();
	}else if (argv[i][0] == '-'){
		usage();
	}else{
		if (call_set_ipv4root(addrs,nbaddrs,bcast,masks)==0){
			if (!silent){
			        int i;
				printf ("ipv4root is now");
				for (i=0; i<nbaddrs; i++){
					unsigned long hostaddr = ntohl(addrs[i]);
					printf (" %ld.%ld.%ld.%ld"
						,hostaddr>>24
						,(hostaddr>>16)&0xff
						,(hostaddr>>8)&0xff
						,hostaddr &0xff);
				}
				printf ("\n");
			}
			execvp (argv[i],argv+i);
			fprintf (stderr,"Can't exec %s (%s)\n",argv[i],strerror(errno));
		}else{
			fprintf (stderr,"Can't set the ipv4 root (%s)\n",strerror(errno));
		}
	}
	return ret;
}


