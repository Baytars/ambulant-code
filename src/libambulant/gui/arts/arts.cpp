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
#include "ambulant/gui/arts/arts_audio.h"


namespace ambulant {

using namespace lib;




active_renderer *
gui::arts::arts_renderer_factory::new_renderer(
		lib::active_playable_events *context,
		lib::active_playable_events::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
		net::passive_datasource *src,
		lib::abstract_rendering_surface *const dest)
{
	active_renderer *rv;
	xml_string tag = node->get_qname().second;
     AM_DBG logger::get_logger()->trace("arts_renderer_factory: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "audio") {
		rv = (active_renderer *) new gui::arts::arts_active_audio_renderer(context, cookie, node, evp, src);
		AM_DBG logger::get_logger()->trace("arts_renderer_factory: node 0x%x: returning arts_active_audio_renderer 0x%x", (void *)node, (void *)rv);
	} else {
	AM_DBG logger::get_logger()->error("arts_renderer_factory: no aRts renderer for tag \"%s\"", tag.c_str());
                return NULL;
	}
	return rv;
}



} // end namespace ambulant
