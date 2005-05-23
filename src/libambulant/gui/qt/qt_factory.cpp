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
 
//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
 
#include "ambulant/gui/qt/qt_factory.h"
#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_image_renderer.h"
#include "ambulant/gui/qt/qt_text_renderer.h"
#include "ambulant/gui/qt/qt_video_renderer.h"


using namespace ambulant;
using namespace gui::qt;
using namespace net;

qt_video_factory::~qt_video_factory()
{
}

qt_renderer_factory::qt_renderer_factory(common::factories *factory)
:	m_factory(factory)
{
	AM_DBG lib::logger::get_logger()->debug("qt_renderer factory (0x%x)", (void*) this);
}
	
qt_window_factory::qt_window_factory( QWidget* parent_widget, int x, int y)
:	m_parent_widget(parent_widget), m_p(lib::point(x,y)) 
{
	AM_DBG lib::logger::get_logger()->debug("qt_window_factory (0x%x)", (void*) this);
}	
  
ambulant_qt_window::ambulant_qt_window(const std::string &name,
	   lib::screen_rect<int>* bounds,
	   common::gui_events *region)
:	common::gui_window(region),
	m_ambulant_widget(NULL),
	m_pixmap(NULL),
	m_oldpixmap(NULL),
	m_tmppixmap(NULL),
#ifdef USE_SMIL21
	m_fullscreen_count(0),
	m_fullscreen_prev_pixmap(NULL),
	m_fullscreen_old_pixmap(NULL),
	m_fullscreen_engine(NULL),
	m_fullscreen_now(0),
#endif
	m_surface(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::ambulant_qt_window(0x%x)",(void *)this);
}

ambulant_qt_window::~ambulant_qt_window()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::~ambulant_qt_window(0x%x): m_ambulant_widget=0x%x, m_pixmap=0x%x",this,m_ambulant_widget, m_pixmap);
	// Note that we don't destroy the widget, only sver the connection.
	// the widget itself is destroyed independently.
	if (m_ambulant_widget ) {
		m_ambulant_widget->set_qt_window(NULL);
		delete m_ambulant_widget;
		m_ambulant_widget = NULL;
		delete m_pixmap;
		m_pixmap = NULL;
		if (m_tmppixmap != NULL) {
			delete m_tmppixmap;
			m_tmppixmap = NULL;
		}
	}
}
	
void
ambulant_qt_window::set_ambulant_widget(qt_ambulant_widget* qaw)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::set_ambulant_widget(0x%x)",(void *)qaw);
	// Don't destroy!
	//if (m_ambulant_widget != NULL)
	//	delete m_ambulant_widget;
	m_ambulant_widget = qaw;
#ifdef	AM_DBG
	// in debugging mode, initialize with purple background
	if (qaw != NULL) {
		QSize size = qaw->frameSize();
		m_pixmap = new QPixmap(size.width(), size.height());
		QPainter paint(m_pixmap);
		QColor bgc = QColor(255,0,255);
		paint.setBrush(bgc);
		paint.drawRect(0,0,size.width(),size.height());
		paint.flush();
		paint.end();
		
	}
#endif/*AM_DBG*/
}

QPixmap*
ambulant_qt_window::get_ambulant_pixmap()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::ambulant_pixmap(0x%x) = 0x%x",(void *)this,(void *)m_pixmap);
	return m_pixmap;
}

QPixmap*
ambulant_qt_window::get_pixmap_from_screen(const lib::screen_rect<int> &r)
{
	QPixmap *rv = new QPixmap(r.width(), r.height());
	bitBlt(rv, 0, 0, m_pixmap, r.left(), r.top(), r.width(), r.height());
	return rv;
}

qt_ambulant_widget*
ambulant_qt_window::get_ambulant_widget()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::get_ambulant_widget(0x%x)",(void *)m_ambulant_widget);
	return m_ambulant_widget;
}

