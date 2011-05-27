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

#include "ambulant/gui/gtk/gtk_includes.h"
#include "ambulant/gui/gtk/gtk_renderer.h"
#include "ambulant/gui/gtk/gtk_transition.h"
#include "ambulant/gui/gtk/gtk_util.h"
#include "ambulant/lib/colors.h"
#include "ambulant/lib/logger.h"

#include <gtk-2.0/gdk/gdkx.h>
#if 1
// This is needed on Ubuntu 10.10, but maybe the old pathnames are still needed
// on another platform? If so, we need to find a define to trigger on.
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf-xlib/gdk-pixbuf-xlib.h>
#else
#include <gtk-2.0/gdk-pixbuf/gdk-pixbuf.h>
#include <gtk-2.0/gdk-pixbuf-xlib/gdk-pixbuf-xlib.h>
#endif
//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace gtk {

// Helper functions to setup transitions

static void
setup_transition(bool outtrans, ambulant_gtk_window *agw, GdkPixmap** oldpxmp, GdkPixmap** newpxmp)
{
	if (outtrans) {
		if (agw->m_tmppixmap == NULL) {
			// make a copy
			//agw->m_tmppixmap = new GdkPixmap(*agw->get_ambulant_oldpixmap());
			agw->m_tmppixmap = agw->get_ambulant_oldpixmap();
		}
		*oldpxmp = agw->get_ambulant_surface();
		*newpxmp = agw->m_tmppixmap;
	} else {
		*oldpxmp = agw->get_ambulant_pixmap();
		*newpxmp = agw->get_ambulant_surface();
	}
}
static void
finalize_transition(bool outtrans, ambulant_gtk_window *agw,  common::surface *dest)
{
	if (outtrans) {
		// copy the pixels in m_tmppixmap to the on-screen pixmap
		GdkPixmap* dest_pixmap = agw->get_ambulant_pixmap();
		GdkPixmap* temp_pixmap = agw->get_ambulant_surface();
		const lib::rect &r=	 dest->get_clipped_screen_rect();
		AM_DBG logger::get_logger()->debug("finalize_transition: dest_pixmap=0x%x: temp_pixmap=0x%x (L,T,W,H)=(%d,%d,%d,%d)", dest_pixmap, temp_pixmap,r.left(),r.top(),r.width(), r.height());
		GdkGC *gc = gdk_gc_new (dest_pixmap);
		gdk_draw_pixmap(dest_pixmap, gc, temp_pixmap, r.left(),r.top(),r.left(),r.top(),r.width(), r.height());
		g_object_unref (G_OBJECT (gc));
	}
}


gtk_transition_blitclass_fade::gtk_transition_blitclass_fade()
:	smil2::transition_blitclass_fade(),
	m_new_pixbuf(NULL)
{
}

gtk_transition_blitclass_fade::~gtk_transition_blitclass_fade()
{
	if (m_new_pixbuf != NULL)
		g_object_unref (G_OBJECT (m_new_pixbuf));
}

void
gtk_transition_blitclass_fade::update()
{

	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_fade::update(%f)", m_progress);
	ambulant_gtk_window *agw = (ambulant_gtk_window *)m_dst->get_gui_window();
	GdkPixmap *npm, *opm;
	const rect& newrect_whole =	 m_dst->get_clipped_screen_rect();
	int L = newrect_whole.left(),  T = newrect_whole.top(),
		W = newrect_whole.width(), H = newrect_whole.height();
	setup_transition(m_outtrans, agw, &opm, &npm);
	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_fade::update(%f) agw=0x%x, opm=0x%x,npm0x%x", m_progress, agw, opm, npm);
	GdkPixbuf* old_pixbuf = gdk_pixbuf_get_from_drawable(NULL, opm, NULL, L, T, 0, 0, W, H);
	if (m_new_pixbuf == NULL)
		m_new_pixbuf = gdk_pixbuf_get_from_drawable(NULL, npm, NULL, L, T, 0, 0, W, H);
	int alpha = static_cast<int>(round(255*m_progress));
	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_fade::update(): ltwh=(%d,%d,%d,%d)",L,T,W,H);
	gdk_pixbuf_composite(m_new_pixbuf, old_pixbuf,0,0,W,H,0,0,1,1,GDK_INTERP_BILINEAR, alpha);
	GdkGC *gc = gdk_gc_new (opm);
	gdk_draw_pixbuf(opm, gc, old_pixbuf, 0, 0, L, T, W, H, GDK_RGB_DITHER_NONE,0,0);
	g_object_unref (G_OBJECT (gc));
	g_object_unref (G_OBJECT (old_pixbuf));
	finalize_transition(m_outtrans, agw, m_dst);
}

