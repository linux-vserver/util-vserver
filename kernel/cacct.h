#ifndef _VX_CACCT_H
#define _VX_CACCT_H


enum sock_acc_field {
	VXA_SOCK_UNSPEC = 0,
	VXA_SOCK_UNIX,
	VXA_SOCK_INET,
	VXA_SOCK_INET6,
	VXA_SOCK_PACKET,
	VXA_SOCK_OTHER,
	VXA_SOCK_SIZE	/* array size */
};

#endif	/* _VX_CACCT_H */
