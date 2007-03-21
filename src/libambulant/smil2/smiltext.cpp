// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
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

#include "ambulant/smil2/smiltext.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {
namespace smil2 {
/// Start the engine.
void
smiltext_engine::start(double t) {
	smiltext_run stdrun;
	m_run_stack.push(stdrun);
	_update();
}

/// Seek the engine
void
smiltext_engine::seek(double t) {
	// To be provided
}

/// Stop the engine.
void
smiltext_engine::stop() {
	m_tree_iterator = m_node->end();
	m_runs.clear();
	m_newbegin = m_runs.begin();
	// XXX Cancel outstanding callback
	_update();
}

void
smiltext_engine::_update() {
	while (!m_tree_iterator.is_end()) {
		assert(!m_run_stack.empty());
		const lib::node *item = (*m_tree_iterator).second;
		if (!(*m_tree_iterator).first) {
			// Pop the stack, if needed
			if (item->get_local_name() == "span")
				m_run_stack.pop();
			m_tree_iterator++;
			continue;
		}
		
		if (item->is_data_node()) {
			// Characters. Add them to the run with current font/size/etc
			smiltext_run run = m_run_stack.top();
			run.m_data = item->get_trimmed_data();
			if (run.m_data != "") {
				m_runs.push_back(run);
				if (m_newbegin == m_runs.end()) {
					m_newbegin = m_runs.end();
					m_newbegin--;
				}
			}
		} else {
			// Element. Check what it is.
			lib::xml_string tag = item->get_local_name();
			if (tag == "tev") {
				lib::logger::get_logger()->debug("smiltext: ignoring <tev>");
			} else if (tag == "clear") {
				lib::logger::get_logger()->debug("smiltext: ignoring <clear>");
			} else if (tag == "span") {
				smiltext_run run = m_run_stack.top();
				// XXXJACK Just guessing here what the attribute names are
				const char *font = item->get_attribute("font");
				if (font) run.m_font = font;
				const char *fontsize = item->get_attribute("fontSize");
				if (fontsize) run.m_fontsize = atoi(fontsize);
				const char *color = item->get_attribute("color");
				if (color) run.m_color = lib::to_color(color);
				m_run_stack.push(run);
			}
		}
		m_tree_iterator++;
	}
	if (m_client)
		m_client->smiltext_changed();
}
	
}
}

#ifdef WITH_SMIL30
#endif // WITH_SMIL30