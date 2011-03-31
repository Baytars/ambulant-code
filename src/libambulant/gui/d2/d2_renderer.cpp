// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2010 Stichting CWI,
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
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


#include "ambulant/gui/d2/d2_player.h"
#include "ambulant/gui/d2/d2_window.h"
#include "ambulant/gui/d2/d2_renderer.h"
#include "ambulant/gui/d2/d2_transition.h"

#include <wincodec.h>
#include <d2d1.h>
#include <d2d1helper.h>

#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

inline D2D1_RECT_F d2_rectf(lib::rect r) {
	return D2D1::RectF((float) r.left(), (float) r.top(), (float) r.right(), (float) r.bottom());
}

using namespace lib;

namespace gui {

namespace d2 {

ID2D1BitmapRenderTarget*
d2_transition_renderer::s_fullscreen_rendertarget = NULL;

d2_transition_renderer::~d2_transition_renderer()
{
	stop();
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~d2_transition_renderer(0x%x)", (void *)this);
	m_intransition = NULL;
	m_outtransition = NULL;
	SafeRelease(&this->m_transition_rendertarget);
	m_lock.leave();
}

d2_player*
d2_transition_renderer::get_d2player()
{
	if (m_d2player == NULL) {
		gui_window *window = m_transition_dest->get_gui_window();
		d2_window *cwindow = (d2_window *)window;
		m_d2player  = cwindow->get_d2_player();
	}
	return m_d2player;
}

ID2D1RenderTarget*
d2_transition_renderer::get_current_rendertarget ()
{
	ID2D1RenderTarget*	rv = d2_transition_renderer::s_fullscreen_rendertarget;
	if (rv == NULL)		rv = this->m_transition_rendertarget;
	if (rv == NULL)		rv = get_d2player()->get_rendertarget();
	return rv;
}

ID2D1BitmapRenderTarget*
d2_transition_renderer::get_transition_rendertarget ()
{
	if (m_trans_engine != NULL) {
		if (m_transition_rendertarget == NULL) {
			// Create a new target for drawing objects that need to be translated
			ID2D1RenderTarget* rt = get_d2player()->get_rendertarget();
			HRESULT hr = rt->CreateCompatibleRenderTarget(&m_transition_rendertarget);
			if (FAILED(hr)) {
				lib::win32::win_trace_error("d2_transition_renderer::get_rendertarget: CreateCompatibleRenderTarget", hr);
			}
			else m_transition_rendertarget->BeginDraw();
		}
		return m_transition_rendertarget;
	}
	return NULL;
}

void
d2_transition_renderer::set_surface(common::surface *dest)
{
	m_transition_dest = dest;
	if (m_transition_dest && m_intransition && m_intransition->m_scope == scope_screen)
		m_transition_dest = m_transition_dest->get_top_surface();
}

void
d2_transition_renderer::set_intransition(const lib::transition_info *info) {
	m_intransition = info;
	if (m_transition_dest && m_intransition && m_intransition->m_scope == scope_screen)
		m_transition_dest = m_transition_dest->get_top_surface();
}

void
d2_transition_renderer::start(double where)
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("d2_transition_renderer.start(%f)", where);
	if (m_intransition && m_transition_dest) {
		AM_DBG logger::get_logger()->debug("d2_transition_renderer.start: with intransition");
		m_trans_engine = d2_transition_engine(m_transition_dest, false, m_intransition);
		if (m_trans_engine) {
#ifdef D2D_NOTYET
			AmbulantView *view = (AmbulantView *)cwindow->view();
			[view incrementTransitionCount];
#endif
			m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
			m_fullscreen = m_intransition->m_scope == scope_screen;
			if (m_fullscreen) {
				get_d2player()->start_screen_transition(false);
			}
		}
	}
	m_lock.leave();
}

void
d2_transition_renderer::start_outtransition(const lib::transition_info *info)
{
	if (m_trans_engine) stop();
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("d2_transition_renderer.start_outtransition(0x%x)", (void *)this);
	m_outtransition = info;
	m_trans_engine = d2_transition_engine(m_transition_dest, true, m_outtransition);
	if (m_transition_dest && m_trans_engine) {
		gui_window *window = m_transition_dest->get_gui_window();
		d2_window *cwindow = (d2_window *)window;
#ifdef D2D_NOTYET
		AmbulantView *view = (AmbulantView *)cwindow->view();
		[view incrementTransitionCount];
#endif
		m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
		m_fullscreen = m_outtransition->m_scope == scope_screen;
		if (m_fullscreen) {
			get_d2player()->start_screen_transition(true);
		}
	}

	m_lock.leave();
	if (m_transition_dest) m_transition_dest->need_redraw();
}

void
d2_transition_renderer::stop()
{
	m_lock.enter();
	if (m_trans_engine == NULL) {
		m_lock.leave();
		return;
	}
	if (m_fullscreen) {
		get_d2player()->end_screen_transition();
		m_fullscreen = false;
	} else {
		delete m_trans_engine;
		m_trans_engine = NULL;
	}
	gui_window *window = m_transition_dest->get_gui_window();
	d2_window *cwindow = (d2_window *)window;
#ifdef D2D_NOTYET
	AmbulantView *view = (AmbulantView *)cwindow->view();
	[view decrementTransitionCount];
	if (m_fullscreen) {
		[view endScreenTransition];
	}
#endif
	m_lock.leave();
	if (m_transition_dest) m_transition_dest->transition_done();
	m_d2player = NULL;
}

void
d2_transition_renderer::redraw_pre(gui_window *window)
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("d2_transition_renderer.redraw_pre(0x%x) m_trans_engine=0x%x m_fullscreen=%d", (void *)this,m_trans_engine,m_fullscreen);
	if (m_trans_engine != NULL) {
		if (m_fullscreen) {
			AM_DBG lib::logger::get_logger()->debug("d2_transition_renderer.redraw_pre(0x%x): now=%d",this, m_event_processor->get_timer()->elapsed());
			set_fullscreen_rendertarget(get_transition_rendertarget());
			AM_DBG lib::logger::get_logger()->debug("d2_transition_renderer.redraw_pre(0x%x): s_fullscreen_rendertarget=0x%x",this, d2_transition_renderer::s_fullscreen_rendertarget);
			m_transition_rendertarget = NULL; // prevents Release() by destructor, to be done in d2_player::_screenTransitionPostRedraw()
		} else {
			get_d2player()->set_transition_rendertarget((ID2D1BitmapRenderTarget*) get_transition_rendertarget());
		}
	}
#ifdef D2D_NOTYET
	d2_window *cwindow = (d2_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();

	// See whether we're in a transition
	NSImage *surf = NULL;
	if (m_trans_engine) {
		surf = [view getTransitionSurface];
		if (surf && [surf isValid]) {
			[surf lockFocus];
			AM_DBG logger::get_logger()->debug("d2_transition_renderer.redraw: drawing to transition surface");
		} else {
			lib::logger::get_logger()->trace("d2_transition_renderer.redraw: cannot lockFocus for transition");
			surf = NULL;
		}
	}
#endif
	m_lock.leave();
}

void
d2_transition_renderer::redraw_post(gui_window *window)
{
	m_lock.enter();
	if (m_trans_engine != NULL) {
		if (m_fullscreen) {
			d2::d2_player* d2_player = get_d2player();
			ID2D1Bitmap* old_bitmap = d2_player->get_fullscreen_old_bitmap();
			if (old_bitmap != NULL) {
				ID2D1RenderTarget* rt =	d2_player->get_rendertarget();
				rt->DrawBitmap(old_bitmap);
				HRESULT hr = rt->Flush();
#ifdef	AM_DMP
				int idx = d2_player->dump(d2_player->get_rendertarget(), "obm");
				lib::logger::get_logger()->debug("d2_transition_renderer.redraw_pre(0x%x) DrawBitmap(oldbitmap) hr=0x%x idx=%d", this, hr, idx);
#endif//AM_DMP
			}
			get_d2player()->screen_transition_step(m_trans_engine, m_event_processor->get_timer()->elapsed());
		} else {
			m_trans_engine->step(m_event_processor->get_timer()->elapsed());
		}
		typedef lib::no_arg_callback<d2_transition_renderer> transition_callback;
		lib::event *ev = new transition_callback(this, &d2_transition_renderer::transition_step);
		lib::transition_info::time_type delay = m_trans_engine->next_step_delay();
		if (delay < 20) delay = 20;
		AM_DBG lib::logger::get_logger()->debug("d2_transition_renderer.redraw: now=%d, schedule step for %d", m_event_processor->get_timer()->elapsed(), m_event_processor->get_timer()->elapsed()+delay);
		m_event_processor->add_event(ev, delay, lib::ep_med);
	}
	if (m_trans_engine && m_trans_engine->is_done()) {
		typedef lib::no_arg_callback<d2_transition_renderer> stop_transition_callback;
		lib::event *ev = new stop_transition_callback(this, &d2_transition_renderer::stop);
		m_event_processor->add_event(ev, 0, lib::ep_med);
	}
#ifdef D2D_NOTYET
	d2_window *cwindow = (d2_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	NSImage *surf = NULL;
	if (m_trans_engine) {
		surf = [view getTransitionSurface];
		if (![surf isValid]) surf = NULL;
	}
	if (surf) {
		[surf unlockFocus];
		AM_DBG logger::get_logger()->debug("d2_transition_renderer.redraw: drawing to view");
		if (m_fullscreen)
			[view screenTransitionStep: m_trans_engine
				elapsed: m_event_processor->get_timer()->elapsed()];
		else
			m_trans_engine->step(m_event_processor->get_timer()->elapsed());
		typedef lib::no_arg_callback<d2_transition_renderer> transition_callback;
		lib::event *ev = new transition_callback(this, &d2_transition_renderer::transition_step);
		lib::transition_info::time_type delay = m_trans_engine->next_step_delay();
		if (delay < 33) delay = 33; // XXX band-aid
		AM_DBG lib::logger::get_logger()->debug("d2_transition_renderer.redraw: now=%d, schedule step for %d", m_event_processor->get_timer()->elapsed(), m_event_processor->get_timer()->elapsed()+delay);
		m_event_processor->add_event(ev, delay, lib::ep_med);
	}

	// Finally, if the transition is done clean it up and signal that freeze_transition
	// can end for our peer renderers.
	// Note that we have to do this through an event because of locking issues.
	if (m_trans_engine && m_trans_engine->is_done()) {
		typedef lib::no_arg_callback<d2_transition_renderer> stop_transition_callback;
		lib::event *ev = new stop_transition_callback(this, &d2_transition_renderer::stop);
		m_event_processor->add_event(ev, 0, lib::ep_med);
		if (m_fullscreen)
			[view screenTransitionStep: NULL elapsed: 0];
	}
#endif
	if ( ! this->m_fullscreen)
		SafeRelease(&this->m_transition_rendertarget);
	m_lock.leave();
}

void
d2_transition_renderer::transition_step()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("d2_transition_renderer.transition_step: now=%d", m_event_processor->get_timer()->elapsed());
	if (m_transition_dest) m_transition_dest->need_redraw();
	m_lock.leave();
}

} // namespace d2

} // namespace gui

} //namespace ambulant

