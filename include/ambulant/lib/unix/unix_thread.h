/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
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

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_LIB_UNIX_THREAD_H
#define AMBULANT_LIB_UNIX_THREAD_H

#include "ambulant/lib/thread.h"
#include <pthread.h>
#undef unix

namespace ambulant {

namespace lib {

namespace unix {

class thread : public ambulant::lib::thread {
  public:
	thread();
	virtual ~thread();

	virtual bool start();
	virtual void stop();
	bool terminate();
		
	bool exit_requested() const;
	bool is_running() const;
		
  protected:
	virtual unsigned long run() = 0;
	
	virtual void signal_exit_thread();

  private:
	static void *threadproc(void *pParam);
	
	pthread_t m_thread;
	bool m_exit_requested; // true as soon as thread exit is imminent
	bool m_running;	// true as soon as thread running is imminent
};

} // namespace unix

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_UNIX_THREAD_H
