// $Id$

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#ifndef H_VSERVER_SYSCALL_H
#define H_VSERVER_SYSCALL_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

  struct vc_ip_mask_pair {
    uint32_t	ip;
    uint32_t	mask;
  };

  int	vc_new_s_context(ctx_t ctx, unsigned int remove_cap, unsigned int flags);
  int	vc_set_ipv4root(uint32_t  bcast, size_t nb, struct vc_ip_mask_pair const *ips);
  int	vc_chrootsafe(char const *dir);

#ifdef __cplusplus
}
#endif

#endif
