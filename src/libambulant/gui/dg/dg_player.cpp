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
 
//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif


#include "ambulant/gui/dg/dg_player.h"
#include "ambulant/gui/dg/dg_viewport.h"
#include "ambulant/gui/dg/dg_window.h"
#include "ambulant/gui/dg/dg_wmuser.h"
#include "ambulant/gui/dg/dg_rgn.h"
#include "ambulant/gui/dg/dg_transition.h"

#include "ambulant/lib/event.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/transition_info.h"

#include "ambulant/common/plugin_engine.h"

#include "ambulant/smil2/transition.h"

// Players
#include "ambulant/smil2/smil_player.h"
#include "ambulant/mms/mms_player.h"
#include "ambulant/smil2/test_attrs.h"

// Renderer playables
#include "ambulant/gui/dg/dg_bgrenderer.h"
#include "ambulant/gui/dg/dg_text.h"
#include "ambulant/gui/dg/dg_img.h"
#include "ambulant/gui/dg/dg_brush.h"

// Playables
#include "ambulant/gui/dg/dg_area.h"

#include "ambulant/gui/dg/dg_audio.h"

// Layout
#include "ambulant/common/region.h"
#include "ambulant/smil2/smil_layout.h"

using namespace ambulant;

int gui::dg::dg_gui_region::s_counter = 0;

gui::dg::dg_player::dg_player(dg_player_callbacks &hoster, const net::url& u) 
:	m_hoster(hoster),
	m_url(u),
	m_player(0),
	m_timer(new timer(realtime_timer_factory(), 1.0, false)),
	m_worker_processor(0),
	m_update_event(0),	
	m_logger(lib::logger::get_logger()) {
	
	// Fill the factory object
	m_factory.rf = (global_playable_factory*) this->get_playable_factory();
	m_factory.df = NULL;
	m_factory.wf = this->get_window_factory();

	// Load the plugins
	common::plugin_engine *pf = common::plugin_engine::get_plugin_engine();
	pf->add_plugins(&m_factory);

	// Parse the provided URL. 
	AM_DBG m_logger->debug("Parsing: %s", u.get_url().c_str());	
	lib::document *doc = lib::document::create_from_url(&m_factory, u);
	if(!doc) {
		m_logger->show("Failed to parse document %s", u.get_url().c_str());
		return;
	}

	
	// Create a player instance
	AM_DBG m_logger->debug("Creating player instance for: %s", u.get_url().c_str());
	m_player = new smil2::smil_player(doc, &m_factory, this);
	
	// Create the worker processor
	m_worker_processor = event_processor_factory(m_timer);
}

gui::dg::dg_player::~dg_player() {
	if(m_player) stop();
	delete m_player;
	while(!m_frames.empty()) {
		frame *pf = m_frames.top();
		m_frames.pop();
		m_windows = pf->windows;
		m_player = pf->player;
		delete pf;
		stop();
		delete m_player;
	}
	m_timer->pause();
	if(m_worker_processor)
		m_worker_processor->cancel_all_events();
	delete m_worker_processor;
	delete m_timer;
	assert(m_windows.empty());
	if(dg_gui_region::s_counter != 0) 
		m_logger->warn("Undeleted gui regions: %d", dg_gui_region::s_counter);
}

void gui::dg::dg_player::start() {
	if(m_player) {
		m_timer->resume();
		m_player->start();
		std::map<std::string, wininfo*>::iterator it;
		for(it=m_windows.begin();it!=m_windows.end();it++) {
			dg_window *dgwin = (dg_window *)(*it).second->w;
			dgwin->need_redraw();
		}
	}
}
void gui::dg::dg_player::stop() {
	if(m_player) {
		m_player->stop();
		m_timer->pause();
		on_done();
	}
}

void gui::dg::dg_player::pause() {
	if(m_player) {
		if(m_player->is_playing()) {
			m_player->pause();
			m_timer->pause();
		} else if(m_player->is_pausing()) {
			m_player->resume();
			m_timer->resume();
		}
	}
}

void gui::dg::dg_player::resume() {
	if(m_player) {
		m_player->resume();
		m_timer->resume();
	}
}

