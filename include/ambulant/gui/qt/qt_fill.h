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
 *` gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception.
 *
 */

/* 
 * $Id$
 */

#ifndef AMBULANT_GUI_QT_QT_FILL_H
#define AMBULANT_GUI_QT_QT_FILL_H

#include "ambulant/lib/mtsync.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer.h"
#include "ambulant/gui/none/none_gui.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

namespace gui {

namespace qt {

class qt_active_fill_renderer : public common::active_basic_renderer {
  public:
	qt_active_fill_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
		net::passive_datasource *src)
: common::active_basic_renderer(context, cookie, node, evp),
	m_dest(NULL),
	m_playing(false) {
};
	~qt_active_fill_renderer();

 	void start(double where) {  // XXXX
		m_playing = 1;
	}
	void freeze() {
	}
	void stop() {
		m_playing = 0;
	}
	void pause() {
	}
	void resume() {
	}
	void wantclicks(bool want) { // XXXX
	}
	void user_event(const lib::point &where) {
		clicked_callback();
	}
	void redraw(const lib::screen_rect<int> &dirty, 
		    common::abstract_window *window);
  private:
	common::surface *const m_dest;
	bool m_playing;
	lib::critical_section m_lock;
};

class qt_background_renderer : public common::abstract_bg_rendering_source {
  public:
	void drawbackground(
		const common::region_info *src,
		const lib::screen_rect<int> &dirty,
		common::surface *dst,
		common::abstract_window *windo);
};

} // namespace qt

} // namespace gui

} // namespace ambulant
#endif  /*AMBULANT_GUI_QT_QT_FILL_H*/
