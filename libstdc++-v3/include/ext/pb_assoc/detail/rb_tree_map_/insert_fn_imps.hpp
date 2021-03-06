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

/*
 * @file insert_fn_imps.hpp
 * Contains an implementation for rb_tree_.
 */

PB_ASSOC_CLASS_T_DEC
inline std::pair<typename PB_ASSOC_CLASS_C_DEC::find_iterator, bool>
PB_ASSOC_CLASS_C_DEC::
insert(const_reference r_value)
{
  PB_ASSOC_DBG_ONLY(PB_ASSOC_CLASS_C_DEC::assert_valid();)

    std::pair<find_iterator, bool> ins_pair =
    PB_ASSOC_BASE_C_DEC::insert_leaf(r_value);

  if (ins_pair.second == true)
    {
      ins_pair.first.m_p_nd->m_red = true;

      PB_ASSOC_DBG_ONLY(PB_ASSOC_BASE_C_DEC::assert_valid(false, true);)

	insert_fixup(ins_pair.first.m_p_nd);
    }

  PB_ASSOC_DBG_ONLY(PB_ASSOC_BASE_C_DEC::assert_valid(true, true);)
    PB_ASSOC_DBG_ONLY(PB_ASSOC_CLASS_C_DEC::assert_valid());

  return (ins_pair);
}

#ifdef PB_ASSOC_DATA_TRUE_INDICATOR
PB_ASSOC_CLASS_T_DEC
inline typename PB_ASSOC_CLASS_C_DEC::data_reference
PB_ASSOC_CLASS_C_DEC::
subscript_imp(const_key_reference r_key)
{
  PB_ASSOC_DBG_ONLY(PB_ASSOC_CLASS_C_DEC::assert_valid();)

    std::pair<find_iterator, bool> ins_pair =
    PB_ASSOC_BASE_C_DEC::insert_leaf(
				     value_type(r_key, data_type()));

  if (ins_pair.second == true)
    {
      ins_pair.first.m_p_nd->m_red = true;

      PB_ASSOC_DBG_ONLY(PB_ASSOC_BASE_C_DEC::assert_valid());

      insert_fixup(ins_pair.first.m_p_nd);
    }

  PB_ASSOC_DBG_ONLY(PB_ASSOC_CLASS_C_DEC::assert_valid());
  PB_ASSOC_DBG_ONLY(PB_ASSOC_BASE_C_DEC::assert_iterators();)

    return (ins_pair.first.m_p_nd->m_value.second);
}
#endif // #ifdef PB_ASSOC_DATA_TRUE

PB_ASSOC_CLASS_T_DEC
inline void
PB_ASSOC_CLASS_C_DEC::
insert_fixup(node_pointer p_nd)
{
  PB_ASSOC_DBG_ASSERT(p_nd->m_red == true);

  while (p_nd != PB_ASSOC_BASE_C_DEC::m_p_head->m_p_parent&& 
	 p_nd->m_p_parent->m_red)
    {
      if (p_nd->m_p_parent == p_nd->m_p_parent->m_p_parent->m_p_left)
	{
	  node_pointer p_y = p_nd->m_p_parent->m_p_parent->m_p_right;

	  if (p_y != NULL&&  p_y->m_red)
	    {
	      p_nd->m_p_parent->m_red = false;

	      p_y->m_red = false;

	      p_nd->m_p_parent->m_p_parent->m_red = true;

	      p_nd = p_nd->m_p_parent->m_p_parent;
	    }
	  else
	    {
	      if (p_nd == p_nd->m_p_parent->m_p_right)
		{
		  p_nd = p_nd->m_p_parent;

		  PB_ASSOC_BASE_C_DEC::rotate_left(p_nd);
		}

	      p_nd->m_p_parent->m_red = false;

	      p_nd->m_p_parent->m_p_parent->m_red = true;

	      PB_ASSOC_BASE_C_DEC::rotate_right(
						p_nd->m_p_parent->m_p_parent);
	    }
	}
      else
	{
	  node_pointer p_y = p_nd->m_p_parent->m_p_parent->m_p_left;

	  if (p_y != NULL&&  p_y->m_red)
	    {
	      p_nd->m_p_parent->m_red = false;

	      p_y->m_red = false;

	      p_nd->m_p_parent->m_p_parent->m_red = true;

	      p_nd = p_nd->m_p_parent->m_p_parent;
	    }
	  else
	    {
	      if (p_nd == p_nd->m_p_parent->m_p_left)
		{
		  p_nd = p_nd->m_p_parent;

		  PB_ASSOC_BASE_C_DEC::rotate_right(p_nd);
		}

	      p_nd->m_p_parent->m_red = false;

	      p_nd->m_p_parent->m_p_parent->m_red = true;

	      PB_ASSOC_BASE_C_DEC::rotate_left(
					       p_nd->m_p_parent->m_p_parent);
	    }
	}
    }

  PB_ASSOC_BASE_C_DEC::update_to_top(p_nd, (Node_Updator* )this);

  PB_ASSOC_BASE_C_DEC::m_p_head->m_p_parent->m_red = false;
}