void
gtk_transition_blitclass_rect::update()
{
	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_rect::update(%f)", m_progress);
	ambulant_gtk_window *agw = (ambulant_gtk_window *)m_dst->get_gui_window();
	GdkPixmap *npm, *opm;
	setup_transition(m_outtrans, agw, &opm, &npm);

	rect newrect_whole = m_newrect;
	newrect_whole.translate(m_dst->get_global_topleft());
	newrect_whole &= m_dst->get_clipped_screen_rect();
	int L = newrect_whole.left(),  T = newrect_whole.top(),
		W = newrect_whole.width(), H = newrect_whole.height();
	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_rect: opm=0x%x, npm=0x%x, (L,T,W,H)=(%d,%d,%d,%d)",opm,npm,L,T,W,H);
	GdkGC *gc = gdk_gc_new (opm);
	gdk_draw_pixmap(opm, gc,  npm, L, T, L, T, W, H);
	g_object_unref (G_OBJECT (gc));
	finalize_transition(m_outtrans, agw, m_dst);
}

void
gtk_transition_blitclass_r1r2r3r4::update()
{
	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_r1r2r3r4::update(%f)", m_progress);
	ambulant_gtk_window *agw = (ambulant_gtk_window *)m_dst->get_gui_window();
	GdkPixmap *npm, *opm;
	setup_transition(m_outtrans, agw, &opm, &npm);
	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_r1r2r3r4::update() opm=0x%x, npm=0x%x.", opm, npm);
	rect oldsrcrect_whole = m_oldsrcrect;
	rect olddstrect_whole = m_olddstrect;
	rect newsrcrect_whole = m_newsrcrect;
	rect newdstrect_whole = m_newdstrect;
	oldsrcrect_whole.translate(m_dst->get_global_topleft());
	oldsrcrect_whole &= m_dst->get_clipped_screen_rect();
	olddstrect_whole.translate(m_dst->get_global_topleft());
	olddstrect_whole &= m_dst->get_clipped_screen_rect();
	newsrcrect_whole.translate(m_dst->get_global_topleft());
	newsrcrect_whole &= m_dst->get_clipped_screen_rect();
	newdstrect_whole.translate(m_dst->get_global_topleft());
	newdstrect_whole &= m_dst->get_clipped_screen_rect();
	int Loldsrc = oldsrcrect_whole.left(),
		Toldsrc = oldsrcrect_whole.top(),
		Woldsrc = oldsrcrect_whole.width(),
		Holdsrc = oldsrcrect_whole.height();
	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_r1r2r3r4: (Loldsrc,Toldsrc,Woldsrc,Holdsrc)=(%d,%d,%d,%d)",Loldsrc,Toldsrc,Woldsrc,Holdsrc);
	int Lolddst = olddstrect_whole.left(),
		Tolddst = olddstrect_whole.top(),
		Wolddst = olddstrect_whole.width(),
		Holddst = olddstrect_whole.height();
	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_r1r2r3r4: (Lolddst,Tolddst,Wolddst,Holddst)=(%d,%d,%d,%d)",Lolddst,Tolddst,Wolddst,Holddst);
	int Lnewsrc = newsrcrect_whole.left(),
		Tnewsrc = newsrcrect_whole.top(),
		Wnewsrc = newsrcrect_whole.width(),
		Hnewsrc = newsrcrect_whole.height();
	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_r1r2r3r4: (Lnewsrc,Tnewsrc,Wnewsrc,Hnewsrc)=(%d,%d,%d,%d)",Lnewsrc,Tnewsrc,Wnewsrc,Hnewsrc);
	int Lnewdst = newdstrect_whole.left(),
		Tnewdst = newdstrect_whole.top(),
		Wnewdst = newdstrect_whole.width(),
		Hnewdst = newdstrect_whole.height();
	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_r1r2r3r4: (Lnewdst,Tnewdst,Wnewdst,Hnewdst)=(%d,%d,%d,%d)",Lnewdst,Tnewdst,Wnewdst,Hnewdst);
	GdkGC *gc = gdk_gc_new (opm);
	gdk_draw_pixmap(opm, gc, opm, Loldsrc, Toldsrc, Lolddst, Tolddst, Woldsrc, Hnewsrc);
	gdk_draw_pixmap(opm, gc, npm, Lnewsrc, Tnewsrc, Lnewdst, Tnewdst, Wnewsrc, Hnewsrc);
	g_object_unref (G_OBJECT (gc));
	finalize_transition(m_outtrans, agw, m_dst);
}

