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

#define CMD_HELP		0x8000
#define CMD_VERSION		0x8001
#define CMD_IMMUTABLE		0x8002
#define CMD_IMMULINK		0x8003
#define CMD_LEGACY		0x8004
#define CMD_IMMU		0x8010
#define CMD_ADMIN		0x8011
#define CMD_WATCH		0x8012
#define CMD_HIDE		0x8013
#define CMD_BARRIER		0x8014
#define CMD_IMMUX		0x8015
#define CMD_WRITE		0x8016
#define CMD_UNSET_IMMU		0x8020
#define CMD_UNSET_ADMIN		0x8021
#define CMD_UNSET_WATCH		0x8022
#define CMD_UNSET_HIDE		0x8023
#define CMD_UNSET_BARRIER	0x8024
#define CMD_UNSET_IMMUX		0x8025
#define CMD_UNSET_IMMUTABLE	0x8026
#define CMD_UNSET_WRITE		0x8027


struct stat;

struct Arguments {
    bool		do_recurse;
    bool		do_display_dot;
    bool		do_display_dir;
    bool		do_mapping;
    char const *	ctx_str;
    xid_t		ctx;
    bool		is_legacy;
    bool		do_set;
    bool		do_unset;
    bool		local_fs;
    bool		no_unified;

    uint32_t		set_mask;
    uint32_t		del_mask;
};

extern struct option const		CMDLINE_OPTIONS[];
extern char const			CMDLINE_OPTIONS_SHORT[];
extern struct Arguments const *		global_args;

void	fixupParams(struct Arguments *, int argc);
bool	handleFile(char const *d_name, char const *full_name);
void	showHelp(int fd, char const *cmd, int res);
void	showVersion();

#endif	//  H_UTIL_VSERVER_SRC_FSTOOL_H
