/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_LIB_NODE_ITERATOR_H
#define AMBULANT_LIB_NODE_ITERATOR_H

#include "ambulant/config/config.h"

// pair
#include <utility>

namespace ambulant {

namespace lib {

/// Node tree iterator.
///
/// Traverses a constant XML tree.
///
/// Root designates the container to be traversed and remains const.
/// Each node is visited twice: once in the 'down' direction and once in the 'up-next'.
///
/// the first element is: (true, root) 
/// the last element is: (false, root) 
/// incrementing last element results to the universal 'end' element (0, *).
///
/// (*it).first is true or false depending on the direction
/// (*it).second is a pointer to the node.
template <class Node>
class const_tree_iterator  {

  //////////////
  public:

	/// The type of the values returned by this iterator.
	typedef std::pair<bool, const Node*> const_deref_type;

	const_tree_iterator()
	:	m_root(0), m_cur(0), m_move(&const_tree_iterator::down) {}
		
	const_tree_iterator(const Node *p)
	:	m_root(p), m_cur(p), m_move(&const_tree_iterator::down) {}

	// pre-increment
	const_tree_iterator& operator++() { if(m_cur)(this->*m_move)(); return *this;}
	
	// post-increment
	const_tree_iterator operator++(int) 
		{ const_tree_iterator temp(*this); ++*this; return temp;}

	// dereferencing this returns a pair of const_deref_type
	const_deref_type operator*() { return const_deref_type((m_move == &const_tree_iterator::down), m_cur);}

	// an instance of this object acts like a pointer to a const_deref_type
	// it->m_cur is the node, it->m_move is true if 'down'
    const_deref_type* operator->() {return (&**this); }

	/// True if iterator is exhausted.
	bool is_end() { return m_cur == 0;}

	/// Returns the current node.
	const Node *get_cur() const { return m_cur;}
	
	/// Returns the root of this iterator.
 	const Node *get_root() const { return m_root;}
	
	/// Returns true if the two iterators move in the same direction.
  	bool same_move(const const_tree_iterator& o) const 
  		{ return m_move == o.m_move;}
  	
///////////////
  protected:
	void down();
	void next();
	void up();

	// container traversed by this iterator
	const Node *m_root;
	
	// iterator state
	const Node *m_cur;
	void (const_tree_iterator::*m_move)();
};

////////////////////////
// tree_iterator

/// Node tree iterator.
///
/// Traverses an XML tree.
///
/// Root designates the container to be traversed and remains const.
/// Each node is visited twice: once in the 'down' direction and once in the 'up-next'.
///
/// the first element is: (true, root) 
/// the last element is: (false, root) 
/// incrementing last element results to the universal 'end' element (0, *).
///
/// (*it).first is true or false depending on the direction
/// (*it).second is a pointer to the node.
template <class Node>
class tree_iterator : public const_tree_iterator<Node> {
  public:

	/// The type of the values returned by this iterator.
	typedef std::pair<bool, Node*> deref_type;

	tree_iterator() : const_tree_iterator<Node>(){}
	tree_iterator(Node *p) : const_tree_iterator<Node>(p){}

	// pre-increment
	tree_iterator& operator++() { if(m_cur)(this->*m_move)(); return *this;}
	
	// post-increment
	tree_iterator operator++(int) 
		{ tree_iterator temp(*this); ++*this; return temp;}

	// dereferencing this returns a pair of deref_type
	deref_type operator*() 
		{ return deref_type( (m_move == &tree_iterator::down), const_cast<Node*>(m_cur));}

	// an instance of this object acts like a pointer to a deref_type
	// it->m_cur is the node, it->m_move is true if 'down'
    deref_type* operator->() {return (&**this); }
};

////////////////////////////
// const_tree_iterator inline implementation

template<class Node>
inline void const_tree_iterator<Node>::down() {
	const Node *p = m_cur->down();
	if(!p) m_move = &const_tree_iterator::next;
	else m_cur = p;
}

template<class Node>
inline void const_tree_iterator<Node>::next() {
	if(m_cur == m_root) { m_cur = 0; return;}
		const Node* it = m_cur->next();
	if(it) {
		m_move = &const_tree_iterator::down;
		m_cur = it;
	} else {
		m_move = &const_tree_iterator::up;
		(this->*m_move)();
	}
}

template<class Node>
inline void const_tree_iterator<Node>::up() {
	if(m_cur == m_root) { m_cur = 0; return;}
	const Node* it = m_cur->up();
	if(it) {
		m_move = &const_tree_iterator::next;
		m_cur = it;
	}
}

} // namespace lib
 
} // namespace ambulant

/// Returns true if two iterators are in exactly the same state on the same tree.
template <class Node>
bool operator==(const ambulant::lib::const_tree_iterator<Node>& lhs, 
	const ambulant::lib::const_tree_iterator<Node>& rhs) {
		if((lhs.get_cur() == 0 && rhs.get_cur() != 0) || 
			(lhs.get_cur() != 0 && rhs.get_cur() == 0)) return false;
		if(lhs.get_cur() == 0 && rhs.get_cur() == 0) return true;
		return lhs.get_root() == rhs.get_root() && 
			lhs.get_cur() == rhs.get_cur() && 
			lhs.same_move(rhs);
	}

template <class Node>
bool operator!=(const ambulant::lib::const_tree_iterator<Node>& lhs, 
	const ambulant::lib::const_tree_iterator<Node>& rhs)
	{ return !(lhs == rhs);}
	
/// Returns true if two iterators are in exactly the same state on the same tree.
template <class Node>
bool operator==(const ambulant::lib::tree_iterator<Node>& lhs, 
	const ambulant::lib::tree_iterator<Node>& rhs) {
		if((lhs.get_cur() == 0 && rhs.get_cur() != 0) || 
			(lhs.get_cur() != 0 && rhs.get_cur() == 0)) return false;
		if(lhs.get_cur() == 0 && rhs.get_cur() == 0) return true;
		return lhs.get_root() == rhs.get_root() && 
			lhs.get_cur() == rhs.get_cur() && 
			lhs.same_move(rhs);
	}

template <class Node>
bool operator!=(const ambulant::lib::tree_iterator<Node>& lhs, 
	const ambulant::lib::tree_iterator<Node>& rhs)
	{ return !(lhs == rhs);}


#endif // AMBULANT_LIB_TREE_NODE_ITERATOR_H
