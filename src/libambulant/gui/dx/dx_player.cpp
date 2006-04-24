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

#include "ambulant/gui/dx/dx_player.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_wmuser.h"
#include "ambulant/gui/dx/dx_rgn.h"
#include "ambulant/gui/dx/dx_transition.h"

#include "ambulant/lib/event.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/transition_info.h"

//#include "ambulant/common/plugin_engine.h"

#include "ambulant/smil2/transition.h"

// Players
#include "ambulant/smil2/smil_player.h"
#include "ambulant/mms/mms_player.h"
#include "ambulant/smil2/test_attrs.h"

// Renderer playables
#include "ambulant/gui/dx/html_bridge.h"
#include "ambulant/gui/dx/dx_bgrenderer.h"
#include "ambulant/gui/dx/dx_text.h"
#include "ambulant/gui/dx/dx_html_renderer.h"
#include "ambulant/gui/dx/dx_img.h"
#include "ambulant/gui/dx/dx_audio.h"
#include "ambulant/gui/dx/dx_video.h"
#include "ambulant/gui/dx/dx_brush.h"

// "Renderer" playables
#include "ambulant/gui/dx/dx_audio.h"

// Playables
#include "ambulant/gui/dx/dx_area.h"

// Layout
#include "ambulant/common/region.h"
#include "ambulant/smil2/smil_layout.h"

// Xerces parser
#ifdef WITH_XERCES_BUILTIN
#include "ambulant/lib/xerces_parser.h"
#endif

// Datasources
#include "ambulant/net/datasource.h"
#include "ambulant/net/win32_datasource.h"
//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

int gui::dx::dx_gui_region::s_counter = 0;

gui::dx::dx_player::dx_player(dx_player_callbacks &hoster, common::player_feedback *feedback, const net::url& u) 
:	m_hoster(hoster),
	m_update_event(0),
	m_logger(lib::logger::get_logger())
{
	set_embedder(this);
	// Fill the factory objects
	init_factories();
	init_plugins();

	// Parse the provided URL. 
	AM_DBG m_logger->debug("Parsing: %s", u.get_url().c_str());	
	m_doc = create_document(u);
	
	if(!m_doc) {
		// message already logged
		return;
	}
#ifdef TEMPORARILY_REMOVED_BECAUSE_IT_IS_BROKEN
	// If there's a fragment ID remember the node it points to,
	// and when we first start playback we'll go there.
	const std::string& idd = u.get_ref();
	if (idd != "") {
		const lib::node *node = doc->get_node(idd);
		if (node) {
			m_goto_node = node;
		} else {
			m_logger->warn(gettext("%s: node ID not found"), idd.c_str());
		}
	}
#endif
	// Create a player instance
	AM_DBG m_logger->debug("Creating player instance for: %s", u.get_url().c_str());	
	m_player = new smil2::smil_player(m_doc, this, m_embedder);
#ifdef USE_SMIL21
	m_player->initialize();
#endif
	if (feedback) m_player->set_feedback(feedback);
	
	// Create a worker processor instance
}

gui::dx::dx_player::~dx_player() {
	if(m_player) stop();
	delete m_player;
	while(!m_frames.empty()) {
		frame *pf = m_frames.top();
		m_frames.pop();
		m_windows = pf->windows;
		m_player = pf->player;
		m_doc = pf->doc;
		delete pf;
		stop();
		delete m_player;
	}
	m_player = NULL;
	assert(m_windows.empty());
	if(dx_gui_region::s_counter != 0) 
		m_logger->warn("Undeleted gui regions: %d", dx_gui_region::s_counter);
}

void
gui::dx::dx_player::init_playable_factory()
{
	m_playable_factory = common::get_global_playable_factory();
	// Add the playable factory
	m_playable_factory->add_factory(new dx_playable_factory(this, m_logger, this));
}

void
gui::dx::dx_player::init_window_factory()
{
		m_window_factory = this; 
}

void
gui::dx::dx_player::init_datasource_factory()
{
	m_datasource_factory = new net::datasource_factory();
	// Add the datasource factories. For now we only need a raw
	// datasource factory.
	m_datasource_factory->add_raw_factory(net::get_win32_datasource_factory());
}

void
gui::dx::dx_player::init_parser_factory()
{
	m_parser_factory = lib::global_parser_factory::get_parser_factory();
	// Add the xerces parser, if available
#ifdef WITH_XERCES_BUILTIN
	m_parser_factory->add_factory(new lib::xerces_factory());
#endif
}


void gui::dx::dx_player::play() {
	if(m_player) {
		common::gui_player::play();
		std::map<std::string, wininfo*>::iterator it;
		for(it=m_windows.begin();it!=m_windows.end();it++) {
			dx_window *dxwin = (dx_window *)(*it).second->w;
			dxwin->need_redraw();
		}
	}
}

