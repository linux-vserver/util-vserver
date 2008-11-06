/* $Id$
 * Copyright (C) 2008 Daniel Hokka Zakrisson
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * vim:set ts=2 sw=2 expandtab:
 */

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <Python.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "compat.h"
#include "vserver.h"

static inline PyObject *NONE(void)
{
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
pyvserver_get_version(PyObject UNUSED *self, PyObject UNUSED *args)
{
  int ver = vc_get_version();
  if (ver == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("i", ver);
}

static PyObject *
pyvserver_get_vci(PyObject UNUSED *self, PyObject UNUSED *args)
{
  vc_vci_t vci = vc_get_vci();
  if (vci == (vc_vci_t)-1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("K", vci);
}

static PyObject *
pyvserver_ctx_create(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid, ret;
  struct vc_ctx_flags flags = { .mask = 0 };

  if (!PyArg_ParseTuple(args, "I|KK", &xid, &flags.flagword, &flags.mask))
    return NULL;

  if (flags.flagword && !flags.mask)
    flags.mask = flags.flagword;

  ret = vc_ctx_create(xid, &flags);
  if (ret == VC_NOXID)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("I", ret);
}

static PyObject *
pyvserver_ctx_migrate(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  uint_least64_t flags;

  if (!PyArg_ParseTuple(args, "IK", &xid, &flags))
    return NULL;

  if (vc_ctx_migrate(xid, flags) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_ctx_stat(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  struct vc_ctx_stat stats;

  if (!PyArg_ParseTuple(args, "I", &xid))
    return NULL;

  if (vc_ctx_stat(xid, &stats) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(II)", stats.usecnt, stats.tasks);
}

static PyObject *
pyvserver_virt_stat(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  struct vc_virt_stat stats;

  if (!PyArg_ParseTuple(args, "I", &xid))
    return NULL;

  if (vc_virt_stat(xid, &stats) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(KKIIIIIIII)", stats.offset, stats.uptime,
                       stats.nr_threads, stats.nr_running,
                       stats.nr_uninterruptible, stats.nr_onhold,
                       stats.nr_forks, stats.load[0], stats.load[1],
                       stats.load[2]);
}

static PyObject *
pyvserver_ctx_kill(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  pid_t pid;
  int signal;

  if (!PyArg_ParseTuple(args, "Iii", &xid, &pid, &signal))
    return NULL;

  if (vc_ctx_kill(xid, pid, signal) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_get_cflags(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  struct vc_ctx_flags flags;

  if (!PyArg_ParseTuple(args, "I", &xid))
    return NULL;

  if (vc_get_cflags(xid, &flags) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(KK)", flags.flagword, flags.mask);
}

static PyObject *
pyvserver_set_cflags(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  struct vc_ctx_flags flags;

  if (!PyArg_ParseTuple(args, "I(KK)", &xid, flags.flagword, flags.mask))
    return NULL;

  if (vc_set_cflags(xid, &flags) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_get_ccaps(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  struct vc_ctx_caps caps;

  if (!PyArg_ParseTuple(args, "I", &xid))
    return NULL;

  if (vc_get_ccaps(xid, &caps) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(KKKK)", caps.bcaps, caps.bmask, caps.ccaps,
                       caps.cmask);
}

static PyObject *
pyvserver_set_ccaps(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  struct vc_ctx_caps caps;

  if (!PyArg_ParseTuple(args, "I(KKKK)", &xid, &caps.bcaps, &caps.bmask,
                        &caps.ccaps, &caps.cmask))
    return NULL;

  if (vc_set_ccaps(xid, &caps) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_get_vx_info(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  struct vc_vx_info info;

  if (!PyArg_ParseTuple(args, "I", &xid))
    return NULL;

  if (vc_get_vx_info(xid, &info) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(Ii)", info.xid, info.initpid);
}

static PyObject *
pyvserver_get_task_xid(PyObject UNUSED *self, PyObject *args)
{
  pid_t pid;
  xid_t xid;

  if (!PyArg_ParseTuple(args, "i", &pid))
    return NULL;

  xid = vc_get_task_xid(pid);
  if (xid == VC_NOXID)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("I", xid);
}

static PyObject *
pyvserver_wait_exit(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;

  if (!PyArg_ParseTuple(args, "I", &xid))
    return NULL;

  if (vc_wait_exit(xid) == -1)
    Py_RETURN_FALSE;
  else
    Py_RETURN_TRUE;
}

static PyObject *
pyvserver_get_rlimit_mask(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  struct vc_rlimit_mask mask;

  if (!PyArg_ParseTuple(args, "I", &xid))
    return NULL;

  if (vc_get_rlimit_mask(xid, &mask) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(III)", mask.min, mask.soft, mask.hard);
}

static PyObject *
pyvserver_get_rlimit(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  int resource;
  struct vc_rlimit limit = { .min = 0 };

  if (!PyArg_ParseTuple(args, "Ii", &xid, &resource))
    return NULL;

  if (vc_get_rlimit(xid, resource, &limit) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(LLL)", limit.min, limit.soft, limit.hard);
}

static PyObject *
pyvserver_set_rlimit(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  int resource;
  struct vc_rlimit limit = {
    .min = VC_LIM_KEEP,
    .soft = VC_LIM_KEEP,
    .hard = VC_LIM_KEEP
  };

  if (!PyArg_ParseTuple(args, "Ii(KKK)", &xid, &resource, &limit.min,
                        &limit.soft, &limit.hard))
    return NULL;

  if (vc_set_rlimit(xid, resource, &limit) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_rlimit_stat(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  int resource;
  struct vc_rlimit_stat stats;

  if (!PyArg_ParseTuple(args, "Ii", &xid, &resource))
    return NULL;

  if (vc_rlimit_stat(xid, resource, &stats) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(IKKK)", stats.hits, stats.value, stats.minimum,
                       stats.maximum);
}

static PyObject *
pyvserver_reset_minmax(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;

  if (!PyArg_ParseTuple(args, "I", &xid))
    return NULL;

  if (vc_reset_minmax(xid) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_get_task_nid(PyObject UNUSED *self, PyObject *args)
{
  pid_t pid;
  nid_t nid;

  if (!PyArg_ParseTuple(args, "i", &pid))
    return NULL;

  nid = vc_get_task_nid(pid);
  if (nid == VC_NONID)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("I", nid);
}

static PyObject *
pyvserver_get_nx_info(PyObject UNUSED *self, PyObject *args)
{
  nid_t nid;
  struct vc_nx_info info;

  if (!PyArg_ParseTuple(args, "I", &nid))
    return NULL;

  if (vc_get_nx_info(nid, &info) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(I)", info.nid);
}

static PyObject *
pyvserver_net_create(PyObject UNUSED *self, PyObject *args)
{
  nid_t nid, ret;

  if (!PyArg_ParseTuple(args, "I", &nid))
    return NULL;

  ret = vc_net_create(nid);
  if (ret == VC_NONID)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("I", ret);
}

static PyObject *
pyvserver_net_migrate(PyObject UNUSED *self, PyObject *args)
{
  nid_t nid;

  if (!PyArg_ParseTuple(args, "I", &nid))
    return NULL;

  if (vc_net_migrate(nid) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_net_handle(PyObject UNUSED *self, PyObject *args,
                      int (*func)(nid_t nid, struct vc_net_addr const *addr))
{
  nid_t nid;
  struct vc_net_addr addr;
  char *ip1, *ip2, *mask;

  if (!PyArg_ParseTuple(args, "I(HHHHsss)", &nid, &addr.vna_type,
                        &addr.vna_flags, &addr.vna_prefix, &addr.vna_parent,
                        &ip1, &ip2, &mask))
    return NULL;

  if (addr.vna_type & VC_NXA_TYPE_IPV6) {
    if (inet_pton(AF_INET6, ip1, &addr.vna_v6_ip) <= 0 ||
        inet_pton(AF_INET6, ip2, &addr.vna_v6_ip2) <= 0 ||
        inet_pton(AF_INET6, mask, &addr.vna_v6_mask) <= 0) {
      PyErr_SetString(PyExc_ValueError, "invalid IPv6 addresses");
      return NULL;
    }
  }
  else if (addr.vna_type & VC_NXA_TYPE_IPV4) {
    if (inet_pton(AF_INET, ip1, &addr.vna_v4_ip) <= 0 ||
        inet_pton(AF_INET, ip2, &addr.vna_v4_ip2) <= 0 ||
        inet_pton(AF_INET, mask, &addr.vna_v4_mask) <= 0) {
      PyErr_SetString(PyExc_ValueError, "invalid IPv4 addresses");
      return NULL;
    }
  }
  else if (addr.vna_type != VC_NXA_TYPE_ANY) {
    PyErr_SetString(PyExc_ValueError, "type");
    return NULL;
  }

  if (func(nid, &addr) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_net_add(PyObject UNUSED *self, PyObject *args)
{
  return pyvserver_net_handle(self, args, vc_net_add);
}

static PyObject *
pyvserver_net_remove(PyObject UNUSED *self, PyObject *args)
{
  return pyvserver_net_handle(self, args, vc_net_remove);
}

static PyObject *
pyvserver_get_nflags(PyObject UNUSED *self, PyObject *args)
{
  nid_t nid;
  struct vc_net_flags flags;

  if (!PyArg_ParseTuple(args, "I", &nid))
    return NULL;

  if (vc_get_nflags(nid, &flags) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(KK)", flags.flagword, flags.mask);
}

static PyObject *
pyvserver_set_nflags(PyObject UNUSED *self, PyObject *args)
{
  nid_t nid;
  struct vc_net_flags flags;

  if (!PyArg_ParseTuple(args, "I(KK)", &nid, &flags.flagword, &flags.mask))
    return NULL;

  if (vc_set_nflags(nid, &flags) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_get_ncaps(PyObject UNUSED *self, PyObject *args)
{
  nid_t nid;
  struct vc_net_caps caps;

  if (!PyArg_ParseTuple(args, "I", &nid))
    return NULL;

  if (vc_get_ncaps(nid, &caps) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(KK)", caps.ncaps, caps.cmask);
}

static PyObject *
pyvserver_set_ncaps(PyObject UNUSED *self, PyObject *args)
{
  nid_t nid;
  struct vc_net_caps caps;

  if (!PyArg_ParseTuple(args, "I(KK)", &nid, &caps.ncaps, &caps.cmask))
    return NULL;

  if (vc_set_ncaps(nid, &caps) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_set_iattr(PyObject UNUSED *self, PyObject *args)
{
  char const *filename;
  tag_t tag;
  uint_least32_t flags, mask;

  if (!PyArg_ParseTuple(args, "sIII", &filename, &tag, &flags, &mask))
    return NULL;

  if (vc_set_iattr(filename, tag, flags, mask) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_fset_iattr(PyObject UNUSED *self, PyObject *args)
{
  int fd;
  tag_t tag;
  uint_least32_t flags, mask;

  if (!PyArg_ParseTuple(args, "iIII", &fd, &tag, &flags, &mask))
    return NULL;

  if (vc_fset_iattr(fd, tag, flags, mask) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_get_iattr(PyObject UNUSED *self, PyObject *args)
{
  char const *filename;
  tag_t tag;
  uint_least32_t flags, mask;

  if (!PyArg_ParseTuple(args, "sI", &filename, &mask))
    return NULL;

  if (vc_get_iattr(filename, &tag, &flags, &mask) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(III)", tag, flags, mask);
}

static PyObject *
pyvserver_fget_iattr(PyObject UNUSED *self, PyObject *args)
{
  int fd;
  tag_t tag;
  uint_least32_t flags, mask;

  if (!PyArg_ParseTuple(args, "iI", &fd, &mask))
    return NULL;

  if (vc_fget_iattr(fd, &tag, &flags, &mask) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(III)", tag, flags, mask);
}

static PyObject *
pyvserver_set_vhi_name(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  vc_uts_type type;
  char const *val;
  int len;

  if (!PyArg_ParseTuple(args, "Iis#", &xid, &type, &val, &len))
    return NULL;

  if (vc_set_vhi_name(xid, type, val, len) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_get_vhi_name(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  vc_uts_type type;
  char val[65];

  if (!PyArg_ParseTuple(args, "Ii", &xid, &type))
    return NULL;

  if (vc_get_vhi_name(xid, type, val, sizeof(val)) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("s#", val, sizeof(val));
}

static PyObject *
pyvserver_enter_namespace(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  uint_least64_t mask;
  uint32_t index;

  if (!PyArg_ParseTuple(args, "IKI", &xid, &mask, &index))
    return NULL;

  if (vc_enter_namespace(xid, mask, index) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_set_namespace(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  uint_least64_t mask;
  uint32_t index;

  if (!PyArg_ParseTuple(args, "IKI", &xid, &mask, &index))
    return NULL;

  if (vc_set_namespace(xid, mask, index) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_get_space_mask(PyObject UNUSED *self, PyObject UNUSED *args)
{
  uint_least64_t mask = vc_get_space_mask();
  return Py_BuildValue("K", mask);
}

static PyObject *
pyvserver_get_space_default(PyObject UNUSED *self, PyObject UNUSED *args)
{
  uint_least64_t mask = vc_get_space_default();
  return Py_BuildValue("K", mask);
}

static PyObject *
pyvserver_add_dlimit(PyObject UNUSED *self, PyObject *args)
{
  char const *filename;
  tag_t tag;
  uint_least32_t flags;

  if (!PyArg_ParseTuple(args, "sII", &filename, &tag, &flags))
    return NULL;

  if (vc_add_dlimit(filename, tag, flags) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_rem_dlimit(PyObject UNUSED *self, PyObject *args)
{
  char const *filename;
  tag_t tag;
  uint_least32_t flags;

  if (!PyArg_ParseTuple(args, "sII", &filename, &tag, &flags))
    return NULL;

  if (vc_rem_dlimit(filename, tag, flags) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_set_dlimit(PyObject UNUSED *self, PyObject *args)
{
  char const *filename;
  tag_t tag;
  uint_least32_t flags;
  struct vc_ctx_dlimit limit;

  if (!PyArg_ParseTuple(args, "sII(IIIII)", &filename, &tag, &flags,
                        &limit.space_used, &limit.space_total,
                        &limit.inodes_used, &limit.inodes_total,
                        &limit.reserved))
    return NULL;

  if (vc_set_dlimit(filename, tag, flags, &limit) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_get_dlimit(PyObject UNUSED *self, PyObject *args)
{
  char const *filename;
  tag_t tag;
  uint_least32_t flags;
  struct vc_ctx_dlimit limit;

  if (!PyArg_ParseTuple(args, "sII", &filename, &tag, &flags))
    return NULL;

  if (vc_get_dlimit(filename, tag, flags, &limit) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(IIIII)", limit.space_used, limit.space_total,
                       limit.inodes_used, limit.inodes_total, limit.reserved);
}

static PyObject *
pyvserver_get_task_tag(PyObject UNUSED *self, PyObject *args)
{
  pid_t pid;
  tag_t tag;

  if (!PyArg_ParseTuple(args, "i", &pid))
    return NULL;

  tag = vc_get_task_tag(pid);
  if (tag == (tag_t) -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("I", tag);
}

static PyObject *
pyvserver_tag_create(PyObject UNUSED *self, PyObject *args)
{
  tag_t tag;

  if (!PyArg_ParseTuple(args, "I", &tag))
    return NULL;

  if (vc_tag_create(tag) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_tag_migrate(PyObject UNUSED *self, PyObject *args)
{
  tag_t tag;

  if (!PyArg_ParseTuple(args, "I", &tag))
    return NULL;

  if (vc_tag_migrate(tag) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_set_sched(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  struct vc_set_sched sched;

  if (!PyArg_ParseTuple(args, "I(Iiiiiiiiiii)", &xid, &sched.set_mask,
                        &sched.fill_rate, &sched.interval, &sched.fill_rate2,
                        &sched.interval2, &sched.tokens, &sched.tokens_min,
                        &sched.tokens_max, &sched.priority_bias, &sched.cpu_id,
                        &sched.bucket_id))
    return NULL;

  if (vc_set_sched(xid, &sched) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_get_sched(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  struct vc_set_sched sched;

  if (!PyArg_ParseTuple(args, "Iii", &xid, &sched.cpu_id, &sched.bucket_id))
    return NULL;

  if (vc_get_sched(xid, &sched) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(Iiiiiiiiiii)", sched.set_mask, sched.fill_rate,
                       sched.interval, sched.fill_rate2, sched.interval2,
                       sched.tokens, sched.tokens_min, sched.tokens_max,
                       sched.priority_bias, sched.cpu_id, sched.bucket_id);
}

static PyObject *
pyvserver_sched_info(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  struct vc_sched_info info;

  if (!PyArg_ParseTuple(args, "Iii", &xid, &info.cpu_id, &info.bucket_id))
    return NULL;

  if (vc_sched_info(xid, &info) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("(iiKKKIi)", info.cpu_id, info.bucket_id,
                       info.user_msec, info.sys_msec, info.hold_msec,
                       info.token_usec, info.vavavoom);
}

static PyObject *
pyvserver_set_mapping(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  const char *device, *target;
  uint32_t flags;

  if (!PyArg_ParseTuple(args, "IssI", &xid, &device, &target, &flags))
    return NULL;

  if (vc_set_mapping(xid, device, target, flags) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_unset_mapping(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  const char *device, *target;
  uint32_t flags;

  if (!PyArg_ParseTuple(args, "IssI", &xid, &device, &target, &flags))
    return NULL;

  if (vc_unset_mapping(xid, device, target, flags) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_get_badness(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  int64_t badness;

  if (!PyArg_ParseTuple(args, "I", &xid))
    return NULL;

  if (vc_get_badness(xid, &badness) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return Py_BuildValue("L", badness);
}

static PyObject *
pyvserver_set_badness(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;
  int64_t badness;

  if (!PyArg_ParseTuple(args, "IL", &xid, &badness))
    return NULL;

  if (vc_set_badness(xid, badness) == -1)
    return PyErr_SetFromErrno(PyExc_OSError);

  return NONE();
}

static PyObject *
pyvserver_get_insecurebcaps(PyObject UNUSED *self, PyObject UNUSED *args)
{
  uint_least64_t bcaps = vc_get_insecurebcaps();
  return Py_BuildValue("K", bcaps);
}

static PyObject *
pyvserver_get_insecureccaps(PyObject UNUSED *self, PyObject UNUSED *args)
{
  uint_least64_t ccaps = vc_get_insecureccaps();
  return Py_BuildValue("K", ccaps);
}

static PyObject *
pyvserver_isSupported(PyObject UNUSED *self, PyObject *args)
{
  int feature;

  if (!PyArg_ParseTuple(args, "i", &feature))
    return NULL;

  if (vc_isSupported(feature))
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
}

static PyObject *
pyvserver_isSupportedString(PyObject UNUSED *self, PyObject *args)
{
  char const *feature;

  if (!PyArg_ParseTuple(args, "s", &feature))
    return NULL;

  if (vc_isSupportedString(feature))
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
}

static PyObject *
pyvserver_getXIDType(PyObject UNUSED *self, PyObject *args)
{
  xid_t xid;

  if (!PyArg_ParseTuple(args, "I", &xid))
    return NULL;

  return Py_BuildValue("i", vc_getXIDType(xid));
}

static PyObject *
pyvserver_xidopt2xid(PyObject UNUSED *self, PyObject *args)
{
  char const *xidopt;
  PyObject *honor_static;
  xid_t xid;
  char const *err_info;

  if (!PyArg_ParseTuple(args, "sO", &xidopt, &honor_static))
    return NULL;

  xid = vc_xidopt2xid(xidopt, honor_static != Py_False, &err_info);
  if (xid == VC_NOXID) {
    PyErr_SetString(PyExc_OSError, err_info);
    return NULL;
  }

  return Py_BuildValue("I", xid);
}

static PyObject *
pyvserver_nidopt2nid(PyObject UNUSED *self, PyObject *args)
{
  char const *nidopt;
  PyObject *honor_static;
  nid_t nid;
  char const *err_info;

  if (!PyArg_ParseTuple(args, "sO", &nidopt, &honor_static))
    return NULL;

  nid = vc_nidopt2nid(nidopt, honor_static != Py_False, &err_info);
  if (nid == VC_NONID) {
    PyErr_SetString(PyExc_OSError, err_info);
    return NULL;
  }

  return Py_BuildValue("I", nid);
}

static PyObject *
pyvserver_tagopt2tag(PyObject UNUSED *self, PyObject *args)
{
  char const *tagopt;
  PyObject *honor_static;
  tag_t tag;
  char const *err_info;

  if (!PyArg_ParseTuple(args, "sO", &tagopt, &honor_static))
    return NULL;

  tag = vc_tagopt2tag(tagopt, honor_static != Py_False, &err_info);
  if (tag == (tag_t) -1) {
    PyErr_SetString(PyExc_OSError, err_info);
    return NULL;
  }

  return Py_BuildValue("I", tag);
}

#define pyvserver_handle_list(name, list_type, flag_member, mask_member) \
static PyObject *  \
pyvserver_text2 ## name(PyObject UNUSED *self, PyObject *args) \
{ \
  char const *str; \
  int len; \
  uint_least64_t val; \
\
  if (!PyArg_ParseTuple(args, "s#", &str, &len)) \
    return NULL; \
\
  val = vc_text2 ## name(str, len); \
  return Py_BuildValue("K", val); \
} \
\
static PyObject * \
pyvserver_lo ## name ## 2text(PyObject UNUSED *self, PyObject *args) \
{ \
  uint_least64_t val; \
  char const *ret; \
\
  if (!PyArg_ParseTuple(args, "K", &val)) \
    return NULL; \
\
  ret = vc_lo ## name ## 2text(&val); \
  return Py_BuildValue("(Kz)", val, ret); \
} \
\
static PyObject * \
pyvserver_list2 ## name(PyObject UNUSED *self, PyObject *args) \
{ \
  char const *str; \
  int len; \
  struct vc_err_listparser err; \
  list_type val = { .mask_member = 0 }; \
\
  if (!PyArg_ParseTuple(args, "s#", &str, &len)) \
    return NULL; \
\
  if (vc_list2 ## name(str, len, &err, &val) == -1) { \
    char *error; \
    if (asprintf(&error, "unknown value '%.*s'", (int)err.len, err.ptr) == -1) \
      return PyErr_SetFromErrno(PyExc_MemoryError); \
    PyErr_SetString(PyExc_ValueError, error); \
    free(error); \
    return NULL; \
  } \
\
  return Py_BuildValue("(KK)", val.flag_member, val.mask_member); \
}

pyvserver_handle_list(bcap, struct vc_ctx_caps, bcaps, bmask)
pyvserver_handle_list(ccap, struct vc_ctx_caps, ccaps, cmask)
pyvserver_handle_list(cflag, struct vc_ctx_flags, flagword, mask)
pyvserver_handle_list(nflag, struct vc_net_flags, flagword, mask)
pyvserver_handle_list(ncap, struct vc_net_caps, ncaps, cmask)

static PyMethodDef methods[] = {
  { "vc_get_version", pyvserver_get_version, METH_NOARGS, "FIXME" },
  { "vc_get_vci", pyvserver_get_vci, METH_NOARGS, "FIXME" },
  { "vc_ctx_create", pyvserver_ctx_create, METH_VARARGS, "FIXME" },
  { "vc_ctx_migrate", pyvserver_ctx_migrate, METH_VARARGS, "FIXME" },
  { "vc_ctx_stat", pyvserver_ctx_stat, METH_VARARGS, "FIXME" },
  { "vc_virt_stat", pyvserver_virt_stat, METH_VARARGS, "FIXME" },
  { "vc_ctx_kill", pyvserver_ctx_kill, METH_VARARGS, "FIXME" },
  { "vc_get_cflags", pyvserver_get_cflags, METH_VARARGS, "FIXME" },
  { "vc_set_cflags", pyvserver_set_cflags, METH_VARARGS, "FIXME" },
  { "vc_get_ccaps", pyvserver_get_ccaps, METH_VARARGS, "FIXME" },
  { "vc_set_ccaps", pyvserver_set_ccaps, METH_VARARGS, "FIXME" },
  { "vc_get_vx_info", pyvserver_get_vx_info, METH_VARARGS, "FIXME" },
  { "vc_get_task_xid", pyvserver_get_task_xid, METH_VARARGS, "FIXME" },
  { "vc_wait_exit", pyvserver_wait_exit, METH_VARARGS, "FIXME" },
  { "vc_get_rlimit_mask", pyvserver_get_rlimit_mask, METH_VARARGS, "FIXME" },
  { "vc_get_rlimit", pyvserver_get_rlimit, METH_VARARGS, "FIXME" },
  { "vc_set_rlimit", pyvserver_set_rlimit, METH_VARARGS, "FIXME" },
  { "vc_rlimit_stat", pyvserver_rlimit_stat, METH_VARARGS, "FIXME" },
  { "vc_reset_minmax", pyvserver_reset_minmax, METH_VARARGS, "FIXME" },
  { "vc_get_task_nid", pyvserver_get_task_nid, METH_VARARGS, "FIXME" },
  { "vc_get_nx_info", pyvserver_get_nx_info, METH_VARARGS, "FIXME" },
  { "vc_net_create", pyvserver_net_create, METH_VARARGS, "FIXME" },
  { "vc_net_migrate", pyvserver_net_migrate, METH_VARARGS, "FIXME" },
  { "vc_net_add", pyvserver_net_add, METH_VARARGS, "FIXME" },
  { "vc_net_remove", pyvserver_net_remove, METH_VARARGS, "FIXME" },
  { "vc_get_nflags", pyvserver_get_nflags, METH_VARARGS, "FIXME" },
  { "vc_set_nflags", pyvserver_set_nflags, METH_VARARGS, "FIXME" },
  { "vc_get_ncaps", pyvserver_get_ncaps, METH_VARARGS, "FIXME" },
  { "vc_set_ncaps", pyvserver_set_ncaps, METH_VARARGS, "FIXME" },
  { "vc_set_iattr", pyvserver_set_iattr, METH_VARARGS, "FIXME" },
  { "vc_fset_iattr", pyvserver_fset_iattr, METH_VARARGS, "FIXME" },
  { "vc_get_iattr", pyvserver_get_iattr, METH_VARARGS, "FIXME" },
  { "vc_fget_iattr", pyvserver_fget_iattr, METH_VARARGS, "FIXME" },
  { "vc_set_vhi_name", pyvserver_set_vhi_name, METH_VARARGS, "FIXME" },
  { "vc_get_vhi_name", pyvserver_get_vhi_name, METH_VARARGS, "FIXME" },
  { "vc_enter_namespace", pyvserver_enter_namespace, METH_VARARGS, "FIXME" },
  { "vc_set_namespace", pyvserver_set_namespace, METH_VARARGS, "FIXME" },
  { "vc_get_space_mask", pyvserver_get_space_mask, METH_NOARGS, "FIXME" },
  { "vc_get_space_default", pyvserver_get_space_default, METH_NOARGS, "FIXME" },
  { "vc_add_dlimit", pyvserver_add_dlimit, METH_VARARGS, "FIXME" },
  { "vc_rem_dlimit", pyvserver_rem_dlimit, METH_VARARGS, "FIXME" },
  { "vc_set_dlimit", pyvserver_set_dlimit, METH_VARARGS, "FIXME" },
  { "vc_get_dlimit", pyvserver_get_dlimit, METH_VARARGS, "FIXME" },
  { "vc_get_task_tag", pyvserver_get_task_tag, METH_VARARGS, "FIXME" },
  { "vc_tag_create", pyvserver_tag_create, METH_VARARGS, "FIXME" },
  { "vc_tag_migrate", pyvserver_tag_migrate, METH_VARARGS, "FIXME" },
  { "vc_set_sched", pyvserver_set_sched, METH_VARARGS, "FIXME" },
  { "vc_get_sched", pyvserver_get_sched, METH_VARARGS, "FIXME" },
  { "vc_sched_info", pyvserver_sched_info, METH_VARARGS, "FIXME" },
  { "vc_set_mapping", pyvserver_set_mapping, METH_VARARGS, "FIXME" },
  { "vc_unset_mapping", pyvserver_unset_mapping, METH_VARARGS, "FIXME" },
  { "vc_get_badness", pyvserver_get_badness, METH_VARARGS, "FIXME" },
  { "vc_set_badness", pyvserver_set_badness, METH_VARARGS, "FIXME" },
  { "vc_get_insecurebcaps", pyvserver_get_insecurebcaps, METH_NOARGS, "FIXME" },
  { "vc_get_insecureccaps", pyvserver_get_insecureccaps, METH_NOARGS, "FIXME" },
  { "vc_isSupported", pyvserver_isSupported, METH_VARARGS, "FIXME" },
  { "vc_isSupportedString", pyvserver_isSupportedString, METH_VARARGS, "FIXME" },
  { "vc_getXIDType", pyvserver_getXIDType, METH_VARARGS, "FIXME" },
  { "vc_xidopt2xid", pyvserver_xidopt2xid, METH_VARARGS, "FIXME" },
  { "vc_nidopt2nid", pyvserver_nidopt2nid, METH_VARARGS, "FIXME" },
  { "vc_tagopt2tag", pyvserver_tagopt2tag, METH_VARARGS, "FIXME" },
  { "vc_text2bcap", pyvserver_text2bcap, METH_VARARGS, "FIXME" },
  { "vc_lobcap2text", pyvserver_lobcap2text, METH_VARARGS, "FIXME" },
  { "vc_list2bcap", pyvserver_list2bcap, METH_VARARGS, "FIXME" },
  { "vc_text2ccap", pyvserver_text2ccap, METH_VARARGS, "FIXME" },
  { "vc_loccap2text", pyvserver_loccap2text, METH_VARARGS, "FIXME" },
  { "vc_list2ccap", pyvserver_list2ccap, METH_VARARGS, "FIXME" },
  { "vc_text2cflag", pyvserver_text2cflag, METH_VARARGS, "FIXME" },
  { "vc_locflag2text", pyvserver_locflag2text, METH_VARARGS, "FIXME" },
  { "vc_list2cflag", pyvserver_list2cflag, METH_VARARGS, "FIXME" },
  { "vc_text2nflag", pyvserver_text2nflag, METH_VARARGS, "FIXME" },
  { "vc_lonflag2text", pyvserver_lonflag2text, METH_VARARGS, "FIXME" },
  { "vc_list2nflag", pyvserver_list2nflag, METH_VARARGS, "FIXME" },
  { "vc_text2ncap", pyvserver_text2ncap, METH_VARARGS, "FIXME" },
  { "vc_loncap2text", pyvserver_loncap2text, METH_VARARGS, "FIXME" },
  { "vc_list2ncap", pyvserver_list2ncap, METH_VARARGS, "FIXME" },
  { NULL, NULL, 0, NULL }
};

static void
PyModule_AddLongLongConstant(PyObject *mod, const char *str, long long val)
{
	PyObject *o = PyLong_FromLongLong(val);
	if (!o || PyModule_AddObject(mod, str, o) == -1)
		/* This ought to be reported somehow... */
		return;
}

PyMODINIT_FUNC init_libvserver(void)
{
  PyObject *mod;

  mod = Py_InitModule("_libvserver", methods);
#include "_libvserver-constants.c"
}
