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

#include "ambulant/smil2/animate_a.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/colors.h"
#include "ambulant/common/region_dim.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/parselets.h"

#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

animate_attrs::animate_attrs(const lib::node *n, const lib::node* tparent)
:	m_node(n),
	m_tparent(tparent), 
	m_target(0) {
	m_logger = lib::logger::get_logger();
	const char *pid = m_node->get_attribute("id");
	m_id = (pid?pid:"no-id");	
	m_tag = m_node->get_local_name();
	locate_target_element();
	locate_target_attr();
	m_animtype = find_anim_type();
	read_enum_atttrs();
	AM_DBG m_logger->trace("%s[%s].%s --> %s.%s", 
		m_tag.c_str(), m_id.c_str(), m_animtype.c_str(), 
		m_target_type.c_str(), m_attrname.c_str());
	apply_constraints();	
}

animate_attrs::~animate_attrs() {
}

void animate_attrs::locate_target_element() {
	const char *p = m_node->get_attribute("targetElement");
	if(p) {
		m_target = m_node->get_context()->get_node(p);
		if(!m_target) {
			m_logger->error("%s[%s] failed to locate node with id --> %s", 
				m_tag.c_str(), m_id.c_str(), p); 
			return;
		}
	} else {
		m_target = m_tparent;
	}
	if(m_target) {
		std::string ttag = m_target->get_local_name();
		if(ttag == "region") m_target_type = "region";
		else if(ttag == "area") m_target_type = "area";
		else m_target_type = "subregion";
	} else {
		m_logger->error("%s[%s] failed to locate target node", 
			m_tag.c_str(), m_id.c_str());
	}
}

static char *attr_types[] = {"int", "double", "color", "reg_dim", "int_tuple"};

void animate_attrs::locate_target_attr() {
	if(m_tag == "animateMotion") {
		m_attrname = "position";
		m_attrtype = "point";
		return;
	}
	const char *p = m_node->get_attribute("attributeName");
	if(!p) {
		m_logger->error("%s[%s] attributeName is missing", 
			m_tag.c_str(), m_id.c_str());
		return;
	}
	m_attrname = p;
	static char *reg_dim_names[] = {"left", "top", "width", "height", "right", "bottom"};
	static int n = sizeof(reg_dim_names)/sizeof(const char *);
	for(int i=0;i<n;i++) {
		if(m_attrname == reg_dim_names[i]) {
			m_attrtype = "reg_dim";
			break;
		}
	}
	if(!m_attrtype.empty()) return;
	
	if(m_attrname == "backgroundColor") {
		m_attrtype = "color";
	} else if(m_attrname == "z-index") {
		m_attrtype = "int";
	} else if(m_attrname == "soundLevel") {
		m_attrtype = "double";
	} else if(m_attrname == "coords") {
		m_attrtype = "int_tuple";
	} else if(m_attrname == "color") {
		m_attrtype = "color";
	} else {
		m_logger->error("%s[%s] seen not animateable attribute --> %s", 
			m_tag.c_str(), m_id.c_str(), m_attrname.c_str());
	}
}

// Return true when 
// a) calcMode is discrete
// b) the attribute is not linear (emum or strings)
// c) for set animations
bool animate_attrs::is_discrete() const {
	return m_calc_mode == "discrete" || m_tag == "set";
	
}

// Returns one of: invalid, values, from-to, from-by, to, by
const char* animate_attrs::find_anim_type() {
	const char *ppath = m_node->get_attribute("path");
	if(ppath) return "path";
	const char *pvalues = m_node->get_attribute("values");
	if(pvalues) return "values";
	const char *pfrom = m_node->get_attribute("from");
	const char *pto = m_node->get_attribute("to");
	const char *pby = m_node->get_attribute("by");
	if(!pto && !pby) return "invalid";
	if(pfrom) {
		if(pto) return "from-to";
		else if(pby) return "from-by";
	} else {
		if(pto) return "to";
		else if(pby) return "by";
	}
	m_logger->error("%s[%s] the animation values are invalid", 
		m_tag.c_str(), m_id.c_str());
	return "invalid";
}

