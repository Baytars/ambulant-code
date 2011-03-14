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

#include "ambulant/gui/gtk/gtk_includes.h"
#include "ambulant/gui/gtk/gtk_renderer.h"
#include "ambulant/gui/gtk/gtk_factory.h"
#include "ambulant/gui/gtk/gtk_fill.h"
#include "ambulant/gui/gtk/gtk_transition.h"
#include "ambulant/gui/gtk/gtk_image_renderer.h"
#include "ambulant/gui/gtk/gtk_text_renderer.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace gui::gtk;

gtk_fill_renderer::~gtk_fill_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("~gtk_fill_renderer(0x%x)", (void *)this);
	m_intransition = NULL;
	m_outtransition = NULL;
	if (m_trans_engine) delete m_trans_engine;
	m_trans_engine = NULL;
	m_lock.leave();
}
	
void
gtk_fill_renderer::start(double where)
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("gtk_fill_renderer.start(0x%x)", (void *)this);
	if (m_is_showing) {
		logger::get_logger()->trace("gtk_fill_renderer.start(0x%x): already started", (void*)this);
		m_lock.leave();
		return;
	}
	m_is_showing = true;
	if (!m_dest) {
		logger::get_logger()->trace("gtk_fill_renderer.start(0x%x): no surface", (void *)this);
		return;
	}
	if (m_intransition) {
		m_trans_engine = gtk_transition_engine(m_dest, false, m_intransition);
		m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
	}
	m_dest->show(this);
	assert(m_context);
	m_context->started(m_cookie);
	m_context->stopped(m_cookie);
	m_lock.leave();
}

void
gtk_fill_renderer::start_outtransition(lib::transition_info *info)
{
	m_lock.enter();
	m_outtransition = info;
	if (m_outtransition) {
		// XXX Schedule beginning of out transition
		//lib::event *ev = new transition_callback(this, &transition_outbegin);
		//m_event_processor->add_event(ev, XXXX);
	}
	m_lock.leave();
}

void
gtk_fill_renderer::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("gtk_fill_renderer.stop(0x%x)", (void *)this);
	if (!m_is_showing) {
		logger::get_logger()->trace("gtk_fill_renderer.stop(0x%x): already stopped", (void*)this);
	} else {
		m_is_showing = false;
		if (m_dest) m_dest->renderer_done(this);
	}
	assert(m_context);
	m_context->stopped(m_cookie);
	m_lock.leave();
}

void
gtk_fill_renderer::redraw(const rect &dirty,
			 gui_window *window)
{

	m_lock.enter();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("gtk_fill_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)",(void *)this,r.left(),r.top(),r.right(),r.bottom());
	
	ambulant_gtk_window* agtkw = (ambulant_gtk_window*) window;
	GdkPixmap *surf = NULL;
	if (m_trans_engine && m_trans_engine->is_done()) {
		delete m_trans_engine;
		m_trans_engine = NULL;
	}
	// See whether we're in a transition
	if (m_trans_engine) {
		GdkPixmap *qpm = agtkw->get_ambulant_pixmap();
		surf = agtkw->get_ambulant_surface();
		if (surf == NULL)
			surf = agtkw->new_ambulant_surface();
		if (surf != NULL) {
			agtkw->set_ambulant_surface(surf);
			// Copy the background pixels
			rect dstrect = r;
			dstrect.translate(m_dest->get_global_topleft());
			AM_DBG logger::get_logger()->debug("gtk_fill.redraw: bitBlt to=0x%x (%d,%d) from=0x%x (%d,%d,%d,%d)",surf, dstrect.left(), dstrect.top(), qpm,dstrect.left(), dstrect.top(), dstrect.width(), dstrect.height());
//			bitBlt(surf, dstrect.left(),dstrect.top(),
//			       qpm,dstrect.left(),dstrect.top(),dstrect.width(),dstrect.height());
//			bitBlt(surf, dstrect.left(), dstrect.top(),
//			       qpm,  dstrect.left(), dstrect.top(), dstrect.width(), dstrect.height());
			AM_DBG logger::get_logger()->debug("gtk_fill_renderer.redraw: drawing to transition surface");
		}
	}

	redraw_body(dirty, window);
	
	if (surf != NULL) {
		agtkw->reset_ambulant_surface();
	}
	if (m_trans_engine && surf) {
		AM_DBG logger::get_logger()->debug
		  ("gtk_fill_renderer.redraw: drawing to view");
		m_trans_engine->step(m_event_processor->get_timer()->elapsed());
		typedef no_arg_callback<gtk_fill_renderer>transition_callback;
		event *ev = new transition_callback(this, &gtk_fill_renderer::transition_step);
		transition_info::time_type delay = m_trans_engine->next_step_delay();
		if (delay < 33) delay = 33; // XXX band-aid
//		delay = 500;
		AM_DBG logger::get_logger()->debug("gtk_fill_renderer.redraw: now=%d, schedule step for %d",m_event_processor->get_timer()->elapsed(), m_event_processor->get_timer()->elapsed()+delay);
		m_event_processor->add_event(ev, delay, ep_low);
	}
	m_lock.leave();
}


