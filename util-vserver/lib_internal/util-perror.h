// $Id$    --*- c -*--

// Copyright (C) 2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#ifndef H_UTIL_VSERVER_LIB_INTERNAL_UTIL_PERROR_H
#define H_UTIL_VSERVER_LIB_INTERNAL_UTIL_PERROR_H

#define PERROR_U(MSG, ARG0) {				\
    size_t	pu_l1 = strlen(MSG);			\
    size_t	pu_l2 = strlen(ARG0);			\
    char	pu_buf[pu_l1 + pu_l2 + sizeof("()")];	\
    memcpy(pu_buf, MSG, pu_l1);				\
    pu_buf[pu_l1] = '(';				\
    memcpy(pu_buf+1+pu_l1, ARG0, pu_l2);		\
    pu_buf[pu_l1+1+pu_l2] = ')';			\
    pu_buf[pu_l1+2+pu_l2] = '\0';			\
    perror(pu_buf);					\
  }

#define PERROR_Q(MSG, ARG0) {			\
    size_t	pq_l = strlen(ARG0);		\
    char	pq_buf[pq_l + 3];		\
    pq_buf[0]    = '"';				\
    memcpy(pq_buf+1, ARG0, pq_l);		\
    pq_buf[pq_l+1] = '"';			\
    pq_buf[pq_l+2] = '\0';			\
    PERROR_U(MSG, pq_buf);			\
  }

#endif	//  H_UTIL_VSERVER_LIB_INTERNAL_UTIL_PERROR_H
