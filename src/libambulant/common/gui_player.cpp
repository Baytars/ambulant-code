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
		m_lock.leave();
		return;
	}
	assert(m_playable_factory);
	assert(m_window_factory);
	assert(m_datasource_factory);
	assert(m_parser_factory);
	common::player *p = m_player;
//XXX	if (p->is_playing()) lib::logger::get_logger()->debug("gui_player::play: already playing!");
	p->start();
	if (m_goto_node) {
		bool ok = p->goto_node(m_goto_node);
		if (!ok)
			ambulant::lib::logger::get_logger()->trace("gui_player::play: goto_node failed");
		m_goto_node = NULL;
	} 
	m_lock.leave();
	AM_DBG ambulant::lib::logger::get_logger()->debug("gui_player::play: returning");
}

void
gui_player::goto_node(const lib::node *n)
{
	m_lock.enter();
	if (m_player && m_player->is_playing()) {
		m_player->goto_node(n);
	} else {
		m_goto_node = n;
	}
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
gui_player::restart(bool reparse)
{
//	lib::logger::get_logger()->trace("restart not implemented yet");
	bool playing = is_play_active();
	bool pausing = is_pause_active();
	stop();
	
       	delete m_player;
	m_player = 0;
	if (reparse) {
		m_doc = create_document(m_url);
		if(!m_doc) {
			lib::logger::get_logger()->show("Failed to parse document %s", m_url.get_url().c_str());
			return;
		}
	}
	AM_DBG lib::logger::get_logger()->debug("Creating player instance for: %s", m_url.get_url().c_str());
	// XXXX
	m_player = common::create_smil2_player(m_doc, this, m_embedder);
#ifdef USE_SMIL21
	m_player->initialize();
#endif
	if (playing || pausing) play();
	if (pausing) pause();
}

bool
gui_player::is_play_enabled() const
{
//	const_cast<gui_player*>(this)->m_lock.enter();
	bool rv = m_player != NULL;
//	const_cast<gui_player*>(this)->m_lock.leave();
	return rv;
}

bool
gui_player::is_stop_enabled() const
{
//	const_cast<gui_player*>(this)->m_lock.enter();
	bool rv = m_player != NULL;
//	const_cast<gui_player*>(this)->m_lock.leave();
	return rv;
}

bool
gui_player::is_pause_enabled() const
{
//	const_cast<gui_player*>(this)->m_lock.enter();
	bool rv = m_player && !m_player->is_done();
//	const_cast<gui_player*>(this)->m_lock.leave();
	return rv;
}

bool
gui_player::is_play_active() const
{
//	const_cast<gui_player*>(this)->m_lock.enter();
	bool rv = m_player?m_player->is_playing():false;
//	const_cast<gui_player*>(this)->m_lock.leave();
	return rv;
}

bool
gui_player::is_stop_active() const
{
//	const_cast<gui_player*>(this)->m_lock.enter();
	bool rv = m_player?m_player->is_done():false;
//	const_cast<gui_player*>(this)->m_lock.leave();
	return rv;
}

bool
gui_player::is_pause_active() const
{
//	const_cast<gui_player*>(this)->m_lock.enter();
	bool rv = m_player?m_player->is_pausing():false;
//	const_cast<gui_player*>(this)->m_lock.leave();
	return rv;
}
	
int
gui_player::after_mousemove()
{
//	const_cast<gui_player*>(this)->m_lock.enter();
	int rv = m_player?m_player->after_mousemove():0;
//	const_cast<gui_player*>(this)->m_lock.leave();
	return rv;
}

void
gui_player::before_mousemove(int cursor)
{
//	m_lock.enter();
	if(m_player) m_player->before_mousemove(cursor);
//	m_lock.leave();
}

void
gui_player::on_char(int c)
{
//	m_lock.enter();
	if(m_player) m_player->on_char(c);
//	m_lock.leave();
}

void
gui_player::on_focus_advance()
{
//	m_lock.enter();
	if(m_player) m_player->on_focus_advance();
//	m_lock.leave();
}

void
gui_player::on_focus_activate()
{
//	m_lock.enter();
	if(m_player) m_player->on_focus_activate();
//	m_lock.leave();
}

lib::document *
gui_player::create_document(const net::url& url)
{
#ifdef AMBULANT_PLATFORM_UNIX
	// Correct for relative pathnames for local files
	if (url.is_local_file() && !url.is_absolute()) {
		char cwdbuf[1024];
		if (getcwd(cwdbuf, sizeof cwdbuf-2) < 0)
			strcpy(cwdbuf, ".");
		strcat(cwdbuf, "/");
		net::url cwd_url = ambulant::net::url::from_filename(cwdbuf);
		m_url = url.join_to_base(cwd_url);
		AM_DBG lib::logger::get_logger()->debug("gui_player::create_document: URL is now \"%s\"", m_url.get_url().c_str());
	} else {
		m_url = url;
	}
#else
	m_url = url;
#endif
	m_url = m_url.get_document();
	lib::logger::get_logger()->trace("%s: Parsing document...", m_url.get_url().c_str());
	lib::document *rv = lib::document::create_from_url(this, m_url);
	if (rv) {
		lib::logger::get_logger()->trace("%s: Parser done", m_url.get_url().c_str());
		rv->set_src_url(url);
	} else {
		lib::logger::get_logger()->trace("%s: Failed to parse document ", m_url.get_url().c_str());
	}
	return rv;
}	

