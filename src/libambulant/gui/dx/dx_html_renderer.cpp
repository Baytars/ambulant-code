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
#include "ambulant/gui/dx/dx_text.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/html_bridge.h"
#include "ambulant/gui/dx/dx_html_renderer.h"
#include "ambulant/gui/dx/dx_transition.h"

#include "ambulant/common/region_info.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/memfile.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/smil2/params.h"

#ifdef	WITH_HTML_WIDGET

#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dx::dx_html_renderer::dx_html_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::gui_window *window,
	dx_playables_context *dxplayer)
:   dx_renderer_playable(context, cookie, node, evp, window, dxplayer),
	m_html_widget(0) {
	AM_DBG lib::logger::get_logger()->debug("dx_html_renderer(0x%x)", this);
}

void gui::dx::dx_html_renderer::set_surface(common::surface *dest) {
	AM_DBG lib::logger::get_logger()->debug("dx_html_renderer::set_surface(0x%x) dest=0x%x", this, dest);

	m_dest = dest;
	
	lib::screen_rect<int> rc = dest->get_rect();
	const lib::point p = dest->get_global_topleft();
	net::url url = m_node->get_url("src");
	dx_window *dxwindow = static_cast<dx_window*>(m_window);
	viewport *v = dxwindow->get_viewport();
	if(!lib::memfile::exists(url)) {
		lib::logger::get_logger()->show("The location specified for the data source does not exist. [%s]",
			url.get_url().c_str());
		return;
	}
	m_html_widget = create_html_widget(url, rc.left()+p.x, rc.top()+p.y, rc.width()+p.x, rc.height()+p.y);
	if (m_html_widget) {
		AM_DBG lib::logger::get_logger()->debug("dx_html_renderer::set_surface(0x%x) html_widget=0x%x",this,m_html_widget);
	} else {
		AM_DBG lib::logger::get_logger()->debug("dx_html_renderer::set_surface(0x%x) no html_widget", this);
	}
	// Pass <param> settings, if applicable
	smil2::params *params = smil2::params::for_node(m_node);
	if (params) {
		delete params;
		params = NULL;
	}
}

gui::dx::dx_html_renderer::~dx_html_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~dx_html_renderer(0x%x)", this);
//KB	delete m_html_widget;
//KB	m_html_widget = NULL;
}

void gui::dx::dx_html_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("dx_html_renderer::start(0x%x)", this);
		if(!m_html_widget) {
		// Notify scheduler
		m_context->stopped(m_cookie);
		return;
	}
		
	// Has this been activated
	if(m_activated) {
		// repeat
		m_dest->need_redraw();
		return;	
	}
	
	// Activate this renderer.
	// Add this renderer to the display list of the region
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	m_activated = true;
		
	// Request a redraw
	// Currently already done by show()
	// m_dest->need_redraw();

	// Notify scheduler that we're done playing
	m_context->stopped(m_cookie);
}

void gui::dx::dx_html_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("dx_html_renderer::stop(0x%x)", this);
//KB	delete m_html_widget;
//KB	m_html_widget = NULL;
	(void) delete_html_widget(m_html_widget);
	m_dest->renderer_done(this);
	m_activated = false;
	m_dxplayer->stopped(this);
}

void gui::dx::dx_html_renderer::user_event(const lib::point& pt, int what) {
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
}

void gui::dx::dx_html_renderer::redraw(const lib::screen_rect<int>& dirty, common::gui_window *window) {
	// Get the top-level surface
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	if(!v) return;
	
	if(!m_html_widget /*JNK|| !m_html->can_play()KNJ*/) {
		// No bits available
		AM_DBG lib::logger::get_logger()->debug("dx_html_renderer::redraw with no dx_html_widget");
		return;
	}
	redraw_html_widget(m_html_widget);
/*JNK	
	lib::screen_rect<int> text_rc = dirty;
	lib::screen_rect<int> reg_rc = dirty;
	
	// Translate img_reg_rc_dirty to viewport coordinates 
	lib::point pt = m_dest->get_global_topleft();
	reg_rc.translate(pt);
		
	dx_transition *tr = get_transition();
#ifdef USE_SMIL21
	if (tr && tr->is_fullscreen()) {
		v->set_fullscreen_transition(tr);
		tr = NULL;
	}
#endif // USE_SMIL21
		
	// Finally blit img_rect_dirty to img_reg_rc_dirty
	v->draw(m_html->get_ddsurf(), html_rc, reg_rc, true, tr);

	if (m_erase_never) m_dest->keep_as_background();
 KNJ*/
}
#endif // WITH_HTML_WIDGET