void gui::dx::dx_player::stop() {
	if(m_player) {
		m_update_event = 0;
		clear_transitions();
		common::gui_player::stop();
	}
}

void gui::dx::dx_player::pause() {
	if(m_player) {
		common::gui_player::pause();
	}
}

void gui::dx::dx_player::restart(bool reparse) {
	bool playing = is_play_active();
	stop();
	
	delete m_player;
	while(!m_frames.empty()) {
		frame *pf = m_frames.top();
		m_frames.pop();
		m_windows = pf->windows;
		m_player = pf->player;
		m_doc = pf->doc;
		delete pf;
		stop();
		delete m_player;
	}
	m_player = 0;
	if (reparse) {
		m_doc = create_document(m_url);
		if(!m_doc) {
			m_logger->show("Failed to parse document %s", m_url.get_url().c_str());
			return;
		}
	}
	AM_DBG m_logger->debug("Creating player instance for: %s", m_url.get_url().c_str());	
	m_player = new smil2::smil_player(m_doc, this, m_embedder);	
	
	if(playing) play();	
}

#if 0
bool gui::dx::dx_player::is_playing() const {
	return (m_player && m_player->is_playing()) || !m_frames.empty();
}

bool gui::dx::dx_player::is_pausing() const {
	return m_player && m_player->is_pausing();
}

bool gui::dx::dx_player::is_done() const {
	return m_player && m_player->is_done() && m_frames.empty();
}

void gui::dx::dx_player::set_preferences(const std::string& url) {
	smil2::test_attrs::load_test_attrs(url);
	if(is_playing()) stop();
	if(m_player) m_player->build_timegraph();
}
#endif
void gui::dx::dx_player::on_click(int x, int y, HWND hwnd) {
	if(!m_player) return;
	lib::point pt(x, y);
	dx_window *dxwin = (dx_window *) get_window(hwnd);
	if(!dxwin) return;
	region *r = dxwin->get_region();
	if(r)
		r->user_event(pt, common::user_event_click);
}

int gui::dx::dx_player::get_cursor(int x, int y, HWND hwnd) {
	if(!m_player) return 0;
	lib::point pt(x, y);
	dx_window *dxwin = (dx_window *) get_window(hwnd);
	if(!dxwin) return 0;
	region *r = dxwin->get_region();
	m_player->before_mousemove(0);
	if(r) r->user_event(pt, common::user_event_mouse_over);
	return m_player->after_mousemove();
}

std::string gui::dx::dx_player::get_pointed_node_str() {
#if 1
	return "";
#else
	if(!m_player || !is_playing()) return "";
	return m_player->get_pointed_node_str();
#endif
}

void gui::dx::dx_player::on_char(int ch) {
	if(m_player) m_player->on_char(ch);
}

void gui::dx::dx_player::redraw(HWND hwnd, HDC hdc) {
	wininfo *wi = get_wininfo(hwnd);
	if(wi) wi->v->redraw(hdc);
}

void gui::dx::dx_player::on_done() {
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		(*it).second->v->clear();
		(*it).second->v->redraw();
	}
}

void gui::dx::dx_player::lock_redraw() {
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		(*it).second->w->lock_redraw();
	}
}

void gui::dx::dx_player::unlock_redraw() {
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		(*it).second->w->unlock_redraw();
	}
}

////////////////////
// common::window_factory implementation

common::gui_window *
gui::dx::dx_player::new_window(const std::string &name, 
	lib::size bounds, common::gui_events *src) {
	
	AM_DBG lib::logger::get_logger()->debug("dx_window_factory::new_window(%s): %s", 
		name.c_str(), ::repr(bounds).c_str());
	
	// wininfo struct that will hold the associated objects
	wininfo *winfo = new wininfo;
	
	// Create an os window
	winfo->h = m_hoster.new_os_window();
	
	// Create the associated dx viewport
	winfo->v = create_viewport(bounds.w, bounds.h, winfo->h);
	
	// Region?
	region *rgn = (region *) src;
	
	// Clear the viewport
	const common::region_info *ri = rgn->get_info();
	winfo->v->set_background(ri?ri->get_bgcolor():CLR_INVALID);
	winfo->v->clear();
	
	// Create a concrete gui_window
	winfo->w = new dx_window(name, bounds, rgn, this, winfo->v);
	winfo->f = 0;
	
	// Store the wininfo struct
	m_windows[name] = winfo;
	AM_DBG m_logger->debug("windows: %d", m_windows.size());
	
	// Return gui_window
	return winfo->w;
}

