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

#include "ambulant/net/raw_video_datasource.h"
#include "ambulant/gui/none/none_video_renderer.h"

#include <math.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <unistd.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif



using namespace ambulant;
using namespace gui;
using namespace none;
typedef
	lib::no_arg_callback <
	gui::none::none_video_renderer >
	dataavail_callback;

none_video_renderer::none_video_renderer (common::playable_notification *
					  context,
					  common::playable_notification::
					  cookie_type cookie,
					  const lib::node * node,
					  lib::event_processor * evp,
					  net::datasource_factory * df):
common::active_basic_renderer (context, cookie, node, evp),
m_evp (evp)
{
	AM_DBG lib::logger::get_logger ()->trace("none_video_renderer::none_video_renderer() (this = 0x%x): Constructor ", (void *) this);
	// XXXX FIXME : The path to the jpg's is fixed !!!!!    
	lib::logger::get_logger ()->warn("none_video_renderer::none_video_renderer(): The path to the video files is fixed. (/ufs/dbenden/testmovie) FIXME");
	m_src = new net::raw_video_datasource ("/ufs/dbenden/testmovie");
}

void
none_video_renderer::start (double where = 1)
{
	int w;
	m_epoch = m_evp->get_timer()->elapsed();
	w = (int) round (where);
	lib::event * e = new dataavail_callback (this, &none_video_renderer::data_avail);
	AM_DBG lib::logger::get_logger ()->trace ("none_video_renderer::start(%d) (this = 0x%x) ", w, (void *) this);
	m_src->start_frame (m_evp, e, w);
}


// now() returns the time in seconds !
double
none_video_renderer::now() 
{
	return (m_evp->get_timer()->elapsed()- m_epoch)/1000;
}

void 
none_video_renderer::show_frame(char* frame)
{
lib::logger::get_logger ()->trace("**** DISPLAYED");
}

void
none_video_renderer::data_avail ()
{
	double ts;
	char *buf;
	unsigned long int event_time;
	bool displayed;
	AM_DBG lib::logger::get_logger ()->trace ("none_video_renderer::data_avial()(this = 0x%x):", (void *) this);
	buf = m_src->get_frame (&ts);
	displayed = false;
	AM_DBG lib::logger::get_logger ()->trace ("none_video_renderer::data_avial()(buf = 0x%x) (ts=%f, now=%f):", (void *) buf,ts, now());	
	if (buf) {
		if (ts <= now()) {
			lib::logger::get_logger ()->trace("**** (this = 0x%x) Display frame with timestamp : %f, now = %f (located at 0x%x) ", (void *) this, ts, now(), (void *) buf);
			show_frame(buf);
			displayed = true;
			m_src->frame_done(ts);
		} else {
			if (!m_src->end_of_file()){
				lib::event * e = new dataavail_callback (this, &none_video_renderer::data_avail);
				event_time = (unsigned long int) round( ts*1000 - now()*1000); 
				m_evp->add_event(e, event_time);
			}
		}
	} else {
		lib::logger::get_logger ()->trace("none_video_renderer::data_avial()(this = 0x%x): buf seems to be NULL !) ", (void *) this);
	}
	
	if ((!m_src->end_of_file() ) && (displayed) ) {
		lib::event * e = new dataavail_callback (this, &none_video_renderer::data_avail);
		m_src->start_frame (m_evp, e, ts);
	} else {
		AM_DBG lib::logger::get_logger ()->trace ("none_video_renderer::data_avial()(this = 0x%x): end_of_file ", (void *) this);
		stopped_callback();
	}
	
}
