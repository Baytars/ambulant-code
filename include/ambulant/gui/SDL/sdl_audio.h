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
#ifndef __SDL_AUDIO__
#define __SDL_AUDIO__

#include <SDL.h>
#include <SDL_mixer.h>
#include <iostream>

#include "ambulant/common/layout.h"
#include "ambulant/common/renderer.h"
#include "ambulant/lib/logger.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/net/datasource.h"
#ifdef WITH_FFMPEG
#include "ambulant/net/ffmpeg_datasource.h"
#endif
#include "ambulant/net/raw_audio_datasource.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"


#define SDL_BUFFER_MAX_BYTES 819200
#define SDL_BUFFER_MIN_BYTES 20480


 
namespace ambulant {
namespace gui {
namespace sdl {	  

class sdl_active_audio_renderer : public common::active_renderer, public lib::timer_events {
  public:
    sdl_active_audio_renderer(
    common::playable_notification *context,
    common::playable_notification::cookie_type cookie,
    const lib::node *node,
    lib::event_processor *evp);

  	~sdl_active_audio_renderer();

      
  	bool is_paused();
  	bool is_stopped();
  	bool is_playing();
  
 
    void start(double where);
    void stop();
    void pause();
    void resume();
	void freeze() {};
    void speed_changed() {};
    void readdone();
    void redraw(const lib::screen_rect<int> &dirty, common::abstract_window *window) {};
	void wantclicks(bool want) {};
    void user_event(const lib::point &where) {};
	void playdone();
		  
  private:
	void new_channel();  
	
	net::audio_datasource *m_audio_src;
 	int inc_channels();
	int init(int rate, int bits, int channels);
    static bool m_sdl_init;
    static int m_mixed_channels;
    
    Mix_Chunk m_audio_chunck;
    int m_rate;
	int m_bits;
    int m_channels;
    int m_buffer_size;
	int m_channel_used;
	Uint16 m_audio_format;
};

} // end namespace sdl
} // end namespace gui
} // end namespace ambulant


#endif
