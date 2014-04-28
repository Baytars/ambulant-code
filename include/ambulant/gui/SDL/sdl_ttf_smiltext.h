/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2014 Stichting CWI, 
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
 * @$Id: sdl_ttf_smiltext.h,v 1.16.2.1 2014/04/09 12:54:44 keesblom Exp $ 
 */

/* 'sdl_ttf_smiltext' is modelled after 'qt_smiltext' as found in ambulant-2.2.
 * It uses the 'smiltext_layout_engine' of the smiltext implementation
 * which computes the correct position of each text fragment, using information
 * provided by the font library (SDL2_ttf), such as the size of each glyph.
 */
#ifndef AMBULANT_GUI_SDL_TTF_SMILTEXT_H
#define AMBULANT_GUI_SDL_TTF_SMILTEXT_H

#ifdef  WITH_SDL_TTF

#include "ambulant/config/config.h"
#include <string>

#include "ambulant/smil2/smiltext.h"
#include "ambulant/gui/SDL/sdl_renderer.h"
#include "ambulant/gui/SDL/sdl_factory.h"
#include "ambulant/gui/SDL/sdl_window.h"

//XX move to impl.
#define FONT "Times 6"
#ifndef ANDROID
#define DEFAULT_FONT_FILE1 "/usr/share/fonts/liberation/LiberationSans-Regular.ttf"
#define DEFAULT_FONT_FILE2 "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf" 
#define DEFAULT_FONT_FILE3 "/usr/local/etc/ginga/files/font/vera.ttf"
#else // ANDROID
#define DEFAULT_FONT_FILE1 "LiberationSans-Regular.ttf"
#endif // ANDROID
#define DEFAULT_FONT_HEIGHT 16
#include "SDL_ttf.h"

namespace ambulant {
 
using namespace lib;
using namespace common;

namespace gui {

namespace sdl {

class smiltext_renderer;

class sdl_ttf_smiltext_renderer : 
		public sdl_renderer<renderer_playable>,
		public smil2::smiltext_notification,
 		public smil2::smiltext_layout_provider  
 {
  public:
	sdl_ttf_smiltext_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const node *node,
		event_processor* evp,
		factories *fp,
		playable_factory_machdep *mdp);
	~sdl_ttf_smiltext_renderer();
	void start(double t);
	void seek(double t) {}
	bool stop();

	// Callbacks from the smiltext engine
	void smiltext_changed();
	void marker_seen(const char *name);
	void redraw_body(const rect &dirty, gui_window *window);

	// Callbacks from smiltext_layout_engine
	smil2::smiltext_metrics get_smiltext_metrics(const smil2::smiltext_run& str);
	void render_smiltext(const smil2::smiltext_run& str, const rect& r);
	void smiltext_stopped();
	const rect& get_rect();

  private:
	// functions required by inheritance
	void smiltext_changed(bool);

	// internal helper functions
	void _sdl_ttf_smiltext_set_font(const smil2::smiltext_run& run);

	// instance variables
//XX?	net::datasource_factory *m_df;
	ambulant_sdl_window* m_window;
	smil2::smiltext_layout_engine m_layout_engine;
//XX bool m_render_offscreen; // True if m_params does not allows rendering in-place
	// Sdl related variables
	// ttf specific stuff
	rect		m_rect;
	TTF_Font*	m_ttf_font;
	color_t		m_text_color; //XX?
	color_t		m_text_bg_color; //XX?
	int			m_text_size;
	const char* m_text_font;
	int			m_ttf_style;
	bool		m_blending;
	SDL_Color	m_sdl_transparent;
	SDL_Color	m_sdl_alternative;
	double		m_bgopacity; 
	critical_section m_lock;
};

} // namespace sdl

} // namespace gui
 
} // namespace ambulant

#endif //WITH_SDL_TTF

#endif // AMBULANT_GUI_SDL_TTF_SMILTEXT_H
