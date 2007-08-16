// $Id$    --*- c -*--

// Copyright (C) 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
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

#include "vserver.h"
#include "pathconfig.h"
#include "compat-c99.h"
#include "lib_internal/util.h"
#include "internal.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#ifdef VC_ENABLE_API_COMPAT
#include <fcntl.h>

static xid_t
extractLegacyXID(char const *dir, char const *basename)
{
  size_t	l1 = strlen(dir);
  size_t	l2 = strlen(basename);
  char		path[l1 + l2 + sizeof("/.ctx")];
  char *	ptr = path;
  int		fd;
  ssize_t	len;
  xid_t		result = VC_NOXID;

  ptr    = Xmemcpy(ptr, dir, l1);
  *ptr++ = '/';
  ptr    = Xmemcpy(ptr, basename, l2);
  ptr    = Xmemcpy(ptr, ".ctx",    5);

  fd = open(path, O_RDONLY);
  if (fd==-1) return VC_NOXID;

  len = lseek(fd, 0, SEEK_END);

  if (len!=-1 && lseek(fd, 0, SEEK_SET)!=-1) {
    char	buf[len+2];
    char const	*pos = 0;

    buf[0] = '\n';
    
    if (read(fd, buf+1, len+1)==len) {
      buf[len+1] = '\0';
      pos        = strstr(buf, "\nS_CONTEXT=");
    }

    if (pos) pos += 11;
    if (*pos>='1' && *pos<='9')
      result = atoi(pos);
  }

  close(fd);
  return result;
}
#else
static xid_t
extractLegacyXID(char const UNUSED *dir, char const UNUSED *basename)
{
  return VC_NOXID;
}
#endif


static xid_t
getCtxFromFile(char const *pathname)
{
  int		fd;
  off_t		len;

  fd = open(pathname, O_RDONLY);

  if (fd==-1) return VC_NOCTX;
  if ((len=lseek(fd, 0, SEEK_END))==-1 ||
      (len>50) ||
      (lseek(fd, 0, SEEK_SET)==-1)) {
    close(fd);
    return VC_NOCTX;
  }

  {
  char		buf[len+1];
  char		*errptr;
  xid_t		res;
  
  if (TEMP_FAILURE_RETRY(read(fd, buf, len+1))!=len) res = VC_NOCTX;
  else {
    buf[len] = '\0';

    res = strtol(buf, &errptr, 10);
    if (*errptr!='\0' && *errptr!='\n') res = VC_NOCTX;
  }

  close(fd);
  return res;
  }
}

xid_t
vc_getVserverCtx(char const *id, vcCfgStyle style, bool honor_static, bool *is_running,
		 vcCtxType type)
{
  size_t		l1 = strlen(id);
  char			buf[sizeof(CONFDIR "//") + l1 + sizeof("/ncontext")];
			    
  if (style==vcCFG_NONE || style==vcCFG_AUTO)
    style = vc_getVserverCfgStyle(id);

  if (is_running) *is_running = false;

  switch (style) {
    case vcCFG_NONE		:  return VC_NOCTX;
    case vcCFG_LEGACY		:
      return extractLegacyXID(DEFAULT_PKGSTATEDIR, id);
    case vcCFG_RECENT_SHORT	:
    case vcCFG_RECENT_FULL	: {
      size_t		idx = 0;
      xid_t		res = 0;

      if (style==vcCFG_RECENT_SHORT) {
	memcpy(buf, CONFDIR "/", sizeof(CONFDIR "/")-1);
	idx  = sizeof(CONFDIR "/") - 1;
      }
      memcpy(buf+idx, id, l1);    idx += l1;
      memcpy(buf+idx, "/run", 5);	// appends '\0' too
      
      res = getCtxFromFile(buf);

	// when context information could be read, we have to verify that
	// it belongs to a running vserver and the both vservers are
	// identically
      if (res!=VC_NOCTX && type == vcCTX_XID) {
	char			*cur_name;
	struct vc_vx_info	info;

	  // determine the vserver which is associated with the xid resp. skip
	  // this step when the context does not exist. When checking whether
	  // the context exists, do not rely on the success of
	  // vc_get_vx_info() alone but check 'errno' for ESRCH also. Else,
	  // wrong results will be caused e.g. for xid 1 which will fail with
	  // ENOSYS.
	cur_name = (vc_get_vx_info(res, &info)!=-1 || errno!=ESRCH ?
		    vc_getVserverByCtx_Internal(res, &style, 0, false) :
		    0);

	buf[idx] = '\0';	// cut off the '/run' from the vserver name
	
	res      = ((cur_name!=0 &&
		     vc_compareVserverById(buf,      vcCFG_RECENT_FULL,
					  cur_name, vcCFG_RECENT_FULL)==0)
		    ? res
		    : VC_NOCTX);	// correct the value of 'res'
	  
	free(cur_name);
      }
      
      if (is_running)			// fill 'is_running' information...
	*is_running = res!=VC_NOCTX;
      
      if (res==VC_NOCTX && honor_static) {
	switch (type) {
	  case vcCTX_XID:
	    memcpy(buf+idx, "/context", 9);	// appends '\0' too
	    break;
	  case vcCTX_NID:
	    memcpy(buf+idx, "/ncontext", 10);
	    break;
	  case vcCTX_TAG:
	    memcpy(buf+idx, "/tag", 5);
	    break;
	}

	res = getCtxFromFile(buf);
      }

      return res;
    }
    default			:  return VC_NOCTX;
  }
}
