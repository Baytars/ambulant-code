
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
#ifndef __QT_MAINLOOP_H__
#define __QT_MAINLOOP_H__

// Environment for testing design classes

#include <iostream>
#include "ambulant/version.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/lib/system.h"
#include "ambulant/common/player.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/gui/qt/qt_renderer.h"
#include "qt_gui.h"

using namespace ambulant;
using namespace common;
using namespace lib;
using namespace gui;
using namespace qt;

void open_web_browser(const std::string &href);

class qt_mainloop_callback_arg {
};
class qt_gui;

class qt_mainloop : public ambulant::lib::system, public ambulant::lib::ref_counted {
  //  static bool m_done;
  public:
        qt_mainloop(qt_gui* parent);
	~qt_mainloop();
	
	// The callback member function.
	void player_done_callback() {
		m_running = false;
	}
	
	void play();
	void stop();
	void set_speed(double speed);
	double get_speed() const { return m_speed; }
	bool is_running() const;
	
	void show_file(const ambulant::net::url&);
	
	static void set_preferences(std::string &path);
 	
	static void* run(void* qt_mainloop);

	long add_ref() {
		return ++m_refcount;
	}
	
	long release() {
		if(--m_refcount == 0) {
			delete this;
			return 0;
		}
		return m_refcount;
	}
	
	long get_ref_count() const {
		return m_refcount;
	}
	
 private: 
	// sorted alphabetically on member name
	net::datasource_factory*		m_df;
	document*				m_doc;
	qt_gui*					m_parent;
	player*					m_player;
	basic_atomic_count<critical_section>	m_refcount;
	global_playable_factory*		m_rf;
 	bool					m_running;
	double					m_speed;
	window_factory* 			m_wf;
};
#endif/*__QT_MAINLOOP_H__*/
