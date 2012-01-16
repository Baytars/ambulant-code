/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
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

#ifndef AMBULANT_LIB_TIMER_SYNC_H
#define AMBULANT_LIB_TIMER_SYNC_H

#include "ambulant/config/config.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/timer.h"

#ifdef WITH_REMOTE_SYNC

namespace ambulant {

namespace lib {

class timer_sync : public timer_observer {
  public:
	virtual void initialize(timer_control *timer) = 0;
	virtual void started() = 0;
	virtual void stopped() = 0;
	virtual void paused() = 0;
	virtual void resumed() = 0;
};

// Factory function that returns a synchronizer 
AMBULANTAPI timer_sync *timer_sync_factory(document *doc);


} // namespace lib

} // namespace ambulant
#endif // WITH_REMOTE_SYNC
#endif // AMBULANT_LIB_TIMER_SYNC_H