QPixmap*
ambulant_qt_window::new_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::new_ambulant_surface(0x%x)",(void *)m_surface);
	if (m_surface != NULL) delete m_surface;
	QSize size = m_pixmap->size();
	m_surface = new QPixmap(size.width(), size.height());
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::new_ambulant_surface(0x%x)",(void *)m_surface);
        return m_surface;
}

QPixmap*
ambulant_qt_window::get_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::get_ambulant_surface(0x%x) = 0x%x",(void *)this,(void *)m_surface);
        return m_surface;
}

QPixmap*
ambulant_qt_window::get_ambulant_oldpixmap()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::get_ambulant_oldpixmap(0x%x) = 0x%x",(void *)this,(void *)m_oldpixmap);
#ifdef USE_SMIL21
	if (m_fullscreen_count && m_fullscreen_old_pixmap)
		return m_fullscreen_old_pixmap;
#endif
        return m_oldpixmap;
}

void
ambulant_qt_window::reset_ambulant_surface(void)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::reset_ambulant_surface(0x%x) m_oldpixmap = 0x%x",(void *)this,(void *)m_oldpixmap);
	if (m_oldpixmap != NULL) m_pixmap = m_oldpixmap;
}

void
ambulant_qt_window::set_ambulant_surface(QPixmap* surf)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::set_ambulant_surface(0x%x) surf = 0x%x",(void *)this,(void *)surf);
	m_oldpixmap = m_pixmap;
	if (surf != NULL) m_pixmap = surf;
}

void
ambulant_qt_window::delete_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::delete_ambulant_surface(0x%x) m_surface = 0x%x",(void *)this, (void *)m_surface);
	delete m_surface;
	m_surface = NULL;
}

void
ambulant_qt_window::need_redraw(const lib::screen_rect<int> &r)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::need_redraw(0x%x): ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	if (m_ambulant_widget == NULL) {
		lib::logger::get_logger()->error("ambulant_qt_window::need_redraw(0x%x): m_ambulant_widget == NULL !!!", (void*) this);
		return;
	}
#ifdef	QT_NO_FILEDIALOG	/* Assume embedded Qt */
	m_ambulant_widget->repaint(r.left(), r.top(), r.width(), r.height(), false);
	qApp->wakeUpGuiThread();
//	qApp->processEvents();
#else	/*QT_NO_FILEDIALOG*/	/* Assume plain Qt */
	m_ambulant_widget->update(r.left(), r.top(), r.width(), r.height());
	qApp->wakeUpGuiThread();
#endif	/*QT_NO_FILEDIALOG*/
}

void
ambulant_qt_window::redraw_now()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::redraw_now()");
	m_ambulant_widget->repaint(false);
}

void
ambulant_qt_window::mouse_region_changed()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::mouse_region_changed needs to be implemented");
}

/* test if there is something new to see */
static QImage* oldImageP;
static bool isEqualToPrevious(QPixmap* qpmP) {
	return false;
	QImage img = qpmP->convertToImage();
	if (oldImageP != NULL && img == *oldImageP) {
		AM_DBG lib::logger::get_logger()->debug("isEqualToPrevious: new image not different from old one");
		return true;
	} else {
		if (oldImageP != NULL) delete oldImageP;
		oldImageP = new QImage(img);
		return false;
	}
}

#ifdef DUMPPIXMAP
// doesn't compile on Zaurus
/**/
/* dumpPixmap on file */
void gui::qt::dumpPixmap(QPixmap* qpm, std::string filename) {
	if ( ! qpm) return;
	QImage img = qpm->convertToImage();
	if ( ! isEqualToPrevious(qpm)) {
		static int i;
		char buf[5];
		sprintf(buf,"%04d",i++);
		std::string newfile = buf + std::string(filename) +".png";
		qpm->save(newfile, "PNG");
		AM_DBG lib::logger::get_logger()->debug("dumpPixmap(%s)", newfile.c_str());
	}
}
/**/
#endif

void
ambulant_qt_window::redraw(const lib::screen_rect<int> &r)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::redraw(0x%x): ltrb=(%d,%d,%d,%d)",(void *)this, r.left(), r.top(), r.right(), r.bottom());
#ifdef USE_SMIL21
	_screenTransitionPreRedraw();
