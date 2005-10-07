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

 
#ifndef NONE_VIDEO_RENDERER
#define NONE_VIDEO_RENDERER

#include "ambulant/common/video_renderer.h"

namespace ambulant {
namespace gui {
namespace none {	  

class none_video_renderer : public common::video_renderer {
  public:
    none_video_renderer(
    common::playable_notification *context,
    common::playable_notification::cookie_type cookie,
    const lib::node *node,
    lib::event_processor *evp,
	common::factories *factory)
	:   common::video_renderer(context, cookie, node, evp, factory)
	{ }

  	~none_video_renderer() {};
	

	void show_frame(const char* frame, int size);
		
    void redraw(const lib::rect &dirty, common::gui_window *window) {};
	void wantclicks(bool want) {};
	void set_intransition(const lib::transition_info *info) {};
	void start_outtransition(const lib::transition_info *info) {};

//	void set_surface(common::surface *dest) {  };
//	common::surface *get_surface() { return NULL; };
};

}
}
}
#endif /* NONE_VIDEO_RENDERER */
