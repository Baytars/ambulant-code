/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

#ifndef __GSTREAMER_H
#define __GSTREAMER_H

#include <gst/gst.h>
#include <iostream>

#include "ambulant/common/factory.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/lib/logger.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/net/datasource.h"
#include "ambulant/lib/event_processor.h"
#ifdef USE_SMIL21
#include "ambulant/smil2/transition.h"
#include "ambulant/lib/transition_info.h"
#endif
#include "ambulant/lib/asb.h"
#include "ambulant/lib/unix/unix_thread.h"


#define GSTREAMER_BUFFER_MAX_BYTES 819200
#define GSTREAMER_BUFFER_MIN_BYTES 20480

#define AMBULANT_MAX_CHANNELS 2
 
namespace ambulant {
namespace gui {
namespace gstreamer {	  

  class gstreamer_audio_renderer;
  class gstreamer_player :  public lib::unix::thread {

  public:
    gstreamer_player(const char* uri,  gstreamer_audio_renderer* rend); 
    ~gstreamer_player(); 
    GstElement* gst_player();
    unsigned long init();

  protected:
    unsigned long run();

  private:
    const char* m_uri;
    gstreamer_audio_renderer* m_audio_renderer;
    GstElement* m_gst_player;
  };

} // end namespace gstreamer
} // end namespace gui
} // end namespace ambulant

#endif // __GSTREAMER_H
