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

#include "ambulant/smil2/smil_time.h"
#include "ambulant/smil2/time_node.h"

using namespace ambulant;

////////////////////////////
// q_smil_time implementation

// The input and ouput times are simple element times.
// This implementation ignores 
// a) the accumulated synchronization offset 
// b) time manipulations effects

lib::q_smil_time::time_type lib::q_smil_time::to_ancestor(const lib::time_node *a) {
	while(first != a && first->up()) up();
	if(first != a) 
		logger::get_logger()->error("q_smil_time::convert_to_ancestor(%s) failed", 
			a->to_string().c_str());
	return second;
}

lib::q_smil_time::time_type lib::q_smil_time::to_descendent(const lib::time_node *d) {
	const lib::time_node *a = first;
	std::list<const lib::time_node*> path;
	typedef lib::node_navigator<const lib::time_node> const_nnhelper;
	const_nnhelper::get_path(d, path);
	std::list<const lib::time_node*>::iterator it = 
		std::find(path.begin(), path.end(), a);
	if(it == path.end()) {
		logger::get_logger()->error("q_smil_time::convert_to_descendent(%s) failed", 
			d->to_string().c_str());
		return second;
	}
	for(it++;it!=path.end();it++) down(*it);
	return second;
}

lib::q_smil_time::time_type lib::q_smil_time::to_node(const lib::time_node *n) {
	typedef lib::node_navigator<const lib::time_node> const_nnhelper;
	const lib::time_node *ca = const_nnhelper::get_common_ancestor(n, first);
	to_ancestor(ca);
	return to_descendent(n);
}

lib::q_smil_time::time_type lib::q_smil_time::to_doc() {
	while(up());
	return second;
}

bool lib::q_smil_time::up() {
	if(first->up()) {
		second += time_type(first->get_rad()) + first->get_interval().begin;
		first = first->up();
	}
	return first->up() != 0;
}
	
void lib::q_smil_time::down(const lib::time_node *child) {
	first = child;
	second -= time_type(first->get_rad()) + first->get_interval().begin;
}

lib::q_smil_time::time_type 
lib::q_smil_time::as_node_time(const time_node *n) const {
	q_smil_time qt = *this;
	return qt.to_node(n);
}

lib::q_smil_time::time_type 
lib::q_smil_time::as_doc_time() const {
	q_smil_time qt = *this;
	return qt.to_doc();
}

lib::q_smil_time::time_type 
lib::q_smil_time::as_time_down_to(const time_node *n) const {
	if(first == n->up()) {
		// down from parent
		return second - time_type(n->get_rad()) - n->get_interval().begin;
	} else if(!n->up()) {
		// root
		return second;
	}
	q_smil_time qt = *this;
	return qt.to_descendent(n);
}

lib::q_smil_time 
lib::q_smil_time::as_qtime_down_to(const time_node *n) const {
	time_type t = as_time_down_to(n);
	return q_smil_time(n, t);
}

//static 
lib::q_smil_time::time_type 
lib::q_smil_time::to_sync_time(const time_node *n, const time_type& n_simple) {
	return n_simple + time_type(n->get_rad()) + n->get_interval().begin;
}
