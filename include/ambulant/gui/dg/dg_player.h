/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_DG_PLAYER_H
#define AMBULANT_GUI_DG_PLAYER_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#include "ambulant/config/config.h"

#include <string>
#include <map>
#include <stack>

// The interfaces implemented by dx_player
#include "ambulant/common/player.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/embedder.h"
#include "ambulant/common/gui_player.h"
#include "ambulant/net/url.h"
#include "ambulant/lib/timer.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/gui/dg/dg_playable.h"


namespace ambulant {

// classes used by dg_player

namespace lib {
	class event_processor;
	class logger;
	class transition_info;
	class event;
	
}

namespace mms {
	class mms_player;
}

namespace smil2 {
	class smil_player;
}

namespace gui {

namespace dg {

// Global functions provided by the hosting application.
class dg_player_callbacks {
  public:
	virtual HWND new_os_window() = 0;
	virtual void destroy_os_window(HWND hwnd) = 0;
};

class viewport;
class dg_window;
class dg_transition;

class dg_playable_factory : public common::playable_factory {
  public:
	  dg_playable_factory(
			common::factories *factory,
			lib::logger *logger,
			dg_playables_context *ctx,
			common::gui_window *window)
	:	m_factory(factory),
		m_logger(logger),
		m_dgplayer(ctx),
		m_window(window) {}
	////////////////////
	// common::playable_factory implementation
	
	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const ambulant::lib::node *node,
		lib::event_processor * evp);

	common::playable *new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src);
  private:
	common::factories *m_factory;
	lib::logger *m_logger;
	dg_playables_context *m_dgplayer;
	common::gui_window *m_window;
};

class dg_player : 
	public common::gui_player, 
	public common::window_factory, 
	public dg_playables_context,
	public common::embedder {
	
  public:
	dg_player(dg_player_callbacks &hoster, const net::url& u);
	~dg_player();
	
	////////////////////
	// common::gui_player implementation
	void init_playable_factory();
	void init_window_factory();
	void init_datasource_factory();
	void init_parser_factory();

	// should these be part of the player interface?
	lib::timer* get_timer() { return 0;}
	lib::event_processor* get_evp() { return 0;}
	
	
	////////////////////
	// common::window_factory implementation
	
	common::gui_window *new_window(const std::string& name, 
		lib::size bounds, common::gui_events *src);
			
	common::bgrenderer *new_background_renderer(const common::region_info *src);
	
	void window_done(const std::string& name);
	
	////////////////////
	////////////////////
	// common::embedder implementation
	void show_file(const net::url& href);
	void close(common::player *p);
	void open(net::url newdoc, bool start, common::player *old=NULL);
	void done(common::player *p);
	
	////////////////////
	// Implementation specific artifacts
	
	void on_char(int ch);
	void on_click(int x, int y, HWND hwnd);
	int get_cursor(int x, int y, HWND hwnd);
	std::string get_pointed_node_str();
//	const net::url& get_url() const { return m_url;}
	
	viewport* create_viewport(int w, int h, HWND hwnd);
	void redraw(HWND hwnd, HDC hdc);
	void on_done();
	
	// Timeslices services and transitions
	void update_callback();
	void schedule_update();
	void update_transitions();
	void clear_transitions();
	bool has_transitions() const;
	void stopped(common::playable *p);
	void paused(common::playable *p);
	void resumed(common::playable *p);
	void set_intransition(common::playable *p, const lib::transition_info *info);
	void start_outtransition(common::playable *p, const lib::transition_info *info);
	dg_transition *get_transition(common::playable *p);
	
  private:
//	common::gui_window* get_window(const lib::node* n);
	common::gui_window* get_window(HWND hwnd);
	HWND get_main_window();

	// Callbacks to the hosting program
  	dg_player_callbacks &m_hoster;
  	
	// The current view	
	struct wininfo {HWND h; viewport *v; dg_window *w; long f;};
	std::map<std::string, wininfo*> m_windows;	
	wininfo* get_wininfo(HWND hwnd);
	
	// The frames stack
	struct frame {std::map<std::string, wininfo*> windows; common::player* player;};
	std::stack<frame*> m_frames;
	
	// The secondary timer and processor
	lib::timer_control *m_timer;
	lib::event_processor *m_worker_processor;	
		
	lib::event *m_update_event;
	typedef std::map<common::playable *, dg_transition*> trmap_t;
	trmap_t m_trmap;
	lib::critical_section m_trmap_cs;
	
	lib::logger *m_logger;
};

} // namespace dg

} // namespace gui
 
} // namespace ambulant
#endif // AMBULANT_GUI_DG_PLAYER_H
