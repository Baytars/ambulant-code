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

#include "ambulant/lib/node.h"
#include "ambulant/lib/parselets.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/smil2/time_attrs.h"
#include "ambulant/smil2/time_node.h"

#include <list>

#include "ambulant/lib/logger.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

// create an instance of this type
// force all compilers to create code for this.
static smil_time<double> dummy;

time_attrs::time_attrs(const node *n) 
:	m_node(n), 
	m_spflags(0) {
	m_logger = logger::get_logger();
	const char *pid = m_node->get_attribute("id");
	m_id = (pid?pid:"no-id");
	m_tag = m_node->get_local_name();
	parse_time_attrs();
}

void time_attrs::parse_time_attrs() {
	parse_dur();
	parse_rcount();
	parse_rdur();
	parse_begin();
	parse_end();
	parse_min();
	parse_max();
	parse_endsync();
	parse_fill();
	parse_restart();
}

bool time_attrs::end_is_indefinite() const {
	return m_elist.size() == 1 && m_elist.front().type == sv_indefinite;
}

bool time_attrs::end_has_event_conditions() const {
	sync_list::const_iterator it;
	for(it=m_elist.begin();it!=m_elist.end();it++) {
		sync_value_type t = (*it).type;
		if(t != sv_offset && t != sv_wallclock && t != sv_indefinite)
			return true;
	}
	return false;
}

bool time_attrs::has_dur_specifier() const {
	return specified_dur() || specified_rdur() || specified_rcount();
}

// dur ::= Clock-value | "media" | "indefinite"
// struct dur_t { dur_type type; time_type value;} m_dur;
// enum dur_type {dt_unspecified, dt_definite, dt_indefinite, dt_media};
void time_attrs::parse_dur() {
	m_dur.type = dt_unspecified;
	m_dur.value = time_type::unspecified;
	const char *p = m_node->get_attribute("dur");
	if(!p) return;
	set_specified(SP_DUR);
	std::string sdur = trim(p);
	if(sdur == "indefinite") {
		m_dur.type = dt_indefinite;
		m_dur.value = time_type::indefinite;
		AM_DBG m_logger->trace("%s[%s].dur=indefinite", m_tag.c_str(), m_id.c_str());	
		return;
	}
	if(sdur == "media") {
		m_dur.type = dt_media;
		AM_DBG m_logger->trace("%s[%s].dur=media", m_tag.c_str(), m_id.c_str());	
		return;
	}
	clock_value_p pl;
	std::string::const_iterator b = sdur.begin();
	std::string::const_iterator e = sdur.end();
	std::ptrdiff_t d = pl.parse(b, e);
	if(d == -1) {
		m_logger->warn("invalid dur attr [%s] for %s[%s]", 
			sdur.c_str(), m_tag.c_str(), m_id.c_str());
		return;
	}
	m_dur.type = dt_definite;
	m_dur.value = time_type(pl.m_result);
	AM_DBG m_logger->trace("%s[%s].dur=%ld", m_tag.c_str(), m_id.c_str(), m_dur.value());	
}

// repeatCount ::= floating_point | "indefinite"
void time_attrs::parse_rcount() {
	const char *p = m_node->get_attribute("repeatCount");
	if(!p) return;
	set_specified(SP_RCOUNT);
	std::string rcount_str = trim(p);
	if(rcount_str == "indefinite") {
		m_rcount = std::numeric_limits<double>::max(); //smil_time<double>::indefinite();
		AM_DBG m_logger->trace("%s[%s].repeatCount=indefinite", m_tag.c_str(), m_id.c_str());	
		return;
	}
	dec_p parser;
	std::string::const_iterator b = rcount_str.begin();
	std::string::const_iterator e = rcount_str.end();
	std::ptrdiff_t d = parser.parse(b, e);
	if(d == -1) {
		m_logger->warn("invalid repeatCount attr [%s] for %s[%s]", 
			rcount_str.c_str(), m_tag.c_str(), m_id.c_str());
		return;
	}
	m_rcount = parser.m_result;
	AM_DBG m_logger->trace("%s[%s].repeatCount=%.3f", m_tag.c_str(), m_id.c_str(), m_rcount);	
}

