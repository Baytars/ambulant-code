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

#include "ambulant/gui/cocoa/cocoa_transition.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/lib/logger.h"

#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

namespace gui {

namespace cocoa {

cocoa_transition_engine::cocoa_transition_engine(common::surface *dst, bool outtrans, lib::transition_info *info)
:   smil2::transition_engine(outtrans, info),
	m_dst(dst)
{
	AM_DBG lib::logger::get_logger()->trace("cocoa_transition_engine::cocoa_transition_engine()");
}

cocoa_transition_engine::~cocoa_transition_engine()
{
	AM_DBG lib::logger::get_logger()->trace("cocoa_transition_engine::~cocoa_transition_engine()");
}

void
cocoa_transition_engine::update()
{
	cocoa_window *window = (cocoa_window *)m_dst->get_abstract_window();
	AmbulantView *view = (AmbulantView *)window->view();
#ifdef COPYOLDSOURCE
	NSImage *oldsrc = [view getTransitionOldSource];
#endif
	NSImage *newsrc = [view getTransitionNewSource];
	/*AM_DBG*/ lib::logger::get_logger()->trace("cocoa_transition_engine::update(%f)", m_progress);
	// This is a fade:
	const lib::screen_rect<int> &r =  m_dst->get_rect();
	lib::screen_rect<int> dstrect_whole = r;
	dstrect_whole.translate(m_dst->get_global_topleft());
	NSRect cocoa_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
//	NSPoint cocoa_dstpoint = NSMakePoint(NSMinX(cocoa_dstrect_whole), NSMinY(cocoa_dstrect_whole));

#ifdef COPYOLDSOURCE
	// Debug: fill with purple
	[[NSColor purpleColor] set];
	NSRectFill(cocoa_dstrect_whole);

	[oldsrc drawInRect: cocoa_dstrect_whole 
		fromRect: cocoa_dstrect_whole
		operation: NSCompositeCopy
		fraction: 1.0];
#endif
	[newsrc drawInRect: cocoa_dstrect_whole 
		fromRect: cocoa_dstrect_whole
		operation: NSCompositeSourceOver
		fraction: m_progress];
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant

