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
	m_viewrc(point(0, 0), point(bounds.w, bounds.h)),
	m_wf(wf),
	m_viewport(v), m_locked(false), m_isnew_redraw_rect(true) {
	//AM_DBG lib::logger::get_logger()->trace_stream() 
	//	<< "dx_window(" << name << ", " << bounds << ")" << lib::endl;
}

gui::dx::dx_window::~dx_window() {
	AM_DBG lib::logger::get_logger()->trace("~dx_window()");
	m_wf->window_done(m_name);
}
  		
void gui::dx::dx_window::need_redraw(const lib::screen_rect<int> &r) {
	// clip rect to this window since the layout does not do this
	lib::screen_rect<int> rc = r;
	rc &= m_viewrc;
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