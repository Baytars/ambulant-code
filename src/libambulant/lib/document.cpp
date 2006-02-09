// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* 
 * @$Id$ 
 */

#include "ambulant/lib/document.h"
#include "ambulant/lib/tree_builder.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/filesys.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/lib/asb.h"

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

lib::document::document()
:	m_root(NULL),
	m_root_owned(false)
{
}

#if 0
lib::document::document(node *root, bool owned) 
:	m_root(root),
	m_root_owned(owned)
{
	build_id2node_map();
	read_custom_attributes();
}

lib::document::document(node *root, const net::url& src_url) 
:	m_root(root),
	m_root_owned(xxx),
	m_src_url(src_url)
{
	build_id2node_map();
	read_custom_attributes();
}
#endif

lib::document::~document() {
	if (m_root_owned) delete m_root;
}

lib::node* 
lib::document::get_root(bool detach) {
	if(!detach)
		return m_root;
	node* tmp = m_root;
	m_root = 0;
	return tmp;
}

const lib::node* 
lib::document::get_root() const {
	return m_root;
}

//static 
lib::document* 
lib::document::create_from_file(common::factories* factory, const std::string& filename) {
	document *d = new document();
	tree_builder builder(factory->get_node_factory(), d);
	if(!builder.build_tree_from_file(filename.c_str())) {
		// build_tree_from_file has reported the error already
		// logger::get_logger()->error(gettext("%s: Not a valid XML document"), filename.c_str());
		delete d;
		return 0;
	}
	if (!builder.assert_root_tag("smil")) {
		logger::get_logger()->error(gettext("%s: Not a SMIL document"), filename.c_str());
		delete d;
		return NULL;
	}
	d->set_root(builder.detach());
	d->set_src_url(ambulant::net::url::from_filename(filename));
	
//	std::string base = filesys::get_base(filename, file_separator.c_str());
//	d->set_src_base(ambulant::net::url(base));
	
	return d;
}

//static 
lib::document* 
lib::document::create_from_url(common::factories* factory, const net::url& u) {
	document *d = new document();
	tree_builder builder(factory->get_node_factory(), d);
	char *data;
	size_t datasize;
	if (!net::read_data_from_url(u, factory->get_datasource_factory(), &data, &datasize)) {
		logger::get_logger()->error(gettext("%s: Not a valid XML document"), u.get_url().c_str());
		delete d;
		return NULL;
	}
	if(!builder.build_tree_from_str(data, data+datasize)) {
		// build_tree_from_url has already given the error message
		// logger::get_logger()->error(gettext("%s: Not a valid XML document"), u.get_url().c_str());
		delete d;
		return NULL;
	}
	if (data) free(data);
	if (!builder.assert_root_tag("smil")) {
		delete d;
		logger::get_logger()->error(gettext("%s: Not a SMIL document"), u.get_url().c_str());
		return NULL;
	}
	d->set_root(builder.detach());
	d->set_src_url(u);
	return d;
}

//static 
lib::document* 
lib::document::create_from_string(common::factories* factory, const std::string& smil_src, const std::string& src_id) {
	document *d = new document();
	tree_builder builder(factory->get_node_factory(), d, src_id.c_str());
	if(!builder.build_tree_from_str(smil_src)) {
		logger::get_logger()->error(gettext("%s: Not a valid XML document"), src_id.c_str());
		delete d;
		return NULL;
	}
	if (!builder.assert_root_tag("smil")) {
		logger::get_logger()->error(gettext("%s: Not a SMIL document"), src_id.c_str());
		delete d;
		return NULL;
	}
	d->set_root(builder.detach());
	return d;
}

#if 0
//static 
lib::document* 
lib::document::create_from_tree(common::factories* factory, lib::node *root, const net::url& u) {
	if (root->get_local_name() != "smil" ) {
		logger::get_logger()->error(gettext("%s: Not a SMIL document"), u.get_url().c_str());
		return NULL;
	}
	document *d = new document(root, false);
	d->set_root(root); // This fills the id2node map and such
	d->set_src_url(u);
	return d;
}
#endif

void 
lib::document::set_prefix_mapping(const std::string& prefix, const std::string& uri) {
	m_namespaces.set_prefix_mapping(prefix, uri);
}

const char* 
lib::document::get_namespace_prefix(const xml_string& uri) const {
	return m_namespaces.get_namespace_prefix(uri);
}

net::url 
lib::document::resolve_url(const net::url& rurl) const {
	net::url loc(rurl);
	if (loc.is_absolute()) {
		AM_DBG lib::logger::get_logger()->debug("document::resolve_url(%s): absolute URL", repr(rurl).c_str());
		return rurl;
	}
	net::url rv(rurl.join_to_base(m_src_url));
	AM_DBG lib::logger::get_logger()->debug("document::resolve_url(%s): %s\n", repr(rurl).c_str(), repr(rv).c_str());
	return rv;
}

void lib::document::set_root(node* n) {
	if(m_root_owned && m_root != n) delete m_root;
	m_root_owned = true;
	m_root = n;
	build_id2node_map();
	read_custom_attributes();
}

void
lib::document::tree_changed() {
	set_root(m_root);
}

void lib::document::build_id2node_map() {
	m_id2node.clear();
	if(!m_root) return;
	lib::node::iterator it;
	lib::node::iterator end = m_root->end();
	for(it = m_root->begin(); it != end; it++) {
		bool start_element = (*it).first;
		const lib::node *n = (*it).second;
		if(start_element) {
			const char *pid = n->get_attribute("id");
			if(pid) {
				const node *o = get_node(pid);
				if(o) logger::get_logger()->trace("Duplicate id: %s", pid);
				else m_id2node[pid] = n;
			}
		}
	}
}

void lib::document::read_custom_attributes() {
	if(!m_root) return;
	m_custom_tests.clear();
	const lib::node* ca = locate_node("/smil/head/customAttributes");
	if(!ca) return;
	lib::node::const_iterator it;
	lib::node::const_iterator end = ca->end();
	for(it = ca->begin(); it != end; it++) {
		std::pair<bool, const lib::node*> pair = *it;
		bool start_element = pair.first;
		const lib::node *n = pair.second;
		const std::string& tag = n->get_local_name();
		if(tag != "customTest") continue;
		const char *p = n->get_attribute("id");
		if(start_element && p) {
			std::string id = to_c_lower(p);
			custom_test t;
			t.idd = id;
			p = n->get_attribute("defaultState");
			std::string s = p?p:"";
			t.state = (s == "true")?true:false;
			p = n->get_attribute("title");
			t.title = p?p:"";
			p = n->get_attribute("override");
			s = p?p:"";
			t.override = (s=="visible")?true:false;
			p = n->get_attribute("uid");
			t.uid = p?p:""; 
			m_custom_tests[id] = t;
			AM_DBG logger::get_logger()->debug("Custom test: %s", ::repr(t).c_str());
		}
	}
}
