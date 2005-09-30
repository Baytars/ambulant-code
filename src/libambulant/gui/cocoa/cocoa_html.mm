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

#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/gui/cocoa/cocoa_html.h"
//#include "ambulant/gui/cocoa/cocoa_transition.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/region.h" // TMP!
#include <WebKit/WebKit.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

static surface_impl::renderer_id my_renderer_id = (surface_impl::renderer_id)"cocoa_html_browser";

// Helper routine - Get a WebView from a surface, or create
// one if it doesn't exist.
static WebView *
_get_html_view(common::surface *surf)
{
	//XXXX for some reason the pointer to the browser is stored in the parent of the current surface node
	common::surface_impl* parent = ((common::surface_impl*)surf)->get_parent();
	// Parent can be NULL, when playing on the default region
	if (parent == NULL) parent = (common::surface_impl*)surf;
	WebView *view = (WebView *)parent->get_renderer_data(my_renderer_id);
	if (view == NULL) {
		NSRect crect = NSMakeRect(0, 0, 500, 500);
		view = [[WebView alloc] initWithFrame: crect frameName: nil groupName: nil];
		[view retain];
	}
	parent->set_renderer_data(my_renderer_id, (surface_impl::renderer_data *)view);
	return view;
}

void
cocoa_html_renderer::start(double where) {
	m_lock.enter();
	renderer_playable::start(where);
	if (m_dest) {
		WebView *view = _get_html_view(m_dest);
		m_html_view = (void *)view;
		
		/*AM_DBG*/ lib::logger::get_logger()->debug("cocoa_html_renderer: view=0x%x", view);
		net::url url = m_node->get_url("src");
		if (view) {
			/*AM_DBG*/ lib::logger::get_logger()->debug("cocoa_html_renderer: display %s", url.get_url().c_str());
			// Setup an URL loader and tell the frame about it
			WebFrame *frame = [view mainFrame];
			assert(frame);
			NSURL *curl = [NSURL URLWithString: [NSString stringWithCString: url.get_url().c_str()]];
			NSURLRequest *request = [NSURLRequest requestWithURL: curl];
			[frame loadRequest: request];
			// Hook the HTML view into the hierarchy. It is retained there,
			// so we release it.
			cocoa_window *amwindow = (cocoa_window *)m_dest->get_gui_window();
			assert(amwindow);
			AmbulantView *mainview = (AmbulantView *)amwindow->view();
			assert(mainview);
			[mainview addSubview: view];
			[view release];
		}
	}
	m_lock.leave();
}

void
cocoa_html_renderer::stop() {
	m_lock.enter();
	if (m_html_view) {
		/*AM_DBG*/ lib::logger::get_logger()->debug("cocoa_html_renderer: stop display");
		// Unhook the view from the view hierarchy. This releases it, so we must
		// retain it beforehand
		WebView *view = (WebView *)m_html_view;
		[view retain];
		[view removeFromSuperviewWithoutNeedingDisplay]; 
	}
	renderer_playable::stop();
	m_lock.leave();
}
} // namespace cocoa

} // namespace gui

} //namespace ambulant

