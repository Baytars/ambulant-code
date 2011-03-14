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
 * $Id: qt_html_renderer.h
 */
 
#ifndef AMBULANT_GUI_QT_HTML_RENDERER_H
#define AMBULANT_GUI_QT_HTML_RENDERER_H

//#define WITH_QT_HTML_WIDGET
#ifdef	WITH_QT_HTML_WIDGET
#endif/*WITH_QT_HTML_WIDGET*/
#ifdef	WITH_QT_HTML_WIDGET

#include "ambulant/common/factory.h"
#include "ambulant/lib/memfile.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/region.h"
#include "ambulant/gui/none/none_gui.h"

#include "ambulant/gui/qt/qt_renderer.h"
#include "ambulant/net/url.h"
#include <qwidget.h>
#include <kapp.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kurl.h>
#include <kparts/browserextension.h>

class KHTMLPart;

namespace ambulant {

namespace gui {

namespace qt {

class browser_container;

class qt_html_renderer : public renderer_playable {
  public:
	qt_html_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		common::factories *factory);
	~qt_html_renderer();
	void start(double t);
	void stop();
	void seek(double t) {}
//	void user_event(const lib::point& pt, int what);
	void redraw(const lib::rect &dirty, common::gui_window *window) {}
//	void set_surface(common::surface *dest);
	void set_intransition(const lib::transition_info *info) {};
	void start_outtransition(const lib::transition_info *info) {};
  private:
	lib::critical_section  m_lock;
	browser_container*     m_html_browser;
};

} // namespace qt

} // namespace gui
 
} // namespace ambulant

#endif/*WITH_QT_HTML_WIDGET*/

#endif/*AMBULANT_GUI_QT_HTML_RENDERER_H*/