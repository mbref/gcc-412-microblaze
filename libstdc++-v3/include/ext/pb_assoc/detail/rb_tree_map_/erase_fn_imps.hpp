// -*- C++ -*-

// Copyright (C) 2005 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

// Copyright (C) 2004 Ami Tavory and Vladimir Dreizin, IBM-HRL.

// Permission to use, copy, modify, sell, and distribute this software
// is hereby granted without fee, provided that the above copyright
// notice appears in all copies, and that both that copyright notice and
// this permission notice appear in supporting documentation. None of
// the above authors, nor IBM Haifa Research Laboratories, make any
// representation about the suitability of this software for any
// purpose. It is provided "as is" without express or implied warranty.

/**
 * @file erase_fn_imps.hpp
 * Contains an implementation for rb_tree_.
 */

PB_ASSOC_CLASS_T_DEC
inline typename PB_ASSOC_CLASS_C_DEC::size_type
PB_ASSOC_CLASS_C_DEC::
erase(const_key_reference r_key)
{
  iterator it = find(r_key);

  if (it == PB_ASSOC_BASE_C_DEC::find_end())
    return (0);

  erase(it);

  return (1);
}

PB_ASSOC_CLASS_T_DEC
inline typename PB_ASSOC_CLASS_C_DEC::const_iterator
PB_ASSOC_CLASS_C_DEC::
erase(const_iterator it)
{
  PB_ASSOC_DBG_ONLY(assert_valid());
  PB_ASSOC_DBG_ONLY(PB_ASSOC_BASE_C_DEC::assert_valid(true, false);)

    if (it == PB_ASSOC_BASE_C_DEC::find_end())
      return (it);

  const_iterator ret_it = it;

  ++ret_it;

  erase_node(it.m_p_nd);

  PB_ASSOC_DBG_ONLY(assert_valid());
  PB_ASSOC_DBG_ONLY(PB_ASSOC_BASE_C_DEC::assert_valid(true, false);)

    return (ret_it);
}

#ifdef PB_ASSOC_DATA_TRUE_INDICATOR
PB_ASSOC_CLASS_T_DEC
inline typename PB_ASSOC_CLASS_C_DEC::iterator
PB_ASSOC_CLASS_C_DEC::
erase(iterator it)
{
  PB_ASSOC_DBG_ONLY(PB_ASSOC_CLASS_C_DEC::assert_valid());
  PB_ASSOC_DBG_ONLY(PB_ASSOC_BASE_C_DEC::assert_iterators();)

    if (it == PB_ASSOC_BASE_C_DEC::find_end())
      return (it);

  iterator ret_it = it;

  ++ret_it;

  erase_node(it.m_p_nd);

  PB_ASSOC_DBG_ONLY(PB_ASSOC_CLASS_C_DEC::assert_valid());
  PB_ASSOC_DBG_ONLY(PB_ASSOC_BASE_C_DEC::assert_iterators();)

    return (ret_it);
}
#endif // #ifdef PB_ASSOC_DATA_TRUE_INDICATOR

PB_ASSOC_CLASS_T_DEC
inline typename PB_ASSOC_CLASS_C_DEC::const_reverse_iterator
PB_ASSOC_CLASS_C_DEC::
erase(const_reverse_iterator it)
{
  PB_ASSOC_DBG_ONLY(PB_ASSOC_CLASS_C_DEC::assert_valid());
  PB_ASSOC_DBG_ONLY(PB_ASSOC_BASE_C_DEC::assert_iterators();)

    if (it == PB_ASSOC_BASE_C_DEC::find_rend())
      return (it);

  const_reverse_iterator ret_it = it;

  ++ret_it;

  erase_node(it.m_p_nd);

  PB_ASSOC_DBG_ONLY(PB_ASSOC_CLASS_C_DEC::assert_valid());
  PB_ASSOC_DBG_ONLY(PB_ASSOC_BASE_C_DEC::assert_iterators();)

    return (ret_it);
}

