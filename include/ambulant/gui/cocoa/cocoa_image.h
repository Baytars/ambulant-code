/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
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

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_COCOA_COCOA_IMAGE_H
#define AMBULANT_GUI_COCOA_COCOA_IMAGE_H

#include "ambulant/gui/cocoa/cocoa_renderer.h"
//#include "ambulant/smil2/transition.h"
//#include "ambulant/lib/mtsync.h"
#include <Cocoa/Cocoa.h>

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace cocoa {

class cocoa_image_renderer : public cocoa_renderer<renderer_playable_dsall> {
  public:
	cocoa_image_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp)
	:	cocoa_renderer<renderer_playable_dsall>(context, cookie, node, evp, factory, mdp),
		m_image(NULL),
		m_nsdata(NULL) {};
	~cocoa_image_renderer();

    void redraw_body(const rect &dirty, gui_window *window);
  private:
	
  	NSImage *m_image;
  	NSData *m_nsdata;
	lib::size m_size;
	critical_section m_lock;
};

} // namespace cocoa

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_COCOA_COCOA_IMAGE_H