// repeatDur ::= Clock-value | "indefinite"
void time_attrs::parse_rdur() {
	const char *p = m_node->get_attribute("repeatDur");
	if(!p) return;
	set_specified(SP_RDUR);
	std::string rdur_str = trim(p);
	if(rdur_str == "indefinite") {
		m_rdur = time_type::indefinite;
		AM_DBG m_logger->trace("%s[%s].repeatDur=indefinite", m_tag.c_str(), m_id.c_str());	
		return;
	}
	clock_value_p parser;
	std::string::const_iterator b = rdur_str.begin();
	std::string::const_iterator e = rdur_str.end();
	std::ptrdiff_t d = parser.parse(b, e);
	if(d == -1) {
		m_logger->warn("invalid repeatDur attr [%s] for %s[%s]", 
			rdur_str.c_str(), m_tag.c_str(), m_id.c_str());
		return;
	}
	m_rdur = parser.m_result;
	AM_DBG m_logger->trace("%s[%s].repeatDur=%ld", m_tag.c_str(), m_id.c_str(), m_rdur());	
}

// min ::= Clock-value | "media" 
void time_attrs::parse_min() {
	const char *p = m_node->get_attribute("min");
	if(!p) return;
	set_specified(SP_MIN);
	std::string min_str = trim(p);
	if(min_str == "media") {
		m_min.media = true;
		m_min.value = time_type::unspecified;
		return;
	}
	m_min.media = false;
	clock_value_p parser;
	std::string::const_iterator b = min_str.begin();
	std::string::const_iterator e = min_str.end();
	std::ptrdiff_t d = parser.parse(b, e);
	if(d == -1) {
		m_logger->warn("invalid min attr [%s] for %s[%s]", 
			min_str.c_str(), m_tag.c_str(), m_id.c_str());
		return;
	}
	m_min.value = parser.m_result;
}

// max ::= Clock-value | "media" | "indefinite" 
void time_attrs::parse_max() {
	const char *p = m_node->get_attribute("max");
	if(!p) return;
	set_specified(SP_MAX);
	std::string max_str = trim(p);
	if(max_str == "media") {
		m_max.media = true;
		m_max.value = time_type::unspecified;
		return;
	} else if(max_str == "indefinite") {
		m_max.media = false;
		m_max.value = time_type::indefinite;
		return;
	}
	m_max.media = false;
	clock_value_p parser;
	std::string::const_iterator b = max_str.begin();
	std::string::const_iterator e = max_str.end();
	std::ptrdiff_t d = parser.parse(b, e);
	if(d == -1) {
		m_logger->warn("invalid max attr [%s] for %s[%s]", 
			max_str.c_str(), m_tag.c_str(), m_id.c_str());
		return;
	}
	m_max.value = parser.m_result;
}

void time_attrs::parse_begin() {
	const char *p = m_node->get_attribute("begin");
	if(!p) return;
	set_specified(SP_BEGIN);
	std::string sbegin = trim(p);	
	std::list<std::string> strlist;
	split_trim_list(sbegin, strlist);
	parse_sync_list(strlist, m_blist);
}

void time_attrs::parse_end() {
	const char *p = m_node->get_attribute("end");
	if(!p) return;
	set_specified(SP_END);
	std::string send = trim(p);	
	std::list<std::string> strlist;
	split_trim_list(send, strlist);
	parse_sync_list(strlist, m_elist);
}