void
gtk_transition_blitclass_rectlist::update()
{

	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_rectlist::update(%f)", m_progress);
	ambulant_gtk_window *agw = (ambulant_gtk_window *)m_dst->get_gui_window();
	GdkPixmap *npm, *opm;
	setup_transition(m_outtrans, agw, &opm, &npm);
	const rect& dstrect_whole = m_dst->get_clipped_screen_rect();
	int Ldst = dstrect_whole.left(), Tdst = dstrect_whole.top(),
		Wdst = dstrect_whole.width(), Hdst = dstrect_whole.height();
	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_rectlist: (L,T,W,H)=(%d,%d,%d,%d)",Ldst,Tdst,Wdst,Hdst);
	GdkGC *gc = gdk_gc_new (opm);
	GdkRegion* region = gdk_region_new();
	std::vector< rect >::iterator newrect;
	for(newrect=m_newrectlist.begin(); newrect != m_newrectlist.end(); newrect++) {
		rect corner_rect = *newrect;
		corner_rect.translate(m_dst->get_global_topleft());
		corner_rect &= m_dst->get_clipped_screen_rect();
		int L = corner_rect.left(), T = corner_rect.top(),
			W = corner_rect.width(), H = corner_rect.height();
		GdkRectangle rectangle;
		rectangle.x = L;
		rectangle.y = T;
		rectangle.width = W;
		rectangle.height = H;
		AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_rectlist: (L,T,W,H)=(%d,%d,%d,%d)",L,T,W,H);
		gdk_region_union_with_rect(region, &rectangle);
	}
	gdk_gc_set_clip_region(gc, region);
	gdk_draw_pixmap(opm, gc, npm, Ldst, Tdst, Ldst, Tdst, Wdst, Hdst);
	g_object_unref (G_OBJECT (gc)); // clears region as well
	finalize_transition(m_outtrans, agw, m_dst);
}

void
gtk_transition_blitclass_poly::update()
{

	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_poly::update(%f)", m_progress);
	ambulant_gtk_window *agw = (ambulant_gtk_window *)m_dst->get_gui_window();
	GdkPixmap *npm, *opm;
	const lib::point& dst_global_topleft = m_dst->get_global_topleft();
	setup_transition(m_outtrans, agw, &opm, &npm);
	uint n_points = m_newpolygon.size();
	if (n_points <= 2) { // cannot create polygon, maybe not yet implemented
		return;
	}
	GdkPoint* points = (GdkPoint*) malloc (n_points*sizeof(GdkPoint));
	uint idx = 0;
	std::vector<point>::iterator newpoint;
	for( newpoint=m_newpolygon.begin(); newpoint != m_newpolygon.end(); newpoint++) {
		point p = *newpoint + dst_global_topleft;
		points[idx].x = p.x;
		points[idx].y = p.y;
		idx++;
	}
	GdkRegion* region = gdk_region_polygon(points, n_points, GDK_WINDING_RULE);
	free(points);
	GdkGC *gc = gdk_gc_new (opm);
	gdk_gc_set_clip_region(gc, region);
	const rect& newrect_whole =	 m_dst->get_clipped_screen_rect();
	int Ldst= newrect_whole.left(), Tdst = newrect_whole.top(),
		Wdst = newrect_whole.width(), Hdst = newrect_whole.height();
	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_poly::update(): ltwh=(%d,%d,%d,%d)",Ldst,Tdst,Wdst,Hdst);
	gdk_draw_pixmap(opm, gc, npm, Ldst, Tdst, Ldst, Tdst, Wdst, Hdst);
	gdk_region_destroy(region);
	g_object_unref (G_OBJECT (gc));
	finalize_transition(m_outtrans, agw, m_dst);
}

