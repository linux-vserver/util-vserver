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


static void
createSkeleton(char const *name)
{
  char const *	app_dir;

  app_dir = vc_getVserverAppDir(name, vcCFG_AUTO, "vunify");
  if (app_dir==0 &&
      vc_createSkeleton(name, vcCFG_AUTO, vcSKEL_FILESYSTEM|vcSKEL_PKGMGMT)==-1) {
    perror("vc_createSkeleton()");
    exit(1);
  }

  if (app_dir==0) {
    app_dir = vc_getVserverAppDir(name, vcCFG_AUTO, "");
      
    PathInfo		path = {
      .d = app_dir,
      .l = strlen(app_dir),
    };
    PathInfo		rhs_path = {
      .d = "vunify",
      .l = sizeof("vunify")-1
    };
      
    char		p_buf[ENSC_PI_APPSZ(path, rhs_path)];
    PathInfo_append(&path, &rhs_path, p_buf);

    Emkdir(path.d, 0755);
  }

  free(const_cast(char *)(app_dir));
}


static void
initModeManually(int argc, char *argv[])
{
  int		count=argc/2;

  if (count!=2) {
    WRITE_MSG(2, "Bad arguments; try '--help' for more information\n");
    exit(1);
  }

  MatchList_initManually(&global_info.dst_list, 0, strdup(argv[0]), argv[1]);
  MatchList_initManually(&global_info.src_list, 0, strdup(argv[2]), argv[3]);
}


static void
initModeVserver(int argc, char *argv[])
{
  int		count=argc;

  if (count!=2) {
    WRITE_MSG(2, "Bad arguments; try '--help' for more information\n");
    exit(1);
  }

  if (!MatchList_initByVserver(&global_info.src_list, argv[1], 0)) {
    WRITE_MSG(2, "unification not configured for source vserver\n");
    exit(1);
  }

  if (!global_args->is_strict)
    createSkeleton(argv[0]);

  if (!MatchList_initByVserver(&global_info.dst_list, argv[0], 0)) {
    WRITE_MSG(2, "unification not configured for destination vserver\n");
    exit(1);
  }
}
