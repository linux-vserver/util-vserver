// $Id$		--*- c++ -*--

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
// based on vutil.p by Jacques Gelinas
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


/* vutil.cc 11/04/2003 14.22.04 */
int file_copy (const char *src, const char *dst, struct stat&st);
int setext2flag (const char *fname, bool set, int ext2flags);
int vbuild_mkdir (const char *path, mode_t mode);
int vbuild_mknod (const char *path, mode_t mode, dev_t dev);
int vbuild_symlink (const char *src, const char *dst);
int vbuild_link (const char *src, const char *dst);
int vbuild_unlink (const char *path);
int vbuild_chown (const char *path, uid_t uid, gid_t gid);
int vbuild_file_copy (const char *src,
	 const char *dst,
	 struct stat&st);
void vutil_loadallpkg (Vserver const &refserver, list<Package>&packages);
int vutil_lstat (string path, struct stat&st);
FILE *vutil_execdistcmd (const char *key,
	 const string&vserver,
	 const char *args);
/* syscall.cc 18/07/2003 09.50.10 */
extern "C" int call_new_s_context (int nbctx,
	 int ctxs[],
	 int remove_cap,
	 int flags);
extern "C" int call_set_ipv4root (unsigned long ip[],
	 int nb,
	 unsigned long bcast,
	 unsigned long mask[]);
extern "C" int call_chrootsafe (const char *dir);
extern "C" int has_chrootsafe (void);
extern "C" int call_set_ctxlimit (int res, long limit);
