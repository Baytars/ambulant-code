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

#include "ambulant/gui/qt/qt_gui.h"
#include "ambulant/gui/qt/qt_renderer.h"
#include "ambulant/gui/qt/qt_image_renderer.h"
#include "ambulant/gui/qt/qt_text_renderer.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {
  
using namespace lib;
  
namespace gui {
namespace qt_renderer {
  
  void
  qt_passive_window::need_redraw(const screen_rect<int> &r)
  {
    AM_DBG logger::get_logger()->trace
      ("qt_passive_window::need_redraw(0x%x), "
       "ltrb=(%d,%d,%d,%d)",
       (void *)this, r.left(), r.top(), r.right(), r.bottom());
    view()->repaint(r.left(), r.top(), 
		    r.width(), r.height(),
		    true);
  }
    active_renderer *
  qt_renderer_factory::new_renderer(event_processor *const evp,
				    net::passive_datasource *src,
				    passive_region *const dest,
				    const node *node)
  {
    xml_string tag = node->get_qname().second;
    active_renderer* rv;
    if (tag == "img") {
      rv = (active_renderer*) 
	new qt_active_image_renderer(evp, src, dest, node);
      AM_DBG logger::get_logger()->trace
	("qt_renderer_factory: node 0x%x: "
	 "returning qt_active_image_renderer 0x%x", 
	 (void*) node, (void*) rv);
    } else if ( tag == "text") {
      rv = (active_renderer*)
	new qt_active_text_renderer(evp, src, dest, node);
      AM_DBG logger::get_logger()->trace
	("qt_renderer_factory: node 0x%x: "
	 "returning qt_active_text_renderer 0x%x",
	 (void*) node, (void*) rv);
    } else {
      AM_DBG logger::get_logger()->error("qt_renderer_factory: "
				  "no Qt renderer for tag \"%s\"",
				  tag.c_str());
      rv = new gui::none::none_active_renderer(evp, src, dest, node);
      AM_DBG logger::get_logger()->trace
	("qt_renderer_factory: node 0x%x: "
	 "returning none_active_renderer 0x%x",
	 (void*) node, (void*) rv);
    }
    return rv;
  }    
  passive_window *
  qt_window_factory::new_window(const std::string &name, size bounds)
  {
    AM_DBG logger::get_logger()->trace
      ("qt_window_factory::new_window (0x%x) name=%s", 
       (void*) this, name.c_str());
      qt_passive_window * qpw = new qt_passive_window(name, bounds, m_view);
      m_view->set_ambulant_window((void*)qpw);
      return qpw;
  }
  
  
} // namespace qt_renderer

} // namespace gui

} //namespace ambulant