void time_attrs::parse_sync_list(
	const std::list<std::string>& strlist, sync_list& svslist) {
	std::list<std::string>::const_iterator it;
	for(it = strlist.begin(); it!=strlist.end();it++) {
		char ch = (*it)[0];
		sync_value_struct svs;
		svs.offset = 0;
		svs.iparam = -1;
		if(isdigit(ch) || ch == '-' || ch == '+') {
			parse_plain_offset(*it, svs, svslist);
		} else if(starts_with((*it), "wallclock")) {
			parse_wallclock(*it, svs, svslist);
		} else if(starts_with((*it), "accesskey")) {
			parse_accesskey(*it, svs, svslist);
		} else if((*it) == "indefinite") {
			svs.type = sv_indefinite;
			svs.offset = time_type::indefinite();
			svslist.push_back(svs);
		} else {
			parse_nmtoken_offset(*it, svs, svslist);
		}
	}
}

void time_attrs::parse_plain_offset(const std::string& s, sync_value_struct& svs, sync_list& sl) {
	svs.type = sv_offset;
	offset_value_p parser;
	if(!parser.matches(s)) {
		m_logger->warn("%s[%s].%s invalid offset [%s]", 
			m_tag.c_str(), m_id.c_str(), time_spec_id(sl), s.c_str());
		return;	
	}
	svs.offset = parser.m_result;
	sl.push_back(svs);	
	AM_DBG m_logger->trace("%s[%s].%s += [%s]", 
		m_tag.c_str(), m_id.c_str(), time_spec_id(sl), repr(svs).c_str());
}

void time_attrs::parse_wallclock(const std::string& s, sync_value_struct& svs, sync_list& sl) {
	svs.type = sv_wallclock;
	m_logger->warn("ignoring wallclock");
	//sl.push_back(svs);	
}

// Accesskey-value  ::= "accesskey(" character ")" ( S? ("+"|"-") S? Clock-value )? 
void time_attrs::parse_accesskey(const std::string& s, sync_value_struct& svs, sync_list& sl) {
	svs.type = sv_accesskey;
	size_type open_par_ix = s.find('(');
	if(open_par_ix == std::string::npos) {
		m_logger->warn("invalid accesskey spec [%s] for %s[%s]", 
			s.c_str(), m_tag.c_str(), m_id.c_str());
		return;
	}
	svs.iparam = int(s[open_par_ix+1]);
	
	size_type close_par_ix = s.find(')', open_par_ix);
	if(close_par_ix == std::string::npos) {
		m_logger->warn("%s[%s].%s invalid accesskey spec [%s]", 
			m_tag.c_str(), m_id.c_str(), time_spec_id(sl), s.c_str());
		return;
	}
	std::string rest = trim(s.substr(close_par_ix+1));
	if(rest.empty()) {
		sl.push_back(svs);
		AM_DBG m_logger->trace("%s[%s].%s += [%s] (as int %d)", 
			m_tag.c_str(), m_id.c_str(), time_spec_id(sl), repr(svs).c_str(), svs.iparam);
		return;	
	}
	
	offset_value_p parser;
	if(!parser.matches(s)) {
		m_logger->warn("%s[%s].%s invalid accesskey offset [%s]", 
			m_tag.c_str(), m_id.c_str(), time_spec_id(sl), s.c_str());
		return;	
	}
	svs.offset = parser.m_result;
	sl.push_back(svs);	
	AM_DBG m_logger->trace("%s[%s].%s += [%s] (as int %d)", 
		m_tag.c_str(), m_id.c_str(), time_spec_id(sl), repr(svs).c_str(), svs.iparam);
}

