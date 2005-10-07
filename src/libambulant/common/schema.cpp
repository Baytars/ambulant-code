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

#include "ambulant/lib/logger.h"
#include "ambulant/common/schema.h"
#include <cmath>

using namespace ambulant;
using namespace common;

// Use hard coding for now
static const char* time_containers[] = {
	"body", "seq", "par", "excl"
};
static const char* discrete_leafs[] = {
	"text", "img", "ref", "brush"
};

static const char* continuous_leafs[] = {
	"audio", "animation", "video", "textstream", "area"
};

static const char* animate_elements[] = {
	"animate", "animateColor", "animateMotion", "set"
};

static const char* layout_elements[] = {
	"root-layout", "topLayout", "region"
};

// Create the smplest possible schema factory
// Its sole purpose is to create privately the schema singleton.
namespace ambulant {
namespace common {
class schema_factory {
  public:
	static schema schema_inst;
};
} // namespace common
} // namespace ambulant

//static 
schema schema_factory::schema_inst;

// static 
const schema* 
schema::get_instance() { 
	return &schema_factory::schema_inst;
}

schema::schema() {
	int n = sizeof(time_containers)/sizeof(const char *);
	int i;
	for(i =0;i<n;i++)
		m_time_elements.insert(time_containers[i]);
	
	n = sizeof(discrete_leafs)/sizeof(const char *);
	for(i =0;i<n;i++) {
		m_time_elements.insert(discrete_leafs[i]);
		m_discrete.insert(discrete_leafs[i]);
	}

	n = sizeof(continuous_leafs)/sizeof(const char *);
	for(i =0;i<n;i++) {
		m_time_elements.insert(continuous_leafs[i]);
		m_continuous.insert(continuous_leafs[i]);
	}
	
	n = sizeof(animate_elements)/sizeof(const char *);
	for(i =0;i<n;i++) {
		m_time_elements.insert(animate_elements[i]);
		m_continuous.insert(animate_elements[i]);
		m_animations.insert(animate_elements[i]);
	}
		
	n = sizeof(layout_elements)/sizeof(const char *);
	for(i=0; i<n; i++) {
		m_layout_elements.insert(layout_elements[i]);
	}
}

schema::~schema() {
	// currently all objects allocated are auto-destr
}

// Returns one of: tc_par | tc_seq | tc_excl | tc_none
time_container_type 
schema::get_time_type(const lib::q_name_pair& qname) const {
	time_container_type type = tc_none;
	if(qname.second == "seq" || qname.second == "body") type = tc_seq;
	else if(qname.second == "par") type = tc_par;
	else if(qname.second == "excl") type = tc_excl;
	return type;
}

bool schema::is_discrete(const lib::q_name_pair& qname) const {
	return m_discrete.find(qname.second) != m_discrete.end();
}

bool schema::is_animation(const lib::q_name_pair& qname) const {
	return m_animations.find(qname.second) != m_animations.end();
}

const char* 
ambulant::common::time_container_type_as_str(time_container_type t) {
	switch(t) {
		case tc_par: return "par";
		case tc_seq: return "seq";
		case tc_excl: return "excl";
		default: return "none";
	}
}

// Returns one of: l_rootlayout, l_region or l_none
layout_type 
schema::get_layout_type(const lib::q_name_pair& qname) const {
	layout_type type = l_none;
	if(qname.second == "layout" ) type = l_layout;
	else if(qname.second == "root-layout" ) type = l_rootlayout;
	else if(qname.second == "topLayout") type = l_toplayout;
	else if(qname.second == "region") type = l_region;
	else if(qname.second == "regPoint") type = l_regpoint;
	else if(m_discrete.find(qname.second) != m_discrete.end()) type = l_media;
	else if(m_continuous.find(qname.second) != m_continuous.end()) type = l_media;
	return type;
}

const char* 
layout_type_as_str(layout_type t) {
	switch(t) {
		case l_layout: return "layout";
		case l_rootlayout: return "root-layout";
		case l_toplayout: return "topLayout";
		case l_region: return "region";
		case l_regpoint: return "regpoint";
		default: return "none";
	}
}


