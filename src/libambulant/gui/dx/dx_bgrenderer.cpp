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

#include "ambulant/gui/dx/dx_bgrenderer.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_viewport.h"

#include "ambulant/common/region_info.h"

#include <ddraw.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dx::dx_bgrenderer::dx_bgrenderer(const common::region_info *src)
:	common::background_renderer(src),
	m_bg_image(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("new dx_bgrenderer<0x%x>", this);
}
	
gui::dx::dx_bgrenderer::~dx_bgrenderer() {
	AM_DBG lib::logger::get_logger()->debug("~dx_bgrenderer(0x%x)", this);
}

void gui::dx::dx_bgrenderer::keep_as_background() {
	AM_DBG lib::logger::get_logger()->debug("dx_bgrenderer::keep_as_background(0x%x)", this);

	dx_window *dxwindow = (dx_window *)m_dst->get_gui_window();
	viewport *v = dxwindow->get_viewport();	
	rect dstrect = m_dst->get_rect();
	rect srcrect = dstrect;
	srcrect.translate(m_dst->get_global_topleft());
	RECT d_srcrect, d_dstrect;
	set_rect(dstrect, &d_dstrect);
	set_rect(srcrect, &d_srcrect);

	if (m_bg_image) m_bg_image->Release();
	m_bg_image = v->create_surface(dstrect.width(), dstrect.height());
	
	m_bg_image->Blt(&d_dstrect, v->get_surface(), &d_srcrect, DDBLT_WAIT, NULL);
}
	
void gui::dx::dx_bgrenderer::redraw(const lib::rect &dirty, common::gui_window *window) {
	AM_DBG lib::logger::get_logger()->debug("dx_bgrenderer::redraw(%s)",repr(dirty).c_str());
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();	
	if (m_bg_image) {
		rect dstrect_whole = m_dst->get_rect();
		dstrect_whole.translate(m_dst->get_global_topleft());
		AM_DBG lib::logger::get_logger()->debug("dx_bgrenderer::redraw: clear to image");
		v->draw(m_bg_image, dstrect_whole);	
	} else {
		lib::rect rc = dirty;
		lib::point pt = m_dst->get_global_topleft();
		rc.translate(pt);
		if(v && m_src && !m_src->get_transparent()) {
			AM_DBG lib::logger::get_logger()->debug("dx_bgrenderer::redraw: clear to color");
			v->clear(rc, m_src->get_bgcolor());
		}
	}
}


