/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_renderer.h"
#include "ambulant/gui/qt/qt_transition.h"
// #include "ambulant/gui/qt/qt_gui.h"
#include "ambulant/lib/colors.h"
#include "ambulant/lib/logger.h"

#define FILL_PURPLE
//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

namespace gui {

namespace qt {

void 
qt_transition_debug::paint_rect(ambulant_qt_window* aqw, // TMP
	   common::surface * dst,
	   lib::color_t color) 
{
	lib::screen_rect<int> dstrect_whole = dst->get_rect();
	QPainter paint;
	paint.begin(aqw->ambulant_pixmap());
	dstrect_whole.translate(dst->get_global_topleft());
	int L = dstrect_whole.left(),
	T = dstrect_whole.top(),
	W = dstrect_whole.width(),
	H = dstrect_whole.height();
	// XXXX Fill with background color
	AM_DBG lib::logger::get_logger()->trace(
				  "qt_transition_common::paint_rect:"
				  " %s0x%x,%s(%d,%d,%d,%d)",
				  " clearing to ", (long)color, 
				  " local_ltwh=",L,T,W,H);
	QColor bgc = QColor(lib::redc(color),
			    lib::greenc(color),
			    lib::bluec(color));
	paint.setBrush(bgc);
	paint.drawRect(L,T,W,H);
	paint.flush();
	paint.end();
}

void
qt_transition_blitclass_fade::update()
{
//	/*AM_DBG*/ lib::logger::get_logger()->trace("qt_transition_blitclass_fade::update(%f)", m_progress);
	ambulant_qt_window *aqw = (ambulant_qt_window *)m_dst->get_gui_window();
	QPixmap *qpm = aqw->ambulant_pixmap();
	QPixmap *npm = aqw->get_ambulant_surface();
	QImage img1 = qpm->convertToImage();
	QImage img2 = npm->convertToImage();
	QImage res = img1.copy();
	int i, j, iw = res.width(), ih = res.height();
//	AM_DBG lib::logger::get_logger()->trace("qt_transition_blitclass_fade::update() qpm=0x%x, npm=0x%x. img2=0x%x, img1=0x%x, res=0x%x, iw=%d, ih=%d", qpm, npm, &img2, &img1, &res, iw, ih);
	// Following code From: Qt-interest Archive, July 2002
	// blending of qpixmaps, Sebastian Loebbert 
#define	OPTIM
#ifndef	OPTIM
	double fac1 = 1.0 - m_progress;
	double fac2 = 1.0 - fac1;
#else /*OPTIM*/
	u_int mul1 = 255 - (u_int) (m_progress * 255);
	u_int mul2 = 255 - mul1;
#endif/*OPTIM*/
	for(int i = 0;i < iw;i++){
		for(int j = 0; j < ih;j++){
	    		QRgb p1 = img1.pixel(i,j);
	    		QRgb p2 = img2.pixel(i,j);
	    		res.setPixel(i,j,
#ifndef	OPTIM
				     qRgb ( (int)( qRed(p1)*fac1 + 
						   qRed(p2)*fac2  ),
					    (int)( qGreen(p1)*fac1 + 
						   qGreen(p2)*fac2  ),
					    (int)( qBlue(p1)*fac1 + 
						   qBlue(p2)*fac2  ) )
#else /*OPTIM*/
				     qRgb ( ( qRed(p1)*mul1 + 
					      qRed(p2)*mul2  ) >> 8,
					    ( qGreen(p1)*mul1 +
					      qGreen(p2)*mul2  ) >> 8,
					    ( qBlue(p1)*mul1 +
					      qBlue(p2)*mul2  ) >> 8 )
#endif/*OPTIM*/
			 );
//	    if (j&4 && !(j&3) && i&4 &&!(i&3)) /* AM_DBG */ lib::logger::get_logger()->trace("qt_transition_blitclass_fade::update(): i=%3d, j=%3d, p1=0x%x, p2=0x%x, res=0x%x", i, j, p1, p2, res.pixel(i,j));
	  }
	}
	lib::screen_rect<int> newrect_whole =  m_dst->get_rect();
	newrect_whole.translate(m_dst->get_global_topleft());
	int L = newrect_whole.left(), T = newrect_whole.top(),
        	W = newrect_whole.width(), H = newrect_whole.height();
	QPainter paint;
	paint.begin(qpm);
	AM_DBG lib::logger::get_logger()->trace(
				  "qt_transition_blitclass_fade::update(): "
				  " ltwh=(%d,%d,%d,%d)",L,T,W,H);
	paint.drawImage(L,T,res,0,0,W,H);
	paint.flush();
	paint.end();
}

void
qt_transition_blitclass_rect::update()
{
//	AM_DBG lib::logger::get_logger()->trace("qt_transition_blitclass_rect::update(%f)", m_progress);
	ambulant_qt_window *aqw = (ambulant_qt_window *)m_dst->get_gui_window();
	QPixmap *qpm = aqw->ambulant_pixmap();
	QPixmap *npm = aqw->get_ambulant_surface();
	lib::screen_rect<int> newrect_whole = m_newrect;
	newrect_whole.translate(m_dst->get_global_topleft());
	int L = newrect_whole.left(), T = newrect_whole.top(),
        	W = newrect_whole.width(), H = newrect_whole.height();
	bitBlt(qpm, L, T, npm, L, T, W, H);
}

void
qt_transition_blitclass_r1r2r3r4::update()
{
//	AM_DBG lib::logger::get_logger()->trace("qt_transition_blitclass_r1r2r3r4::update(%f)", m_progress);
	ambulant_qt_window *aqw = (ambulant_qt_window *)m_dst->get_gui_window();
	QPixmap *qpm = aqw->ambulant_pixmap();
	QPixmap *npm = aqw->get_ambulant_surface();
	lib::screen_rect<int> oldsrcrect_whole = m_oldsrcrect;
	lib::screen_rect<int> olddstrect_whole = m_olddstrect;
	lib::screen_rect<int> newsrcrect_whole = m_newsrcrect;
	lib::screen_rect<int> newdstrect_whole = m_newdstrect;
	oldsrcrect_whole.translate(m_dst->get_global_topleft());
	olddstrect_whole.translate(m_dst->get_global_topleft());
	newsrcrect_whole.translate(m_dst->get_global_topleft());
	newdstrect_whole.translate(m_dst->get_global_topleft());
	int	Loldsrc = oldsrcrect_whole.left(),
		Toldsrc = oldsrcrect_whole.top(),
		Woldsrc = oldsrcrect_whole.width(),
		Holdsrc = oldsrcrect_whole.height();
//	lib::logger::get_logger()->trace("qt_transition_blitclass_r1r2r3r4: (Loldsrc,Toldsrc,Woldsrc,Holdsrc)=(%d,%d,%d,%d)",Loldsrc,Toldsrc,Woldsrc,Holdsrc);
	int	Lolddst = olddstrect_whole.left(), 
		Tolddst = olddstrect_whole.top(),
		Wolddst = olddstrect_whole.width(),
		Holddst = olddstrect_whole.height();
//	lib::logger::get_logger()->trace("qt_transition_blitclass_r1r2r3r4: (Lolddst,Tolddst,Wolddst,Holddst)=(%d,%d,%d,%d)",Lolddst,Tolddst,Wolddst,Holddst);
	int	Lnewsrc = newsrcrect_whole.left(),
		Tnewsrc = newsrcrect_whole.top(),
		Wnewsrc = newsrcrect_whole.width(),
		Hnewsrc = newsrcrect_whole.height();
//	lib::logger::get_logger()->trace("qt_transition_blitclass_r1r2r3r4: (Lnewsrc,Tnewsrc,Wnewsrc,Hnewsrc)=(%d,%d,%d,%d)",Lnewsrc,Tnewsrc,Wnewsrc,Hnewsrc);
	int	Lnewdst = newdstrect_whole.left(),
		Tnewdst = newdstrect_whole.top(),
		Wnewdst = newdstrect_whole.width(),
		Hnewdst = newdstrect_whole.height();
//	lib::logger::get_logger()->trace("qt_transition_blitclass_r1r2r3r4: (Lnewdst,Tnewdst,Wnewdst,Hnewdst)=(%d,%d,%d,%d)",Lnewdst,Tnewdst,Wnewdst,Hnewdst);
	bitBlt(qpm, Lolddst, Tolddst, qpm, Loldsrc, Toldsrc, Woldsrc, Holdsrc);
	bitBlt(qpm, Lnewdst, Tnewdst, npm, Lnewsrc, Tnewsrc, Wnewsrc, Hnewsrc);
}

void
qt_transition_blitclass_rectlist::update()
{
//	AM_DBG lib::logger::get_logger()->trace("qt_transition_blitclass_rectlist::update(%f)", m_progress);
	ambulant_qt_window *aqw = (ambulant_qt_window *)m_dst->get_gui_window();
	QPixmap *qpm = aqw->ambulant_pixmap();
	QPixmap *npm = aqw->get_ambulant_surface();
	QImage img1 = qpm->convertToImage();
	QImage img2 = npm->convertToImage();
	lib::screen_rect<int> dstrect_whole = m_dst->get_rect();
	dstrect_whole.translate(m_dst->get_global_topleft());
	dstrect_whole.translate(m_dst->get_global_topleft());
	int L = dstrect_whole.left(), T = dstrect_whole.top(),
		W = dstrect_whole.width(), H = dstrect_whole.height();
//	lib::logger::get_logger()->trace("qt_transition_blitclass_rectlist: (L,T,W,H)=(%d,%d,%d,%d)",L,T,W,H);
	QPainter paint;
	QRegion clip_region;
	paint.begin(qpm);
	paint.drawImage(L,T,img1,0,0,W,H);
	std::vector< lib::screen_rect<int> >::iterator newrect;
	for(newrect=m_newrectlist.begin(); newrect != m_newrectlist.end(); newrect++) {
		lib::screen_rect<int> corner_rect = *newrect;
		corner_rect.translate(m_dst->get_global_topleft());
		int L = corner_rect.left(), T = corner_rect.top(),
        		W = corner_rect.width(), H = corner_rect.height();
//		lib::logger::get_logger()->trace("qt_transition_blitclass_rectlist: (L,T,W,H)=(%d,%d,%d,%d)",L,T,W,H);
		QRegion newcorner(L,T,W,H);
		clip_region += newcorner;
	}
	paint.setClipRegion(clip_region);
	paint.drawImage(L,T,img2,0,0,W,H);
	paint.flush();
	paint.end();
}

void
qt_transition_blitclass_poly::update()
{
//	AM_DBG lib::logger::get_logger()->trace("qt_transition_blitclass_poly::update(%f)", m_progress);
	ambulant_qt_window *aqw = (ambulant_qt_window *)m_dst->get_gui_window();
	QPixmap *qpm = aqw->ambulant_pixmap();
	QPixmap *npm = aqw->get_ambulant_surface();
	QImage img1 = qpm->convertToImage();
	QImage img2 = npm->convertToImage();
	std::vector<lib::point>::iterator newpoint;
	QPointArray qpa;
	int idx = 0;
	for( newpoint=m_newpolygon.begin();
	     newpoint != m_newpolygon.end(); newpoint++) {
		lib::point p = *newpoint;
		qpa.putPoints(idx++, 1, p.x, p.y);
	}
	QRegion qreg(qpa, true);
	lib::screen_rect<int> newrect_whole =  m_dst->get_rect();
	newrect_whole.translate(m_dst->get_global_topleft());
	int L = newrect_whole.left(), T = newrect_whole.top(),
        	W = newrect_whole.width(), H = newrect_whole.height();
	QPainter paint;
	paint.begin(qpm);
//	AM_DBG lib::logger::get_logger()->trace("qt_transition_blitclass_poly::update(): ltwh=(%d,%d,%d,%d)",L,T,W,H);
	paint.drawImage(L,T,img1,0,0,W,H);
	paint.setClipRegion(qreg);
	paint.drawImage(L,T,img2,0,0,W,H);
	paint.flush();
	paint.end();
}

void
qt_transition_blitclass_polylist::update()
{
	AM_DBG lib::logger::get_logger()->trace("qt_transition_blitclass_polylist::update(%f)", m_progress);
	lib::logger::get_logger()->trace("qt_transition_blitclass_polylist: not yet tested");
	ambulant_qt_window *aqw = (ambulant_qt_window *)m_dst->get_gui_window();
	QPixmap *qpm = aqw->ambulant_pixmap();
	QPixmap *npm = aqw->get_ambulant_surface();
	QImage img1 = qpm->convertToImage();
	QImage img2 = npm->convertToImage();
	QRegion clip_region;
	  lib::logger::get_logger()->trace("qt_transition_blitclass_polylist: m_newpolygonlist.size()=%d", m_newpolygonlist.size());
	std::vector< std::vector<lib::point> >::iterator partpolygon;
	for (partpolygon=m_newpolygonlist.begin(); 
	     partpolygon!=m_newpolygonlist.end(); partpolygon++) {
	  std::vector<lib::point>::iterator newpoint;
	  lib::logger::get_logger()->trace("qt_transition_blitclass_polylist: partpolygon.size()=%d", partpolygon->size());
	  QPointArray qpa;
	  int idx = 0;
	  for( newpoint=partpolygon->begin();
	       newpoint != partpolygon->end(); newpoint++) {
	    lib::point p = *newpoint;
	    qpa.putPoints(idx++, 1, p.x, p.y);
	  lib::logger::get_logger()->trace("qt_transition_blitclass_polylist: idx=%d, p=(%d,%d)", idx, p.x, p.y);
	  }
	  QRegion qreg(qpa, true);
	  clip_region += qpa;
	}
	lib::screen_rect<int> newrect_whole =  m_dst->get_rect();
	newrect_whole.translate(m_dst->get_global_topleft());
	int L = newrect_whole.left(), T = newrect_whole.top(),
        	W = newrect_whole.width(), H = newrect_whole.height();
	QPainter paint;
	paint.begin(qpm);
	paint.drawImage(L,T,img1,0,0,W,H);
	paint.setClipRegion(clip_region);
	paint.drawImage(L,T,img2,0,0,W,H);
	paint.flush();
	paint.end();
}

smil2::transition_engine *
qt_transition_engine(common::surface *dst, bool is_outtrans, lib::transition_info *info)
{
	smil2::transition_engine *rv;
	
	switch(info->m_type) {
	// Series 1: edge wipes
	case lib::barWipe:
		rv = new qt_transition_engine_barwipe();
		break;
	case lib::boxWipe:
		rv = new qt_transition_engine_boxwipe();
		break;
	case lib::fourBoxWipe:
		rv = new qt_transition_engine_fourboxwipe();
		break;
	case lib::barnDoorWipe:
		rv = new qt_transition_engine_barndoorwipe();
		break;
	case lib::diagonalWipe:
		rv = new qt_transition_engine_diagonalwipe();
		break;
	case lib::miscDiagonalWipe:
		rv = new qt_transition_engine_miscdiagonalwipe();
		break;
	case lib::veeWipe:
		rv = new qt_transition_engine_veewipe();
		break;
	case lib::barnVeeWipe:
		rv = new qt_transition_engine_barnveewipe();
		break;
	case lib::zigZagWipe:
		rv = new qt_transition_engine_zigzagwipe();
		break;
	case lib::barnZigZagWipe:
		rv = new qt_transition_engine_barnzigzagwipe();
		break;
	case lib::bowTieWipe:
		rv = new qt_transition_engine_bowtiewipe();
		break;
	// series 2: iris wipes
	case lib::irisWipe:
		rv = new qt_transition_engine_iriswipe();
		break;
	case lib::pentagonWipe:
		rv = new qt_transition_engine_pentagonwipe();
		break;
	case lib::arrowHeadWipe:
		rv = new qt_transition_engine_arrowheadwipe();
		break;
	case lib::triangleWipe:
		rv = new qt_transition_engine_trianglewipe();
		break;
	case lib::hexagonWipe:
		rv = new qt_transition_engine_hexagonwipe();
		break;
	case lib::eyeWipe:
		rv = new qt_transition_engine_eyewipe();
		break;
	case lib::roundRectWipe:
		rv = new qt_transition_engine_roundrectwipe();
		break;
	case lib::ellipseWipe:
		rv = new qt_transition_engine_ellipsewipe();
		break;
	case lib::starWipe:
		rv = new qt_transition_engine_starwipe();
		break;
	case lib::miscShapeWipe:
		rv = new qt_transition_engine_miscshapewipe();
		break;
	// series 3: clock-type wipes
	case lib::clockWipe:
		rv = new qt_transition_engine_clockwipe();
		break;
	case lib::singleSweepWipe:
		rv = new qt_transition_engine_singlesweepwipe();
		break;
	case lib::doubleSweepWipe:
		rv = new qt_transition_engine_doublesweepwipe();
		break;
	case lib::saloonDoorWipe:
		rv = new qt_transition_engine_saloondoorwipe();
		break;
	case lib::windshieldWipe:
		rv = new qt_transition_engine_windshieldwipe();
		break;
	case lib::fanWipe:
		rv = new qt_transition_engine_fanwipe();
		break;
	case lib::doubleFanWipe:
		rv = new qt_transition_engine_doublefanwipe();
		break;
	case lib::pinWheelWipe:
		rv = new qt_transition_engine_pinwheelwipe();
		break;
	// series 4: matrix wipe types
	case lib::snakeWipe:
		rv = new qt_transition_engine_snakewipe();
		break;
	case lib::waterfallWipe:
		rv = new qt_transition_engine_waterfallwipe();
		break;
	case lib::spiralWipe:
		rv = new qt_transition_engine_spiralwipe();
		break;
	case lib::parallelSnakesWipe:
		rv = new qt_transition_engine_parallelsnakeswipe();
		break;
	case lib::boxSnakesWipe:
		rv = new qt_transition_engine_boxsnakeswipe();
		break;
	// series 5: SMIL-specific types
	case lib::pushWipe:
		rv = new qt_transition_engine_pushwipe();
		break;
	case lib::slideWipe:
		rv = new qt_transition_engine_slidewipe();
		break;
	case lib::fade:
		rv = new qt_transition_engine_fade();
		break;
	default:
		lib::logger::get_logger()->warn("qt_transition_engine: transition type %s not yet implemented",
			repr(info->m_type).c_str());
		rv = NULL;
	}
	if (rv)
		rv->init(dst, is_outtrans, info);
	return rv;
}

} // namespace qt

} // namespace gui

} //namespace ambulant

