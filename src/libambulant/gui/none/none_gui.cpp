// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* 
 * @$Id$ 
 */

#include "ambulant/gui/none/none_gui.h"
#include "ambulant/gui/none/none_area.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/common/region_info.h"
#include "ambulant/lib/logger.h"

using namespace ambulant;
using namespace lib;
using namespace common;

gui::none::none_playable::none_playable(
	common::playable_notification *context,
#ifdef AMBULANT_PLATFORM_WIN32_WCE
	// Workaround for bug in emVC 4.0: it gets confused
	// when getting a subtype from a class within a function
	// signature, or something like that
	int cookie,
#else
	common::playable_notification::cookie_type cookie,
#endif
	const lib::node *node,
	lib::event_processor *evp)
:	common::playable_imp(context, cookie, node, evp)
{
	lib::xml_string tag = node->get_qname().second;
	std::string url = repr(node->get_url("src"));
	lib::logger::get_logger()->warn("No renderer found for <%s src=\"%s\">, using none_playable", tag.c_str(), url.c_str());
}

void
gui::none::none_playable::start(double where)
{
	lib::logger::get_logger()->trace("none_playable.start(0x%x)", m_node);
	m_context->stopped(m_cookie, 0);
}

void
gui::none::none_playable::stop()
{
	lib::logger::get_logger()->trace("none_playable.stop(0x%x)", (void *)this);
}

void
gui::none::none_background_renderer::redraw(const rect &dirty, gui_window *window)
{
	lib::logger::get_logger()->trace("none_background_renderer.redraw(0x%x) from 0x%x to 0x%x", (void *)this, (void*)m_src, (void*)m_dst);
}

void
gui::none::none_background_renderer::keep_as_background()
{
	lib::logger::get_logger()->trace("none_background_renderer.keep_as_background(0x%x)", (void *)this);
}

playable *
gui::none::none_playable_factory::new_playable(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp)
{
	lib::xml_string tag = node->get_qname().second;
	if(tag == "area" || tag == "a")
		return new none_area_renderer(context, cookie, node, evp);
	return new none_playable(context, cookie, node, evp);
}

common::playable *
gui::none::none_playable_factory::new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src)
{
	return NULL;
}

bgrenderer *
gui::none::none_window_factory::new_background_renderer(const region_info *src)
{
	return new none_background_renderer(src);
}

gui_window *
gui::none::none_window_factory::new_window(const std::string &name, size bounds, gui_events *handler)
{
	return new none_window(name, bounds, handler);
}
