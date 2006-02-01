// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/common/gui_player.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/smil2/test_attrs.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace common;

// static
void
gui_player::load_test_attrs(std::string& filename)
{
	ambulant::smil2::test_attrs::load_test_attrs(filename);
}

gui_player::~gui_player()
{
	m_lock.enter();
	if (m_player) delete m_player;
	m_player = NULL;
	m_lock.leave();
}

void
gui_player::init_plugins()
{
	m_lock.enter();
	common::plugin_engine *plf = common::plugin_engine::get_plugin_engine();
	plf->add_plugins(this, this);
	m_lock.leave();
}

void
gui_player::play()
{
	m_lock.enter();
	if (!m_player) {
		ambulant::lib::logger::get_logger()->error(gettext("Cannot play document: no player"));
		return;
	}
	assert(m_playable_factory);
	assert(m_window_factory);
	assert(m_datasource_factory);
	assert(m_parser_factory);
	m_player->start();
	if (m_goto_node) {
		bool ok = m_player->goto_node(m_goto_node);
		if (!ok)
			ambulant::lib::logger::get_logger()->trace("gui_player::run: goto_node failed");
	} 
	AM_DBG ambulant::lib::logger::get_logger()->debug("gui_player::run(): returning");
	m_lock.leave();
}

void
gui_player::stop()
{
	m_lock.enter();
	if (m_player) m_player->stop();
	AM_DBG ambulant::lib::logger::get_logger()->debug("gui_player::run(): returning");
	m_lock.leave();
}

void
gui_player::pause()
{
	m_lock.enter();
	if (m_player) m_player->pause();
	AM_DBG ambulant::lib::logger::get_logger()->debug("gui_player::pause(): returning");
	m_lock.leave();
}

void
gui_player::restart()
{
	ambulant::lib::logger::get_logger()->error("Restarting presentation not implemented in this release.");
}

bool
gui_player::is_play_enabled() const
{
	const_cast<gui_player*>(this)->m_lock.enter();
	bool rv = m_player != NULL;
	const_cast<gui_player*>(this)->m_lock.leave();
	return rv;
}

bool
gui_player::is_stop_enabled() const
{
	const_cast<gui_player*>(this)->m_lock.enter();
	bool rv = m_player != NULL;
	const_cast<gui_player*>(this)->m_lock.leave();
	return rv;
}

bool
gui_player::is_pause_enabled() const
{
	const_cast<gui_player*>(this)->m_lock.enter();
	bool rv = m_player && !m_player->is_done();
	const_cast<gui_player*>(this)->m_lock.leave();
	return rv;
}

bool
gui_player::is_play_active() const
{
	const_cast<gui_player*>(this)->m_lock.enter();
	bool rv = m_player?m_player->is_playing():false;
	const_cast<gui_player*>(this)->m_lock.leave();
	return rv;
}

bool
gui_player::is_stop_active() const
{
	const_cast<gui_player*>(this)->m_lock.enter();
	bool rv = m_player?m_player->is_done():false;
	const_cast<gui_player*>(this)->m_lock.leave();
	return rv;
}

bool
gui_player::is_pause_active() const
{
	const_cast<gui_player*>(this)->m_lock.enter();
	bool rv = m_player?m_player->is_pausing():false;
	const_cast<gui_player*>(this)->m_lock.leave();
	return rv;
}
	
int
gui_player::get_cursor() const
{
	const_cast<gui_player*>(this)->m_lock.enter();
	int rv = m_player?m_player->get_cursor():0;
	const_cast<gui_player*>(this)->m_lock.leave();
	return rv;
}

void
gui_player::set_cursor(int cursor)
{
	m_lock.enter();
	if(m_player) m_player->set_cursor(cursor);
	m_lock.leave();
}

