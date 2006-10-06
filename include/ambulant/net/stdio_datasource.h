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

 
#ifndef AMBULANT_NET_STDIO_DATASOURCE_H
#define AMBULANT_NET_STDIO_DATASOURCE_H

#include "ambulant/config/config.h"

#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/databuffer.h"
#include "ambulant/net/url.h"

#include <stdio.h>

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

//#include <stdio.h>
//#include <stdlib.h>
//#include <iomanip>
#include <cstring>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>


namespace ambulant {

namespace net {

/// Implementation of raw_datasource_factory that creates stdio_datasource objects.
class stdio_datasource_factory : public raw_datasource_factory {
  public:
	~stdio_datasource_factory() {};
	datasource* new_raw_datasource(const url& url);
};

/// Implementation of datasource that uses stdio to access local files.
class stdio_datasource : virtual public datasource, virtual public lib::ref_counted_obj {
  public:
	stdio_datasource();
	stdio_datasource(const url& url, FILE* file);
  	~stdio_datasource();
  	
  	void start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback);
	void stop();
	void readdone(int len);
	bool end_of_file();
	
		
	char* get_read_ptr();
	int size() const;

	ts_packet_t get_ts_packet_t();
  
	void read(char *data, int size);
  	
#ifndef AMBULANT_NO_IOSTREAMS_HEADERS
  	friend inline std::ostream& operator<<(std::ostream& os, const stdio_datasource& n) {
		os << "stdio_datasource(" << (void *)&n << ", source=" << n.m_url.get_url() << ")";
		return os;
	}
#endif
  private: 
    bool _end_of_file();
	void filesize();
    void read_file();
	const url m_url;
	databuffer *m_buffer;
	int m_filesize;
	FILE *m_stream;
	bool m_end_of_file;
	lib::critical_section m_lock;
};

AMBULANTAPI raw_datasource_factory *create_stdio_datasource_factory();

} // end namespace net

} //end namespace ambulant

#endif  //AMBULANT_NET_STDIO_DATASOURCE_H
