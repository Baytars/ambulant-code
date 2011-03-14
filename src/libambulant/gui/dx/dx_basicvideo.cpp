// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2010 Stichting CWI,
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/*
 * @$Id$
 */

#include "ambulant/gui/dx/dx_basicvideo.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_basicvideo_player.h"
#include "ambulant/gui/dx/dx_transition.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_asb.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
extern const char dx_basicvideo_playable_tag[] = "video";
extern const char dx_basicvideo_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererDirectXBasicVideo");

common::playable_factory *
gui::dx::create_dx_basicvideo_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirectXBasicVideo"), true);
	return new common::single_playable_factory<
		gui::dx::dx_basicvideo_renderer,
		dx_basicvideo_playable_tag,
		dx_basicvideo_playable_renderer_uri,
		dx_basicvideo_playable_renderer_uri,
		dx_basicvideo_playable_renderer_uri >(factory, mdp);
}

gui::dx::dx_basicvideo_renderer::dx_basicvideo_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories *fp,
	common::playable_factory_machdep *dxplayer)
:	dx_renderer_playable(context, cookie, node, evp, fp, dynamic_cast<dx_playables_context*>(dxplayer)),
	m_player(0),
	m_update_event(0) {
	AM_DBG lib::logger::get_logger()->debug("dx_basicvideo_renderer(0x%x)", this);
}

gui::dx::dx_basicvideo_renderer::~dx_basicvideo_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~dx_basicvideo_renderer(0x%x)", this);
	if(m_player) stop();
}

void gui::dx::dx_basicvideo_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("start: %s", m_node->get_xpath().c_str());
	common::surface *surf = get_surface();

	dx_window *dxwindow = static_cast<dx_window*>(surf->get_gui_window());
	viewport *v = dxwindow->get_viewport();
	HWND parent = v->get_hwnd();
	net::url url = m_node->get_url("src");
	_init_clip_begin_end();
	if(url.is_local_file() || lib::win32::file_exists(url.get_file())) {
		m_player = new gui::dx::basicvideo_player(url.get_file(), parent);
	} else if(url.is_absolute()) {
		m_player = new gui::dx::basicvideo_player(url.get_url(), parent);
	} else {
		lib::logger::get_logger()->show("The location specified for the data source does not exist. [%s]",
			url.get_url().c_str());
	}
	if(!m_player) {
		// Not created or stopped (gone)

		// Notify scheduler
		m_context->stopped(m_cookie);
		return;
	}

	// Does it have all the resources to play?
	if(!m_player->can_play()) {
		// Notify scheduler
		m_context->stopped(m_cookie);
		return;
	}
	lib::rect r = surf->get_rect();
	r.translate(surf->get_global_topleft());
	m_player->setrect(r);
	// Has this been activated
	if(m_activated) {
		// repeat
		m_player->start(t + (m_clip_begin / 1000000.0));
//XXXJACK		m_player->update();
		m_dest->need_redraw();
		schedule_update();
		return;
	}

	// Activate this renderer.
	// Add this renderer to the display list of the region
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	m_activated = true;

	// Start the underlying player
	m_player->start(t + (m_clip_begin / 1000000.0));
//XXXJACK	m_player->update();

	// Request a redraw
	m_dest->need_redraw();

	// Notify the scheduler; may take benefit
	m_context->started(m_cookie);

	// Schedule a self-update
	schedule_update();
}

void gui::dx::dx_basicvideo_renderer::seek(double t) {
	assert( t >= 0);
	if (m_player) m_player->seek(t + (m_clip_begin / 1000000.0));
	// ?? if(!m_update_event) schedule_update();
	// ?? m_dest->need_redraw();
}
std::pair<bool, double> gui::dx::dx_basicvideo_renderer::get_dur() {
	if(m_player) {
		std::pair<bool, double> durp = m_player->get_dur();
		if (!durp.first) return durp;
		double dur = durp.second;
		if (m_clip_end > 0 && dur > m_clip_end / 1000000.0)
			dur = m_clip_end / 1000000.0;
		dur -= (m_clip_begin / 1000000.0);
		return std::pair<bool, double>(true, dur);
	}
	return std::pair<bool, double>(false, 0.0);
}

bool gui::dx::dx_basicvideo_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("stop: %s", m_node->get_xpath().c_str());
	if(!m_player) return true;
	m_cs.enter();
	m_update_event = 0;
	basicvideo_player *p = m_player;
	m_player = 0;
	p->stop();
	delete p;
	m_cs.leave();
	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;
	m_activated = false;
	m_dxplayer->stopped(this);
	m_context->stopped(m_cookie);
	return false;
}

void gui::dx::dx_basicvideo_renderer::pause(common::pause_display d) {
	AM_DBG lib::logger::get_logger()->debug("dx_basicvideo_renderer.pause(0x%x)", this);
	m_update_event = 0;
	if(m_player) m_player->pause();
}

void gui::dx::dx_basicvideo_renderer::resume() {
	AM_DBG lib::logger::get_logger()->debug("dx_basicvideo_renderer.resume(0x%x)", this);
	if(m_player) m_player->resume();
	if(!m_update_event) schedule_update();
	m_dest->need_redraw();
}

bool gui::dx::dx_basicvideo_renderer::user_event(const lib::point& pt, int what) {
	if (!user_event_sensitive(pt)) return false;
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
	return true;
}

void gui::dx::dx_basicvideo_renderer::redraw(const lib::rect &dirty, common::gui_window *window) {

}

void gui::dx::dx_basicvideo_renderer::update_callback() {
	// Schedule a redraw callback
	m_cs.enter();
	if(!m_update_event || !m_player) {
		m_cs.leave();
		return;
	}
	m_dest->need_redraw();
	bool need_callback = m_player->is_playing();

	m_cs.leave();

	if( need_callback ) {
		schedule_update();
	} else {
		m_update_event = 0;
		m_context->stopped(m_cookie);
	}
}

void gui::dx::dx_basicvideo_renderer::schedule_update() {
	m_update_event = new lib::no_arg_callback<dx_basicvideo_renderer>(this,
		&dx_basicvideo_renderer::update_callback);
	m_event_processor->add_event(m_update_event, 50, lib::ep_med);
}