void
gtk_transition_blitclass_polylist::update()
{

	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_polylist::update(%f)", m_progress);
	ambulant_gtk_window *agw = (ambulant_gtk_window *)m_dst->get_gui_window();
	GdkPixmap *npm, *opm;
	const lib::point& dst_global_topleft = m_dst->get_global_topleft();
	setup_transition(m_outtrans, agw, &opm, &npm);
	GdkRegion* clip_region = gdk_region_new();
	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_polylist: m_newpolygonlist.size()=%d", m_newpolygonlist.size());
	std::vector< std::vector<point> >::iterator partpolygon;
	for (partpolygon=m_newpolygonlist.begin(); partpolygon!=m_newpolygonlist.end(); partpolygon++) {
		uint n_points = partpolygon->size();
		if (n_points <= 2) { // cannot create polygon
			logger::get_logger()->warn("gtk_transition_blitclass_polylist: cannot create polygon, partpolygon.size()=%d", n_points);			break;
		}
		GdkPoint* points = (GdkPoint*) malloc (n_points*sizeof(GdkPoint));
		uint idx = 0;
		std::vector<point>::iterator newpoint;
		AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_polylist: partpolygon.size()=%d", partpolygon->size());
		for( newpoint=partpolygon->begin(); newpoint != partpolygon->end(); newpoint++) {
			point p = *newpoint + dst_global_topleft;
			points[idx].x = p.x;
			points[idx].y = p.y;
			idx++;
			AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_polylist: idx=%d, p=(%d,%d)", idx, p.x, p.y);
		}
		GdkRegion* next_region = gdk_region_polygon(points, n_points, GDK_WINDING_RULE);
		free(points);
		gdk_region_union (clip_region, next_region);
		gdk_region_destroy(next_region);
	}
	rect newrect_whole =  m_dst->get_rect();
	newrect_whole.translate(dst_global_topleft);
	newrect_whole &= m_dst->get_clipped_screen_rect();
	int Ldst= newrect_whole.left(), Tdst = newrect_whole.top(),
		Wdst = newrect_whole.width(), Hdst = newrect_whole.height();
	AM_DBG logger::get_logger()->debug("gtk_transition_blitclass_polylist::update(): ltwh=(%d,%d,%d,%d)",Ldst,Tdst,Wdst,Hdst);
	GdkGC *gc = gdk_gc_new (opm);
	gdk_gc_set_clip_region(gc, clip_region);
	gdk_draw_pixmap(opm, gc, npm, Ldst, Tdst, Ldst, Tdst, Wdst, Hdst);
	gdk_region_destroy(clip_region);
	g_object_unref (G_OBJECT (gc));
	finalize_transition(m_outtrans, agw, m_dst);
}

