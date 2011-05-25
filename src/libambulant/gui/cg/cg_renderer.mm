// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
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

#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/gui/cg/cg_renderer.h"
#include "ambulant/gui/cg/cg_transition.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
//#define AM_DMP
#ifndef AM_DMP
#define AM_DMP if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cg {

#ifndef WITH_UIKIT
cg_transition_renderer::~cg_transition_renderer()
{
	stop();
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~cg_transition_renderer(0x%x)", (void *)this);
//	m_intransition = NULL;
//	m_outtransition = NULL;
	m_lock.leave();
}

void
cg_transition_renderer::set_surface(common::surface *dest)
{
	m_transition_dest = dest;
	if (m_transition_dest && m_intransition && m_intransition->m_scope == scope_screen)
		m_transition_dest = m_transition_dest->get_top_surface();
}

void
cg_transition_renderer::set_intransition(const lib::transition_info *info) {
	m_intransition = info;
	if (m_transition_dest && m_intransition && m_intransition->m_scope == scope_screen)
		m_transition_dest = m_transition_dest->get_top_surface();
}

void
cg_transition_renderer::start(double where)
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("cg_transition_renderer.start(%f)", where);
	if (m_intransition && m_transition_dest) {
		AM_DBG logger::get_logger()->debug("cg_transition_renderer.start: with intransition");
		m_trans_engine = cg_transition_engine(m_transition_dest, false, m_intransition);
		if (m_trans_engine) {
			gui_window *window = m_transition_dest->get_gui_window();
			cg_window *cwindow = (cg_window *)window;
			AmbulantView *view = (AmbulantView *)cwindow->view();
			[view incrementTransitionCount];
			m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
			m_fullscreen = m_intransition->m_scope == scope_screen;
			if (m_fullscreen) {
				[view startScreenTransition];
			}
		}
	}
	m_lock.leave();
}

void
cg_transition_renderer::start_outtransition(const lib::transition_info *info)
{
	if (m_trans_engine) stop();
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("cg_transition_renderer.start_outtransition(0x%x)", (void *)this);
	m_outtransition = info;
	m_trans_engine = cg_transition_engine(m_transition_dest, true, m_outtransition);
	if (m_transition_dest && m_trans_engine) {
		gui_window *window = m_transition_dest->get_gui_window();
		cg_window *cwindow = (cg_window *)window;
		AmbulantView *view = (AmbulantView *)cwindow->view();
		[view incrementTransitionCount];
		m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
		m_fullscreen = m_outtransition->m_scope == scope_screen;
		if (m_fullscreen) {
			[view startScreenTransition];
		}
	}
	m_lock.leave();
	if (m_transition_dest) m_transition_dest->need_redraw();
}

void
cg_transition_renderer::stop()
{
	m_lock.enter();
	if (m_trans_engine == NULL) {
		m_lock.leave();
		return;
	}
	delete m_trans_engine;
	m_trans_engine = NULL;
	gui_window *window = m_transition_dest->get_gui_window();
	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	[view decrementTransitionCount];
	if (m_fullscreen) {
		[view endScreenTransition];
	}
	m_lock.leave();
	if (m_transition_dest) m_transition_dest->transition_done();
}

void
cg_transition_renderer::redraw_pre(gui_window *window)
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("cg_transition_renderer.redraw(0x%x)", (void *)this);

	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();

	// See whether we're in a transition
	NSImage *surf = NULL;
	if (m_trans_engine) {
		surf = [view getTransitionSurface];
		if (surf && [surf isValid]) {
			[surf lockFocus];
			AM_DBG logger::get_logger()->debug("cg_transition_renderer.redraw: drawing to transition surface");
		} else {
			lib::logger::get_logger()->trace("cg_transition_renderer.redraw: cannot lockFocus for transition");
			surf = NULL;
		}
	}
	m_lock.leave();
}

