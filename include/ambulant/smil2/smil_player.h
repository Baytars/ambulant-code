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

#ifndef AMBULANT_LIB_SMIL_PLAYER_H
#define AMBULANT_LIB_SMIL_PLAYER_H

//////////////////////////////////////////////
//
// EXPERIMENTAL TEST IMPLEMENTATION
// 
//////////////////////////////////////////////

#include "ambulant/lib/timer.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/event.h"
#include "ambulant/smil2/time_node.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/player.h"
#include <map>

namespace ambulant {

namespace lib {

class document;
class window_factory;
class renderer_factory;
class node;
class active_basic_renderer;
class layout_manager;
class logger;

class smil_player : public abstract_player, public time_node_context, public playable_events {
  public:
	typedef time_traits::value_type time_value_type;
	
	smil_player(document *doc, window_factory *wf, renderer_factory *rf);
	~smil_player();
		
	timer* get_timer() { return m_timer;}
	event_processor* get_evp() { return m_event_processor;}
	
	// Builds or re-builds the layout
	// The layout may need to be rebuild when the
	// user changes custom test preferences. 
	void build_layout();
	
	// Builds or re-builds the timegraph
	// The timegraph may need to be rebuild when the
	// user changes custom test preferences. 
	void build_timegraph();
	
	///////////////////
	// UI commands
	
	void start();
	void stop();
	void pause();
	void resume();
	bool is_done() const;
		
	//////////////////////
	// Time node context: Services
	
	virtual time_traits::value_type elapsed() const { return m_timer->elapsed();}
	virtual void schedule_event(event *ev, time_type t, event_priority ep = ep_low);
	virtual void cancel_event(event *ev, event_priority ep = ep_low) 
		{ m_event_processor->cancel_event(ev, (event_processor::event_priority)ep);}
	virtual void cancel_all_events() { m_event_processor->cancel_all_events();}
	
	//////////////////////
	// Time node context: Playable commands
	
	virtual void start_playable(const node *n, double t);
	virtual void stop_playable(const node *n);
	virtual void pause_playable(const node *n, pause_display d = display_show);
	virtual void resume_playable(const node *n);
	virtual void wantclicks_playable(const node *n, bool want);
	
	//////////////////////
	// Time node context: Playable queries
	virtual std::pair<bool, double> get_dur(const node *n);
	
	//////////////////
	// Time node context: Notifications
	
	virtual void done_playback();

	///////////////////
	// playable_events interface
	
	virtual void started(int n, double t);
	virtual void stopped(int n, double t);
	virtual void clicked(int n, double t);	
	
	// raw notifications from the UI
	virtual void on_click(int x, int y);
	virtual void on_char(int ch);
	
  private:
	active_basic_renderer *create_renderer(const node *n); 
	void destroy_renderer(active_basic_renderer *r, const node *n); 
	active_basic_renderer *get_renderer(const node *n);
	document *m_doc;
	window_factory *m_wf;
	renderer_factory *m_rf;
	time_node* m_root;
	std::map<int, time_node*> *m_dom2tn;
	layout_manager *m_layout_manager;
	timer *m_timer;
	event_processor *m_event_processor;	
	std::map<const node*, active_basic_renderer *> m_renderers;
	logger *m_logger;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_REGION_H