bool gui::dg::dg_player::is_playing() const {
	return (m_player && m_player->is_playing()) || !m_frames.empty();
}

bool gui::dg::dg_player::is_pausing() const {
	return m_player && m_player->is_pausing();
}

bool gui::dg::dg_player::is_done() const {
	return m_player && m_player->is_done() && m_frames.empty();
}

void gui::dg::dg_player::on_click(int x, int y, HWND hwnd) {
	if(!m_player || !is_playing()) return;
	lib::point pt(x, y);
	dg_window *dgwin = (dg_window *) get_window(hwnd);
	if(!dgwin) return;
	region *r = dgwin->get_region();
	if(r) r->user_event(pt, common::user_event_click);
}

int gui::dg::dg_player::get_cursor(int x, int y, HWND hwnd) {
	if(!m_player || !is_playing()) return 0;
	lib::point pt(x, y);
	dg_window *dgwin = (dg_window *) get_window(hwnd);
	if(!dgwin) return 0;
	region *r = dgwin->get_region();
	m_player->set_cursor(0);
	if(r) r->user_event(pt, common::user_event_mouse_over);
	return m_player->get_cursor();
}

std::string gui::dg::dg_player::get_pointed_node_str() {
	if(!m_player || !is_playing()) return "";
	return m_player->get_pointed_node_str();
}

void gui::dg::dg_player::on_char(int ch) {
	if(m_player) m_player->on_char(ch);
}

void gui::dg::dg_player::redraw(HWND hwnd, HDC hdc) {
	wininfo *wi = get_wininfo(hwnd);
	if(wi) wi->v->redraw(hdc);
}

void gui::dg::dg_player::on_done() {
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		(*it).second->v->clear();
		(*it).second->v->redraw();
	}
}

////////////////////
// common::window_factory implementation

common::gui_window *
gui::dg::dg_player::new_window(const std::string &name, 
	lib::size bounds, common::gui_events *src) {
	
	AM_DBG lib::logger::get_logger()->debug("dg_window_factory::new_window(%s): %s", 
		name.c_str(), ::repr(bounds).c_str());
	
	// wininfo struct that will hold the associated objects
	wininfo *winfo = new wininfo;
	
	// Create an os window
	winfo->h = m_hoster.new_os_window();
	
	// Create the associated dg viewport
	winfo->v = create_viewport(bounds.w, bounds.h, winfo->h);
	
	// Region?
	region *rgn = (region *) src;
	
	// Clear the viewport
	const common::region_info *ri = rgn->get_info();
	winfo->v->set_background(ri?ri->get_bgcolor():CLR_INVALID);
	winfo->v->clear();
	
	// Create a concrete gui_window
	winfo->w = new dg_window(name, bounds, rgn, this, winfo->v);
	winfo->f = 0;
	
	// Store the wininfo struct
	m_windows[name] = winfo;
	AM_DBG m_logger->debug("windows: %d", m_windows.size());
	
	// Return gui_window
	return winfo->w;
}

void 
gui::dg::dg_player::window_done(const std::string &name) {
	// called when the window is destructed (wi->w)
	std::map<std::string, wininfo*>::iterator it = m_windows.find(name);
	assert(it != m_windows.end());
	wininfo *wi = (*it).second;
	m_windows.erase(it);
	wi->v->clear();
	wi->v->redraw();
	delete wi->v;
	m_hoster.destroy_os_window(wi->h);
	delete wi;
	AM_DBG m_logger->debug("windows: %d", m_windows.size());
}

common::bgrenderer*
gui::dg::dg_player::new_background_renderer(const common::region_info *src) {
	return new dg_bgrenderer(src);
}

gui::dg::viewport* gui::dg::dg_player::create_viewport(int w, int h, HWND hwnd) {
	AM_DBG m_logger->debug("dg_player::create_viewport(%d, %d)", w, h);
	PostMessage(hwnd, WM_SET_CLIENT_RECT, w, h);
	viewport *v = new gui::dg::viewport(w, h, hwnd);
	v->redraw();
	return v;
}