void time_attrs::parse_nmtoken_offset(const std::string& s, sync_value_struct& svs, sync_list& sl) {
	std::string::const_iterator b;
	std::string::const_iterator e;
	std::ptrdiff_t d;
	
	std::string s1 = s;
	std::string offset_str;
	size_type last_pm_ix = s.find_last_of("+-");
	if(last_pm_ix != std::string::npos) {
		// Parse offset, limit raw nmtoken
		offset_str = s.substr(last_pm_ix);
		offset_value_p parser;
		if(parser.matches(offset_str)) {
			svs.offset = parser.m_result;
			s1 = trim(s.substr(0, last_pm_ix));
		}
	}
	// s1 holds the nmtoken + optional event specifier
	// extract nmtokek from s1
	xml_nmtoken_p parser;
	b = s1.begin(); e = s1.end();
	d = parser.parse(b, e);
	if(d == -1) {
		m_logger->warn("%s[%s].%s invalid attr [%s]", 
			m_tag.c_str(), m_id.c_str(), time_spec_id(sl), s.c_str());
		return;
	}
	std::string nmtoken = parser.m_result;
	
	// the nmtoken suffix
	static std::set<std::string> events;
	if(events.empty()) {
		events.insert("begin");
		events.insert("end");
		events.insert("beginEvent");
		events.insert("endEvent");
		events.insert("repeat");
		events.insert("activateEvent");
		events.insert("click");
		events.insert("marker");
	}
	
	std::string event;
	size_type last_dot_ix = nmtoken.find_last_of(".");
	
	if(last_dot_ix == std::string::npos) {
		// an event-value with default eventbase-element
		svs.type = sv_event;
		svs.base  = ""; // default
		event = nmtoken;	// check repeat(d+)?
	} else if(ends_with(nmtoken, ".begin") || ends_with(nmtoken, ".end")) {
		// syncbase-value
		svs.type = sv_syncbase;
		svs.base = nmtoken.substr(0, last_dot_ix);
		event = nmtoken.substr(last_dot_ix+1);
	} else if(ends_with(nmtoken, ".marker")) {
		// media-marker-value
		svs.type = sv_media_marker;
		svs.base = nmtoken.substr(0, last_dot_ix);
		event = "marker";
		bool succeeded = false;
		size_type open_par_ix = s1.find('(');
		size_type close_par_ix = s1.find(')');
		if(open_par_ix != std::string::npos && close_par_ix != std::string::npos) {
			svs.sparam = trim(s1.substr(open_par_ix+1, close_par_ix - open_par_ix - 1));
			succeeded = true;
		}
		if(!succeeded) {
			m_logger->warn("%s[%s].%s invalid marker [%s]", 
				m_tag.c_str(), m_id.c_str(), time_spec_id(sl), s.c_str());
			return;
		}	
	} else if(ends_with(nmtoken, ".repeat")) {
		// repeat event-value
		svs.type = sv_repeat;
		svs.base = nmtoken.substr(0, last_dot_ix);
		event = "repeat";
		bool succeeded = false;
		size_type open_par_ix = s1.find('(');
		if(open_par_ix != std::string::npos) {
			std::string sn = s1.substr(open_par_ix+1);
			int_p iparser;
			b = sn.begin(); e = sn.end();
			if(iparser.parse(b, e) != -1) {
				svs.iparam = iparser.m_result;
				succeeded = true;
			}
		}
		if(!succeeded) {
			m_logger->warn("%s[%s].%s invalid repeat [%s]", 
				m_tag.c_str(), m_id.c_str(), time_spec_id(sl), s.c_str());
			return;
		}	
	} else {
		// event-value other than repeat
		svs.type = sv_event;
		svs.base = nmtoken.substr(0, last_dot_ix);
		event = nmtoken.substr(last_dot_ix+1);
	}
	
	if(events.find(event) == events.end()) {
		m_logger->warn("%s[%s].%s invalid event [%s]", 
			m_tag.c_str(), m_id.c_str(), time_spec_id(sl), s.c_str());
	} else {
		svs.event = event;
		AM_DBG m_logger->trace("%s[%s].%s += [%s]", 
			m_tag.c_str(), m_id.c_str(), time_spec_id(sl), repr(svs).c_str());
	}
	
	// if base is not empty, locate node
	// else base is the default
	if(!svs.event.empty()) 
		sl.push_back(svs);	
}

