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
#include "GroupsockHelper.hh"

#include "avcodec.h"
#ifdef WITH_FFMPEG_AVFORMAT
#include "avformat.h"
#endif // WITH_FFMPEG_AVFORMAT
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
namespace ambulant
{

namespace net
{


struct rtsp_context_t {
	RTSPClient* rtsp_client;
  	MediaSession* media_session;
	char* sdp;
	int audio_stream;
	int video_stream;
	int nstream;
};
	


class datasink {
  public:
    virtual void data_avail(double pts, uint8_t *data, int size) = 0;
	virtual bool buffer_full() = 0;
};
	
class rtsp_demux : public lib::unix::thread, public lib::ref_counted_obj {
  public:
	rtsp_demux(rtsp_context_t* context);
	~rtsp_demux();
	
	static rtsp_context_t* supported(const net::url& url);
	
	void add_datasink(datasink *parent, int stream_index);
	void remove_datasink(int stream_index);
	void cancel();
  protected:
	unsigned long run();
  private:	
	
  	int audio_stream_nr();
	int video_stream_nr();  
    void after_reading_audio(void* data, unsigned sz, unsigned truncated, struct timeval pts, unsigned duration);
	void after_reading_video(void* data, unsigned sz, unsigned truncated, struct timeval pts, unsigned duration);
	void on_source_close(void* clientdata);
	
    datasink *m_sinks[MAX_STREAMS];
  	RTSPClient *m_rtsp_client;
	MediaSession *m_media_session;
  	char* m_sdp;
	int m_nstream;
	int m_audio_stream;
	int m_video_stream;		
	unsigned char* m_audio_packet;
	unsigned char* m_video_packet;
	char* m_blocking_flag;
	bool m_eof;
};

class live_ffmpeg_audio_datasource: 
	virtual public audio_datasource,
	public datasink,
	virtual public lib::ref_counted_obj
{
  public:
	 static live_ffmpeg_audio_datasource *new_live_ffmpeg_audio_datasource(
  		const net::url& url, 
  		AVFormatContext *context,
		rtsp_demux *thread);
  	
  	live_ffmpeg_audio_datasource(
  		const net::url& url, 
  		AVFormatContext *context,
		rtsp_demux *thread, 
  		int stream_index);
  
    ~live_ffmpeg_audio_datasource();

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
	AVFormatContext *m_con;
	int m_stream_index;
	audio_format m_fmt;
	bool m_src_end_of_file;
    lib::event_processor *m_event_processor;

	databuffer m_buffer;
	rtsp_demux *m_thread;
	lib::event *m_client_callback;  // This is our calllback to the client
	lib::critical_section m_lock;
  
};

class live_ffmpeg_video_datasource:
	virtual public video_datasource,
	public datasink,
	virtual public lib::ref_counted_obj {
  public:
	 static live_ffmpeg_video_datasource *new_live_ffmpeg_video_datasource(
		const net::url& url, AVFormatContext *context,
		rtsp_demux *thread);

	 live_ffmpeg_video_datasource(const net::url& url, AVFormatContext *context,
		rtsp_demux *thread, int stream_index);
    ~live_ffmpeg_video_datasource();

	bool has_audio();
    int width();
  	int height();
	audio_datasource *get_audio_datasource();

    void start_frame(lib::event_processor *evp, lib::event *callback, double timestamp);  
	void stop();  

    bool end_of_file();
	char* get_frame(double *timestamp, int *size);
	void frame_done(double timestamp, bool keepdata);
	
    void data_avail(int64_t ipts, uint8_t *data, int size);
	bool buffer_full();
	std::pair<bool, double> get_dur();

  private:
	int get_audio_stream_nr();
    bool _end_of_file();
	const net::url m_url;
	AVFormatContext *m_con;
	int m_stream_index;
	bool m_src_end_of_file;
    lib::event_processor *m_event_processor;
	std::queue<std::pair<double, char*> > m_frames;
	std::pair<double, char*> m_old_frame;
	int m_size;		// NOTE: this assumes all decoded frames are the same size!
//	databuffer m_buffer;
	rtsp_demux *m_thread;
	lib::event *m_client_callback;  // This is our calllback to the client
  	bool m_thread_started;
  	double m_pts_last_frame;
  	double m_last_p_pts;
    lib::critical_section m_lock;
};


}
} //end namespaces

#endif