#ifdef PB_ASSOC_DATA_TRUE_INDICATOR
PB_ASSOC_CLASS_T_DEC
inline typename PB_ASSOC_CLASS_C_DEC::reverse_iterator
PB_ASSOC_CLASS_C_DEC::
erase(reverse_iterator it)
{
  PB_ASSOC_DBG_ONLY(PB_ASSOC_CLASS_C_DEC::assert_valid());

  if (it == PB_ASSOC_BASE_C_DEC::find_rend())
    return (it);

  reverse_iterator ret_it = it;

  ++ret_it;

  erase_node(it.m_p_nd);

  PB_ASSOC_DBG_ONLY(PB_ASSOC_CLASS_C_DEC::assert_valid());

  return (ret_it);
}
#endif // #ifdef PB_ASSOC_DATA_TRUE_INDICATOR

PB_ASSOC_CLASS_T_DEC
template<class Pred>
inline typename PB_ASSOC_CLASS_C_DEC::size_type
PB_ASSOC_CLASS_C_DEC::
erase_if(Pred pred)
{
  PB_ASSOC_DBG_ONLY(PB_ASSOC_CLASS_C_DEC::assert_valid();)

    size_type num_ersd = 0;

  iterator it = PB_ASSOC_BASE_C_DEC::begin();

  while (it != PB_ASSOC_BASE_C_DEC::end())
    if (pred(*it))
      {
	++num_ersd;

	it = erase(it);
      }
    else
      ++it;

  PB_ASSOC_DBG_ONLY(PB_ASSOC_CLASS_C_DEC::assert_valid();)

    return (num_ersd);
}

PB_ASSOC_CLASS_T_DEC
void
PB_ASSOC_CLASS_C_DEC::
erase_node(node_pointer p_nd)
{
  remove_node(p_nd);

  PB_ASSOC_BASE_C_DEC::actual_erase_node(p_nd);

  PB_ASSOC_DBG_ONLY(assert_valid());
  PB_ASSOC_DBG_ONLY(PB_ASSOC_BASE_C_DEC::assert_valid(true, false);)
    }

PB_ASSOC_CLASS_T_DEC
void
PB_ASSOC_CLASS_C_DEC::
remove_node(node_pointer p_z)
{
  update_min_max_for_erased_node(p_z);

  node_pointer p_y = p_z;

  node_pointer p_x = NULL;

  node_pointer p_new_x_parent = NULL;

  if (p_y->m_p_left == NULL)
    p_x = p_y->m_p_right;
  else if (p_y->m_p_right == NULL)
    p_x = p_y->m_p_left;
  else
    {
      p_y = p_y->m_p_right;

      while (p_y->m_p_left != NULL)
	p_y = p_y->m_p_left;

      p_x = p_y->m_p_right;
    }

  if (p_y == p_z)
    {
      p_new_x_parent = p_y->m_p_parent;

      if (p_x != NULL)
	p_x->m_p_parent = p_y->m_p_parent;

      if (PB_ASSOC_BASE_C_DEC::m_p_head->m_p_parent == p_z)
	PB_ASSOC_BASE_C_DEC::m_p_head->m_p_parent = p_x;
      else if (p_z->m_p_parent->m_p_left == p_z)
	{
	  p_y->m_p_left = p_z->m_p_parent;

	  p_z->m_p_parent->m_p_left = p_x;
	}
      else
	{
	  p_y->m_p_left = NULL;

	  p_z->m_p_parent->m_p_right = p_x;
	}
    }
  else
    {
      p_z->m_p_left->m_p_parent = p_y;

      p_y->m_p_left = p_z->m_p_left;

      if (p_y != p_z->m_p_right)
	{
	  p_new_x_parent = p_y->m_p_parent;

	  if (p_x != NULL)
	    p_x->m_p_parent = p_y->m_p_parent;

	  p_y->m_p_parent->m_p_left = p_x;

	  p_y->m_p_right = p_z->m_p_right;

	  p_z->m_p_right->m_p_parent = p_y;
	}
      else
	p_new_x_parent = p_y;

      if (PB_ASSOC_BASE_C_DEC::m_p_head->m_p_parent == p_z)
	PB_ASSOC_BASE_C_DEC::m_p_head->m_p_parent = p_y;
      else if (p_z->m_p_parent->m_p_left == p_z)
	p_z->m_p_parent->m_p_left = p_y;
      else
	p_z->m_p_parent->m_p_right = p_y;

      p_y->m_p_parent = p_z->m_p_parent;

      std::swap(p_y->m_red, p_z->m_red);

      p_y = p_z;
    }

  update_to_top(p_new_x_parent, (Node_Updator* )this);

  if (p_y->m_red)
    return;

  remove_fixup(p_x, p_new_x_parent);
}