void animate_attrs::read_enum_atttrs() {
	const char *p = m_node->get_attribute("additive");
	if(p && strcmp(p, "sum") == 0 /* && attr additive */) m_additive = true;
	else if(p && strcmp(p, "replace") == 0) m_additive = false;
	else {
		m_additive = false; // default is 'replace'
		if(m_animtype == "by" /* && attr additive */) m_additive = true;
	}
	
	p = m_node->get_attribute("accumulate");
	if(p && strcmp(p, "sum") == 0 /* && attr additive */) m_accumulate = true;
	else if(p && strcmp(p, "none") == 0) m_accumulate = false;
	else m_accumulate = false; // default is 'none'
	
	p = m_node->get_attribute("calcMode");
	if(p) m_calc_mode = p; // verify in (linear, paced, spline, discrete)
	else {
		m_calc_mode = "linear";
		if(m_tag == "animateMotion") m_calc_mode = "paced";
	}
}

// Apply any constraints after reading attributes
// Validate attributes
void animate_attrs::apply_constraints() {
	if(m_tag == "set") 
		m_calc_mode = "discrete";
				
	// validate
	if(!m_target || m_attrtype.empty()) {
		m_animtype = "invalid";
	}
	
	// Override additive for "by" animations
	if(m_animtype == "by" /* && attr additive */) 
		m_additive = true;
		
	// XXX: check values
	// ...
}

void animate_attrs::get_key_times(std::vector<double>& v) {
	const char *p = m_node->get_attribute("keyTimes");
	if(!p) return;
	std::list<std::string> c;
	lib::split_trim_list(p, c, ';');
	lib::number_p parser;
	for(std::list<std::string>::iterator it=c.begin();it!=c.end();it++) {
		if(parser.matches(*it))
			v.push_back(parser.get_result());
		else {
			m_logger->error("%s[%s] invalid attr keyTimes --> %s", 
				m_tag.c_str(), m_id.c_str(), p);
		}
	}
}

void animate_attrs::get_key_splines(std::vector<qtuple>& v) {
	const char *p = m_node->get_attribute("keySplines");
	if(!p) return;
	std::list<std::string> c;
	lib::split_trim_list(p, c, ';');
	lib::number_p parser;
	for(std::list<std::string>::iterator it=c.begin();it!=c.end();it++) {
		std::list<std::string> sc;
		lib::split_trim_list(*it, sc, ' ');
		if(sc.size() != 4) {
			m_logger->error("%s[%s] invalid attr keySplines --> %s", 
				m_tag.c_str(), m_id.c_str(), p); 
		}
		qtuple qt;
		double *p = qt.v;
		bool succeeded = true;
		for(std::list<std::string>::iterator it2=sc.begin();it2!=sc.end();it2++) {
			if(parser.matches(*it))
				*p++ = parser.get_result();
			else {
				m_logger->error("%s[%s] invalid attr keySplines --> %s", 
					m_tag.c_str(), m_id.c_str(), p);
				succeeded = false;
				break;
			}
		}
		if(succeeded) v.push_back(qt);
	}
}

int animate_attrs::safeatoi(const char *p) {
	if(!p) return 0;
	return atoi(p);
}

void animate_attrs::get_values(std::vector<int>& v) {
	const char *pvalues = m_node->get_attribute("values");
	if(m_animtype == "values") {
		const char *pvalues = m_node->get_attribute("values");
		std::list<std::string> c;
		if(pvalues) 
			lib::split_trim_list(pvalues, c, ';');
		for(std::list<std::string>::iterator it=c.begin();it!=c.end();it++)
			v.push_back(safeatoi((*it).c_str()));
	} else if(m_animtype == "from-to") {
		const char *pfrom = m_node->get_attribute("from");
		const char *pto = m_node->get_attribute("to");
		v.push_back(safeatoi(pfrom));
		v.push_back(safeatoi(pto));
	} else if(m_animtype == "from-by") {
		const char *pfrom = m_node->get_attribute("from");
		const char *pby = m_node->get_attribute("by");
		int v1 = safeatoi(pfrom);
		int dv = safeatoi(pby);
		v.push_back(v1);
		v.push_back(v1+dv);
	} else if(m_animtype == "to") {
		const char *pto = m_node->get_attribute("to");
		v.push_back(safeatoi(pto));
	} else if(m_animtype == "by") {
		const char *pby = m_node->get_attribute("by");
		v.push_back(0);
		v.push_back(safeatoi(pby));
	} else {
		assert(false);
	}
}

