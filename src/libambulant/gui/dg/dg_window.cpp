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

#include "ambulant/gui/dg/dg_window.h"
#include "ambulant/gui/dg/dg_viewport.h"
#include "ambulant/common/region.h"

#include "ambulant/lib/logger.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dg::dg_window::dg_window(const std::string& name, 
  	lib::size bounds,
  	region *rgn,
  	common::window_factory *wf,
  	viewport* v)
:	common::gui_window(rgn),
	m_rgn(rgn),
	m_name(name),
	m_viewrc(point(0, 0), point(bounds.w, bounds.h)),
	m_wf(wf),
	m_viewport(v) {
}

gui::dg::dg_window::~dg_window() {
	AM_DBG lib::logger::get_logger()->trace("~dg_window()");
	m_wf->window_done(m_name);
}
  		
void gui::dg::dg_window::need_redraw(const lib::screen_rect<int> &r) {
	lib::screen_rect<int> rc = r;
	rc &= m_viewrc;
	m_rgn->redraw(rc, this);
	m_viewport->redraw(rc);
}

void gui::dg::dg_window::need_redraw() {
	m_rgn->redraw(m_viewrc, this);
	m_viewport->redraw();
}

void gui::dg::dg_window::mouse_region_changed() {
}


