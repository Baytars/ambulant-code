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

 
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
 
#include "ambulant/gui/arts/arts.h"

using namespace ambulant;

lib::active_renderer *
gui::arts::arts_renderer_factory::new_renderer(lib::event_processor *const evp,
	net::passive_datasource *src,
	lib::passive_region *const dest,
	const lib::node *node)
{
	active_renderer *rv;

	xml_string tag = node->get_qname().second;
	if ( tag == "audio") {
		rv = (active_renderer *)new arts_active_audio_renderer(evp, src, node);
		AM_DBG logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning cocoa_active_audio_renderer 0x%x", (void *)node, (void *)rv);
	} else {
	// logger::get_logger()->error("arts_renderer_factory: no aRts renderer for tag \"%s\"", tag.c_str());
                return NULL;
	}
	return rv;
}

gui::arts::arts_active_audio_renderer::arts_active_audio_renderer(event_processor *const evp,
	net::passive_datasource *src,
	const node *node)
:	active_basic_renderer(evp, node),
	m_url(src->get_url())
{
    m_evp = evp;
    m_src = src;
    m_node = node;
}

int
gui::arts::arts_active_audio_renderer::arts_setup(int rate, int bits, int channels, char *name)
{
    int err;

    err = arts_init();
    if (err < 0) {
        return err;
    }

    m_stream = arts_play_stream(rate, bits, channels, name);
    return 0;
}


gui::arts::arts_active_audio_renderer::~arts_active_audio_renderer()
{
    arts_close_stream(m_stream);
    arts_free();
}

int
gui::arts::arts_active_audio_renderer::arts_play(char *data, int size)
{
    return ::arts_write(m_stream, data, size);
}

void
lib::arts_active_audio_renderer::readdone()
{
	AM_DBG lib::logger::get_logger()->trace("active_renderer.readdone(0x%x)", (void *)this);
    size=m_ads->size();
    data = new char[data];
    m_ads->read(data,size);
    arts_setup()
    arts_play(data,size);
stopped_callback();
}

void
lib::arts_active_audio_renderer::playdone()
{
	AM_DBG lib::logger::get_logger()->trace("active_renderer.playdone(0x%x)", (void *)this);
    
	stopped_callback();
}

void
lib::arts_active_audio_renderer::start(double where)
{
    char *data;
	if (!m_node) abort();
	m_playdone = playdone;
	std::ostringstream os;
	os << *m_node;
    
	AM_DBG lib::logger::get_logger()->trace("active_renderer.start(0x%x, %s, playdone=0x%x)", (void *)this, os.str().c_str(), (void *)playdone);
	m_dest->show(this);
	if (m_ads) {
		m_ads->start(m_event_processor, m_readdone);    
	} else {
		lib::logger::get_logger()->error("active_renderer.start: no datasource");
		if (m_playdone)
			m_event_processor->add_event(m_playdone, 0, event_processor::low);
            stopped_callback();
	}
    
}



