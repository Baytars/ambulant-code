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
smiltext_engine::start() {
	_update();
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
		if (!(*m_tree_iterator).first) {
			/* XXX Should pop font/size/etc */
			m_tree_iterator++;
			continue;
		}
		const lib::node *item = (*m_tree_iterator).second;
		/* XXX Should break for tev */
		smiltext_run run;
		run.m_font = "";
		run.m_fontsize = 10;
		run.m_color = lib::color_t(0xff0000);
		
		if (!item->is_data_node()) {
			run.m_data = item->get_sig();
		} else {
			run.m_data = item->get_trimmed_data();
		}
		
		if (run.m_data != "") {
			m_runs.push_back(run);
			if (m_newbegin == m_runs.end()) {
				m_newbegin = m_runs.end();
				m_newbegin--;
			}
		}
		m_tree_iterator++;
	}
	/* XXX Should schedule callback */
}
	
}
}

#ifdef WITH_SMIL30
#endif // WITH_SMIL30