double animate_attrs::safeatof(const char *p) {
	if(!p) return 0;
	return atof(p);
}

void animate_attrs::get_values(std::vector<double>& v) {
	const char *pvalues = m_node->get_attribute("values");
	if(m_animtype == "values") {
		const char *pvalues = m_node->get_attribute("values");
		std::list<std::string> c;
		if(pvalues) 
			lib::split_trim_list(pvalues, c, ';');
		for(std::list<std::string>::iterator it=c.begin();it!=c.end();it++)
			v.push_back(safeatof((*it).c_str()));
	} else if(m_animtype == "from-to") {
		const char *pfrom = m_node->get_attribute("from");
		const char *pto = m_node->get_attribute("to");
		v.push_back(safeatof(pfrom));
		v.push_back(safeatof(pto));
	} else if(m_animtype == "from-by") {
		const char *pfrom = m_node->get_attribute("from");
		const char *pby = m_node->get_attribute("by");
		double v1 = safeatof(pfrom);
		double dv = safeatof(pby);
		v.push_back(v1);
		v.push_back(v1+dv);
	} else if(m_animtype == "to") {
		const char *pto = m_node->get_attribute("to");
		v.push_back(safeatof(pto));
	} else if(m_animtype == "by") {
		const char *pby = m_node->get_attribute("by");
		v.push_back(0);
		v.push_back(safeatof(pby));
	} else {
		assert(false);
	}
}

void animate_attrs::get_values(std::vector<std::string>& v) {
	const char *pvalues = m_node->get_attribute("values");
	if(m_animtype == "values") {
		const char *pvalues = m_node->get_attribute("values");
		std::list<std::string> c;
		if(pvalues) 
			lib::split_trim_list(pvalues, c, ';');
		for(std::list<std::string>::iterator it=c.begin();it!=c.end();it++)
			v.push_back(*it);
	} else if(m_animtype == "from-to") {
		const char *pfrom = m_node->get_attribute("from");
		const char *pto = m_node->get_attribute("to");
		v.push_back(pfrom);
		v.push_back(pto);
	} else if(m_animtype == "from-by") {
		assert(false);
	} else if(m_animtype == "to") {
		const char *pto = m_node->get_attribute("to");
		v.push_back(pto);
	} else if(m_animtype == "by") {
		assert(false);
	} else {
		assert(false);
	}
}

// common::region_dim ::= S? (int|dec) ((px)? | %)
common::region_dim animate_attrs::to_region_dim(const std::string& s) {
	lib::region_dim_p parser;
	std::string::const_iterator b = s.begin();
	std::string::const_iterator e = s.end();
	std::ptrdiff_t d = parser.parse(b, e);
	if(d == -1) {
		m_logger->error("%s[%s] invalid region dim --> %s", 
			m_tag.c_str(), m_id.c_str(), s.c_str());
		return common::region_dim();
	}
	if(parser.is_relative())
		return common::region_dim(parser.get_relative_val());
	return common::region_dim(parser.get_px_val());
}
void animate_attrs::get_values(std::vector<common::region_dim>& v) {
	const char *pvalues = m_node->get_attribute("values");
	if(m_animtype == "values") {
		const char *pvalues = m_node->get_attribute("values");
		std::list<std::string> c;
		if(pvalues) 
			lib::split_trim_list(pvalues, c, ';');
		for(std::list<std::string>::iterator it=c.begin();it!=c.end();it++)
			v.push_back(to_region_dim(*it));
	} else if(m_animtype == "from-to") {
		const char *pfrom = m_node->get_attribute("from");
		const char *pto = m_node->get_attribute("to");
		v.push_back(to_region_dim(pfrom));
		v.push_back(to_region_dim(pto));
	} else if(m_animtype == "from-by") {
		const char *pfrom = m_node->get_attribute("from");
		const char *pby = m_node->get_attribute("by");
		common::region_dim v1 = to_region_dim(pfrom);
		common::region_dim dv = to_region_dim(pby);
		v.push_back(v1);
		v.push_back(v1+dv);
	} else if(m_animtype == "to") {
		const char *pto = m_node->get_attribute("to");
		v.push_back(to_region_dim(pto));
	} else if(m_animtype == "by") {
		const char *pby = m_node->get_attribute("by");
		v.push_back(common::region_dim(0));
		v.push_back(to_region_dim(pby));
	} else {
		assert(false);
	}
}

