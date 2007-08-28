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

#include "ambulant/gui/dx/dx_dsvideo.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/lib/win32/win32_error.h"
using ambulant::lib::win32::win_report_error;

#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace dx {

class video_dib_surface {
  public:
	HBITMAP get_handle() { return NULL;}
};

dx_dsvideo_renderer::dx_dsvideo_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp,
	common::factories *factory)
:	common::video_renderer(context, cookie, node, evp, factory),
	m_ddsurf(NULL),
	m_dibsurf(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("dx_dsvideo_renderer(): 0x%x created", (void*)this);
}

void
dx_dsvideo_renderer::_init_surfaces(common::gui_window *window)
{
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	m_ddsurf = v->create_surface(m_size);
	// m_dibsurf = xxxx;
}

dx_dsvideo_renderer::~dx_dsvideo_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~dx_dsvideo_renderer(0x%x)", (void *)this);
	if (m_ddsurf)
		m_ddsurf->Release();
	m_ddsurf = NULL;
	if (m_dibsurf)
			delete m_dibsurf;
	m_dibsurf = 0;
	m_lock.leave();
}
	
void
dx_dsvideo_renderer::show_frame(const char* frame, int size)
{
	if (m_ddsurf == NULL || m_dibsurf == NULL) return;
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("dx_dsvideo_renderer::show_frame: size=%d", size);
	assert(size == (int)(m_size.w * m_size.h * 4));
	// xxx_copy(frame, m_dibsurf, size);
	HRESULT hr;
	HDC hdc = ::GetDC(NULL);
	hr = m_ddsurf->GetDC(&hdc);
	if (FAILED(hr)) {
		win_report_error("DirectDrawSurface::GetDC()", hr);
		m_lock.leave();
		return;
	}
	HDC bmp_hdc = CreateCompatibleDC(hdc);
	HBITMAP hbmp_old = (HBITMAP) SelectObject(bmp_hdc, m_dibsurf->get_handle());
	::BitBlt(hdc, 0, 0, m_size.w, m_size.h, bmp_hdc, 0, 0, SRCCOPY);
	SelectObject(bmp_hdc, hbmp_old);
	DeleteDC(bmp_hdc);
	m_ddsurf->ReleaseDC(hdc);
	if (m_dest) m_dest->need_redraw();
	m_lock.leave();
}

void
dx_dsvideo_renderer::redraw(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	if (m_ddsurf == NULL || m_dibsurf == NULL) {
		// This is really the wrong place to initialize the surfaces: it
		// will make us lose the first frame. Need to rethink, maybe do in
		// set_surface?
		_init_surfaces(window);
		m_lock.leave();
		return;
	}
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("dx_dsvideo_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
#if 0	
	dx_window *cwindow = (dx_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
#if 0
	// See whether we're in a transition
	NSImage *surf = NULL;
	if (m_trans_engine && m_trans_engine->is_done()) {
		delete m_trans_engine;
		m_trans_engine = NULL;
	}
	if (m_trans_engine) {
		surf = [view getTransitionSurface];
		if ([surf isValid]) {
			[surf lockFocus];
			AM_DBG logger::get_logger()->debug("dx_dsvideo_renderer.redraw: drawing to transition surface");
		} else {
			lib::logger::get_logger()->trace("dx_dsvideo_renderer.redraw: cannot lockFocus for transition");
			surf = NULL;
		}
	}
#endif

	if (m_image) {

		// Now find both source and destination area for the bitblit.
		NSSize cocoa_srcsize = [m_image size];
		size srcsize = size((int)cocoa_srcsize.width, (int)cocoa_srcsize.height);
		rect srcrect = rect(size(0, 0));
		rect dstrect = m_dest->get_fit_rect(srcsize, &srcrect, m_alignment);
		dstrect.translate(m_dest->get_global_topleft());
		
		NSRect cocoa_srcrect = NSMakeRect(0, 0, srcrect.width(), srcrect.height()); // XXXX 0, 0 is wrong
		NSRect cocoa_dstrect = [view NSRectForAmbulantRect: &dstrect];
		AM_DBG logger::get_logger()->debug("cocoa_dsvideo_renderer.redraw: draw image %f %f -> (%f, %f, %f, %f)", cocoa_srcsize.width, cocoa_srcsize.height, NSMinX(cocoa_dstrect), NSMinY(cocoa_dstrect), NSMaxX(cocoa_dstrect), NSMaxY(cocoa_dstrect));
#ifdef WITH_SMIL30
		double alfa = 1.0;
		const common::region_info *ri = m_dest->get_info();
		if (ri) alfa = ri->get_mediaopacity();
		[m_image drawInRect: cocoa_dstrect fromRect: cocoa_srcrect operation: NSCompositeSourceAtop fraction: alfa];
#else
		[m_image drawInRect: cocoa_dstrect fromRect: cocoa_srcrect operation: NSCompositeSourceAtop fraction: 1.0];
#endif
	} else {
	}
#if 0
	if (surf) [surf unlockFocus];
	if (m_trans_engine && surf) {
		AM_DBG logger::get_logger()->debug("cocoa_dsvideo_renderer.redraw: drawing to view");
		m_trans_engine->step(m_event_processor->get_timer()->elapsed());
		typedef lib::no_arg_callback<cocoa_dsvideo_renderer> transition_callback;
		lib::event *ev = new transition_callback(this, &cocoa_dsvideo_renderer::transition_step);
		lib::transition_info::time_type delay = m_trans_engine->next_step_delay();
		if (delay < 33) delay = 33; // XXX band-aid
		AM_DBG lib::logger::get_logger()->debug("cocoa_dsvideo_renderer.redraw: now=%d, schedule step for %d", m_event_processor->get_timer()->elapsed(), m_event_processor->get_timer()->elapsed()+delay);
		m_event_processor->add_event(ev, delay);
	}
#endif
#endif
	m_lock.leave();
}

} // namespace dx

} // namespace gui

} //namespace ambulant

