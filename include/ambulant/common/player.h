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

#ifndef AMBULANT_LIB_PLAYER_H
#define AMBULANT_LIB_PLAYER_H

#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/timelines.h"
#include "ambulant/common/region.h"
#include "ambulant/common/renderer.h"

namespace ambulant {

namespace lib {

// Forward
class active_player;

namespace detail {

} // namespace detail

// I'm not clear on the split between active and passive player yet. The
// passive_player could do loading of the document and parsing it, and
// possibly even generating the timelines.
// Splitting the player in active/passive may be a case of over-generality,
// in which case we get rid of the passive_player later.
class passive_player {
  public:
	friend class active_player;
	
	passive_player() 
	:	m_url("") {}
	passive_player(const char *url)
	:	m_url(url) {}
	~passive_player() {}
	
	active_player *activate(window_factory *wf, renderer_factory *rf);
  private:
  	const char *m_url;
};

class active_player : public ref_counted_obj {
  public:
	active_player(passive_player *const source, node *tree, window_factory *wf, renderer_factory *rf);
	~active_player();
	
	void start(event_processor *evp, event *playdone);
	void stop();
	void set_speed(double speed);
	
	inline void timeline_done_callback() {
		m_done = true;
	}
	
	void start() { start(NULL, NULL);}
	bool is_done() const {return m_done;}
	double get_speed() const;
	
  private:
  	passive_timeline *build_timeline();
	
	node *m_tree;
	timer *m_timer;
  	event_processor *const m_event_processor;
	passive_player *const m_source;
	bool m_playing;
	std::vector<active_timeline *> m_active_timelines;
	bool m_done;
	window_factory *m_window_factory;
	renderer_factory *m_renderer_factory;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_PLAYER_H
