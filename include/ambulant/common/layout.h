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

#ifndef AMBULANT_LIB_LAYOUT_H
#define AMBULANT_LIB_LAYOUT_H

#include "ambulant/lib/gtypes.h"

namespace ambulant {

namespace lib {

class node;
class passive_window;

// abstract_rendering_source is an pure virtual baseclass for renderers that
// render to a region (as opposed to audio renderers, etc) and for subregions
// themselves. It is used to commmunicate redraw requests (and, eventually,
// other things like mouse clicks) from the GUI window all the way down to
// the renderer.
class abstract_rendering_source {
  public:
	virtual void redraw(const screen_rect<int> &dirty, passive_window *window) = 0;
};

// abstract_rendering_surface is a pure virtual baseclass for a region of screenspace.
// It is the only interface that renderers use when talking to regions, and regions
// use when talking to their parent regions.
class abstract_rendering_surface {
  public:
	virtual ~abstract_rendering_surface() {};
	
	virtual void show(abstract_rendering_source *renderer) = 0;
	virtual void renderer_done() = 0;

	virtual void need_redraw(const screen_rect<int> &r) = 0;
	virtual void need_redraw() = 0;

	virtual const screen_rect<int>& get_rect() const = 0;
	virtual const point &get_global_topleft() const = 0;
};

class layout_manager {
  public:
	virtual ~layout_manager() {};
	
	virtual abstract_rendering_surface *get_rendering_surface(const node *node) = 0;
};
	
} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_LAYOUT_H
