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
#include "ambulant/version.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/refcount.h"
//#include "ambulant/lib/event_processor.h"
//#include "ambulant/lib/asb.h"
#include "ambulant/common/player.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/renderer.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/system.h"

class mainloop : public ambulant::lib::system, public ambulant::lib::ref_counted_obj {
  public:
	mainloop(const char *filename, ambulant::common::window_factory *wf, bool use_mms);
	~mainloop();
	
	// The callback member function.
	void player_done_callback() {
		m_running = false;
	}
	
	void play();
	void stop();
	void set_speed(double speed);
	double get_speed() const { return m_speed; }
	bool is_running() const;
	
	int get_cursor() const {return m_player?m_player->get_cursor():0; };
	void set_cursor(int cursor) { if (m_player) m_player->set_cursor(cursor); }
	
	void show_file(const std::string& href);
	
	static void set_preferences(std::string &path);
  private:
	ambulant::lib::document *create_document(const char *filename);
  	bool m_running;
	double m_speed;
	ambulant::lib::document *m_doc;
	ambulant::common::player *m_player;
	ambulant::common::global_playable_factory *m_rf;
	ambulant::common::window_factory *m_wf;
	ambulant::net::datasource_factory *m_df;
};
