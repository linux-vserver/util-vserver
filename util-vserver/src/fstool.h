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


#ifndef H_UTIL_VSERVER_SRC_FSTOOL_H
#define H_UTIL_VSERVER_SRC_FSTOOL_H

#include <getopt.h>
#include <stdbool.h>

#define CMD_HELP	0x8000
#define CMD_VERSION	0x8001
#define CMD_IMMUTABLE	0x8002
#define CMD_IMMULINK	0x8003
#define CMD_LEGACY	0x8004

struct stat;

struct Arguments {
    bool		do_recurse;
    bool		do_display_dot;
    bool		do_display_dir;
    bool		do_mapping;
    bool		immutable;
    bool		immulink;
    char const *	ctx_str;
    xid_t		ctx;
    bool		is_legacy;
    bool		do_set;
    bool		do_unset;
    bool		local_fs;
};

extern struct option const		CMDLINE_OPTIONS[];
extern char const			CMDLINE_OPTIONS_SHORT[];
extern struct Arguments const *		global_args;

bool	checkForRace(int fd, char const * name, struct stat const *exp_st);
void	fixupParams(struct Arguments *, int argc);
bool	handleFile(char const *d_name, char const *full_name, struct stat const *);
void	showHelp(int fd, char const *cmd, int res);
void	showVersion();
xid_t	resolveCtx(char const *str);

#endif	//  H_UTIL_VSERVER_SRC_FSTOOL_H
