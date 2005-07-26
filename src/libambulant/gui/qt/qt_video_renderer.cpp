/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

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


#include "ambulant/gui/qt/qt_factory.h"
#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_renderer.h"
#include "ambulant/gui/qt/qt_video_renderer.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/factory.h"
#include <stdlib.h>
#include "ambulant/common/playable.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace gui::qt;



qt_video_renderer::qt_video_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
    	common::factories *factory)
:	 common::video_renderer(context, cookie, node, evp, factory),
 	//m_image(NULL),
  	//m_data(NULL),
	m_img_displayed(0)
{
#if 0
    // This is ridiculous! how could something be in m_frames?
	while ( m_frames.size() > 0 ) {
		std::pair<int, char*> element = m_frames.front();
		//free(element.second);
		m_frames.pop();
	}
#endif
    assert(m_frames.size() == 0);
	
}

qt_video_renderer::~qt_video_renderer()
{
}

void 
qt_video_renderer::show_frame(const char* frame, int size)
{
	m_lock.enter();
	
	AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.show_frame: frame=0x%x, size=%d, this=0x%x", (void*) frame, size, (void*) this);
    char* data = NULL;
	
    data = (char*) malloc(size);
    if (data) {
        AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.show_frame: allocated m_data=0x%x, size=%d", data, size);
        
    }

	//XXX this seems to work but framedroping shouldn't be nessecery here !
	// so i gues it is somesort of wrong 
	if (data && frame) {
		if (m_frames.size() < 2) {
			if (memcpy(data, frame, size)) {
				std::pair<int, char*> element(size, data);
				m_frames.push(element);
				AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.show_frame: m_data(0x%x) stored !", (void*) element.second);
			}
		} else {
			free(data);
			data = NULL;
		}
	} else {
		lib::logger::get_logger()->debug("qt_video_renderer.show_frame: m_data is NULL or frame is NULL!");
		if (data) {
			free(data);
			data = NULL;
		}			
	}

	//if (m_image) {
	//	delete m_image;
	//	m_image = NULL;
	//}

	//~ if (m_data ) {
		//~ int width = m_size.w;
		//~ int height = m_size.h;
		//~ AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.show_frame(0x%x): width = %d, height = %d",(void *)this, width, height);

		//~ m_image = new QImage((uchar*) m_data, width, height, 32, NULL, 0, QImage::IgnoreEndian);
	//~ } else {
		//~ AM_DBG lib::logger::get_logger()->debug("qt_video_renderer, m_data=0x%x (this=0x%x)",(void*) m_data, (void *)this);
	//~ }
	if (m_dest) {
		AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.show_frame: About to calll need_redraw, (m_dest=0x%x)", (void*) m_dest);
		m_dest->need_redraw();	
		AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.show_frame: need_redraw called");
	} else {
		lib::logger::get_logger()->error("qt_video_renderer.show_frame: m_dest is NULL !");
	}
	AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.show_frame done");

	m_lock.leave();
}



void
qt_video_renderer::redraw(const lib::rect &dirty, common::gui_window* w) 
{
	char *data=NULL;
	if (m_frames.size() > 1) {
		std::pair<int, char*> element = m_frames.front();
		data = element.second;
		free(data);
		data = NULL;
		m_frames.pop();
	}	
	
	if (m_frames.size() > 0) {
		//m_lock.enter();
		AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.redraw(0x%x)",(void*) this);

		//char* data = NULL;
		QImage* image = NULL;
		const lib::point p = m_dest->get_global_topleft();
		const lib::rect &r = m_dest->get_rect();
	
		// XXXX WRONG! This is the info for the region, not for the node!
		const common::region_info *info = m_dest->get_info();
		AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.redraw: info=0x%x", info);
		ambulant_qt_window* aqw = (ambulant_qt_window*) w;
		QPainter paint;
		paint.begin(aqw->get_ambulant_pixmap());
		// background drawing

		if (info && !info->get_transparent()) {
		// First find our whole area (which we have to clear to 
		// background color)
			lib::rect dstrect_whole = r;
			dstrect_whole.translate(m_dest->get_global_topleft());
			int L = dstrect_whole.left(),
		    T = dstrect_whole.top(),
		    W = dstrect_whole.width(),
		    H = dstrect_whole.height();
		// XXXX Fill with background color
			lib::color_t bgcolor = info->get_bgcolor();
			AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.redraw: clearing to 0x%x", (long)bgcolor);
			QColor bgc = QColor(lib::redc(bgcolor), lib::greenc(bgcolor), lib::bluec(bgcolor));
			paint.setBrush(bgc);
			paint.drawRect(L,T,W,H);
		}
	
	
		
		//data = NULL;
	
		if (m_frames.size() > 0 ) {
			std::pair<int, char*> element = m_frames.front();
			data = element.second;
			AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.redraw, data(0x%x) retrieved (this=0x%x) (still %d frames)",(void*) data, (void *)this, m_frames.size());
		} else {
			data = NULL;
		}

		if (data ) {
			int width = m_size.w;
			int height = m_size.h;
			AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.show_frame(0x%x): width = %d, height = %d",(void *)this, width, height);
			image = new QImage((uchar*) data, width, height, 32, NULL, 0, QImage::IgnoreEndian);
		} else {
			AM_DBG lib::logger::get_logger()->debug("qt_video_renderer, m_data=0x%x (this=0x%x)",(void*) data, (void *)this);
		}
		if (image) {
			QSize qsize = image->size();
			lib::size srcsize = lib::size(qsize.width(), qsize.height());
			lib::rect srcrect = lib::rect(lib::size(0,0));
			lib::rect dstrect = m_dest->get_fit_rect(srcsize, &srcrect, m_alignment);
			dstrect.translate(m_dest->get_global_topleft());
			int L = dstrect.left(), 
		    	T = dstrect.top(),
		    	W = dstrect.width(),
		    	H = dstrect.height();
			AM_DBG lib::logger::get_logger()->debug(" qt_video_renderer.redraw(0x%x): drawImage at (L=%d,T=%d,W=%d,H=%d)", (void *)this,L,T,W,H);
			paint.drawImage(L,T,*image,0,0,W,H);
		} else {
	//		AM_DBG lib::logger::get_logger()->error("qt_video_renderer.redraw(0x%x): no m_image", (void *) this);
			AM_DBG lib::logger::get_logger()->debug("qt_video_renderer.redraw(0x%x): no m_image", (void *) this);
		}
		paint.flush();
		paint.end();
	
		if(image) {
			delete( image );
			image = NULL;
		}
	
		//~ if (data) {
			//~ free(data);
			//~ data = NULL;
		//~ }
		//~ m_frames.pop();
	//m_lock.leave();
	}
}