// endsync ::= first | last | all | media | Id-value | smil1.0-Id-value
void time_attrs::parse_endsync() {
	m_endsync.rule = esr_last;
	const char *p = m_node->get_attribute("endsync");
	if(!p) return;
	set_specified(SP_ENDSYNC);
	std::string endsync_str = trim(p);
	if(endsync_str == "first") m_endsync.rule = esr_first;
	else if(endsync_str == "last") m_endsync.rule = esr_last;
	else if(endsync_str == "all") m_endsync.rule = esr_all;
	else if(endsync_str == "media") m_endsync.rule = esr_media;
	else {
		m_endsync.rule = esr_id;
		xml_nmtoken_p parser;
		if(!parser.matches(endsync_str)) {
			m_logger->warn("invalid endsync attr [%s] for %s[%s]", 
				endsync_str.c_str(), m_tag.c_str(), m_id.c_str());
		} else {
			m_endsync.ident = endsync_str;
		}
	}
	AM_DBG m_logger->trace("%s[%s].endsync = [%s]", 
			m_tag.c_str(), m_id.c_str(), endsync_str.c_str());
	
}

// fill ::= remove | freeze | hold | transition | auto | default
void time_attrs::parse_fill() {
	m_fill = modulated_fill(get_default_fill());
	const char *p = m_node->get_attribute("fill");
	if(!p) {
		//AM_DBG m_logger->trace("%s[%s].fill = [%s]", 
		//	m_tag.c_str(), m_id.c_str(), repr(m_fill).c_str());
		return;
	}
	set_specified(SP_FILL);
	std::string fill = trim(p);
	if(fill == "remove") m_fill = fill_remove;
	else if(fill == "freeze") m_fill = fill_freeze;
	else if(fill == "hold") m_fill = fill_hold;
	else if(fill == "transition") m_fill = fill_transition;
	else if(fill == "auto") m_fill = fill_auto;
	// else default or invalid
	if(m_fill == fill_auto) m_fill = modulated_fill(m_fill);
	AM_DBG m_logger->trace("%s[%s].fill = [%s]", 
			m_tag.c_str(), m_id.c_str(), repr(m_fill).c_str());
	
}

fill_behavior time_attrs::modulated_fill(fill_behavior fb) {
	if(fb != fill_auto) return fb;
	bool dv = (specified_dur() || specified_rdur() || specified_rcount() ||
		specified_end());
	return dv?fill_remove:fill_freeze;
}

// Returns the fillDefault attribute active for this. 
// fillDefault ::= remove | freeze | hold | transition | auto | inherit
// Applicable for an element and all descendents
fill_behavior time_attrs::get_default_fill() {
	fill_behavior retfb = fill_auto;
	const node *curr = m_node;
	while(curr) {
		const char *p = curr->get_attribute("fillDefault");
		if(p) {
			//if(p && valid_fill_default(p) && not_inherit(p)) break;
			std::string fill = trim(p);
			if(fill == "remove") {retfb = fill_remove; break;}
			else if(fill == "freeze") {retfb = fill_freeze; break;}
			else if(fill == "hold") {retfb = fill_hold; break;}
			else if(fill == "transition") {retfb = fill_transition; break;}
			else if(fill == "auto") {retfb = fill_auto; break;}
			// else inherit or invalid e.g. continue
		}
		curr = curr->up();
	}
	return retfb;
}

// restart ::= always | whenNotActive | never | default
void time_attrs::parse_restart() {
	m_restart = get_default_restart();
	const char *p = m_node->get_attribute("restart");
	if(!p) return;
	set_specified(SP_RESTART);
	std::string restart = trim(p);
	if(restart == "always") m_restart = restart_always;
	else if(restart == "whenNotActive") m_restart = restart_when_not_active;
	else if(restart == "never") m_restart = restart_never;
	// else restart == "default"
	AM_DBG m_logger->trace("%s[%s].restart = [%s]", 
		m_tag.c_str(), m_id.c_str(), repr(m_restart).c_str());
}