gui::dg::dg_player::wininfo*
gui::dg::dg_player::get_wininfo(HWND hwnd) {
	wininfo *winfo = 0;
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		wininfo *wi = (*it).second;
		if(wi->h = hwnd) {winfo = wi;break;}
	}
	return winfo;
}

common::gui_window *
gui::dg::dg_player::get_window(HWND hwnd) {
	wininfo *wi = get_wininfo(hwnd);
	return wi?wi->w:0;
}

////////////////////
// common::playable_factory implementation

common::playable *
gui::dg::dg_player::new_playable(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp) {
	
	common::gui_window *window = get_window(node);
	common::playable *p = 0;
	lib::xml_string tag = node->get_qname().second;
	if(tag == "text") {
		p = new dg_text_renderer(context, cookie, node, evp, window);
	} else if(tag == "img") {
		p = new dg_img_renderer(context, cookie, node, evp, window, this);
	} else if(tag == "audio") {
		p = new dg_audio_renderer(context, cookie, node, evp, window, m_worker_processor);
	} else if(tag == "video") {
		p = new dg_area(context, cookie, node, evp, window);
	} else if(tag == "area") {
		p = new dg_area(context, cookie, node, evp, window);
	} else if(tag == "brush") {
		p = new dg_brush(context, cookie, node, evp, window);
	} else {
		p = new dg_area(context, cookie, node, evp, window);
	}
	return p;
}

void gui::dg::dg_player::set_intransition(common::playable *p, lib::transition_info *info) { 
	lib::logger::get_logger()->debug("set_intransition : %s", repr(info->m_type).c_str());
	lib::timer *timer = new lib::timer(m_timer, 1.0, false);
	dg_transition *tr = make_transition(info->m_type, p, timer);
	m_trmap[p] = tr;
	tr->init(p->get_renderer()->get_surface(), false, info);
	tr->first_step();
	if(!m_update_event) schedule_update();
}

void gui::dg::dg_player::start_outtransition(common::playable *p, lib::transition_info *info) {  
	lib::logger::get_logger()->debug("start_outtransition : %s", repr(info->m_type).c_str());
	lib::timer *timer = new lib::timer(m_timer, 1.0, false);
	dg_transition *tr = make_transition(info->m_type, p, timer);
	m_trmap[p] = tr;
	tr->init(p->get_renderer()->get_surface(), true, info);
	tr->first_step();
	if(!m_update_event) schedule_update();
}

bool gui::dg::dg_player::has_transitions() const {
	return !m_trmap.empty();
}

void gui::dg::dg_player::update_transitions() {
	m_trmap_cs.enter();
	for(trmap_t::iterator it=m_trmap.begin();it!=m_trmap.end();it++) {
		if(!(*it).second->next_step()) {
			delete (*it).second;
			m_trmap.erase(it);
			break;
		}
	}
	m_trmap_cs.leave();
}

void gui::dg::dg_player::clear_transitions() {
	m_trmap_cs.enter();
	for(trmap_t::iterator it=m_trmap.begin();it!=m_trmap.end();it++)
		delete (*it).second;
	m_trmap.clear();
	m_trmap_cs.leave();
}

gui::dg::dg_transition *gui::dg::dg_player::get_transition(common::playable *p) {
	trmap_t::iterator it = m_trmap.find(p);
	return (it != m_trmap.end())?(*it).second:0;
}

void gui::dg::dg_player::stopped(common::playable *p) {
	m_trmap_cs.enter();
	trmap_t::iterator it = m_trmap.find(p);
	if(it != m_trmap.end()) {
		delete (*it).second;
		m_trmap.erase(it);
	}
	m_trmap_cs.leave();
}

void gui::dg::dg_player::paused(common::playable *p) {
	trmap_t::iterator it = m_trmap.find(p);
	if(it != m_trmap.end()) {
		(*it).second->pause();
	}
}

void gui::dg::dg_player::resumed(common::playable *p) {
	trmap_t::iterator it = m_trmap.find(p);
	if(it != m_trmap.end()) {
		(*it).second->resume();
	}
}

