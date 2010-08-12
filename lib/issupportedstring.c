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

#include "vserver.h"
#include "lib_internal/util-dimof.h"
#include <strings.h>

#define DECL(F) \
  { vcFEATURE_ ## F, #F }

static struct {
    vcFeatureSet	feature;
    char const *	name;
} FEATURES[] = {
  DECL(VKILL),   DECL(IATTR),     DECL(RLIMIT),   DECL(COMPAT),
  DECL(MIGRATE), DECL(NAMESPACE), DECL(SCHED),    DECL(VINFO),
  DECL(VHI),     DECL(VSHELPER0), DECL(VSHELPER), DECL(VWAIT),
  DECL(VNET),    DECL(VSTAT),     DECL(PPTAG),    DECL(PIDSPACE),
  DECL(SPACES),  DECL(PERSISTENT),DECL(PIVOT_ROOT),DECL(MEMCG),
  DECL(DYNAMIC), 
};

bool
vc_isSupportedString(char const *str)
{
  size_t	i;
  for (i=0; i<DIM_OF(FEATURES); ++i) {
    if (strcasecmp(FEATURES[i].name, str)==0)
      return vc_isSupported(FEATURES[i].feature);
  }

  return false;
}
