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

/////////////////////////////
// region_node
//
// A representation of a region as a node.
//
// A region node may be used to build a pure layout tree
// or participate in a tree of plain nodes.
//
/////////////////////////////

#ifndef AMBULANT_SMIL2_REGION_NODE_H
#define AMBULANT_SMIL2_REGION_NODE_H

#include "ambulant/config/config.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/node_navigator.h"
#include "ambulant/lib/node_iterator.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/colors.h"
#include "ambulant/common/region_dim.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/layout.h"

namespace ambulant {

namespace smil2 {

enum dimension_inheritance { di_none, di_parent, di_rootlayout };
	

class region_node : public common::animation_destination {
  public:
	typedef lib::node_navigator<region_node> nnhelper;
	typedef lib::node_navigator<const region_node> const_nnhelper;

	///////////////////////////////
	// tree iterators
	typedef lib::tree_iterator<region_node> iterator;
	typedef lib::const_tree_iterator<region_node> const_iterator;
	
	// static method which tests whether a body node needs
	// a region counterpart (because it uses subregion positioning
	// or some such)
	static bool needs_region_node(const lib::node *n);
	
	// constructs a region node with local name and attrs
	region_node(const lib::node *n, dimension_inheritance di);
	virtual ~region_node();
	
	// Initialize data structures from DOM node attributes.
	bool fix_from_dom_node();

	// Tie together region and surface_template trees
	void set_surface_template(common::surface_template *surf) { m_surface_template = surf; }
	common::surface_template *get_surface_template() { return m_surface_template; }
	common::animation_notification *get_animation_notification(const lib::node *node) { return NULL /* m_surface_template */; };
	common::animation_destination *get_animation_destination() { return this; };
	
	// query for this region's rectangle
	// the rectangle is evaluaded on the fly
	// the evaluation takes into account relative coordinates
	lib::basic_rect<int> get_rect() const;
	lib::screen_rect<int> get_screen_rect() const;
	
	// gets the underlying region_dim_spec for modification
	common::region_dim_spec& rds() {return m_rds;}
	
	// Set and get where this node inherits dimension information from
	void set_dimension_inheritance(dimension_inheritance di) { m_dim_inherit = di; }
	dimension_inheritance get_dimension_inheritance() const { return m_dim_inherit; }
	
	// region_info implementation
	std::string get_name() const;
	common::fit_t get_fit() const { return m_fit; }
	lib::color_t get_bgcolor() const;
	bool get_transparent() const;
	bool get_showbackground() const;
	common::zindex_t get_zindex() const { return m_zindex; }
	bool is_subregion() const { return m_is_subregion; }
	// And corresponding setting interface
	void reset() {(void)fix_from_dom_node(); };
	void set_fit(common::fit_t f) { m_fit = f; }
	void set_bgcolor(lib::color_t c, bool transparent, bool inherit);
	void set_bgcolor(lib::color_t c) { set_bgcolor(c, false, false); };
	void set_showbackground(bool showbackground) { m_showbackground = showbackground; }
	void set_zindex(common::zindex_t z) { m_zindex = z; }
	void set_as_subregion(bool b) { m_is_subregion = b; }
	
	// sets explicitly the dimensions of this region
	template <class L, class W, class R, class T, class H, class B>
	void set_dims(L l, W w, R r, T t, H h, B b) {
		m_rds.left = l; m_rds.width = w;  m_rds.right = r;
		m_rds.top = t; m_rds.height = h;  m_rds.bottom = b;
	}
	
	///////////////////////////////
	// iterators

    iterator begin() { return iterator(this);}
    const_iterator begin() const { return const_iterator(this);}

    iterator end() { return iterator(0);}
    const_iterator end() const { return const_iterator(0);}

	// Std xml tree interface
	const region_node *down() const { return m_child;}
	const region_node *up() const { return m_parent;}
	const region_node *next() const { return m_next;}

	region_node *down()  { return m_child;}
	region_node *up()  { return m_parent;}
	region_node *next()  { return m_next;}

	void down(region_node *n)  { m_child = n;}
	void up(region_node *n)  { m_parent = n;}
	void next(region_node *n)  { m_next = n;}
	
	const region_node* previous() const {return const_nnhelper::previous(this);}
	region_node* previous() { return nnhelper::previous(this);}
	
	region_node* last_child() { return nnhelper::last_child(this);}
	const region_node* last_child() const { return const_nnhelper::last_child(this);}
	
	region_node* get_root() {return nnhelper::get_root(this);}
	const region_node* get_root() const {return const_nnhelper::get_root(this);}
	
	bool is_descendent_of(region_node *tn) const {return const_nnhelper::is_descendent(this, tn);}
	
	region_node *append_child(region_node *child) {return nnhelper::append_child(this, child);}
	void get_children(std::list<region_node*>& l) { nnhelper::get_children(this, l); }
	region_node *get_first_child(const char *name);
	const region_node *get_first_child(const char *name) const;
	
	const lib::node *dom_node() const { return m_node; }

	static int get_node_counter() { return node_counter;}
	
  private:
	
	const lib::node *m_node;
	common::region_dim_spec m_rds;
	dimension_inheritance m_dim_inherit;
	common::fit_t m_fit;
	common::zindex_t m_zindex;
	lib::color_t m_bgcolor;
	bool m_transparent;
	bool m_showbackground;
	bool m_inherit_bgcolor;
	common::surface_template *m_surface_template;
	bool m_is_subregion;
	static int node_counter;
	
	// XML tree glue
	region_node *m_parent;
	region_node *m_child;
	region_node *m_next;
};
} // namespace smil2
 
} // namespace ambulant

#endif // AMBULANT_SMIL2_REGION_NODE_H