void gui::dg::dg_player::update_callback() {
	if(!m_update_event) return;
	if(has_transitions()) {
		update_transitions();
		schedule_update();
	} else {
		m_update_event = 0;
	}
}

void gui::dg::dg_player::schedule_update() {
	if(!m_player) return;
	m_update_event = new lib::no_arg_callback_event<dg_player>(this, 
		&dg_player::update_callback);
	m_worker_processor->add_event(m_update_event, 50);
}

////////////////////////
// Layout helpers with a lot of hacks

typedef common::surface_template iregion;
typedef common::passive_region region;

static const region* 
get_top_layout(smil2::smil_layout_manager *layout, const lib::node* n) {
	iregion *ir = layout->get_region(n);
	if(!ir) return 0;
	const region *r = (const region*) ir;
	while(r->get_parent()) r = r->get_parent();
	return r;
}

static const char*
get_top_layout_name(smil2::smil_layout_manager *layout, const lib::node* n) {
	const region* r = get_top_layout(layout, n);
	if(!r) return 0;
	const common::region_info *ri = r->get_info();
	return ri?ri->get_name().c_str():0;
}

common::gui_window *
gui::dg::dg_player::get_window(const lib::node* n) {
	typedef common::surface_template region;
	smil2::smil_layout_manager *layout = m_player->get_layout();
	const char *tlname = get_top_layout_name(layout, n);
	if(tlname) {
		std::map<std::string, wininfo*>::iterator it;
		it = m_windows.find(tlname);
		if(it != m_windows.end())
			return (*it).second->w;
	}
	std::map<std::string, wininfo*>::iterator it = m_windows.begin();
	assert(it != m_windows.end());
	wininfo* winfo = (*it).second;
	return winfo->w;
}

void gui::dg::dg_player::show_file(const net::url& href) {
#ifndef _WIN32_WCE
	std::string s = href.get_url();
	ShellExecute(GetDesktopWindow(), text_str("open"), textptr(s.c_str()), NULL, NULL, SW_SHOWNORMAL);
#else
	SHELLEXECUTEINFO si;
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	si.lpVerb = text_str("open"); 
	std::string s = href.get_url();
	si.lpFile = textptr(s.c_str()).c_wstr(); 
	si.nShow = SW_SHOWNORMAL; 
	ShellExecuteEx(&si);
#endif
}

void gui::dg::dg_player::done(common::player *p) {
	m_timer->pause();
	m_update_event = 0;
	clear_transitions();
	if(!m_frames.empty()) {
		frame *pf = m_frames.top();
		m_frames.pop();
		m_windows = pf->windows;
		m_player = pf->player;
		delete pf;
		resume();
		std::map<std::string, wininfo*>::iterator it;
		for(it=m_windows.begin();it!=m_windows.end();it++) {
			dg_window *dgwin = (dg_window *)(*it).second->w;
			dgwin->need_redraw();
		}		
	}
}

void gui::dg::dg_player::close(common::player *p) {
	PostMessage(m_hoster.get_main_window(), WM_CLOSE, 0, 0);
}

void gui::dg::dg_player::open(net::url newdoc, bool startnewdoc, common::player *old) {
	if(old) {
		// Replace the current document
		PostMessage(m_hoster.get_main_window(), WM_REPLACE_DOC, 
			startnewdoc?1:0, LPARAM(new std::string(newdoc.get_url()))); 
		return;
	}
	
	// Parse the provided URL. 
	lib::document *doc = lib::document::create_from_url(&m_factory, newdoc);
	if(!doc) {
		m_logger->show("Failed to parse document %s", newdoc.get_url().c_str());
		return;
	}
	
	// Push the old frame on the stack
	if(m_player) {
		pause();
		frame *pf = new frame();
		pf->windows = m_windows;
		pf->player = m_player;
		m_windows.clear();
		m_player = 0;
		m_frames.push(pf);
	}
	
	// Create a player instance
	AM_DBG m_logger->debug("Creating player instance for: %s", newdoc.get_url().c_str());
	m_player = new smil2::smil_player(doc, &m_factory, this);
	if(startnewdoc) start();
}