smil2::transition_engine *
gtk_transition_engine(common::surface *dst, bool is_outtrans, const transition_info *info)
{
	smil2::transition_engine *rv;
	AM_DBG logger::get_logger()->debug("gtk_transition_engine: info=0x%x info->m_type=%d", info, info->m_type);

	switch(info->m_type) {
	// Series 1: edge wipes
	case barWipe:
		rv = new gtk_transition_engine_barwipe();
		break;
	case boxWipe:
		rv = new gtk_transition_engine_boxwipe();
		break;
	case fourBoxWipe:
		rv = new gtk_transition_engine_fourboxwipe();
		break;
	case barnDoorWipe:
		rv = new gtk_transition_engine_barndoorwipe();
		break;
	case diagonalWipe:
		rv = new gtk_transition_engine_diagonalwipe();
		break;
	case miscDiagonalWipe:
		rv = new gtk_transition_engine_miscdiagonalwipe();
		break;
	case veeWipe:
		rv = new gtk_transition_engine_veewipe();
		break;
	case barnVeeWipe:
		rv = new gtk_transition_engine_barnveewipe();
		break;
	case zigZagWipe:
		rv = new gtk_transition_engine_zigzagwipe();
		break;
	case barnZigZagWipe:
		rv = new gtk_transition_engine_barnzigzagwipe();
		break;
	case bowTieWipe:
		rv = new gtk_transition_engine_bowtiewipe();
		break;
	// series 2: iris wipes
	case irisWipe:
		rv = new gtk_transition_engine_iriswipe();
		break;
	case pentagonWipe:
		rv = new gtk_transition_engine_pentagonwipe();
		break;
	case arrowHeadWipe:
		rv = new gtk_transition_engine_arrowheadwipe();
		break;
	case triangleWipe:
		rv = new gtk_transition_engine_trianglewipe();
		break;
	case hexagonWipe:
		rv = new gtk_transition_engine_hexagonwipe();
		break;
	case eyeWipe:
		rv = new gtk_transition_engine_eyewipe();
		break;
	case roundRectWipe:
		rv = new gtk_transition_engine_roundrectwipe();
		break;
	case ellipseWipe:
		rv = new gtk_transition_engine_ellipsewipe();
		break;
	case starWipe:
		rv = new gtk_transition_engine_starwipe();
		break;
	case miscShapeWipe:
		rv = new gtk_transition_engine_miscshapewipe();
		break;
	// series 3: clock-type wipes
	case clockWipe:
		rv = new gtk_transition_engine_clockwipe();
		break;
	case singleSweepWipe:
		rv = new gtk_transition_engine_singlesweepwipe();
		break;
	case doubleSweepWipe:
		rv = new gtk_transition_engine_doublesweepwipe();
		break;
	case saloonDoorWipe:
		rv = new gtk_transition_engine_saloondoorwipe();
		break;
	case windshieldWipe:
		rv = new gtk_transition_engine_windshieldwipe();
		break;
	case fanWipe:
		rv = new gtk_transition_engine_fanwipe();
		break;
	case doubleFanWipe:
		rv = new gtk_transition_engine_doublefanwipe();
		break;
	case pinWheelWipe:
		rv = new gtk_transition_engine_pinwheelwipe();
		break;
	// series 4: matrix wipe types
	case snakeWipe:
		rv = new gtk_transition_engine_snakewipe();
		break;
	case waterfallWipe:
		rv = new gtk_transition_engine_waterfallwipe();
		break;
	case spiralWipe:
		rv = new gtk_transition_engine_spiralwipe();
		break;
	case parallelSnakesWipe:
		rv = new gtk_transition_engine_parallelsnakeswipe();
		break;
	case boxSnakesWipe:
		rv = new gtk_transition_engine_boxsnakeswipe();
		break;
	// series 5: SMIL-specific types
	case pushWipe:
		rv = new gtk_transition_engine_pushwipe();
		break;
	case slideWipe:
		rv = new gtk_transition_engine_slidewipe();
		break;
	case fade:
	case audioVisualFade:
		rv = new gtk_transition_engine_fade();
		break;
	default:
		logger::get_logger()->warn(gettext("%s: transition type %s not yet implemented"),"gtk_transition_engine",repr(info->m_type).c_str());
		rv = NULL;
	}
	if (rv)
		rv->init(dst, is_outtrans, info);
	return rv;
}

} // namespace gtk

} // namespace gui

} //namespace ambulant

