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

 
#ifndef AMBULANT_NET_WIN32_DATASOURCE_H
#define AMBULANT_NET_WIN32_DATASOURCE_H

#include "ambulant/config/config.h"

#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/databuffer.h"
#include "ambulant/net/url.h"


//////////////////////////////////////
#ifndef AMBULANT_NO_IOSTREAMS_HEADERS

// temporary debug messages
#include <iostream>
#ifndef AMBULANT_NO_OSTREAM
#include <ostream>
#else /*AMBULANT_NO_OSTREAM*/
#include <ostream.h>
#endif/*AMBULANT_NO_OSTREAM*/

#endif // AMBULANT_NO_IOSTREAMS_HEADERS
//////////////////////////////////////

#include <stdio.h>
//#include <stdlib.h>
//#include <iomanip>
#include <cstring>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>


namespace ambulant {

namespace net {


class win32_datasource_factory : public raw_datasource_factory {
  public:
	~win32_datasource_factory() {};
	datasource* new_raw_datasource(const url& url);
};

extern raw_datasource_factory *get_win32_datasource_factory();

class win32_datasource : virtual public datasource, virtual public lib::ref_counted_obj {
  public:
	win32_datasource();
	win32_datasource(const url& url);
  	~win32_datasource();
  	
  	void start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback);
	void stop();
	void readdone(int len);
    bool end_of_file();
	
		
	char* get_read_ptr();
	int size() const;

  
	void read(char *data, int size);
  	
  protected: 
    bool _end_of_file();
    virtual void _read_file(); // Defined in the concrete subclasses
	const url m_url;
	databuffer *m_buffer;
	bool m_end_of_file;
	lib::critical_section m_lock;
};

// The concrete subclasses win32_net_datasource and win32_file_datasource are
// declared in win32_datasource.cpp, so we don't have to include win32 include
// files here.

} // end namespace net

} //end namespace ambulant

#endif  //AMBULANT_NET_WIN32_DATASOURCE_H
