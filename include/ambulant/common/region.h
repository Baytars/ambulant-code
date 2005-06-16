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

#ifndef AMBULANT_COMMON_REGION_H
#define AMBULANT_COMMON_REGION_H

#include "ambulant/config/config.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/region_info.h"

namespace ambulant {

namespace common {

	using namespace ambulant::lib;

class passive_region : public surface_template, public surface, public gui_events {
  // The only constructor is protected: 
  protected:
	passive_region(const std::string &name, passive_region *parent, screen_rect_int bounds,
		const region_info *info, bgrenderer *bgrenderer);
  public:
	virtual ~passive_region();
	
	// The surface_template interface:
	common::surface_template *new_subsurface(const region_info *info, bgrenderer *bgrenderer);
	surface *activate();
	void animated();

	// The surface interface:
	void show(gui_events *cur);
	void renderer_done(gui_events *renderer);
	virtual void need_redraw(const screen_rect_int &r);
	void need_redraw();
	virtual void need_events(bool want);
	const screen_rect_int& get_rect() const { return m_inner_bounds; }
	virtual const point &get_global_topleft() const;
	screen_rect_int get_fit_rect(const size& src_size, rect* out_src_rect, const common::alignment *align) const;
#ifdef USE_SMIL21
	bool is_tiled() const;
	tile_positions get_tiles(lib::size image_size, lib::screen_rect_int surface_rect) const;
#endif
	const region_info *get_info() const { return m_info; }	
#ifdef USE_SMIL21
	surface *get_top_surface() { return m_parent->get_top_surface(); }
#endif
	gui_window *get_gui_window() { return m_parent->get_gui_window(); }

	void transition_done() { transition_done(m_inner_bounds); }

	void keep_as_background();
	
	// The gui_events interface:
	void redraw(const screen_rect_int &dirty, gui_window *window);
	void user_event(const point &where, int what = 0);
		
	// Win32 code needs this, but I don't like it:
	const passive_region *get_parent() const { return m_parent; }
  private:
	void clear_cache();					// invalidate cached sizes (after animation)
	void need_bounds();					// recompute cached sizes
	screen_rect_int get_fit_rect_noalign(const size& src_size, rect* out_src_rect) const;
	void draw_background(const screen_rect_int &r, gui_window *window);

  protected:
	virtual void transition_done(lib::screen_rect_int area);
	void transition_freeze_end(lib::screen_rect_int area);

  	std::string m_name;					// for debugging

	bool m_bounds_inited;				// True if bounds and topleft initialized
  	screen_rect_int m_inner_bounds;	// region rectangle (0, 0) based
  	screen_rect_int m_outer_bounds;	// region rectangle in parent coordinate space
	point m_window_topleft;				// region top-left in window coordinate space

  	passive_region *m_parent;			// parent region

  	std::list<gui_events *> m_renderers; // active regions currently responsible for redraws
	lib::critical_section m_children_cs; // Protects m_renderers, m_active_children, m_subregions

	typedef std::list<passive_region*> children_list_t;
	typedef std::map<zindex_t, children_list_t> children_map_t;
	children_map_t m_active_children;	// all child regions
	children_map_t m_subregions;		// all active children that are subregions
	void add_subregion(zindex_t z, passive_region *rgn);
	void del_subregion(zindex_t z, passive_region *rgn);

	const region_info *m_info;			// Information such as z-order, etc.
	bgrenderer *m_bg_renderer;			// Background renderer
};

class passive_root_layout : public passive_region {
  public:
	passive_root_layout(const region_info *info, size bounds, bgrenderer *bgrenderer, window_factory *wf);
	~passive_root_layout();
	
	void need_redraw(const screen_rect_int &r);
	void need_events(bool want);
	const point &get_global_topleft() const { static point p = point(0, 0); return p; }
#ifdef USE_SMIL21
	surface *get_top_surface() { return this; }
#endif
	gui_window *get_gui_window() { return m_gui_window; }
  protected:
	void transition_done(lib::screen_rect_int area) { transition_freeze_end(area); }
  private:
	gui_window *m_gui_window;
};

class smil_surface_factory : public surface_factory {
  public:
	surface_template *new_topsurface(const region_info *info, bgrenderer *bgrend, window_factory *wf);
};

} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_REGION_H
