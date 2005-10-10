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

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_COCOA_COCOA_RENDERER_H
#define AMBULANT_GUI_COCOA_COCOA_RENDERER_H

#include "ambulant/common/renderer_impl.h"
#include "ambulant/smil2/transition.h"
#include "ambulant/lib/mtsync.h"
#include <Cocoa/Cocoa.h>

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace cocoa {

class cocoa_transition_renderer : public ref_counted_obj {
  public:
	cocoa_transition_renderer(event_processor *evp)
	:	m_event_processor(evp),
		m_transition_dest(NULL),
		m_intransition(NULL),
		m_outtransition(NULL),
		m_trans_engine(NULL) {}
	~cocoa_transition_renderer();
	
	void set_surface(common::surface *dest);
	void start(double where);
	void stop();
	void redraw_pre(gui_window *window);
	void redraw_post(gui_window *window);
	void set_intransition(const lib::transition_info *info);
	void start_outtransition(const lib::transition_info *info);
  private:
	void transition_step();

	event_processor *m_event_processor;
	common::surface *m_transition_dest;
	const lib::transition_info *m_intransition;
	const lib::transition_info *m_outtransition;
	smil2::transition_engine *m_trans_engine;
#ifdef USE_SMIL21
	bool m_fullscreen;
#endif
	critical_section m_lock;
};

template <class RP_Base>
class cocoa_renderer : public RP_Base {
  public:
	cocoa_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory)
	:	RP_Base(context, cookie, node, evp, factory),
		m_transition_renderer(new cocoa_transition_renderer(evp)) {};
	cocoa_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp)
	:	RP_Base(context, cookie, node, evp),
		m_transition_renderer(new cocoa_transition_renderer(evp)) {};
	~cocoa_renderer() {
		m_transition_renderer->release();
	}

	void set_surface(common::surface *dest) {
		RP_Base::set_surface(dest);
		m_transition_renderer->set_surface(dest);
	}

	virtual void start(double where) {
		start_transition(where);
		RP_Base::start(where);
	}
	
 	virtual void stop() {
		stop_transition();
		RP_Base::stop();
	}
	
    void redraw(const rect &dirty, gui_window *window) {
		m_transition_renderer->redraw_pre(window);
		redraw_body(dirty, window);
		m_transition_renderer->redraw_post(window);
		if (RP_Base::m_erase_never) RP_Base::m_dest->keep_as_background();
	}

	void set_intransition(const lib::transition_info *info) {
		m_transition_renderer->set_intransition(info);
	}
	
	void start_outtransition(const lib::transition_info *info) {
		m_transition_renderer->start_outtransition(info);
	}
  protected:
	void start_transition(double where) {
		m_transition_renderer->start(where);
	}
	void stop_transition() {
		m_transition_renderer->stop();
	}
    virtual void redraw_body(const rect &dirty, gui_window *window) = 0;

  private:
	cocoa_transition_renderer *m_transition_renderer;
};

} // namespace cocoa

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_COCOA_COCOA_RENDERER_H
