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

#ifndef AMBULANT_NET_FFMPEG_RAW_H
#define AMBULANT_NET_FFMPEG_RAW_H

#include "ambulant/config/config.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/unix/unix_thread.h"
#include "ambulant/net/databuffer.h"
#include "ambulant/net/datasource.h"

#include "avformat.h"
#include "avio.h"
#include "common.h"

// temporary debug messages
#include <iostream>
#ifndef AMBULANT_NO_OSTREAM
#include <ostream>
#else /*AMBULANT_NO_OSTREAM */
#include <ostream.h>
#endif /*AMBULANT_NO_OSTREAM */
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace ambulant
{

namespace net
{
	
class ffmpeg_raw_datasource_factory : public raw_datasource_factory {
  public:
	~ffmpeg_raw_datasource_factory() {};
	datasource* new_raw_datasource(const net::url& url);
};

namespace detail {

// Actually, these classes could easily be used for a general demultiplexing
// datasource, there is very little code that is ffmpeg-dependent.

class rawdatasink {
  public:
	virtual ~rawdatasink(){}
    virtual int get_sinkbuffer(uint8_t **datap) = 0;
	virtual void pushdata(int size) = 0;
};
	
class ffmpeg_rawreader : public lib::unix::thread, public lib::ref_counted_obj {
  public:
	ffmpeg_rawreader(URLContext *con);
	~ffmpeg_rawreader();
	
	static URLContext *supported(const net::url& url);
	
	void set_datasink(rawdatasink *parent);
	void cancel();
  protected:
	unsigned long run();
  private:
	URLContext *m_con;
    rawdatasink *m_sink;
    lib::critical_section m_lock;
};

}

class ffmpeg_raw_datasource: 
	virtual public datasource,
	public detail::rawdatasink,
	virtual public lib::ref_counted_obj
{
  public:
	 ffmpeg_raw_datasource(const net::url& url, URLContext *context,
		detail::ffmpeg_rawreader *thread);
    ~ffmpeg_raw_datasource();

    void start(lib::event_processor *evp, lib::event *callback);  
	void stop();
	
	char* get_read_ptr();
    void readdone(int len);
    bool end_of_file();
	int size() const;   

    int get_sinkbuffer(uint8_t **datap);
	void pushdata(int size);

  private:
    bool _end_of_file();
	const net::url m_url;
	URLContext *m_con;
	bool m_src_end_of_file;
    lib::event_processor *m_event_processor;

	databuffer m_buffer;
	detail::ffmpeg_rawreader *m_thread;
	lib::event *m_client_callback;  // This is our calllback to the client
	lib::critical_section m_lock;
};

}	// end namespace net
}	// end namespace ambulant

#endif // AMBULANT_NET_FFMPEG_RAW_H