void
gtk_fill_renderer::transition_step()
{
	if (m_dest) m_dest->need_redraw();
}

void 
gtk_fill_renderer::user_event(const point &where, int what)
{
	if (what == user_event_click) m_context->clicked(m_cookie, 0);
	else if (what == user_event_mouse_over) m_context->pointed(m_cookie, 0);
	else assert(0);
}

void
gtk_fill_renderer::redraw_body(const lib::rect &dirty,
				     common::gui_window *window) {

	const common::region_info *info = m_dest->get_info();
	const lib::rect &r = m_dest->get_rect();
	ambulant_gtk_window* agtkw = (ambulant_gtk_window*) window;
	// <brush> drawing
	// First find our whole area to be cleared to <brush> color
	lib::rect dstrect_whole = r;
	dstrect_whole.translate(m_dest->get_global_topleft());
	int	L = dstrect_whole.left(), 
		T = dstrect_whole.top(),
		W = dstrect_whole.width(), 
		H = dstrect_whole.height();
	// Fill with  color
	const char *color_attr = m_node->get_attribute("color");
	if (!color_attr) {
		lib::logger::get_logger()->trace("<brush> element without color attribute");
		return;
	}
	// Fill with <brush> color
	color_t color = lib::to_color(color_attr);
	lib::color_t bgcolor = info->get_bgcolor();
	AM_DBG lib::logger::get_logger()->debug
		("gtk_fill_renderer.redraw_body: clearing to 0x%x", 
		 (long)color);
	GdkColor bgc;
	bgc.red = redc(color)*0x101;
	bgc.blue = bluec(color)*0x101;
	bgc.green = greenc(color)*0x101;
	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
	gdk_gc_set_rgb_fg_color (gc, &bgc);
	gdk_draw_rectangle (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()), gc, TRUE, L, T, W, H);
	g_object_unref (G_OBJECT (gc));
	AM_DBG lib::logger::get_logger()->debug("gtk_fill_renderer.redraw_body(0x%x, local_ltrb=(%d,%d,%d,%d)",(void *)this, L,T,W,H);
}

void
gtk_background_renderer::redraw(const lib::rect &dirty,
			       common::gui_window *window)
{
	const lib::rect &r = m_dst->get_rect();
	AM_DBG lib::logger::get_logger()->debug("gtk_background_renderer::redraw(0x%x)", (void *)this);
	if (m_src && !m_src->get_transparent()) {
	// First find our whole area to be cleared to background color
		ambulant_gtk_window* agtkw = (ambulant_gtk_window*) window;
		lib::rect dstrect_whole = r;
		dstrect_whole.translate(m_dst->get_global_topleft());
		int L = dstrect_whole.left(),
		    T = dstrect_whole.top(),
		    W = dstrect_whole.width(),
		    H = dstrect_whole.height();
		// XXXX Fill with background color
		lib::color_t bgcolor = m_src->get_bgcolor();
		AM_DBG lib::logger::get_logger()->debug("gtk_background_renderer::redraw: clearing to %x, agtkw=0x%x local_ltwh(%d,%d,%d,%d)",(long)bgcolor,(void*)agtkw,L,T,W,H);
		GdkColor bgc;
		bgc.red = redc(bgcolor)*0x101;
		bgc.blue = bluec(bgcolor)*0x101;
		bgc.green = greenc(bgcolor)*0x101;
		GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
		gdk_gc_set_rgb_fg_color (gc, &bgc);
		gdk_draw_rectangle (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()), gc, TRUE, L, T, W, H);
		g_object_unref (G_OBJECT (gc));
		//gtk_widget_modify_bg (GTK_WIDGET (agtkw->get_ambulant_widget()->get_gtk_widget()), GTK_STATE_NORMAL, &bgc );
		if (m_background_pixmap) {
			AM_DBG lib::logger::get_logger()->debug("gtk_background_renderer::redraw: drawing pixmap");
		//	paint.drawPixmap(L, T, *m_background_pixmap);
		}
	}
}

void gtk_background_renderer::highlight(gui_window *window)
{
}

void
gtk_background_renderer::keep_as_background()
{
	AM_DBG lib::logger::get_logger()->debug("gtk_background_renderer::keep_as_background(0x%x) called", (void *)this);
	const lib::rect &r = m_dst->get_rect();
	ambulant_gtk_window* agtkw = (ambulant_gtk_window*) m_dst->get_gui_window();
	lib::rect dstrect_whole = r;
	dstrect_whole.translate(m_dst->get_global_topleft());
	if (m_background_pixmap) {
		delete m_background_pixmap;
		m_background_pixmap = NULL;
	}
	m_background_pixmap = agtkw->get_pixmap_from_screen(dstrect_whole);
//XXXX	dumpPixmap(m_background_pixmap, "/tmp/keepbg");
}