// Returns the restartDefault attribute active for this. 
// restartDefault := always | whenNotActive | never | inherit 
// Applicable for an element and all descendents
// Returns one of : always | whenNotActive | never
restart_behavior time_attrs::get_default_restart() {
	restart_behavior rb = restart_always;
	const node *curr = m_node;
	while(curr) {
		const char *p = curr->get_attribute("restartDefault");
		if(p) {
			std::string restart = trim(p);
			if(restart == "always") { rb = restart_always; break;}
			else if(restart == "whenNotActive") { rb = restart_when_not_active; break;}
			else if(restart == "never") { rb = restart_never; break;}
		}
		curr = curr->up();
	}
	return rb;
}

////////////////
// helpers


///////////////////////
// Tracing

std::string smil2::repr(sync_value_type sv) {
	switch(sv) {
		case sv_offset: return "offset";
		case sv_syncbase: return "syncbase";
		case sv_event: return "event";
		case sv_repeat: return "repeat";
		case sv_accesskey: return "accesskey";
		case sv_media_marker: return "marker";
		case sv_wallclock: return "wallclock";
		case sv_indefinite: return "indefinite";
	}
	assert(false);
	return "";
}

std::string smil2::repr(const sync_value_struct& svs) {
	std::string os;
	if(svs.type == sv_offset) {
		os << svs.offset;
	} else if(svs.type == sv_accesskey) {
		os << "accesskey(" << char(svs.iparam) << ')';
		if(svs.offset>0) os << " + " << svs.offset;
		else if(svs.offset<0) os << " - " << -svs.offset;
	} else  {
		if(svs.base.empty())
			os << svs.event;
		else
			os << svs.base << "." << svs.event;
		if(svs.iparam != -1)
			os << "(" << svs.iparam << ")";
		else if(!svs.sparam.empty())
			os << "(" << svs.sparam << ")";
		if(svs.offset>0) os << " + " << svs.offset;
		else if(svs.offset<0) os << " - " << -svs.offset;
	} 
	return os;
}

std::string smil2::repr(fill_behavior f) {
	switch(f) {
		case fill_remove: return "remove";
		case fill_freeze: return "freeze";
		case fill_hold: return "hold";
		case fill_transition: return "transition";
		case fill_auto: return "auto";
		case fill_default: return "default";
		case fill_inherit: return "inherit";
	}
	assert(false);
	return "";

}

std::string smil2::repr(restart_behavior f) {
	switch(f) {
		case restart_always: return "always";
		case restart_when_not_active: return "whenNotActive";
		case restart_never: return "never";
		case restart_default: return "default";
		case restart_inherit: return "inherit";
	}
	assert(false);
	return "";
}

/////////////////////////////
// priority_attrs implementation

// static
priority_attrs* 
priority_attrs::create_instance(const lib::node *n) {
	assert(n->get_local_name() == "priorityClass");
	priority_attrs *pa = new priority_attrs();
	const char *p;
	std::string spec;
	
	p = n->get_attribute("peers");
	spec = p?p:"stop";
	if(spec == "stop" || spec == "pause" || spec == "defer" || spec == "never")
		pa->peers = interrupt_from_str(spec);
	
	p = n->get_attribute("higher");
	spec = p?p:"pause";
	if(spec == "stop" || spec == "pause")
		pa->higher = interrupt_from_str(spec);
	
	p = n->get_attribute("lower");
	spec = p?p:"defer";
	if(spec == "defer" || spec == "never")
		pa->lower = interrupt_from_str(spec);
	
	p = n->get_attribute("pauseDisplay");
	spec = p?p:"show";
	if(spec == "disable" || spec == "hide" || spec == "show")
		pa->display = display_from_str(spec);
	
	return pa;
}

// static 
interrupt_type 
priority_attrs::interrupt_from_str(const std::string& spec) {
	if(spec == "stop") return int_stop;
	else if(spec == "pause") return int_pause;
	else if(spec == "defer") return int_defer;
	else if(spec == "never") return int_never;
	return int_stop;
}

//static 
pause_display 
priority_attrs::display_from_str(const std::string& spec) {
	if(spec == "disable") return display_disable;
	else if(spec == "hide") return display_hide;
	else if(spec == "show") return display_show;
	return display_show;
}

