/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
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

#ifndef AMBULANT_GUI_UIKIT_UIKIT_IMAGE_H
#define AMBULANT_GUI_UIKIT_UIKIT_IMAGE_H

#include "ambulant/gui/uikit/uikit_renderer.h"
//#include "ambulant/smil2/transition.h"
//#include "ambulant/lib/mtsync.h"
#include <UIKit/UIKit.h>

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace uikit {

class uikit_image_renderer : public uikit_renderer<renderer_playable_dsall> {
  public:
	uikit_image_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory)
	:	uikit_renderer<renderer_playable_dsall>(context, cookie, node, evp, factory),
		m_nsdata(NULL),
		m_image(NULL),
		m_image_cropped(NULL) {};
	~uikit_image_renderer();

    void redraw_body(const rect &dirty, gui_window *window);
  private:
  	CGImage *_cropped_image(const lib::rect& rect);
	
  	CFDataRef m_nsdata;
  	CGImageRef m_image;
	lib::size m_size;
  	CGImageRef m_image_cropped;
  	lib::rect m_rect_cropped;
	critical_section m_lock;
};

} // namespace uikit

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_UIKIT_UIKIT_IMAGE_H