#endif
	m_handler->redraw(r, this);
//XXXX	if ( ! isEqualToPrevious(m_pixmap))
#ifdef USE_SMIL21
	_screenTransitionPostRedraw(r);
#endif
	bitBlt(m_ambulant_widget,r.left(),r.top(), m_pixmap,r.left(),r.top(), r.right(),r.bottom());
//XXXX	dumpPixmap(m_pixmap, "top"); //AM_DBG 
}

void
ambulant_qt_window::user_event(const lib::point &where, int what) 
{
        AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::user_event(0x%x): point=(%d,%d)", this, where.x, where.y);
	m_handler->user_event(where, what);
}

void
ambulant_qt_window::need_events(bool want) 
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::need_events(0x%x): want=%d", this, want);
}

// XXXX
qt_ambulant_widget::qt_ambulant_widget(const std::string &name,
	lib::screen_rect<int>* bounds,
	QWidget* parent_widget)
:	QWidget(parent_widget,"qt_ambulant_widget",0),
	m_qt_window(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("qt_ambulant_widget::qt_ambulant_widget(0x%x-0x%x(%d,%d,%d,%d))",
		(void *)this,
		(void*)  parent_widget,
		bounds->left(),
		bounds->top(),
		bounds->right(),
		bounds->bottom());
	setGeometry(bounds->left(), bounds->top(), bounds->right(), bounds->bottom());
#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
	setMouseTracking(true); // enable mouseMoveEvent() to be called
#endif/*QT_NO_FILEDIALOG*/
}

qt_ambulant_widget::~qt_ambulant_widget()
{
	AM_DBG lib::logger::get_logger()->debug("qt_ambulant_widget::~qt_ambulant_widget(0x%x): m_qt_window=0x%x", (void*)this, m_qt_window);
	if (m_qt_window) {
		m_qt_window->set_ambulant_widget(NULL);
		m_qt_window = NULL;
	}
}
	
void
qt_ambulant_widget::paintEvent(QPaintEvent* e)
{
	AM_DBG lib::logger::get_logger()->debug("qt_ambulant_widget::paintEvent(0x%x): e=0x%x)", (void*) this, (void*) e);
	QRect qr = e->rect();
	lib::screen_rect<int> r =  lib::screen_rect<int>(
		lib::point(qr.left(),qr.top()),
		lib::point(qr.right(),qr.bottom()));
	if (m_qt_window == NULL) {
		lib::logger::get_logger()->debug("qt_ambulant_widget::paintEvent(0x%x): e=0x%x m_qt_window==NULL",
			(void*) this, (void*) e);
		return;
	}
	m_qt_window->redraw(r);
}

void
qt_ambulant_widget::mouseReleaseEvent(QMouseEvent* e) {
	AM_DBG lib::logger::get_logger()->debug("qt_ambulant_widget::mouseReleaseEvxent(0x%x): e=0x%x, position=(%d, %d))",
		(void*) this, (void*) e, e->x(), e->y());
	if (m_qt_window == NULL) {
		lib::logger::get_logger()->debug("qt_ambulant_widget::mouseReleaseEvent(0x%x): e=0x%x  position=(%d, %d) m_qt_window==NULL",
			(void*) this, (void*) e, e->x(), e->y());
		return;
	}
	lib::point amwhere = lib::point(e->x(), e->y());
	m_qt_window->user_event(amwhere);
}

#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
void 
qt_ambulant_widget::mouseMoveEvent(QMouseEvent* e) {
	int m_o_x = 0, m_o_y = 0; //27; // XXXX Origin of MainWidget
	AM_DBG lib::logger::get_logger()->debug("%s:(%d,%d)\n",
	       "qt_ambulant_widget::mouseMoveEvent", e->x(),e->y());
	ambulant::lib::point ap = ambulant::lib::point(e->x()-m_o_x,
						       e->y()-m_o_y);
	m_qt_window->user_event(ap, 1);
	qApp->mainWidget()->unsetCursor(); //XXXX
}
#endif/*QT_NO_FILEDIALOG*/

