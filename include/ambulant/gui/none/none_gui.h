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

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_NONE_NONE_GUI_H
#define AMBULANT_GUI_NONE_NONE_GUI_H

#include "ambulant/config/config.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer.h"

namespace ambulant {

namespace gui {

namespace none {

class none_window : public common::abstract_window {
  public:
  	none_window(const std::string &name, lib::size bounds, common::abstract_rendering_source *region)
  	:	common::abstract_window(region) {};
  		
	void need_redraw(const lib::screen_rect<int> &r) { m_region->redraw(r, this); };
//	void need_events(lib::abstract_mouse_region *rgn) {};
	void mouse_region_changed() {};
};

class none_window_factory : public common::window_factory {
  public:
  	none_window_factory() {}
  	
	common::abstract_window *new_window(const std::string &name, lib::size bounds, common::abstract_rendering_source *region);
	common::abstract_mouse_region *new_mouse_region() { return NULL; }
	common::abstract_bg_rendering_source *new_background_renderer();
};

class none_active_renderer : public common::active_renderer {
  public:
	none_active_renderer(
		common::active_playable_events *context,
#ifdef AMBULANT_PLATFORM_WIN32_WCE
		// Workaround for bug in emVC 4.0: it gets confused
		// when getting a subtype from a class within a function
		// signature, or something like that
		int cookie,
#else
		common::active_playable_events::cookie_type cookie,
#endif
		const lib::node *node,
		lib::event_processor *const evp,
		net::passive_datasource *src,
		common::abstract_rendering_surface *const dest)
	:	common::active_renderer(context, cookie, node, evp, src, dest) {};
	
	void start(double where);
	void redraw(const lib::screen_rect<int> &r, common::abstract_window *window);
	void stop();
};

class none_background_renderer : public common::abstract_bg_rendering_source {
  public:
	void drawbackground(const common::abstract_smil_region_info *src, 
		const lib::screen_rect<int> &dirty, 
		common::abstract_rendering_surface *dst, 
		common::abstract_window *window);
};

class common::abstract_smil_region_info;

class none_renderer_factory : public common::renderer_factory {
  public:
  	none_renderer_factory() {}
  	
	common::active_basic_renderer *new_renderer(
		common::active_playable_events *context,
		common::active_playable_events::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
		net::passive_datasource *src,
		common::abstract_rendering_surface *const dest);
};

} // namespace none

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_NONE_NONE_GUI_H
