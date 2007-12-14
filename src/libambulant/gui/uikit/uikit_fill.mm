// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
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

#include "ambulant/gui/uikit/uikit_gui.h"
#include "ambulant/gui/uikit/uikit_fill.h"
//#include "ambulant/gui/uikit/uikit_transition.h"
#include "ambulant/common/region_info.h"
#include <CoreGraphics/CoreGraphics.h>

#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace uikit {

uikit_fill_renderer::~uikit_fill_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("~uikit_fill_renderer(0x%x)", (void *)this);
	m_lock.leave();
}
	
void
uikit_fill_renderer::start(double where)
{
//	start_transition(where);
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("uikit_fill_renderer.start(0x%x)", (void *)this);
	if (m_activated) {
		logger::get_logger()->trace("uikit_fill_renderer.start(0x%x): already started", (void*)this);
		m_lock.leave();
		return;
	}
	m_activated = true;
	if (!m_dest) {
		logger::get_logger()->trace("uikit_fill_renderer.start(0x%x): no surface", (void *)this);
		return;
	}
	m_dest->show(this);
	m_context->started(m_cookie);
	m_context->stopped(m_cookie);
	m_lock.leave();
}

void
uikit_fill_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("uikit_fill_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	
	uikit_window *cwindow = (uikit_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();

	// First find our whole area (which we have to clear to background color)
	rect dstrect_whole = r;
	dstrect_whole.translate(m_dest->get_global_topleft());
	CGRect uikit_dstrect_whole = [view CGRectForAmbulantRect: &dstrect_whole];
	// Fill with  color
	const char *color_attr = m_node->get_attribute("color");
	if (!color_attr) {
		lib::logger::get_logger()->trace("<brush> element without color attribute");
		m_lock.leave();
		return;
	}
	color_t color = lib::to_color(color_attr);
	AM_DBG lib::logger::get_logger()->debug("uikit_fill_renderer.redraw: clearing to 0x%x", (long)color);
	double alfa = 1.0;
#ifdef WITH_SMIL30
	const common::region_info *ri = m_dest->get_info();
	if (ri) alfa = ri->get_mediaopacity();
#endif
	float components[] = {redf(color), greenf(color), bluef(color), alfa};
//	CGColorRef cgcolor = CGColorCreate(CGColorSpaceCreateDeviceRGB(), components);
			
//	PLOVER [nscolor set];
//	PLOVER CGRectFillUsingOperation(uikit_dstrect_whole, NSCompositeSourceAtop);
	CGContextRef myContext = UICurrentContext();
//	CGContextRef myContext = [[NSGraphicsContext currentContext] graphicsPort];
	CGContextSetFillColor(myContext, components);
	CGContextFillRect(myContext, uikit_dstrect_whole);
//	CGColorRelease(cgcolor);
	m_lock.leave();
}

uikit_background_renderer::~uikit_background_renderer()
{
	if (m_bgimage)
		CGImageRelease(m_bgimage);
	m_bgimage = NULL;
}

void
uikit_background_renderer::redraw(const lib::rect &dirty, common::gui_window *window)
{
	const rect &r =  m_dst->get_rect();
	AM_DBG logger::get_logger()->debug("uikit_bg_renderer::drawbackground(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	
	uikit_window *cwindow = (uikit_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	CGContextRef myContext = UICurrentContext();
//	CGContextRef myContext = [[NSGraphicsContext currentContext] graphicsPort];
	AM_DBG lib::logger::get_logger()->debug("uikit_bg_renderer::drawbackground: clearing to 0x%x opacity %f", (long)m_src->get_bgcolor(), m_src->get_bgopacity());
	rect dstrect_whole = r;
	dstrect_whole.translate(m_dst->get_global_topleft());
	CGRect uikit_dstrect_whole = [view CGRectForAmbulantRect: &dstrect_whole];
	double opacity = m_src->get_bgopacity();
	if (m_src && opacity > 0) {
		// First find our whole area (which we have to clear to background color)
		// XXXX Fill with background color
		color_t bgcolor = m_src->get_bgcolor();
		AM_DBG lib::logger::get_logger()->debug("uikit_bg_renderer::drawbackground: clearing to 0x%x opacity %f", (long)bgcolor, opacity);
		float components[] = {redf(bgcolor), greenf(bgcolor), bluef(bgcolor), opacity};
//		CGColorRef cgcolor = CGColorCreate(CGColorSpaceCreateDeviceRGB(), components);
//		PLOVER [uikit_bgcolor set];
//		PLOVER CGRectFillUsingOperation(uikit_dstrect_whole, NSCompositeSourceAtop);
		CGContextSetFillColor(myContext, components);
		CGContextFillRect(myContext, uikit_dstrect_whole);
//		CGColorRelease(cgcolor);
	}
	if (m_bgimage) {
		AM_DBG lib::logger::get_logger()->debug("uikit_background_renderer::redraw(): drawing image");
//		CGRect srcrect = CGRectMake(0, 0, 
//			CGImageGetWidth(m_bgimage), CGImageGetHeight(m_bgimage));
//		PLOVER [m_bgimage drawInRect: uikit_dstrect_whole fromRect: srcrect
//			operation: NSCompositeSourceAtop fraction: 1.0];
		CGContextDrawImage (myContext, uikit_dstrect_whole, m_bgimage); 
	}
}

void
uikit_background_renderer::highlight(common::gui_window *window)
{
	const rect &r =  m_dst->get_rect();
	AM_DBG logger::get_logger()->debug("uikit_bg_renderer::highlight(0x%x)", (void *)this);
	
	uikit_window *cwindow = (uikit_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	rect dstrect_whole = r;
	dstrect_whole.translate(m_dst->get_global_topleft());
	CGRect uikit_dstrect_whole = [view CGRectForAmbulantRect: &dstrect_whole];
	color_t hicolor = 0x0000ff;
	AM_DBG lib::logger::get_logger()->debug("uikit_bg_renderer::highlight: framing with color 0x%x", (long)hicolor);
	float components[] = {redf(hicolor), greenf(hicolor), bluef(hicolor), 1.0};
//	CGColorRef cgcolor = CGColorCreate(CGColorSpaceCreateDeviceRGB(), components);
//	PLOVER [cgcolor set];
//	PLOVER NSFrameRect(uikit_dstrect_whole);
	CGContextRef myContext = UICurrentContext();
//	CGContextRef myContext = [[NSGraphicsContext currentContext] graphicsPort];
	CGContextSetStrokeColor(myContext, components);
	CGContextStrokeRect(myContext, uikit_dstrect_whole);
//	CGColorRelease(cgcolor);
}

void
uikit_background_renderer::keep_as_background()
{
	AM_DBG lib::logger::get_logger()->debug("uikit_background_renderer::keep_as_background() called");
	if (m_bgimage) {
		AM_DBG lib::logger::get_logger()->debug("uikit_background_renderer::keep_as_background: delete old m_image");
		CGImageRelease(m_bgimage);
		m_bgimage = NULL;
	}
	uikit_window *cwindow = (uikit_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)cwindow->view();
	rect dstrect_whole = m_dst->get_rect();
	dstrect_whole.translate(m_dst->get_global_topleft());
	CGRect uikit_dstrect_whole = [view CGRectForAmbulantRect: &dstrect_whole];	
	
//	XYZZY m_bgimage = [[view getOnScreenImageForRect: uikit_dstrect_whole] retain];
}

} // namespace uikit

} // namespace gui

} //namespace ambulant

