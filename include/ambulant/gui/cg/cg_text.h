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

#ifndef AMBULANT_GUI_CG_GC_TEXT_H
#define AMBULANT_GUI_CG_GC_TEXT_H

#include "ambulant/gui/cg/cg_renderer.h"
#include "ambulant/lib/mtsync.h"
//#include <CoreGraphics/CGContext.h>
#ifdef WITH_UIKIT
#include <UIKit/UIKit.h>
#else
#include <AppKit/AppKit.h>
#endif

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace cg {

class cg_text_renderer : public cg_renderer<renderer_playable_dsall> {
  public:
	cg_text_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory);
        ~cg_text_renderer();
	
    void redraw_body(const rect &dirty, gui_window *window);
  private:
	bool _calc_fit(CGContextRef ctx, float width, int& lbegin, int& lend);
	bool _fits(CGContextRef ctx, float width, const char *str, int strlen);
	const char *m_font_name;
	float m_font_size;
	lib::color_t m_text_color;
	critical_section m_lock;
};

} // namespace cg

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_CG_GC_TEXT_H
