// $Id$    --*- c -*--

// Copyright (C) 2007 Daniel Hokka Zakrisson
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <lib_internal/util.h>
#include <lib_internal/matchlist.h>

int wrapper_exit_code = 255;

int main(int argc, char *argv[])
{
	struct MatchList list;
	static const char *files[] = {
		"/bin",
		"+/bin/a",
	};
	int test = 0;
	uint32_t result = 0;

	MatchList_init(&list, "/", sizeof(files) / sizeof(*files));
	MatchList_appendFiles(&list, 0, files, sizeof(files) / sizeof(*files), true);

#define DO_TEST(x)	switch (MatchList_compare(&list, x)) { \
			case stINCLUDE:	result |= 1 << test; break; \
			case stEXCLUDE:	result |= 2 << test; break; \
			case stSKIP:	result |= 4 << test; break; \
			} \
			test += 3;
	DO_TEST("/bin");
	list.skip_depth++;
	DO_TEST("/bin/a");
	DO_TEST("/bin/b");
	list.skip_depth--;
	DO_TEST("/sbin");
	DO_TEST("/usr/lib/a");

	MatchList_destroy(&list);

	if (result == 011212)
		return 0;
	else {
		char buf[(sizeof(result) * 8) / 3 + 2], *ptr;
		ssize_t i;
		WRITE_MSG(1, "result = ");
		buf[sizeof(buf) - 1] = '\0';
		for (i = 0, ptr = buf + sizeof(buf) - 2; i < (sizeof(result) * 8); i += 3, ptr--)
			*ptr = '0' + ((result & (7 << i)) >> i);
		WRITE_STR(1, buf);
		WRITE_MSG(1, "\n");
		return 1;
	}
}
