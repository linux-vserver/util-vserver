#!/usr/bin/python -tt
# 
# $Id$
# Copyright (C) 2008 Daniel Hokka Zakrisson
# vim:set ts=4 sw=4 expandtab:
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

import _libvserver

class struct:
    _default_ = 0
    def __init__(self, *args, **kwargs):
        l = len(args)
        if l > len(self._fields_):
            raise KeyError, "%s has only %d fields" % (self.__class__,
                                                       len(self._fields_))
        for i in range(0, l):
            self.__dict__[self._fields_[i]] = args[i]
        for i in kwargs.iterkeys():
            if i not in self._fields_:
                raise KeyError, "%s has no such field '%s'" % (self.__class__,
                                                               i)
            self.__dict__[i] = kwargs[i]
    def __totuple(self):
        return tuple(self.__dict__.get(f, self._default_) for f in self._fields_)
    def __iter__(self):
        return self.__totuple().__iter__()
    def __repr__(self):
        return repr(self.__dict__)
    def __addsub(self, other, negator):
        import copy
        c = copy.deepcopy(self)
        for i in vars(other):
            if i in self._fields_:
                c.__dict__[i] = (self.__dict__.get(i, self._default_) +
                                 (negator * getattr(other, i)))
        return c
    def __add__(self, other):
        return self.__addsub(other, 1)
    def __sub__(self, other):
        return self.__addsub(other, -1)

get_vci = _libvserver.vc_get_vci

class struct_ctx_flags(struct):
    _fields_ = ["flagword", "mask"]

def ctx_create(xid, flags=None):
    if flags is None:
        flags = (0, 0)
    elif not isinstance(flags, struct_ctx_flags):
        raise TypeError, "flags must be of type struct_ctx_flags"
    return _libvserver.vc_ctx_create(xid, *flags)
def ctx_migrate(xid, flags=0L):
    return _libvserver.vc_ctx_migrate(xid, flags)

class struct_ctx_stat(struct):
    _fields_ = ["usecnt", "tasks"]
def ctx_stat(xid):
    return struct_ctx_stat(*_libvserver.vc_ctx_stat(xid))

class struct_virt_stat(struct):
    _fields_ = ["offset", "uptime", "nr_threads", "nr_running",
                "nr_uninterruptible", "nr_onhold", "nr_forks",
                "load0", "load1", "load2"]
def virt_stat(xid):
    return struct_virt_stat(*_libvserver.vc_virt_stat(xid))

ctx_kill = _libvserver.vc_ctx_kill
def get_cflags(xid):
    return struct_ctx_flags(*_libvserver.vc_get_cflags(xid))
def set_cflags(xid, flags):
    if not isinstance(flags, struct_ctx_flags):
        raise TypeError, "flags must be of type struct_ctx_flags"
    _libvserver.vc_set_cflags(xid, *flags)

class struct_ctx_caps(struct):
    _fields_ = ["bcaps", "bmask", "ccaps", "cmask"]
def get_ccaps(xid):
    return struct_ctx_caps(*_libvserver.vc_get_ccaps(xid))
def set_ccaps(xid, caps):
    if not isinstance(caps, struct_ctx_caps):
        raise TypeError, "caps must be of type struct_ctx_caps"
    _libvserver.vc_set_ccaps(xid, *caps)

class struct_vx_info(struct):
    _fields_ = ["xid", "initpid"]
def get_vx_info(xid):
    return struct_vx_info(*_libvserver.vc_get_vx_info(xid))

get_task_xid = _libvserver.vc_get_task_xid
wait_exit = _libvserver.vc_wait_exit

class struct_rlimit_mask(struct):
    _fields_ = ["min", "soft", "hard"]
def get_rlimit_mask(xid):
    return struct_rlimit_mask(*_libvserver.vc_get_rlimit_mask(xid))

class struct_rlimit(struct):
    _fields_ = ["min", "soft", "hard"]
    _default_ = _libvserver.VC_LIM_KEEP
def get_rlimit(xid, resource):
    return struct_rlimit(*_libvserver.vc_get_rlimit(xid, resource))
def set_rlimit(xid, resource, limit):
    if not isinstance(limit, struct_rlimit):
        raise TypeError, "limit must be of type struct_rlimit"
    _libvserver.vc_set_rlimit(xid, resource, *limit)

class stuct_rlimit_stat(struct):
    _fields_ = ["hits", "value", "minimum", "maximum"]
def rlimit_stat(xid, resource):
    return struct_rlimit_stat(*_libvserver.vc_rlimit_stat(xid, resource))

reset_minmax = _libvserver.vc_reset_minmax

