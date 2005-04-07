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

#ifndef AMBULANT_LIB_WIN32_MTSYNC_H
#define AMBULANT_LIB_WIN32_MTSYNC_H

#ifndef _INC_WINDOWS

#include <windows.h>
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#endif // _INC_WINDOWS

#include "ambulant/config/config.h"

#include "ambulant/lib/abstract_mtsync.h"

namespace ambulant {

namespace lib {

namespace win32 {

class AMBULANTAPI critical_section : public ambulant::lib::abstract_critical_section {
	friend class condition;
  public:
	critical_section() { InitializeCriticalSection(&m_cs);}
	~critical_section() { DeleteCriticalSection(&m_cs);}

	void enter() { EnterCriticalSection(&m_cs);}
	void leave() { LeaveCriticalSection(&m_cs);}

  private:
	CRITICAL_SECTION m_cs;
};

class AMBULANTAPI condition : public ambulant::lib::abstract_condition {
  public:
	  condition() { m_event = CreateEvent(NULL, TRUE, FALSE, NULL);}
	  ~condition() { CloseHandle(m_event); }
	
	  void signal() { SetEvent(m_event); }
	  void signal_all() { abort(); }
	  bool wait(int microseconds, critical_section &cs) {
		  DWORD timeout = microseconds < 0 ? INFINITE : microseconds/1000;
		  bool rv = WaitForSingleObject(m_event, timeout) == WAIT_OBJECT_0;
		  if (rv) {
			  EnterCriticalSection(&cs.m_cs);
			  ResetEvent(m_event);
		  }
		  return rv;
	  }
  private:
	HANDLE m_event;
};
} // namespace win32

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_MTSYNC_H
