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

#include "ambulant/common/region_eval.h"
#include "ambulant/smil2/region_node.h"

using namespace ambulant;
using namespace smil2;

region_node::region_node(const lib::node *n)
:	m_node(n),
	m_dim_inherit(di_parent),
	m_fit(common::fit_hidden),
	m_zindex(0),
	m_bgcolor(lib::to_color(0,0,0)),
	m_transparent(true),
	m_showbackground(true),
	m_inherit_bgcolor(false) {}
 
lib::basic_rect<int>
region_node::get_rect() const {
	const region_node *inherit_region = NULL;
	const region_node *parent_node = up();
	switch(m_dim_inherit) {
	  case di_parent:
		if (parent_node)
			inherit_region = parent_node;
		break;
	  case di_region_attribute:
	    inherit_region = NULL; // XXXX
		break;
	  case di_rootlayout:
		{
			const region_node *root_node = get_root();
			const region_node *rootlayout_node = root_node->get_first_child("root-layout");
			if (rootlayout_node)
				inherit_region = rootlayout_node;
		}
		break;
	  case di_none:
		break;
	}
	if(inherit_region == NULL) {
		int w = m_rds.width.get_as_int();
		int h = m_rds.height.get_as_int();
		
		return lib::basic_rect<int, int>(lib::basic_size<int>(w, h)); 
	}
	lib::basic_rect<int> rc = inherit_region->get_rect();
	common::region_evaluator re(rc.w, rc.h);
	re.set(m_rds);
	return re.get_rect();
}
 
lib::screen_rect<int>
region_node::get_screen_rect() const {
	return lib::screen_rect<int>(get_rect());
}

std::string
region_node::get_name() const {
	const char *pid = m_node->get_attribute("id");
	if (pid) return pid;
	return "";
}

lib::color_t
region_node::get_bgcolor() const
{
	if(m_inherit_bgcolor) {
		const region_node *parent_node = up();
		if (parent_node)
			return parent_node->get_bgcolor();
	}
	return m_bgcolor;
}

bool
region_node::get_transparent() const
{
	return m_transparent;
}

bool
region_node::get_showbackground() const
{
	return m_showbackground;
}

void
region_node::set_bgcolor(lib::color_t c, bool transparent, bool inherit) { 
	m_bgcolor = c;
	m_transparent = transparent;
	m_inherit_bgcolor = inherit;
}
