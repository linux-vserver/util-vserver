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

#include "util.h"
#include "wrappers.h"
#include "wrappers-vserver.h"
#include "pathconfig.h"

#include <lib/vserver.h>
#include <lib/fmt.h>
#include <assert.h>

#define CTXNR_WIDTH	5
#define HUNK_SIZE	0x4000
#define CONTEXT_WIDTH	20
#define CONTEXT_PLACE	"                    "

int wrapper_exit_code = 254;

struct ContextMapping {
    xid_t		ctx;
    char const *	id;
};

static struct ContextMapping		*mapping    = 0;
static size_t				mapping_len = 0;


static void
showHelp(int fd, char const *cmd, int res)
{
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, cmd);
  WRITE_MSG(fd,
	    " <ps-opts>*\n\n"
	    "Please report bugs to " PACKAGE_BUGREPORT "\n");
  exit(res);
}

static void
showVersion()
{
  WRITE_MSG(1,
	    "vps " VERSION " -- shows processes in vserver-contexts\n"
	    "This program is part of " PACKAGE_STRING "\n\n"
	    "Copyright (C) 2004 Enrico Scholz\n"
	    VERSION_COPYRIGHT_DISCLAIMER);
  exit(0);
}


static size_t
writeContextInfo(xid_t ctx, char const *name)
{
  char		buf[sizeof(ctx)*3+1];
  size_t	l   = utilvserver_fmt_ulong(buf, ctx);
  size_t	l1  = name==0 ? 0 : strlen(name);
  size_t	res = CTXNR_WIDTH + 1;

  if (l<CTXNR_WIDTH) write(1, CONTEXT_PLACE, CTXNR_WIDTH-l);
  write(1, buf, l);
  write(1, " ", 1);

  if (l1!=0) {
    assert(name!=0);
    write(1, name, l1);
  }

  return res+l1;
}

static xid_t
extractCtx(char *pid_str)
{
  pid_t		pid;
  
  while (*pid_str==' ') ++pid_str;
  pid = atoi(pid_str);

  return vc_get_task_xid(pid);
}

static char const *
resolveCtx(xid_t ctx)
{
  char const *	res;
  size_t	i;

  for (i=0; i<mapping_len; ++i)
    if (mapping[i].ctx==ctx) return mapping[i].id;

  ++mapping_len;
  mapping = Erealloc(mapping, mapping_len * sizeof(mapping[0]));
  
  if (ctx==0)      res = strdup("MAIN");
  else if (ctx==1) res = strdup("ALL_PROC");
  else {
    vcCfgStyle	style = vcCFG_AUTO;
    char	*tmp  = vc_getVserverByCtx(ctx, &style,0);
    if (tmp) res = vc_getVserverName(tmp, style);
    else     res = 0;
    free(tmp);
  }

  mapping[mapping_len-1].ctx = ctx;
  mapping[mapping_len-1].id  = res;
  return res;
}

static char *
readOutput(int fd, size_t *total_len)
{
  size_t	len  = 2*HUNK_SIZE;
  char		*buf = Emalloc(len+1);
  size_t	offset = 0;

  for (;;) {
    size_t	l;

    while (offset >= len) {
      len += HUNK_SIZE;
      buf  = Erealloc(buf, len+1);
    }
    
    l = Eread(fd, buf+offset, len - offset);
    if (l==0) break;

    offset += l;
  }

  buf[offset] = '\0';

  if (total_len)
    *total_len = offset;

  return buf;
}

static void
processOutput(char *data, size_t len)
{
  size_t	pid_end;
  char *	eol_pos = strchr(data, '\n');
  char *	pos;

  if (eol_pos==0) eol_pos  = data + len;
  else            *eol_pos = '\0';
  
  pos = strstr(data, "PID");
  if (pos==0) {
    WRITE_MSG(2, "Failed to parse ps-output\n");
    exit(wrapper_exit_code);
  }

  pid_end = pos-data + 4;

  write(1, data, pid_end);
  write(1, "CONTEXT" CONTEXT_PLACE, CONTEXT_WIDTH);
  write(1, data+pid_end, eol_pos-(data+pid_end));
  write(1, "\n", 1);

  len -= eol_pos-data;
  data = eol_pos+1;
  
  while (len > 1) {
    char const	*vserver_name = 0;
    xid_t	ctx;
    size_t	l;

    --len;
    eol_pos = strchr(data, '\n');
    
    if (eol_pos==0) eol_pos  = data + len;

    ctx          = extractCtx(data + pid_end - 6);
    vserver_name = resolveCtx(ctx);

    write(1, data, pid_end);
    l = writeContextInfo(ctx, vserver_name);
    if (l<CONTEXT_WIDTH) write(1, CONTEXT_PLACE, CONTEXT_WIDTH-l);
    write(1, data+pid_end, eol_pos-(data+pid_end));
    write(1, "\n", 1);
    
    len -= eol_pos-data;
    data = eol_pos+1;
  }
}

int main(int argc, char *argv[])
{
  int		p[2];
  pid_t		pid;
  char *	data;
  size_t	len;

  if (argc>1) {
    if (strcmp(argv[1], "--help")   ==0) showHelp(1, argv[0], 0);
    if (strcmp(argv[1], "--version")==0) showVersion();
  }
    
  if (vc_get_task_xid(0)!=1)
    Evc_new_s_context(1, vc_get_securecaps(), 0);

  Epipe(p);
  pid = Efork();

  if (pid==0) {
    int		fd = Eopen("/dev/null", O_RDONLY, 0);
    Edup2(fd,   0);
    Edup2(p[1], 1);
    Eclose(p[0]);
    Eclose(p[1]);
    Eclose(fd);

    argv[0] = "ps";

    Eexecv(PS_PROG, argv);
  }

  Eclose(p[1]);
  data = readOutput(p[0], &len);
  Eclose(p[0]);
  
  processOutput(data, len);

  exitLikeProcess(pid);
  perror("exitLikeProcess()");
  return wrapper_exit_code;
}
