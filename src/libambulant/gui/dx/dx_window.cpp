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

#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/common/region.h"

#include "ambulant/lib/logger.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dx::dx_window::dx_window(const std::string& name, 
  	lib::size bounds,
  	region *rgn,
  	common::window_factory *wf,
  	viewport* v)
:	common::gui_window(rgn),
	m_rgn(rgn),
	m_name(name),
	m_viewrc(point(0, 0), size(bounds.w, bounds.h)),
	m_wf(wf),
	m_viewport(v), m_locked(false), m_isnew_redraw_rect(true) {
	//AM_DBG lib::logger::get_logger()->trace_stream() 
	//	<< "dx_window(" << name << ", " << bounds << ")" << lib::endl;
}

gui::dx::dx_window::~dx_window() {
	AM_DBG lib::logger::get_logger()->debug("~dx_window()");
	m_wf->window_done(m_name);
}
  		
void gui::dx::dx_window::need_redraw(const lib::rect &r) {
	// clip rect to this window since the layout does not do this
	lib::rect rc = r;
	rc &= m_viewrc;
#ifdef USE_SMIL21
	m_viewport->set_fullscreen_transition(NULL);
#endif
	m_rgn->redraw(rc, this);
	if(!m_locked)
		m_viewport->redraw(rc);
	else {
		if(m_isnew_redraw_rect) {
			m_redraw_rect = rc;
			m_isnew_redraw_rect = false;
		} else m_redraw_rect |= rc;
	}
}

void gui::dx::dx_window::need_redraw() {
	m_rgn->redraw(m_viewrc, this);
	m_viewport->redraw();
}

void gui::dx::dx_window::lock_redraw() {
	m_locked = true;
	m_isnew_redraw_rect = true;
}

void gui::dx::dx_window::unlock_redraw() {
	m_locked = false;
	m_viewport->redraw(m_redraw_rect);
}
