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

#include "ambulant/lib/logger.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/document.h"
#include "ambulant/common/schema.h"
#include "ambulant/common/region_dim.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/preferences.h"
#include "ambulant/smil2/region_node.h"
#include "ambulant/smil2/smil_layout.h"
#include <stack>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

// Factory function
common::layout_manager *
common::create_smil2_layout_manager(common::window_factory *wf,lib::document *doc)
{
	return new smil_layout_manager(wf, doc);
}

smil_layout_manager::smil_layout_manager(common::window_factory *wf,lib::document *doc)
:   m_schema(common::schema::get_instance()),
	m_surface_factory(common::create_smil_surface_factory())
{
	fix_document_layout(doc);
	const region_node *layout_root = NULL; // doc->get_layout();

	if (layout_root) {
		build_layout_tree(wf, layout_root);
	} else {
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: no layout section");
	}
	// Finally we make sure there is at least one root-layout. This allows us
	// to use this as the default region. XXXX Should be auto-show eventually.
	if (m_rootlayouts.empty()) {
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: no rootLayouts, creating one");
		create_top_surface(wf, NULL, NULL);
	}
}

smil_layout_manager::~smil_layout_manager()
{
	m_schema = NULL;
	std::vector<common::surface_template*>::iterator i;
	for (i=m_rootlayouts.begin(); i != m_rootlayouts.end(); i++)
		delete (*i);
}

void
smil_layout_manager::fix_document_layout(lib::document *doc)
{
	// If we have a layout section already we're done
	if (doc->get_layout()) {
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: using existing layout");
		return;
	}
	// Otherwise we first have to find the layout section in the tree
	lib::node *doc_root = doc->get_root();
	lib::node *head = doc_root->get_first_child("head");
	if (!head) {
		lib::logger::get_logger()->warn("smil_layout_manager: no <head> section");
		return;
	}
	lib::node *layout_root = head->get_first_child("layout");
	region_node *layout = NULL; 
	if (!layout_root) {
		lib::logger::get_logger()->trace("smil_layout_manager: no <layout> section");
		return;
	}
	// Now we iterate over all the elements, set their dimensions
	// from the attributes and determine their inheritance
	lib::node::iterator it;
	lib::node::const_iterator end = layout_root->end();
	int level = -1;
	for(it = layout_root->begin(); it != end; it++) {
		std::pair<bool, lib::node*> pair = *it;
		if (pair.first) {
			level++;
			if (level == 0) continue; // Skip layout section itself
			lib::node *n = pair.second;
			// Find inheritance type
			common::layout_type tp = m_schema->get_layout_type(n->get_qname());
			dimension_inheritance di;
			if (tp == common::l_rootlayout || tp == common::l_toplayout) {
				di = di_none;
			} else if (level == 1) {
				// Toplevel region node
				di = di_rootlayout;
			} else {
				// lower-level region node
				di = di_parent;
			}

			region_node *rn = new region_node(n, di);
			rn->fix_from_dom_node();
			// XXXX Tie into tree and set layout
		} else {
			level--;
		}
	}

	// XXXX Undecided on what to do for subregion positioning: maybe best to
	// simply create new subregions with "impossible" ids.
	AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: setting layout");
	doc->set_layout(layout_root); // XXXX Should be layout
}

