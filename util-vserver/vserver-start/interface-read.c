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

#include "interface.h"

#include <lib_internal/filecfg.h>
#include <lib_internal/util.h>

#include <netinet/in.h>
#include <arpa/inet.h>

static inline char *
readEntryStr(PathInfo const *cfgdir, char const *file, char const *dflt)
{
  return FileCfg_readEntryStr(cfgdir, file, false, dflt);
}

static inline bool
readEntryFlag(PathInfo const *cfgdir, char const *file, bool dflt)
{
  return FileCfg_readEntryFlag(cfgdir, file, dflt);
}

static int
assumeNonNull(PathInfo const *cfgdir, char const *file, char const *val)
{
  if (val!=0) return 0;

  WRITE_MSG(2, "vserver-start: no value configured for '");
  write(2, cfgdir->d, cfgdir->l);
  WRITE_MSG(2, "/");
  WRITE_STR(2, file);
  WRITE_STR(2, "'\n");
  return 1;
}

bool
Iface_read(struct Interface *res, PathInfo *cfgdir,
	   struct Interface const *dflt)
{
  char const *	extip;
  char const *	ip;
  char const *	mask;
  char const *	prefix;
  char const *	bcast;
  bool		rc = false;

    // skip 'disabled' interfaces
  if (readEntryFlag(cfgdir, "disabled", false)) return true;
    
  ip          =  readEntryStr (cfgdir, "ip",       0);
  mask        =  readEntryStr (cfgdir, "mask",     0);
  prefix      =  readEntryStr (cfgdir, "prefix",   0);
  extip       =  readEntryStr (cfgdir, "extip",    0);
  bcast       =  readEntryStr (cfgdir, "bcast",    0);
  res->mac    =  readEntryStr (cfgdir, "mac",      0);
  res->name   =  readEntryStr (cfgdir, "name",     0);
  res->dev    =  readEntryStr (cfgdir, "dev",      dflt ? dflt->dev   : 0);
  res->scope  =  readEntryStr (cfgdir, "scope",    dflt ? dflt->scope : 0);
  res->nodev  =  readEntryFlag(cfgdir, "nodev",    false);
  res->direct = !readEntryFlag(cfgdir, "indirect", false);
  res->up     = !readEntryFlag(cfgdir, "down",     false);

  if (dflt && (
	assumeNonNull(cfgdir, "ip",  ip) +
	assumeNonNull(cfgdir, "dev", res->dev) +
	(dflt->addr.ipv4.mask>0) ? 0 : (
	  (mask   ? 0 : assumeNonNull(cfgdir, "prefix", prefix)) +
	  (prefix ? 0 : assumeNonNull(cfgdir, "mask",   mask))
	  )))
    goto err;

  if (mask && prefix) {
    WRITE_MSG(2, "vserver-start: both 'prefix' and 'mask' specified in '");
    write(2, cfgdir->d, cfgdir->l);
    WRITE_MSG(2, "'\n");
    goto err;
  }

  if (bcast)
    res->addr.ipv4.bcast = inet_addr(bcast);

  if (ip)
    res->addr.ipv4.ip    = inet_addr(ip);
  
  if (extip)
    res->addr.ipv4.extip = inet_addr(extip);

  if (prefix) {
    int		p = atoi(prefix);
    if (p==0) {
      WRITE_MSG(2, "vserver-start: invalid 'prefix' specified in '");
      write(2, cfgdir->d, cfgdir->l);
      WRITE_MSG(2, "'\n");
      goto err;
    }
      
    res->addr.ipv4.mask = htonl(-1u << (32-p));
  }
  else if (mask)
    res->addr.ipv4.mask = inet_addr(mask);
  else if (dflt)
    res->addr.ipv4.mask = dflt->addr.ipv4.mask;

  rc = true;

  err:
  free(const_cast(void *)(bcast));
  free(const_cast(void *)(extip));
  free(const_cast(void *)(ip));
  free(const_cast(void *)(mask));
  free(const_cast(void *)(prefix));

  return rc;
}
