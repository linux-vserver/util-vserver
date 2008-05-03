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
#include "assert.h"

bool
vc_isSupported(vcFeatureSet feature)
{
  int		ver = vc_get_version();
  vc_vci_t	conf = vc_get_vci();
  if (ver==-1) return false;
  if (conf==(vc_vci_t)-1) conf = 0;

  switch (feature) {
    case vcFEATURE_COMPAT	:  return true;
    case vcFEATURE_VSHELPER0	:  return ver >= 0x00010000 && ver < 0x00010010;
    case vcFEATURE_VSHELPER	:  return ver >= 0x00010000;
    case vcFEATURE_VKILL	:  return ver >= 0x00010004;
    case vcFEATURE_RLIMIT	:  return ver >= 0x00010004;
    case vcFEATURE_VINFO	:  return ver >= 0x00010010;
    case vcFEATURE_VHI		:  return ver >= 0x00010010;
    case vcFEATURE_IATTR	:  return ver >= 0x00010011;
    case vcFEATURE_MIGRATE	:  return ver >= 0x00010012;
    case vcFEATURE_NAMESPACE	:  return ver >= 0x00010012;
    case vcFEATURE_VWAIT	:  return ver >= 0x00010025;
    case vcFEATURE_SCHED	:  return ver >= 0x00020000;  // todo
    case vcFEATURE_VNET		:  return ver >= 0x00020001;
    case vcFEATURE_VSTAT	:  return ver >= 0x00020103;
    case vcFEATURE_PPTAG	:  return conf & VC_VCI_PPTAG;
    case vcFEATURE_PIDSPACE	:  return ver >= 0x00020303 || ver >= 0x00020201;
    default			:  assert(false); 
  }

  return false;
}
