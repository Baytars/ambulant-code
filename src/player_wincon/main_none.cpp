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

#include "ambulant/version.h"

#include "ambulant/lib/player.h"
#include "ambulant/lib/event.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/lib/logger.h"

#include "ambulant/gui/none/none_gui.h"

using namespace ambulant;

int
main(int argc, char **argv) {
	std::cout << "Ambulant version " << get_version() << std::endl;
	
	if(argc == 1) {
		std::cerr << "Usage: player_wincon file" << std::endl;
		return 0;
	}
	char *filename = argv[1];
	
	lib::logger *logger = lib::logger::get_logger();
	logger->trace("Play: %s", filename);
	
	// Create passive_player from filename
	lib::passive_player *pplayer = new lib::passive_player(filename);
	if (!pplayer) {
		logger->error("Failed to construct passive_player from file %s", filename);
		return 1;
	}
	
	// Create GUI window_factory and renderer_factory
	lib::window_factory *wf = new gui::none::none_window_factory();
	lib::renderer_factory *rf = new ambulant::gui::none::none_renderer_factory();
	
	// Request an active_player for the provided factories 
	lib::active_player *aplayer = pplayer->activate(wf, rf);
	if (!aplayer) {
		logger->error("passive_player::activate() failed to create active_player");
		return 1;
	}
	
	// Create an event_processor
	lib::event_processor *processor = 
		lib::event_processor_factory(lib::realtime_timer_factory());
	
	// Start event_processor. 
	// Pass a flag_event to be set when done.
	bool done = false;
	logger->trace("Start playing");
	aplayer->start(processor, new lib::flag_event(done));
	while(!done)
		lib::sleep_msec(50);
	logger->trace("Finished playing");
	
	// cleanup
	delete processor;
	delete pplayer;
	aplayer = lib::release(aplayer);
	// verify:
	if(aplayer != 0)
		logger->warn("active_player ref_count: " + aplayer->get_ref_count());
	delete rf;
	delete wf;
	
	logger->trace("Normal exit");
	return 0;
}
