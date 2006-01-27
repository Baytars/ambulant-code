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

#define WITH_FFMPEG_VIDEO
//#define TEST_PLAYBACK_FEEDBACK
// Define NONE_PLAYER to skip all cocoa support but use the dummy
// none_window and none_playable in stead.
//#define NONE_PLAYER

#include <iostream>
#include <ApplicationServices/ApplicationServices.h>
#include "mainloop.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/timer.h"
#include "ambulant/lib/document.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"
#ifdef WITH_SDL
#include "ambulant/gui/SDL/sdl_gui.h"
#endif
#ifdef NONE_PLAYER
#include "ambulant/gui/none/none_gui.h"
#endif
#include "ambulant/net/datasource.h"
#include "ambulant/net/posix_datasource.h"
//#define WITH_STDIO_DATASOURCE
#ifdef WITH_STDIO_DATASOURCE
#include "ambulant/net/stdio_datasource.h"
#endif
#ifdef WITH_FFMPEG
#include "ambulant/net/ffmpeg_factory.h"
#endif
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/common/plugin_engine.h"
#ifdef WITH_LIVE
#include "ambulant/net/rtsp_factory.h"
#endif

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

#ifdef TEST_PLAYBACK_FEEDBACK
#include "ambulant/common/player.h"
#include "ambulant/lib/node.h"

class pbfeedback : public ambulant::common::player_feedback {
  public:
	void node_started(ambulant::lib::node *n) {
		ambulant::lib::logger::get_logger()->trace("%s started", n->get_sig().c_str());
	}

	void node_stopped(ambulant::lib::node *n) {
		ambulant::lib::logger::get_logger()->trace("%s stopped", n->get_sig().c_str());
	}
};

class pbfeedback pbfeedback;
#endif

mainloop::mainloop(const char *urlstr, ambulant::common::window_factory *wf,
	bool use_mms, ambulant::common::embedder *app)
:   common::gui_player(),
	m_doc(NULL),
	m_embedder(app)
{
	AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop(0x%x): created", (void*)this);
	init_factories();
	
#ifdef NONE_PLAYER
	// Replace the real window factory with a none_window_factory instance.
	wf = new gui::none::none_window_factory();
#endif // NONE_PLAYER
	m_window_factory = wf;

	init_plugins();
	
	ambulant::net::url url = ambulant::net::url::from_url(urlstr);
	m_doc = create_document(url);
	if (!m_doc) {
		lib::logger::get_logger()->error(gettext("%s: Cannot build DOM tree"), urlstr);
		return;
	}
	if (use_mms)
		m_player = common::create_mms_player(m_doc, this);
	else
		m_player = common::create_smil2_player(m_doc, this, m_embedder);
#ifdef USE_SMIL21
	m_player->initialize();
#endif
#ifdef TEST_PLAYBACK_FEEDBACK
	m_player->set_feedback(&pbfeedback);
#endif
	const std::string& id = url.get_ref();
	if (id != "") {
		const ambulant::lib::node *node = m_doc->get_node(id);
		if (!node) {
			lib::logger::get_logger()->warn(gettext("%s: node ID not found"), id.c_str());
		} else {
			m_goto_node = node;
		}
	}
}

void
mainloop::init_playable_factory()
{
	m_playable_factory = common::get_global_playable_factory();
#ifndef NONE_PLAYER
	m_playable_factory->add_factory(new gui::cocoa::cocoa_renderer_factory(this));
#ifdef WITH_SDL
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add factory for SDL");
	m_playable_factory->add_factory( new gui::sdl::sdl_renderer_factory(this) );      
#endif // WITH_SDL
#endif // NONE_PLAYER
}

void
mainloop::init_window_factory()
{
}

void
mainloop::init_datasource_factory()
{
	m_datasource_factory = new net::datasource_factory();
#ifndef NONE_PLAYER
#ifdef WITH_LIVE	
	AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add live_audio_datasource_factory");
	m_datasource_factory->add_video_factory(new net::live_video_datasource_factory());
	m_datasource_factory->add_audio_factory(new net::live_audio_datasource_factory()); 
#endif
#ifdef WITH_FFMPEG
#ifdef WITH_FFMPEG_VIDEO
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_video_datasource_factory");
	m_datasource_factory->add_video_factory(net::get_ffmpeg_video_datasource_factory());
#endif // WITH_FFMPEG_VIDEO
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_audio_datasource_factory");
	m_datasource_factory->add_audio_factory(net::get_ffmpeg_audio_datasource_factory());
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_audio_parser_finder");
	m_datasource_factory->add_audio_parser_finder(net::get_ffmpeg_audio_parser_finder());
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_audio_filter_finder");
	m_datasource_factory->add_audio_filter_finder(net::get_ffmpeg_audio_filter_finder());
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_raw_datasource_factory");
	m_datasource_factory->add_raw_factory(net::get_ffmpeg_raw_datasource_factory());
#endif // WITH_FFMPEG
#endif // NONE_PLAYER
#ifdef WITH_STDIO_DATASOURCE
	// This is for debugging only, really: the posix datasource
	// should always perform better, and is always available on OSX.
	// If you define WITH_STDIO_DATASOURCE we prefer to use the stdio datasource,
	// however.
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add stdio_datasource_factory");
	m_datasource_factory->add_raw_factory(net::get_stdio_datasource_factory());
#endif
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add posix_datasource_factory");
	m_datasource_factory->add_raw_factory(net::get_posix_datasource_factory());
}

void
mainloop::init_parser_factory()
{
	m_parser_factory = lib::global_parser_factory::get_parser_factory();	
}


ambulant::lib::document *
mainloop::create_document(ambulant::net::url& url)
{
	// XXXX Needs work for URLs
	// Correct for relative pathnames for local files
	if (url.is_local_file() && !url.is_absolute()) {
		char cwdbuf[1024];
		if (getcwd(cwdbuf, sizeof cwdbuf-2) < 0)
			strcpy(cwdbuf, ".");
		strcat(cwdbuf, "/");
		ambulant::net::url cwd_url = ambulant::net::url::from_filename(cwdbuf);
		url = url.join_to_base(cwd_url);
		AM_DBG ambulant::lib::logger::get_logger()->debug("mainloop::create_document: URL is now \"%s\"", url.get_url().c_str());
	}
	ambulant::lib::logger::get_logger()->trace("%s: Parsing document...", url.get_url().c_str());
	ambulant::lib::document *rv = ambulant::lib::document::create_from_url(this, url);
	if (rv) {
		ambulant::lib::logger::get_logger()->trace("%s: Parser done", url.get_url().c_str());
		rv->set_src_url(url);
	} else {
		ambulant::lib::logger::get_logger()->trace("%s: Failed to parse document ", url.get_url().c_str());
	}
	return rv;
}	

mainloop::~mainloop()
{
//  m_doc will be cleaned up by the smil_player.
//	if (m_doc) delete m_doc;
//	m_doc = NULL;
}
