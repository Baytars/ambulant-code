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

#ifndef AMBULANT_GUI_DG_IMAGE_RENDERER_H
#define AMBULANT_GUI_DG_IMAGE_RENDERER_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#include "ambulant/config/config.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/colors.h"
#include "ambulant/net/url.h"

#include "ambulant/gui/dg/dg_surface.h"
#include "ambulant/gui/dg/dg_dib_surface.h"

#include <string>

namespace ambulant {

namespace gui {

namespace dg {

class viewport;

class image_renderer {
  public:
	image_renderer(const net::url& u, viewport* v);
	~image_renderer();
	
	bool can_play() const { return m_dibsurf != 0;}
	bool is_transparent() const { return m_transparent;}
	const lib::size& get_size() const { return m_size;}
	dib_surface_t *get_dibsurf() { return m_dibsurf;}
	lib::color_t get_transp_color() const { return m_transp_color;}
	
  private:
	void open(const net::url& u, viewport* v);
	net::url m_url;
	dib_surface_t *m_dibsurf;
	lib::size m_size;
	bool m_transparent;
	lib::color_t m_transp_color;
};

} // namespace dg

} // namespace gui

} // namespace ambulant 

#endif // AMBULANT_GUI_DG_IMAGE_RENDERER_H
