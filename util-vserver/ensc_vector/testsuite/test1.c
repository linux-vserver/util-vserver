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

#include "ensc_vector/vector.h"
#include <assert.h>

int	wrapper_exit_code = 2;

static int
cmp(void const *lhs_v, void const *rhs_v)
{
  int const * const	lhs = lhs_v;
  int const * const	rhs = rhs_v;

  return *lhs - *rhs;
}

struct Vector		v;

static void	I(int val)
{
  *(int *)Vector_insert(&v, &val, cmp) = val;
}

static void	P(int val)
{
  *(int *)Vector_pushback(&v) = val;
}

static int	E(size_t idx)
{
  return ((int const *)Vector_begin_const(&v))[idx];
}

static int const *	S(int val)
{
  return Vector_search_const(&v, &val, cmp);
}

int main()
{
  Vector_init(&v, sizeof(int));

  I(0); I(1); I(2); I(3);
  assert(Vector_count(&v)==4);
  assert(E(0)==0 && E(1)==1 && E(2)==2 && E(3)==3);

  // clear-test
  Vector_clear(&v);
  assert(Vector_count(&v)==0);
  I(1);
  assert(Vector_count(&v)==1);
  assert(E(0)==1);


  Vector_clear(&v);
  I(3); I(0); I(2); I(1); I(5); I(4); I(7); I(6);
  assert(Vector_count(&v)==8);
  assert((E(0)==0 && E(1)==1 && E(2)==2 && E(3)==3 &&
	  E(4)==4 && E(5)==5 && E(6)==6 && E(7)==7));

  assert(S(0) && *S(0)==0);
  
  
  Vector_clear(&v);
  assert(Vector_count(&v)==0);

  P(3); P(0); P(2); P(1); P(5); P(4); P(7); P(6);
  assert(Vector_count(&v)==8);
  assert((E(0)==3 && E(1)==0 && E(2)==2 && E(3)==1 &&
	  E(4)==5 && E(5)==4 && E(6)==7 && E(7)==6));
  
  Vector_sort(&v, cmp);
  assert(Vector_count(&v)==8);
  assert((E(0)==0 && E(1)==1 && E(2)==2 && E(3)==3 &&
	  E(4)==4 && E(5)==5 && E(6)==6 && E(7)==7));

  Vector_popback(&v);
  assert(Vector_count(&v)==7);
  assert((E(0)==0 && E(1)==1 && E(2)==2 && E(3)==3 &&
	  E(4)==4 && E(5)==5 && E(6)==6));

  Vector_unique(&v, cmp);
  assert(Vector_count(&v)==7);
  assert((E(0)==0 && E(1)==1 && E(2)==2 && E(3)==3 &&
	  E(4)==4 && E(5)==5 && E(6)==6));

  Vector_clear(&v);
  assert(Vector_count(&v)==0);

  Vector_clear(&v);
  P(3); P(7); P(0); P(2); P(1); P(2); P(5); P(4); P(5); P(7); P(6);
  assert(Vector_count(&v)==11);
  Vector_sort(&v, cmp);
  assert(Vector_count(&v)==11);
  assert((E(0)==0 && E(1)==1 && E(2)==2 && E(3)==2 &&
	  E(4)==3 && E(5)==4 && E(6)==5 && E(7)==5 &&
	  E(8)==6 && E(9)==7 && E(10)==7));

  Vector_unique(&v, cmp);
  assert(Vector_count(&v)==8);
  assert((E(0)==0 && E(1)==1 && E(2)==2 && E(3)==3 &&
	  E(4)==4 && E(5)==5 && E(6)==6 && E(7)==7)); 

  Vector_free(&v);
}