void
cg_transition_renderer::redraw_post(gui_window *window)
{
	m_lock.enter();
	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	NSImage *surf = NULL;
	if (m_trans_engine) {
		surf = [view getTransitionSurface];
		if (![surf isValid]) surf = NULL;
	}
	if (surf) {
		[surf unlockFocus];
		AM_DBG logger::get_logger()->debug("cg_transition_renderer.redraw: drawing to view");
		if (m_fullscreen)
			[view screenTransitionStep: m_trans_engine
				elapsed: m_event_processor->get_timer()->elapsed()];
		else
			m_trans_engine->step(m_event_processor->get_timer()->elapsed());
		typedef lib::no_arg_callback<cg_transition_renderer> transition_callback;
		lib::event *ev = new transition_callback(this, &cg_transition_renderer::transition_step);
		lib::transition_info::time_type delay = m_trans_engine->next_step_delay();
		if (delay < 50) delay = 50; // XXX band-aid
		AM_DBG lib::logger::get_logger()->debug("cg_transition_renderer.redraw: now=%d, schedule step for %d", m_event_processor->get_timer()->elapsed(), m_event_processor->get_timer()->elapsed()+delay);
		m_event_processor->add_event(ev, delay, lib::ep_med);
	}

	// Finally, if the transition is done clean it up and signal that freeze_transition
	// can end for our peer renderers.
	// Note that we have to do this through an event because of locking issues.
	if (m_trans_engine && m_trans_engine->is_done()) {
		typedef lib::no_arg_callback<cg_transition_renderer> stop_transition_callback;
		lib::event *ev = new stop_transition_callback(this, &cg_transition_renderer::stop);
		m_event_processor->add_event(ev, 0, lib::ep_med);
		if (m_fullscreen)
			[view screenTransitionStep: NULL elapsed: 0];
	}
	m_lock.leave();
}

void
cg_transition_renderer::transition_step()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cg_transition_renderer.transition_step: now=%d", m_event_processor->get_timer()->elapsed());
	if (m_transition_dest) m_transition_dest->need_redraw();
	m_lock.leave();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////	
#endif// UIKIT_NOT_YET
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef	WITH_UIKIT

	cg_transition_renderer::~cg_transition_renderer()
{
	stop();
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~cg_transition_renderer(0x%x)", (void *)this);
//	m_intransition = NULL;
//	m_outtransition = NULL;
	m_lock.leave();
}
#ifdef _OBJC_
#endif//_OBJC_
#ifndef	AM_NO_DMP
#endif//AM_NO_DMP	

void
cg_transition_renderer::set_surface(common::surface *dest)
{
	m_transition_dest = (common::surface*) dest;
	if (m_transition_dest && m_intransition && m_intransition->m_scope == scope_screen)
		m_transition_dest = m_transition_dest->get_top_surface();
}
	
void
cg_transition_renderer::set_intransition(const lib::transition_info *info) {
	m_intransition = info;
	if (m_transition_dest && m_intransition && m_intransition->m_scope == scope_screen)
		m_transition_dest = m_transition_dest->get_top_surface();
}
	
void
cg_transition_renderer::start(double where)
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("cg_transition_renderer.start(%f)", where);
	if (m_intransition && m_transition_dest) {
		AM_DBG logger::get_logger()->debug("cg_transition_renderer.start: with intransition");
		m_trans_engine = cg_transition_engine(m_transition_dest, false, m_intransition);
		if (m_trans_engine) {
			gui_window *window = m_transition_dest->get_gui_window();
			cg_window *cwindow = (cg_window *)window;
			AmbulantView *view = (AmbulantView *)cwindow->view();
			[view incrementTransitionCount];
			m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
			m_fullscreen = m_intransition->m_scope == scope_screen;
			if (m_fullscreen) {
				[view startScreenTransition: NO];
			}
		}
	}
	m_lock.leave();
}
	
void
cg_transition_renderer::start_outtransition(const lib::transition_info *info)
{
	if (m_trans_engine) stop();
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("cg_transition_renderer.start_outtransition(0x%x)", (void *)this);
	m_outtransition = info;
	m_trans_engine = cg_transition_engine(m_transition_dest, true, m_outtransition);
	if (m_transition_dest && m_trans_engine) {
		gui_window *window = m_transition_dest->get_gui_window();
		cg_window *cwindow = (cg_window *)window;
		AmbulantView *view = (AmbulantView *)cwindow->view();
		[view incrementTransitionCount];
		m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
		m_fullscreen = m_outtransition->m_scope == scope_screen;
		if (m_fullscreen) {
			[view startScreenTransition: YES];
		}
	}
	m_lock.leave();
	if (m_transition_dest) m_transition_dest->need_redraw();
}
	
void
cg_transition_renderer::stop()
{
	m_lock.enter();
	if (m_trans_engine == NULL) {
		m_lock.leave();
		return;
	}
	delete m_trans_engine;
	m_trans_engine = NULL;
	gui_window *window = m_transition_dest->get_gui_window();
	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	[view decrementTransitionCount];
	if (m_fullscreen) {
		[view endScreenTransition];
	}
	m_lock.leave();
	if (m_transition_dest) m_transition_dest->transition_done();
	if (m_old_screen_image != NULL) {
		CFRelease(m_old_screen_image);
		m_old_screen_image = NULL;
	}
	if (m_new_screen_image != NULL) {
		CFRelease(m_new_screen_image);
		m_new_screen_image = NULL;
	}
}