PB_ASSOC_CLASS_T_DEC
void
PB_ASSOC_CLASS_C_DEC::
remove_fixup(node_pointer p_x, node_pointer p_new_x_parent)
{
  PB_ASSOC_DBG_ASSERT(p_x == NULL || p_x->m_p_parent == p_new_x_parent);

  while (p_x != PB_ASSOC_BASE_C_DEC::m_p_head->m_p_parent&& 
	 is_effectively_black(p_x))
    if (p_x == p_new_x_parent->m_p_left)
      {
	node_pointer p_w = p_new_x_parent->m_p_right;

	if (p_w->m_red)
	  {
	    p_w->m_red = false;

	    p_new_x_parent->m_red = true;

	    PB_ASSOC_BASE_C_DEC::rotate_left(p_new_x_parent);

	    p_w = p_new_x_parent->m_p_right;
	  }

	if (is_effectively_black(p_w->m_p_left)&& 
	    is_effectively_black(p_w->m_p_right))
	  {
	    p_w->m_red = true;

	    p_x = p_new_x_parent;

	    p_new_x_parent = p_new_x_parent->m_p_parent;
	  }
	else
	  {
	    if (is_effectively_black(p_w->m_p_right))
	      {
		if (p_w->m_p_left != NULL)
		  p_w->m_p_left->m_red = false;

		p_w->m_red = true;

		PB_ASSOC_BASE_C_DEC::rotate_right(p_w);

		p_w = p_new_x_parent->m_p_right;
	      }

	    p_w->m_red = p_new_x_parent->m_red;

	    p_new_x_parent->m_red = false;

	    if (p_w->m_p_right != NULL)
	      p_w->m_p_right->m_red = false;

	    PB_ASSOC_BASE_C_DEC::rotate_left(p_new_x_parent);

	    update_to_top(p_new_x_parent, (Node_Updator* )this);

	    break;
	  }
      }
    else
      {
	node_pointer p_w = p_new_x_parent->m_p_left;

	if (p_w->m_red == true)
	  {
	    p_w->m_red = false;

	    p_new_x_parent->m_red = true;

	    PB_ASSOC_BASE_C_DEC::rotate_right(p_new_x_parent);

	    p_w = p_new_x_parent->m_p_left;
	  }

	if (is_effectively_black(p_w->m_p_right)&& 
	    is_effectively_black(p_w->m_p_left))
	  {
	    p_w->m_red = true;

	    p_x = p_new_x_parent;

	    p_new_x_parent = p_new_x_parent->m_p_parent;
	  }
	else
	  {
	    if (is_effectively_black(p_w->m_p_left))
	      {
		if (p_w->m_p_right != NULL)
		  p_w->m_p_right->m_red = false;

		p_w->m_red = true;

		PB_ASSOC_BASE_C_DEC::rotate_left(p_w);

		p_w = p_new_x_parent->m_p_left;
	      }

	    p_w->m_red = p_new_x_parent->m_red;

	    p_new_x_parent->m_red = false;

	    if (p_w->m_p_left != NULL)
	      p_w->m_p_left->m_red = false;

	    PB_ASSOC_BASE_C_DEC::rotate_right(p_new_x_parent);

	    update_to_top(p_new_x_parent, (Node_Updator* )this);

	    break;
	  }
      }

  if (p_x != NULL)
    p_x->m_red = false;
}
