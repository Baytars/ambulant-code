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


#ifndef AMBULANT_NET_RTSP_DATASOURCE_H
#define AMBULANT_NET_RTSP_DATASOURCE_H

#define MAX_RTP_FRAME_SIZE 50000

// LiveMedia includes
#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"


#include "avcodec.h"
#include "avformat.h"
#include "common.h"

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
#include "ambulant/net/ffmpeg_datasource.h"

#include <map>

static void 
after_reading_audio(void* data, unsigned sz, unsigned truncated, struct timeval pts, unsigned duration);

static void 
after_reading_video(void* data, unsigned sz, unsigned truncated, struct timeval pts, unsigned duration);
	
static void 
on_source_close(void* data);

namespace ambulant
{

namespace net
{

class ffmpeg_codec_id {
  public:
	static ffmpeg_codec_id* instance();
  	~ffmpeg_codec_id() {};

	void add_codec(const char* codec_name, 	CodecID id);
	CodecID get_codec_id(const char* codec_name);
  private:
	ffmpeg_codec_id(); 
	static ffmpeg_codec_id* m_uniqueinstance;
	std::map<std::string, CodecID> m_codec_id;		  
};


	
class datasink {
  public:
    virtual void data_avail(double pts, uint8_t *data, int size) = 0;
	virtual bool buffer_full() = 0;
};

struct rtsp_context_t {
	RTSPClient* rtsp_client;
  	MediaSession* media_session;
	char* sdp;
	int audio_stream;
	int video_stream;
	unsigned char* audio_packet;
	unsigned char* video_packet;
	bool need_audio;
	bool need_video;
	int nstream;
	char blocking_flag;
	bool eof;
	const char* codec_name;
	audio_format fmt;	
	datasink *sinks[MAX_STREAMS];
};
	
class rtsp_demux : public lib::unix::thread, public lib::ref_counted_obj {
  public:
	rtsp_demux(rtsp_context_t* context);
	~rtsp_demux() {};
	
	static rtsp_context_t* supported(const net::url& url);
	
	void add_datasink(datasink *parent, int stream_index);
	void remove_datasink(int stream_index);
	void cancel() {};
  	int audio_stream_nr() { return m_context->audio_stream; };
	int video_stream_nr() { return m_context->video_stream; };  
	const char* codec_name(); 
	CodecID get_codec_id();
	
  protected:
	unsigned long run();
  private:	
	
  	
    //void after_reading_audio(void* data, unsigned sz, unsigned truncated, struct timeval pts, unsigned duration);
	//void after_reading_video(void* data, unsigned sz, unsigned truncated, struct timeval pts, unsigned duration);

	
    //datasink *m_sinks[MAX_STREAMS];
	rtsp_context_t* m_context;
  	//RTSPClient *m_rtsp_client;
	//MediaSession *m_media_session;
  	//char* m_sdp;
	//int m_nstream;
	//int m_audio_stream;
	//int m_video_stream;		
	//unsigned char* m_audio_packet;
	//unsigned char* m_video_packet;
	//char* m_blocking_flag;
	//bool m_eof;
};


class live_audio_datasource: 
	virtual public audio_datasource,
	public datasink,
	virtual public lib::ref_counted_obj {
  public:
	 static live_audio_datasource *new_live_audio_datasource(
  		const net::url& url, 
  		AVCodecContext *context,
		rtsp_demux *thread);
  	
  	live_audio_datasource(
  		const net::url& url, 
  		AVCodecContext *context,
		rtsp_demux *thread, 
  		int stream_index);
  
    ~live_audio_datasource();

    void start(lib::event_processor *evp, lib::event *callback);
	void stop();  

    void readdone(int len);
    void data_avail(double pts, uint8_t *data, int size);
    bool end_of_file();
	bool buffer_full();
		
	char* get_read_ptr();
	int size() const;   
	audio_format& get_audio_format();

	std::pair<bool, double> get_dur();

  private:
    bool _end_of_file();
	const net::url m_url;
	AVCodecContext *m_con;
	int m_stream_index;
	audio_format m_fmt;
	bool m_src_end_of_file;
    lib::event_processor *m_event_processor;

	databuffer m_buffer;
	rtsp_demux *m_thread;
	lib::event *m_client_callback;  // This is our calllback to the client
	lib::critical_section m_lock;
  
};


}
} //end namespaces

#endif