void animate_attrs::get_values(std::vector<lib::color_t>& v) {
	using lib::to_color;
	const char *pvalues = m_node->get_attribute("values");
	if(m_animtype == "values") {
		const char *pvalues = m_node->get_attribute("values");
		std::list<std::string> c;
		if(pvalues) 
			lib::split_trim_list(pvalues, c, ';');
		for(std::list<std::string>::iterator it=c.begin();it!=c.end();it++)
			v.push_back(to_color((*it).c_str()));
	} else if(m_animtype == "from-to") {
		const char *pfrom = m_node->get_attribute("from");
		const char *pto = m_node->get_attribute("to");
		v.push_back(to_color(pfrom));
		v.push_back(to_color(pto));
	} else if(m_animtype == "from-by") {
		const char *pfrom = m_node->get_attribute("from");
		const char *pby = m_node->get_attribute("by");
		lib::color_t v1 = to_color(pfrom);
		lib::color_t dv = to_color(pby);
		v.push_back(v1);
		v.push_back(v1+dv);
	} else if(m_animtype == "to") {
		const char *pto = m_node->get_attribute("to");
		v.push_back(to_color(pto));
	} else if(m_animtype == "by") {
		const char *pby = m_node->get_attribute("by");
		v.push_back(0);
		v.push_back(to_color(pby));
	} else {
		assert(false);
	}
}

// point := S? (? x S? , S? y S? )?
lib::point animate_attrs::to_point(const std::string& s) {
	lib::point_p parser;
	std::string::const_iterator b = s.begin();
	std::string::const_iterator e = s.end();
	std::ptrdiff_t d = parser.parse(b, e);
	if(d == -1) {
		m_logger->error("%s[%s] invalid point --> %s", 
			m_tag.c_str(), m_id.c_str(), s.c_str());
		return lib::point();
	}
	return lib::point(parser.get_x(), parser.get_y());
}

void animate_attrs::get_values(std::vector<lib::point>& v) {
	const char *pvalues = m_node->get_attribute("values");
	if(m_animtype == "path") {
		m_logger->show("path is not implemented");
		v.push_back(lib::point());
	} else if(m_animtype == "values") {
		const char *pvalues = m_node->get_attribute("values");
		std::list<std::string> c;
		if(pvalues) 
			lib::split_trim_list(pvalues, c, ';');
		for(std::list<std::string>::iterator it=c.begin();it!=c.end();it++)
			v.push_back(to_point(*it));
	} else if(m_animtype == "from-to") {
		const char *pfrom = m_node->get_attribute("from");
		const char *pto = m_node->get_attribute("to");
		v.push_back(to_point(pfrom));
		v.push_back(to_point(pto));
	} else if(m_animtype == "from-by") {
		const char *pfrom = m_node->get_attribute("from");
		const char *pby = m_node->get_attribute("by");
		lib::point v1 = to_point(pfrom);
		lib::point dv = to_point(pby);
		v.push_back(v1);
		v.push_back(v1+dv);
	} else if(m_animtype == "to") {
		const char *pto = m_node->get_attribute("to");
		v.push_back(to_point(pto));
	} else if(m_animtype == "by") {
		const char *pby = m_node->get_attribute("by");
		v.push_back(lib::point(0, 0));
		v.push_back(to_point(pby));
	} else {
		assert(false);
	}
}