get_task_nid = _libvserver.vc_get_task_nid
net_create = _libvserver.vc_net_create
net_migrate = _libvserver.vc_net_migrate

class struct_net_addr(struct):
    _fields_ = ["vna_type", "vna_flags", "vna_prefix", "vna_parent", "ip1",
                "ip2", "mask"]

def net_add(nid, addr):
    if not isinstance(addr, struct_net_addr):
        raise TypeError, "addr must be of type struct_net_addr"
    _libvserver.vc_net_add(nid, *addr)

def net_remove(nid, addr):
    if not isinstance(addr, struct_net_addr):
        raise TypeError, "addr must be of type struct_net_addr"
    _libvserver.vc_net_remove(nid, *addr)

class struct_net_flags(struct):
    _fields_ = ["flagword", "mask"]
def get_nflags(nid):
    return struct_net_flags(*_libvserver.vc_get_nflags(nid))
def set_nflags(nid, flags):
    if not isinstance(flags, struct_net_flags):
        raise TypeError, "flags must be of type struct_net_flags"
    _libvserver.vc_set_nflags(nid, *flags)

class struct_net_caps(struct):
    _fields_ = ["ncaps", "cmask"]
def get_ncaps(nid):
    return struct_net_caps(*_libvserver.vc_get_ncaps(nid))
def set_ncaps(nid, caps):
    if not isinstance(caps, struct_net_caps):
        raise TypeError, "caps must be of type struct_net_caps"
    _libvserver.vc_set_ncaps(nid, *caps)

def _vc_set_iattr(f, obj, tag, flags, mask):
    if tag is None:
        tag = 0
    else:
        mask |= _libvserver.VC_IATTR_XID
    f(obj, tag, flags, mask)
def set_iattr(filename, tag=None, flags=0, mask=0):
    _vc_set_iattr(_libvserver.vc_set_iattr, filename, tag, flags, mask)
def fset_iattr(fd, tag=None, flags=0, mask=0):
    _vc_set_iattr(_libvserver.vc_fset_iattr, fd, tag, flags, mask)
get_iattr = lambda f: _libvserver.vc_get_iattr(f, -1)
fget_iattr = lambda f: _libvserver.vc_fget_iattr(f, -1)

def vhi_type(type):
    if isinstance(type, int):
        return type
    else:
        return _libvserver.__dict__["vcVHI_%s" % type]
def set_vhi_name(xid, type, val):
    _libvserver.vc_set_vhi_name(xid, vhi_type(type), val)
def get_vhi_name(xid, type):
    return _libvserver.vc_get_vhi_name(xid, vhi_type(type))

enter_namespace = _libvserver.vc_enter_namespace
set_namespace = _libvserver.vc_set_namespace
get_space_mask = _libvserver.vc_get_space_mask
get_space_default = _libvserver.vc_get_space_default

def add_dlimit(path, tag, flags=0):
    _libvserver.vc_add_dlimit(path, tag, flags)
def rem_dlimit(path, tag, flags=0):
    _libvserver.vc_rem_dlimit(path, tag, flags)
class struct_ctx_dlimit(struct):
    _fields_ = ["space_used", "space_total", "inodes_used", "inodes_total",
                "reserved"]
    _default_ = _libvserver.VC_CDLIM_KEEP
def set_dlimit(path, tag, limit, flags=0):
    if not isinstance(limit, struct_ctx_dlimit):
        raise TypeError, "limit must be of type struct_ctx_dlimit"
    _libvserver.vc_set_dlimit(path, tag, flags, *limit)
def get_dlimit(path, tag, flags=0):
    return struct_ctx_dlimit(*_libvserver.vc_get_dlimit(path, tag, flags))

get_task_tag = _libvserver.vc_get_task_tag
tag_create = _libvserver.vc_tag_create
tag_migrate = _libvserver.vc_tag_migrate

class struct_set_sched(struct):
    _fields_ = ["set_mask", "fill_rate", "interval", "fill_rate2", "interval2",
                "tokens", "tokens_min", "tokens_max", "priority_bias",
                "cpu_id", "bucket_id"]
    def fill_set_mask(self):
        if "set_mask" not in self.__dict__:
            self.set_mask = 0
        for field in self.__dict__:
            f = field.replace("priority", "prio").upper()
            self.set_mask |= _libvserver.__dict__.get("VC_VXSM_" + f, 0)
def set_sched(xid, sched):
    if not isinstance(sched, struct_set_sched):
        raise TypeError, "sched must be of type struct_set_sched"
    sched.fill_set_mask()
    _libvserver.vc_set_sched(xid, *sched)
def get_sched(xid, cpu_id=0, bucket_id=0):
    return struct_set_sched(*_libvserver.vc_get_sched(xid, cpu_id, bucket_id))