void 
gui::dx::dx_player::window_done(const std::string &name) {
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
gui::dx::dx_player::new_background_renderer(const common::region_info *src) {
	return new dx_bgrenderer(src);
}

gui::dx::viewport* gui::dx::dx_player::create_viewport(int w, int h, HWND hwnd) {
	AM_DBG m_logger->debug("dx_player::create_viewport(%d, %d)", w, h);
	PostMessage(hwnd, WM_SET_CLIENT_RECT, w, h);
	viewport *v = new gui::dx::viewport(w, h, hwnd);
	v->redraw();
	return v;
}

gui::dx::dx_player::wininfo*
gui::dx::dx_player::get_wininfo(HWND hwnd) {
	wininfo *winfo = 0;
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		wininfo *wi = (*it).second;
		if(wi->h = hwnd) {winfo = wi;break;}
	}
	return winfo;
}

common::gui_window *
gui::dx::dx_player::get_window(HWND hwnd) {
	wininfo *wi = get_wininfo(hwnd);
	return wi?wi->w:0;
}

HWND
gui::dx::dx_player::get_main_window() {
	// XXXX Unsure that this is correct: we just return any window
	std::map<std::string, wininfo*>::iterator it = m_windows.begin();
	if (it == m_windows.end()) return NULL;
	return (*it).second->h;
}

////////////////////
// common::playable_factory implementation

common::playable *
gui::dx::dx_playable_factory::new_playable(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp) {
	common::playable *p = 0;
	lib::xml_string tag = node->get_qname().second;
	AM_DBG m_logger->debug("dx_player::new_playable: %s", tag.c_str());
	if(tag == "text") {
#ifdef	WITH_HTML_WIDGET
		net::url url = net::url(node->get_url("src"));
		if (url.guesstype() == "text/html") {
			p = new dx_html_renderer(context, cookie, node, evp, m_dxplayer);
			AM_DBG lib::logger::get_logger()->debug("dx_player: node 0x%x: returning dx_html_renderer 0x%x", (void*) node, (void*) p);
		} else 
#endif/*WITH_HTML_WIDGET*/
		p = new dx_text_renderer(context, cookie, node, evp, m_factory, m_dxplayer);
	} else if(tag == "img") {
		p = new dx_img_renderer(context, cookie, node, evp, m_factory, m_dxplayer);
	} else if(tag == "audio") {
		p = new dx_audio_renderer(context, cookie, node, evp);
	} else if(tag == "video") {
		p = new dx_video_renderer(context, cookie, node, evp, m_dxplayer);
	} else if(tag == "area") {
		p = new dx_area(context, cookie, node, evp);
	} else if(tag == "brush") {
		p = new dx_brush(context, cookie, node, evp, m_dxplayer);
	} else {
		p = new dx_area(context, cookie, node, evp);
	}
	return p;
}

common::playable *
gui::dx::dx_playable_factory::new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src)
{
	return NULL;
}

void gui::dx::dx_player::set_intransition(common::playable *p, const lib::transition_info *info) { 
	AM_DBG lib::logger::get_logger()->debug("set_intransition : %s", repr(info->m_type).c_str());
	dx_transition *tr = set_transition(p, info, false);
	// XXXX Note by Jack: the next two steps really shouldn't be done for
	// intransitions, they should be done later (when playback starts).
	tr->first_step();
	if(!m_update_event) schedule_update();
}

void gui::dx::dx_player::start_outtransition(common::playable *p, const lib::transition_info *info) {  
	lib::logger::get_logger()->debug("start_outtransition : %s", repr(info->m_type).c_str());
	dx_transition *tr = set_transition(p, info, true);
	// XXXX Note by Jack: the next two steps really shouldn't be done for
	// intransitions, they should be done later (when playback starts).
	tr->first_step();
	if(!m_update_event) schedule_update();
}

gui::dx::dx_transition *
gui::dx::dx_player::set_transition(common::playable *p, 
								   const lib::transition_info *info, 
								   bool is_outtransition)
{  
	assert(m_player);
	lib::timer_control *timer = new lib::timer_control_impl(m_player->get_timer(), 1.0, false);
	dx_transition *tr = make_transition(info->m_type, p, timer);
	m_trmap[p] = tr;
	common::surface *surf = p->get_renderer()->get_surface();
#ifdef USE_SMIL21
	if (info->m_scope == lib::scope_screen) surf = surf->get_top_surface();
#endif
	tr->init(surf, is_outtransition, info);
	return tr;
}

bool gui::dx::dx_player::has_transitions() const {
	return !m_trmap.empty();
}

void gui::dx::dx_player::update_transitions() {
	m_trmap_cs.enter();
	assert(m_player);
	lib::timer::time_type pt = m_player->get_timer()->elapsed();
	//lock_redraw();
	for(trmap_t::iterator it=m_trmap.begin();it!=m_trmap.end();it++) {
		if(!(*it).second->next_step(pt)) {
			delete (*it).second;
			common::playable *p = (*it).first;
			it = m_trmap.erase(it);
			common::renderer *r = p->get_renderer();
			if (r) {
				common::surface *surf = r->get_surface();
				if (surf) surf->transition_done();
			}
		}
	}
	//unlock_redraw();
	m_trmap_cs.leave();
}

