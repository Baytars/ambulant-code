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


#ifndef AMBULANT_NET_FFMPEG_DATASOURCE_H
#define AMBULANT_NET_FFMPEG_DATASOURCE_H

#include "ambulant/config/config.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/unix/unix_thread.h"
#include "ambulant/net/databuffer.h"
#include "ambulant/net/posix_datasource.h"
#include "ambulant/net/datasource.h"

#include "avcodec.h"
#ifdef WITH_FFMPEG_AVFORMAT
#include "avformat.h"
#endif // WITH_FFMPEG_AVFORMAT
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
	
class ffmpeg_audio_datasource_factory : public audio_datasource_factory {
  public:
	~ffmpeg_audio_datasource_factory() {};
	audio_datasource* new_audio_datasource(const std::string& url, audio_format_choices fmts);
};

class ffmpeg_audio_parser_finder : public audio_parser_finder {
  public:
	~ffmpeg_audio_parser_finder() {};
	audio_datasource* new_audio_parser(const std::string& url, audio_format_choices hint, datasource *src);
};

class ffmpeg_audio_filter_finder : public audio_filter_finder {
  public:
	~ffmpeg_audio_filter_finder() {};
	audio_datasource* new_audio_filter(audio_datasource *src, audio_format_choices fmts);
};

#ifdef WITH_FFMPEG_AVFORMAT

class ffmpeg_parser_datasource;

namespace detail {

class ffmpeg_parser_thread : public lib::unix::thread {
  public:
	ffmpeg_parser_thread(ffmpeg_parser_datasource *parent, AVFormatContext *con)
	:   m_parent(parent),
		m_con(con) {}
	~ffmpeg_parser_thread() {}
  protected:
	unsigned long run();
  private:
    ffmpeg_parser_datasource *m_parent;
	AVFormatContext *m_con;
};

}

class ffmpeg_parser_datasource: virtual public audio_datasource, virtual public lib::ref_counted_obj {
  public:
	 ffmpeg_parser_datasource(const std::string& url, AVFormatContext *context);
    ~ffmpeg_parser_datasource();

    void start(lib::event_processor *evp, lib::event *callback);  

    void readdone(int len);
    void data_avail(int64_t pts, uint8_t *data, int size);
    bool end_of_file();
	bool buffer_full();
		
	char* get_read_ptr();
	int size() const;   
	audio_format& get_audio_format();

	static AVFormatContext *supported(const std::string& url);
	  
  private:
    bool _end_of_file();
	const std::string m_url;
	AVFormatContext *m_con;
	audio_format m_fmt;
	bool m_src_end_of_file;
    lib::event_processor *m_event_processor;

	databuffer m_buffer;
	detail::ffmpeg_parser_thread *m_thread;
	lib::event *m_client_callback;  // This is our calllback to the client
	lib::critical_section m_lock;
};

#endif // WITH_FFMPEG_AVFORMAT

class ffmpeg_decoder_datasource: virtual public audio_datasource, virtual public lib::ref_counted_obj {
  public:
	 ffmpeg_decoder_datasource(const std::string& url, datasource *src);
	 ffmpeg_decoder_datasource(audio_datasource *src);
    ~ffmpeg_decoder_datasource();
     
		  
    void start(lib::event_processor *evp, lib::event *callback);  

    void readdone(int len);
    void data_avail();
    bool end_of_file();
	bool buffer_full();
		
	char* get_read_ptr();
	int size() const;   
	
 
	audio_format& get_audio_format();
	bool select_decoder(const char* file_ext);
	bool select_decoder(audio_format &fmt);
	
	static bool supported(const std::string& url);
  protected:
  	int decode(uint8_t* in, int size, uint8_t* out, int &outsize);
	  
  private:
    bool _end_of_file();
    AVCodecContext *m_con;
	audio_format m_fmt;
    lib::event_processor *m_event_processor;
  	datasource* m_src;

	databuffer m_buffer;
	bool m_blocked_full;
		
	lib::event *m_client_callback;  // This is our calllback to the client
	lib::critical_section m_lock;
};

class ffmpeg_resample_datasource: virtual public audio_datasource, virtual public lib::ref_counted_obj {
  public:
     ffmpeg_resample_datasource(audio_datasource *src, audio_format_choices fmts);
    ~ffmpeg_resample_datasource();
    
    void start(lib::event_processor *evp, lib::event *callback);  

    void readdone(int len);
    void data_avail();
  
    bool end_of_file();
    bool buffer_full();
		
    char* get_read_ptr();
    int size() const;   
   
//    void get_input_format(audio_context &fmt);  
//    void get_output_format(audio_context &fmt);
	audio_format& get_audio_format() { return m_out_fmt; };
		
  protected:
    int init(); 
  	
	  
  private:
    bool _end_of_file();

    audio_datasource* m_src;

    bool m_context_set;
    ReSampleContext *m_resample_context;
  
    databuffer m_buffer;
  	
    lib::event_processor *m_event_processor;
    lib::event *m_client_callback;  // This is our calllback to the client
    lib::critical_section m_lock;
  	audio_format m_in_fmt;
  	audio_format m_out_fmt;
};

}	// end namespace net
}	// end namespace ambulant

#endif