class struct_sched_info(struct):
    _fields_ = ["cpu_id", "bucket_id", "user_msec", "sys_msec", "hold_msec",
                "token_usec", "vavavoom"]
def sched_info(xid, cpu_id=-1, bucket_id=0):
    if cpu_id == -1:
        import os
        ret = struct_sched_info()
        ncpus = os.sysconf("SC_NPROCESSORS_ONLN")
        seen = 0
        # * 2 is to make sure we get all the processors. CPU hot-plug...
        for cpu in range(0, ncpus * 2):
            try:
                ret += struct_sched_info(*_libvserver.vc_sched_info(xid,
                                                                    cpu, 0))
                seen += 1
            except:
                pass
            if seen == ncpus:
                break
        return ret
    else:
        return struct_sched_info(*_libvserver.vc_sched_info(xid, cpu_id,
                                                               bucket_id))

set_mapping = _libvserver.vc_set_mapping
unset_mapping = _libvserver.vc_unset_mapping

get_badness = _libvserver.vc_get_badness
set_badness = _libvserver.vc_set_badness

get_insecurebcaps = _libvserver.vc_get_insecurebcaps
get_insecureccaps = _libvserver.vc_get_insecureccaps

isSupported = _libvserver.vc_isSupported
isSupportedString = _libvserver.vc_isSupportedString

getXIDType = _libvserver.vc_getXIDType

def xidopt2xid(opt, honor_static=True):
    return _libvserver.vc_xidopt2xid(opt, honor_static)
def nidopt2nid(opt, honor_static=True):
    return _libvserver.vc_nidopt2nid(opt, honor_static)
def tagopt2tag(opt, honor_static=True):
    return _libvserver.vc_tagopt2tag(opt, honor_static)

# XXX: bcap, ccap, cflag, nflag, ncap could all use the same code here.
def text2bcap(text):
    ret = _libvserver.vc_text2bcap(text)
    if ret == 0:
        raise ValueError, "%s is not a valid bcap" % text
    return ret
lobcap2text = _libvserver.vc_lobcap2text
def bcap2list(bcaps):
    list = []
    while True:
        bcaps, text = _libvserver.vc_lobcap2text(bcaps)
        if text is None:
            break
        list.append(text)
    return ",".join(list)
def list2bcap(list):
    bcaps, bmask = _libvserver.vc_list2bcap(list)
    return struct_ctx_caps(bcaps=bcaps, bmask=bmask)

def text2ccap(text):
    ret = _libvserver.vc_text2ccap(text)
    if ret == 0:
        raise ValueError, "%s is not a valid ccap" % text
    return ret
loccap2text = _libvserver.vc_loccap2text
def ccap2list(ccaps):
    list = []
    while True:
        ccaps, text = _libvserver.vc_loccap2text(ccaps)
        if text is None:
            break
        list.append(text)
    return ",".join(list)
def list2ccap(list):
    ccaps, cmask = _libvserver.vc_list2ccap(list)
    return struct_ctx_caps(ccaps=ccaps, cmask=cmask)

def text2cflag(text):
    ret = _libvserver.vc_text2cflag(text)
    if ret == 0:
        raise ValueError, "%s is not a valid cflag" % text
    return ret
locflag2text = _libvserver.vc_locflag2text
def cflag2list(cflags):
    list = []
    while True:
        cflags, text = _libvserver.vc_locflag2text(cflags)
        if text is None:
            break
        list.append(text)
    return ",".join(list)
def list2cflag(list):
    return struct_ctx_flags(*_libvserver.vc_list2cflag(list))

def text2nflag(text):
    ret = _libvserver.vc_text2nflag(text)
    if ret == 0:
        raise ValueError, "%s is not a valid nflag" % text
    return ret
lonflag2text = _libvserver.vc_lonflag2text
def nflag2list(nflags):
    list = []
    while True:
        nflags, text = _libvserver.vc_lonflag2text(nflags)
        if text is None:
            break
        list.append(text)
    return ",".join(list)
def list2nflag(list):
    return struct_net_flags(*_libvserver.vc_list2nflag(list))

def text2ncap(text):
    ret = _libvserver.vc_text2ncap(text)
    if ret == 0:
        raise ValueError, "%s is not a valid ncap" % text
    return ret
loncap2text = _libvserver.vc_loncap2text
def ncap2list(ncaps):
    list = []
    while True:
        ncaps, text = _libvserver.vc_loncap2text(ncaps)
        if text is None:
            break
        list.append(text)
    return ",".join(list)
def list2ncap(list):
    return struct_net_caps(*_libvserver.vc_list2ncap(list))

