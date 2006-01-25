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

#ifndef FACTORY_H
#define FACTORY_H

#include "ambulant/lib/parser_factory.h"
#include "ambulant/net/datasource.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/player.h"

namespace ambulant {

namespace common {
	
class factories {
public:
	factories();
	virtual ~factories();
	virtual void init_playable_factory();
	virtual void init_window_factory();
	virtual void init_datasource_factory();
	virtual void init_parser_factory();
	
	ambulant::common::playable_factory *get_playable_factory() const { return m_playable_factory; }
	ambulant::common::window_factory *get_window_factory() const { return m_window_factory; }
	ambulant::net::datasource_factory *get_datasource_factory() const { return m_datasource_factory; }
	ambulant::lib::global_parser_factory *get_parser_factory() const { return m_parser_factory; }
protected:
	ambulant::common::playable_factory *m_playable_factory;
	ambulant::common::window_factory *m_window_factory;
	ambulant::net::datasource_factory *m_datasource_factory;
	ambulant::lib::global_parser_factory *m_parser_factory;
};

class gui_player : public factories {
  public:
	gui_player()
	:	m_player(NULL) {}
	virtual ~gui_player();
	
	virtual void play() = 0;
	virtual void stop() = 0;
	virtual void pause() = 0;
	
//	virtual void set_speed(double speed) = 0;
//	virtual double get_speed() const = 0;
	
	virtual bool is_play_enabled() const { return m_player != NULL; };
	virtual bool is_stop_enabled() const { return m_player != NULL; };
	virtual bool is_pause_enabled() const { return m_player != NULL; };
	virtual bool is_play_active() const { return m_player?m_player->is_playing():false; };
	virtual bool is_stop_active() const { return m_player?m_player->is_done():false; };
	virtual bool is_pause_active() const { return m_player?m_player->is_pausing():false; };
	
	virtual int get_cursor() const { return m_player?m_player->get_cursor():0; };
	virtual void set_cursor(int cursor) {if(m_player) m_player->set_cursor(cursor); };
	
	virtual void set_preferences(std::string& filename) = 0;
  protected:
	player *m_player;
};

} // end namespaces
} // end namespaces


	


	

#endif /* _FACTORY_H */