void 
qt_ambulant_widget::set_qt_window( ambulant_qt_window* aqw)
{
	// Note: the window and widget are destucted independently.
	//	if (m_qt_window != NULL)
	//	  delete m_qt_window;
	m_qt_window = aqw;
	AM_DBG lib::logger::get_logger()->debug("qt_ambulant_widget::set_qt_window(0x%x): m_qt_window==0x%x)",
		(void*) this, (void*) m_qt_window);
}

ambulant_qt_window* 
qt_ambulant_widget::qt_window() {
	return m_qt_window;
}

// XXXX
common::playable *
qt_renderer_factory::new_playable(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp) {

	lib::xml_string tag = node->get_qname().second;
	common::playable* rv;
	if (tag == "img") {
 		rv = new qt_active_image_renderer(context, cookie, node,
						  evp, m_factory);
		AM_DBG lib::logger::get_logger()->debug("qt_renderer_factory: node 0x%x: returning qt_active_image_renderer 0x%x", 
			(void*) node, (void*) rv);
	} else if (tag == "brush") {
 		rv = new qt_fill_renderer(context, cookie, node,
					  evp, m_factory);
		AM_DBG lib::logger::get_logger()->debug("qt_renderer_factory: node 0x%x: returning qt_active_fill_renderer 0x%x", 
			(void*) node, (void*) rv);
	} else if ( tag == "text") {
		rv = new qt_active_text_renderer(context, cookie, node,
						 evp, m_factory);
		AM_DBG lib::logger::get_logger()->debug("qt_renderer_factory: node 0x%x: returning qt_active_text_renderer 0x%x",
			(void*) node, (void*) rv);
	} else {
		return NULL;
	}
    return rv;
}
  
common::gui_window *
qt_window_factory::new_window (const std::string &name,
			       lib::size bounds,
			       common::gui_events *region)
{
	lib::screen_rect<int>* r = new lib::screen_rect<int>(m_p, bounds);
	AM_DBG lib::logger::get_logger()->debug("qt_window_factory::new_window (0x%x): name=%s %d,%d,%d,%d",
		(void*) this, name.c_str(), r->left(),r->top(),r->right(),r->bottom());
 	ambulant_qt_window * aqw = new ambulant_qt_window(name, r, region);
	qt_ambulant_widget * qaw = new qt_ambulant_widget(name, r, m_parent_widget);
#ifndef	QT_NO_FILEDIALOG     /* Assume plain Qt */
	qaw->setBackgroundMode(Qt::NoBackground);
	if (qApp == NULL || qApp->mainWidget() == NULL) {
		lib::logger::get_logger()->error("qt_window_factory::new_window (0x%x) %s",
			(void*) this,
	   		"qApp == NULL || qApp->mainWidget() == NULL");
	}
	qApp->mainWidget()->resize(bounds.w + m_p.x, bounds.h + m_p.y);
#else	/*QT_NO_FILEDIALOG*/  /* Assume embedded Qt */
	qaw->setBackgroundMode(QWidget::NoBackground);
	/* No resize implemented for embedded Qt */
#endif	/*QT_NO_FILEDIALOG*/
	aqw->set_ambulant_widget(qaw);
	qaw->set_qt_window(aqw);
 	AM_DBG lib::logger::get_logger()->debug("qt_window_factory::new_window(0x%x): ambulant_widget=0x%x qt_window=0x%x",
		(void*) this, (void*) qaw, (void*) aqw);
	qaw->show();
	return aqw;
}

common::bgrenderer *
qt_window_factory::new_background_renderer(const common::region_info 
					   *src)
{
	AM_DBG lib::logger::get_logger()->debug("qt_window_factory::new_background_renderer(0x%x): src=0x%x",
		(void*) this, src);
	return new qt_background_renderer(src);
}

