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

#ifndef AMBULANT_MMS_MMS_PLAYER_H
#define AMBULANT_MMS_MMS_PLAYER_H

#include "ambulant/config/config.h"

#include "ambulant/common/player.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/layout.h"
#include "ambulant/mms/timelines.h"
#include "ambulant/common/playable.h"

namespace ambulant {

namespace mms {

class lib::document;

class mms_player : public common::abstract_player, public lib::ref_counted_obj {
  public:
	mms_player(lib::document *doc, common::window_factory *wf, common::playable_factory *rf);
	~mms_player();
	
	virtual lib::timer* get_timer() { return m_event_processor->get_timer(); }
	virtual lib::event_processor* get_evp() { return m_event_processor; }
	void start();
	void stop();
	void set_speed(double speed);
	
	void pause();
	void resume();
	bool is_done() const {return m_done;}
	double get_speed() const;
	
  private:
	inline void timeline_done_callback() {
		m_done = true;
	}
	
  	passive_timeline *build_timeline();
	
	lib::document *m_doc;
	lib::node *m_tree;
	lib::timer *m_timer;
  	double m_pause_speed;
	lib::event_processor *m_event_processor;
	bool m_playing;
	std::vector<active_timeline *> m_active_timelines;
	bool m_done;
	common::window_factory *m_window_factory;
	common::playable_factory *m_playable_factory;
};

} // namespace mms
 
} // namespace ambulant

#endif // AMBULANT_MMS_MMS_PLAYER_H