void
smil_layout_manager::build_layout_tree(common::window_factory *wf, const region_node *layout_root) {
	std::stack<common::surface_template*> stack;
	region_node::const_iterator it;
	region_node::const_iterator end = layout_root->end();
	
	AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::build_layout_tree called");
	// First we check for a root-layout node. This will be used as the parent
	// of toplevel region nodes. If there is no root-layout but there are
	// toplevel region nodes we will create it later.
	common::surface_template *root_layout = NULL;
	const region_node *rrlnode = layout_root->get_first_child("root-layout");
	if (rrlnode) {
		common::renderer *bgrenderer = wf->new_background_renderer(rrlnode);
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::build_layout_tree: create root_layout");
		root_layout = create_top_surface(wf, rrlnode, bgrenderer);
	}
		
	// Loop over all the layout elements, create the regions and root_layouts,
	// and keep a stack to tie everything together.
	for(it = layout_root->begin(); it != end; it++) {
		std::pair<bool, const region_node*> pair = *it;
		const region_node *rn = pair.second;
		const lib::node *n = rn->dom_node();
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: examining %s node", n->get_qname().second.c_str());
		common::layout_type tag = m_schema->get_layout_type(n->get_qname());
		if(tag == common::l_none || tag == common::l_rootlayout) {
			// XXXX Will need to handle switch here
			continue;
		}
		if(pair.first) {
			// On the way down we create the regions and remember
			// them
			common::renderer *bgrenderer = wf->new_background_renderer(rn);
			common::surface_template *rgn;
			const char *pid = n->get_attribute("id");
			std::string ident = "<unnamed>";
			if(pid) {
				ident = pid;
			}
			// Test that rootlayouts are correctly nested.
			if (!stack.empty()) {
				if (tag != common::l_region) {
					lib::logger::get_logger()->error("topLayout element inside other element: %s", ident.c_str());
					tag = common::l_region;
				}
			}
			// Create the region or the root-layout
			if (tag == common::l_toplayout) {	
				AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::build_layout_tree: create topLayout");
				common::surface_template *rootrgn = create_top_surface(wf, rn, bgrenderer);
				rgn = rootrgn;
			} else if (tag == common::l_region && !stack.empty()) {
				common::surface_template *parent = stack.top();
				rgn = parent->new_subsurface(rn, bgrenderer);
			} else if (tag == common::l_region && stack.empty()) {
				// Create root-layout if it doesn't exist yet
				if (root_layout == NULL) {
					AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::build_layout_tree: create default root-layout");
					root_layout = create_top_surface(wf, NULL, NULL);
				}
				rgn = root_layout->new_subsurface(rn, bgrenderer);
			} else {
				assert(0);
			}
			
			// Enter the region ID into the id-mapping
			if(pid) {
				AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: mapping id %s", pid);
				m_id2surface[ident] = rgn;
			}
			
			// And the same for the regionName multimap
			const char *pname = n->get_attribute("regionName");
			if(pname) {
				AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: mapping regionName %s", pname);
				std::string name;
				name = pname;
				m_name2surface.insert(std::pair<std::string, common::surface_template*>(name, rgn));
			}
			// Finally push on to the stack for reference by child nodes
			stack.push(rgn);
		} else {
			// On the way back up we only need to pop the stack
			stack.pop();
		}
	}
}

common::surface_template *
smil_layout_manager::create_top_surface(common::window_factory *wf, const region_node *rn, common::renderer *bgrenderer)
{
	common::surface_template *rootrgn;
	rootrgn = m_surface_factory->new_topsurface(rn, bgrenderer, wf);
	m_rootlayouts.push_back(rootrgn);
	return rootrgn;
}

common::surface *
smil_layout_manager::get_surface(const lib::node *n) {
	// XXXX This code is blissfully unaware of subregion positioning right now.
	const char *prname = n->get_attribute("region");
	const char *nid = n->get_attribute("id");
	if (prname == NULL) {
		AM_DBG lib::logger::get_logger()->trace(
			"smil_layout_manager::get_surface(): no region attribute on %s",
			(nid?nid:""));
		return get_default_rendering_surface(n);
	}
	std::string rname = prname;
	std::map<std::string, common::surface_template*>::size_type namecount = m_name2surface.count(rname);
	if (namecount > 1)
		lib::logger::get_logger()->warn("smil_layout_manager::get_surface(): Using first region %s only", prname);
	if (namecount > 0) {
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::get_surface(): matched %s by regionName", prname);
		return (*m_name2surface.find(rname)).second->activate();
	}
	std::multimap<std::string, common::surface_template*>::size_type idcount = m_id2surface.count(rname);
	if (idcount > 0) {
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::get_surface(): matched %s by id", prname);
		return m_id2surface[rname]->activate();
	}
	AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::get_surface(): no match for %s", prname);
	return get_default_rendering_surface(n);
}

common::surface *
smil_layout_manager::get_default_rendering_surface(const lib::node *n) {
	const char *nid = n->get_attribute("id");
	lib::logger::get_logger()->warn("Returning default rendering surface for node %s", (nid?nid:""));
	return m_rootlayouts[0]->activate();
}


