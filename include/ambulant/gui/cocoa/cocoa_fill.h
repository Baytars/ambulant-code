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

#ifndef AMBULANT_GUI_COCOA_COCOA_FILL_H
#define AMBULANT_GUI_COCOA_COCOA_FILL_H

#include "ambulant/common/renderer.h"
#include "ambulant/smil2/transition.h"
#include "ambulant/lib/mtsync.h"
#include <Cocoa/Cocoa.h>

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace cocoa {

class cocoa_active_fill_renderer : public active_basic_renderer {
  public:
	cocoa_active_fill_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp)
	:	active_basic_renderer(context, cookie, node, evp),
		m_dest(NULL),
		m_intransition(NULL),
		m_outtransition(NULL),
		m_trans_engine(NULL),
		m_playing(false) {};
	~cocoa_active_fill_renderer();

	void freeze() {}
	void start(double where);
	void stop();
	void pause() {}
	void resume() {}
	void wantclicks(bool want) { if (m_dest) m_dest->need_events(want); }

	renderer *get_renderer() { return this; }
	void set_surface(surface *dest) { m_dest = dest; }
	void set_intransition(lib::transition_info *info) { m_intransition = info; }
	void set_outtransition(lib::transition_info *info) { m_outtransition = info; }
	surface *get_surface() { return m_dest;}
	void user_event(const point &where, int what = 0) { user_event_callback(what); }
    void redraw(const screen_rect<int> &dirty, abstract_window *window);
  private:
	void transition_step();
	
	surface *m_dest;
	lib::transition_info *m_intransition, *m_outtransition;
	smil2::transition_engine *m_trans_engine;
	bool m_playing;
	critical_section m_lock;
};

class cocoa_background_renderer : public background_renderer {
  public:
    cocoa_background_renderer(const common::region_info *src)
	:   background_renderer(src) {}
	void redraw(const lib::screen_rect<int> &dirty, common::abstract_window *window);
  private:
	
};

} // namespace cocoa

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_COCOA_COCOA_FILL_H
