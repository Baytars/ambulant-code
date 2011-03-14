/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2010 Stichting CWI,
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 * @$Id$
 */

#ifndef AMBULANT_GUI_DX_IMG_WIC_H
#define AMBULANT_GUI_DX_IMG_WIC_H

#include "ambulant/config/config.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/gui/dx/dx_playable.h"
#include "ambulant/gui/dx/dx_viewport.h"

#define WITH_WIC
#ifdef WITH_WIC
#include <wincodec.h>
#include <ddraw.h>

namespace ambulant {

namespace gui {

namespace dx {

class image_renderer;
class dx_gui_region;

common::playable_factory *create_dx_image_wic_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);

class dx_img_wic_renderer : public dx_renderer_playable {
  public:
	dx_img_wic_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		common::factories *factory,
		common::playable_factory_machdep *dxplayer);
	~dx_img_wic_renderer();
	void start(double t);
	//void stop();
	bool stop();
	void seek(double t) {}
	bool user_event(const lib::point& pt, int what);
	void redraw(const lib::rect &dirty, common::gui_window *window);
  private:
	void _create_ddsurf(viewport *v);	// Create m_ddsurf from m_original

	static IWICImagingFactory *s_wic_factory;
	IWICBitmapSource *m_original;	// The original image data reader
	IDirectDrawSurface *m_ddsurf;	// The DD surface, read-to-draw
	char *m_databuf;					// For non-local-url based images.
	lib::rect m_msg_rect;
	dx_gui_region *m_rgn;
	common::factories *m_factory;
};

} // namespace dx

} // namespace gui

} // namespace ambulant
#endif // WITH_WIC
#endif // AMBULANT_GUI_DX_IMG_WIC_H