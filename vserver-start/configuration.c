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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "configuration.h"
#include "interface.h"

#include <lib_internal/util.h>
#include <lib_internal/filecfg.h>
#include <ensc_vector/vector.h>
#include <lib/internal.h>

#include <dirent.h>
#include <string.h>

static inline bool
getSingleInterface(struct Interface *res,
		   struct Interface const *tmpl,
		   PathInfo const *basedir, char const *d_entry)
{
  PathInfo	ent  = { .d = d_entry, .l = strlen(d_entry) };
  PathInfo	path = *basedir;
  char		path_buf[ENSC_PI_APPSZ(path, ent)];

  PathInfo_append(&path, &ent, path_buf);
  if (!utilvserver_isDirectory(path.d, true))
    return true;	// skip non-directories

  return Iface_read(res, &path, tmpl);
}

static inline bool
getInterfaces(struct Configuration *cfg)
{
  ENSC_PI_DECLARE(iface_subdir, "interfaces");
  PathInfo		ifacepath = cfg->cfgdir;
  char			path_buf[ENSC_PI_APPSZ(ifacepath, iface_subdir)];
  struct Interface	iface_default;
  DIR			*dir;
  bool			rc = true;

  PathInfo_append(&ifacepath, &iface_subdir, path_buf);

  if (!utilvserver_isDirectory(ifacepath.d, true))
    return true;	// no interface configuration -> ok
  
  Iface_init(&iface_default);
  if (!Iface_read(&iface_default, &ifacepath, 0))
    return false;

    // iterate through dir-entries...
  dir = opendir(ifacepath.d);
  while (dir!=0) {
    struct dirent	*ent = readdir(dir);
    struct Interface	iface;
    
    if (ent==0)                 break;
    if (isDotfile(ent->d_name)) continue;	// skip dot-files

    Iface_init(&iface);
    if (!getSingleInterface(&iface, &iface_default, &ifacepath, ent->d_name))
      rc = false;
    else if (iface.addr.ipv4.ip!=0) {	// HACK: non-directory entries would return true also
      struct Interface	*new_iface = Vector_pushback(&cfg->interfaces);
      *new_iface = iface;
    }
  }

  if (dir!=0)
    closedir(dir);

  return rc;
}

static bool
initVdir(char const **vdir, PathInfo const *cfgdir)
{
  *vdir = vc_getVserverVdir(cfgdir->d, vcCFG_RECENT_FULL, true);
  if (*vdir==0) {
    WRITE_MSG(2, "Can not find root-directory of the vserver\n");
    return false;
  }

  return true;
}

static bool
setFlag(void *flags_v, char const *str, size_t len)
{
  struct vc_ctx_flags	*flags = flags_v;
  int			rc = vc_list2cflag(str,len, 0,flags);

  return rc!=-1;
}

static bool
setCCap(void *caps_v, char const *str, size_t len)
{
  struct vc_ctx_caps	*caps = caps_v;
  int			rc = vc_list2ccap(str,len, 0,caps);

  return rc!=-1;
}

static bool
setBCap(void *caps_v, char const *str, size_t len)
{
  struct vc_ctx_caps	*caps = caps_v;
  int			rc = vc_list2bcap(str,len, 0,caps);

  return rc!=-1;
}

static bool
readSomething(void *dest, PathInfo const *cfgdir, char const *item,
	      FileCfg_MultiLineHandler handler)
{
  char const	*data = FileCfg_readEntryStr(cfgdir, item, true, 0);
  bool		res   = false;

  if (!data) return true;
  if (!FileCfg_iterateOverMultiLine(data, handler, dest)) {
    WRITE_MSG(2, "Failed to parse '");
    WRITE_STR(2, item);
    WRITE_MSG(2, "' configuration\n");
    goto finish;
  }

  res = true;
  finish:
  free(const_cast(char *)(data));
  return res;
}

bool
getConfiguration(struct Configuration *cfg, PathInfo const *cfgdir)
{
  cfg->cfgdir = *cfgdir;
  cfg->nice   = FileCfg_readEntryStr(cfgdir, "nice", false, 0);
  
  return (initVdir(&cfg->vdir, cfgdir) &&
	  readSomething(&cfg->ctx_flags, cfgdir, "flags", setFlag) &&
	  readSomething(&cfg->ctx_caps,  cfgdir, "ccapabilities", setCCap) &&
	  readSomething(&cfg->ctx_caps,  cfgdir, "bcapabilities", setBCap) &&
	  getInterfaces(cfg));
}