void gui::dx::dx_player::clear_transitions() {
	m_trmap_cs.enter();
	for(trmap_t::iterator it=m_trmap.begin();it!=m_trmap.end();it++)
		delete (*it).second;
	m_trmap.clear();
	m_trmap_cs.leave();
}

gui::dx::dx_transition *gui::dx::dx_player::get_transition(common::playable *p) {
	trmap_t::iterator it = m_trmap.find(p);
	return (it != m_trmap.end())?(*it).second:0;
}

void gui::dx::dx_player::stopped(common::playable *p) {
	m_trmap_cs.enter();
	trmap_t::iterator it = m_trmap.find(p);
	if(it != m_trmap.end()) {
		delete (*it).second;
		common::playable *p = (*it).first;
		it = m_trmap.erase(it);
		common::renderer *r = p->get_renderer();
		if (r) {
			common::surface *surf = r->get_surface();
			if (surf) surf->transition_done();
		}
	}
	m_trmap_cs.leave();
}

void gui::dx::dx_player::paused(common::playable *p) {
	trmap_t::iterator it = m_trmap.find(p);
	if(it != m_trmap.end()) {
		(*it).second->pause();
	}
}

void gui::dx::dx_player::resumed(common::playable *p) {
	trmap_t::iterator it = m_trmap.find(p);
	if(it != m_trmap.end()) {
		(*it).second->resume();
	}
}

void gui::dx::dx_player::update_callback() {
	if(!m_update_event) return;
	if(has_transitions()) {
		update_transitions();
		schedule_update();
	} else {
		m_update_event = 0;
	}
}

void gui::dx::dx_player::schedule_update() {
	if(!m_player) return;
	lib::event_processor *evp = m_player->get_evp();
	m_update_event = new lib::no_arg_callback_event<dx_player>(this, 
		&dx_player::update_callback);
	evp->add_event(m_update_event, 50, ep_high);
}

////////////////////////
// Layout helpers with a lot of hacks

typedef common::surface_template iregion;
typedef common::surface_impl region;

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

#if 0
common::gui_window *
gui::dx::dx_player::get_window(const lib::node* n) {
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
#endif

void gui::dx::dx_player::show_file(const net::url& href) {
	ShellExecute(GetDesktopWindow(), text_str("open"), textptr(href.get_url().c_str()), NULL, NULL, SW_SHOWNORMAL);
	
	// Or for smil
	//std::string this_exe = lib::win32::get_module_filename();
	//std::string cmd = this_exe + " " + newdoc.get_url();
	//if(start) cmd += " /start";
	//WinExec(cmd.c_str(), SW_SHOW);
}


void gui::dx::dx_player::done(common::player *p) {
	m_update_event = 0;
	clear_transitions();
	if(!m_frames.empty()) {
		frame *pf = m_frames.top();
		m_frames.pop();
		m_windows = pf->windows;
		m_player = pf->player;
		m_doc = pf->doc;
		delete pf;
		assert(0); // resume();
		std::map<std::string, wininfo*>::iterator it;
		for(it=m_windows.begin();it!=m_windows.end();it++) {
			dx_window *dxwin = (dx_window *)(*it).second->w;
			dxwin->need_redraw();
		}		
	}
}

void gui::dx::dx_player::close(common::player *p) {
	PostMessage(get_main_window(), WM_CLOSE, 0, 0);
}

void gui::dx::dx_player::open(net::url newdoc, bool startnewdoc, common::player *old) {
	std::string urlstr = newdoc.get_url();
	if(old) {
		// Replace the current document
		PostMessage(get_main_window(), WM_REPLACE_DOC, 
			startnewdoc?1:0, LPARAM(new std::string(urlstr))); 
		return;
	}
	if(!lib::ends_with(urlstr, ".smil") && !lib::ends_with(urlstr, ".smi") &&
		!lib::ends_with(urlstr, ".grins")) {
		show_file(newdoc);
		return;
	}
	
	// Parse the document. 
	m_doc = create_document(newdoc);
	
	// Push the old frame on the stack
	if(m_player) {
		pause();
		frame *pf = new frame();
		pf->windows = m_windows;
		pf->player = m_player;
		pf->doc = m_doc;
		m_windows.clear();
		m_player = 0;
		m_frames.push(pf);
	}
	
	// Create a player instance
	AM_DBG m_logger->debug("Creating player instance for: %s", newdoc.get_url().c_str());
	m_player = new smil2::smil_player(m_doc, this, m_embedder);
	if(startnewdoc) play();
}