#ifndef	AM_NO_DMP
static CGImageRef
get_partial_image (AmbulantView* view, lib::rect r) {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];	
	UIImage* screen_img = [AmbulantView UIImageFromUIView: view];
	CGImageRef cg_screen_img = screen_img.CGImage;
//X	CFRetain(cg_screen_img); //X
//X	if (1) return cg_screen_img; //X
	CGRect rect = CGRectFromAmbulantRect(r);
	rect = CGRectApplyAffineTransform(rect, view.transform);
	CGImageRef cg_partial_image = CGImageCreateWithImageInRect(cg_screen_img, rect);
	CFRetain(cg_partial_image);
	[pool release];
	return cg_partial_image;
}
#endif//AM_NO_DMP
	
void
cg_transition_renderer::redraw_pre(gui_window *window)
{
	m_lock.enter();
	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	int i = 0;
	AM_DMP i = [AmbulantView dumpUIView:view withId: @"pre"];
	AM_DBG logger::get_logger()->debug("cg_transition_renderer.redraw_pre(0x%x) i=%d", (void *)this, i);
		
	// See whether we're in a transition
	if (m_trans_engine && ! m_fullscreen) {
//X		surf = [view getTransitionSurface];
//X		UIGraphicsPushContext(CGLayerGetContext(surf));
		[view pushTransitionSurface];
	} else if (m_fullscreen && m_outtransition != NULL) {
		// activate the transition now
		[view pushTransitionSurface];
		m_fullscreen_outtrans_active = true;
	}

	m_lock.leave();
}
	
void
cg_transition_renderer::redraw_post(gui_window *window)
{
	m_lock.enter();
	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	CGLayerRef surf = NULL;
//X	dumpUIView (view, @"pst");
	
	if (m_trans_engine) {
		AM_DMP	[AmbulantView dumpUIView: view withId: @"bef"];	
		if ( ! m_fullscreen ) {
			[view popTransitionSurface];
		}
		surf = [view getTransitionSurface];
	
		AM_DMP	[AmbulantView dumpCGLayer: [view getTransitionSurface] withId: @"ts1"];
		AM_DBG logger::get_logger()->debug("cg_transition_renderer.redraw_post: drawing to view");
		if (m_fullscreen)
			[view screenTransitionStep: m_trans_engine
							   elapsed: m_event_processor->get_timer()->elapsed()];
		else {
			m_trans_engine->step(m_event_processor->get_timer()->elapsed());
		}
		AM_DMP	[AmbulantView dumpCGLayer: [view getTransitionSurface] withId: @"ts2"];
		AM_DMP	[AmbulantView dumpUIView: view withId: @"aft"];
		// schedule next step
		typedef lib::no_arg_callback<cg_transition_renderer> transition_callback;
		lib::event *ev = new transition_callback(this, &cg_transition_renderer::transition_step);
		lib::transition_info::time_type delay = m_trans_engine->next_step_delay();
		if (delay < 33) delay = 33; // XXX band-aid
		AM_DBG lib::logger::get_logger()->debug("cg_transition_renderer.redraw_post: now=%d, schedule step for %d", m_event_processor->get_timer()->elapsed(), m_event_processor->get_timer()->elapsed()+delay);
		m_event_processor->add_event(ev, delay, lib::ep_med);
	}
	//XX Finally, if the transition is done clean it up and signal that freeze_transition
	// can end for our peer renderers.
	// Note that we have to do this through an event because of locking issues.
	if (m_trans_engine && m_trans_engine->is_done()) {
		typedef lib::no_arg_callback<cg_transition_renderer> stop_transition_callback;
		lib::event *ev = new stop_transition_callback(this, &cg_transition_renderer::stop);
		m_event_processor->add_event(ev, 0, lib::ep_med);
		if ( ! m_fullscreen) {
			m_trans_engine->step(m_event_processor->get_timer()->elapsed());
		}
	}
	m_lock.leave();
}
	
void
cg_transition_renderer::transition_step()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cg_transition_renderer.transition_step: now=%d", m_event_processor->get_timer()->elapsed());
	if (m_transition_dest) m_transition_dest->need_redraw();
	m_lock.leave();
}
#endif//WITH_UIKIT

} // namespace cg

} // namespace gui

} //namespace ambulant

