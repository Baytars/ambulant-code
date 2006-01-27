/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
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

#ifndef GUI_PLAYER_H
#define GUI_PLAYER_H

#include "ambulant/config/config.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/player.h"

namespace ambulant {

namespace common {
	
class gui_player : public factories {
  public:
	gui_player()
	:	factories(),
		m_player(NULL),
		m_goto_node(NULL) {}
	virtual ~gui_player();
	
	virtual void init_playable_factory() { factories::init_playable_factory(); }
	virtual void init_window_factory() { factories::init_window_factory(); }
	virtual void init_datasource_factory() { factories::init_datasource_factory(); }
	virtual void init_parser_factory() { factories::init_parser_factory(); }
	virtual void init_plugins();
	
	virtual void play();
	virtual void stop();
	virtual void pause();
	
	virtual void restart();

//	virtual void set_speed(double speed) = 0;
//	virtual double get_speed() const = 0;
	
	virtual bool is_play_enabled() const;
	virtual bool is_stop_enabled() const;
	virtual bool is_pause_enabled() const;
	virtual bool is_play_active() const;
	virtual bool is_stop_active() const;
	virtual bool is_pause_active() const;
	
	virtual int get_cursor() const;
	virtual void set_cursor(int cursor);
	
	static void load_test_attrs(std::string& filename);
  protected:
	player *m_player;
	const lib::node *m_goto_node;	// XXX Quick hack
	lib::critical_section m_lock;
};

} // end namespaces
} // end namespaces
#endif /* GUI_PLAYER_H */