common::playable *
qt_video_factory::new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp)
{
	common::playable *rv;
	
	lib::xml_string tag = node->get_qname().second;
    AM_DBG lib::logger::get_logger()->debug("qt_video_factory: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "video") {
	  rv = new qt_active_video_renderer(context, cookie, node, evp, m_factory);
		AM_DBG lib::logger::get_logger()->debug("qt_video_factory: node 0x%x: returning qt_video_renderer 0x%x", (void *)node, (void *)rv);
	} else {
		AM_DBG lib::logger::get_logger()->debug("qt_video_factory: no renderer for tag \"%s\"", tag.c_str());
		return NULL;
	}
	return rv;
}
#ifdef USE_SMIL21

void 
ambulant_qt_window::startScreenTransition()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::startScreenTransition()");
	if (m_fullscreen_count)
		logger::get_logger()->warn("ambulant_qt_window::startScreenTransition():multiple Screen transitions in progress (m_fullscreen_count=%d)",m_fullscreen_count);
	m_fullscreen_count++;
	if (m_fullscreen_old_pixmap) delete m_fullscreen_old_pixmap;
	m_fullscreen_old_pixmap = m_fullscreen_prev_pixmap;
	m_fullscreen_prev_pixmap = NULL;
}

void 
ambulant_qt_window::endScreenTransition()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::endScreenTransition()");
	assert(m_fullscreen_count > 0);
	m_fullscreen_count--;
}

void 
ambulant_qt_window::screenTransitionStep(smil2::transition_engine* engine, lib::transition_info::time_type now)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::screenTransitionStep()");
	assert(m_fullscreen_count > 0);
	m_fullscreen_engine = engine;
	m_fullscreen_now = now;
}
		
void 
ambulant_qt_window::_screenTransitionPreRedraw()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::_screenTransitionPreRedraw()");
	if (m_fullscreen_count == 0) return;
	// XXX setup drawing to transition surface
//	[[self getTransitionSurface] lockFocus];
}

void 
ambulant_qt_window::_screenTransitionPostRedraw(const lib::screen_rect<int> &r)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::_screenTransitionPostRedraw()");
	if (m_fullscreen_count == 0 && m_fullscreen_old_pixmap == NULL) {
		// Neither in fullscreen transition nor wrapping one up.
		// Take a snapshot of the screen and return.
		AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::_screenTransitionPostRedraw: screen snapshot");
		if (m_fullscreen_prev_pixmap) delete m_fullscreen_prev_pixmap;
		m_fullscreen_prev_pixmap = get_pixmap_from_screen(r); // XXX wrong
//		dumpPixmap(m_fullscreen_prev_pixmap, "snap");
		return;
	}
	if (m_fullscreen_old_pixmap == NULL) {
		// Just starting a new fullscreen transition. Get the
		// background bits from the snapshot saved during the previous
		// redraw.
		m_fullscreen_old_pixmap = m_fullscreen_prev_pixmap;
		m_fullscreen_prev_pixmap = NULL;
	}
	
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::_screenTransitionPostRedraw: bitblit");
	if (m_fullscreen_engine) {
		// Do the transition step
		QPixmap* new_src = get_ambulant_surface();
		if ( ! new_src) new_src = new_ambulant_surface();
		bitBlt(m_surface, 0, 0, m_pixmap);
		bitBlt(m_pixmap, 0, 0, m_fullscreen_old_pixmap);
//		dumpPixmap(new_src, "fnew");
//		dumpPixmap(m_pixmap, "fold");
		m_fullscreen_engine->step(m_fullscreen_now);
//		dumpPixmap(m_pixmap, "fres");
	}

	if (m_fullscreen_count == 0) {
		// Finishing a fullscreen transition.
		AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::_screenTransitionPostRedraw: cleanup after transition done");
		if (m_fullscreen_old_pixmap) delete m_fullscreen_old_pixmap;
		m_fullscreen_old_pixmap = NULL;
		m_fullscreen_engine = NULL;
	}
}
#endif
