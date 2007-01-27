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


#ifndef H_UTIL_VSERVER_SRC_VHASHIFY_H
#define H_UTIL_VSERVER_SRC_VHASHIFY_H

#include "ensc_vector/list.h"
#include "lib_internal/pathinfo.h"

#include <sys/types.h>
#include <stdbool.h>

struct Arguments {
    enum {mdMANUALLY, mdVSERVER}	mode; 
    unsigned int			verbosity;
    unsigned int			insecure;
    char const *			hash_dir;
    bool				dry_run;
    bool				do_refresh;
    bool				ignore_mtime;
};

struct HashDirInfo {
    PathInfo const		path;
    dev_t			device;
};

struct SkipReason {
    enum { rsDOTFILE, rsEXCL, rsTOOSMALL, rsUNSUPPORTED,
	   rsFSTAT,   rsSYMLINK, rsUNIFIED, rsWRONGDEV,
	   rsSPECIAL, rsGENERAL }	r;
};

typedef struct Vector		HashDirCollection;

int			HashDirInfo_compareDevice(void const *lhs, void const *rhs);
PathInfo const *	HashDirInfo_findDevice(HashDirCollection const *, dev_t dev);

#endif	//  H_UTIL_VSERVER_SRC_VHASHIFY_H
