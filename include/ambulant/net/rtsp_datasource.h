/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
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

#ifndef AMBULANT_NET_RTSP_DATASOURCE_H
#define AMBULANT_NET_RTSP_DATASOURCE_H

#include "ambulant/config/config.h"

#define MAX_RTP_FRAME_SIZE 50000

#ifdef AMBULANT_PLATFORM_MACOS
// Both MacHeaders.h and Live typedef Boolean, but to imcompatible
// types.
#define Boolean LiveBoolean
// Similarly for EventTime
#define EventTime LiveEventTime
#endif

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
#ifdef AMBULANT_PLATFORM_UNIX
#include "ambulant/lib/unix/unix_thread.h"
#define BASE_THREAD lib::unix::thread
#endif
#ifdef AMBULANT_PLATFORM_WIN32
#include "ambulant/lib/win32/win32_thread.h"

//xxxbo for using StrDup
#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>

#define BASE_THREAD lib::win32::thread
#endif
#include "ambulant/net/databuffer.h"
//#include "ambulant/net/posix_datasource.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/ffmpeg_common.h"

namespace ambulant
{

namespace net
{

struct rtsp_context_t {
	~rtsp_context_t();
	
	TaskScheduler* scheduler;
	UsageEnvironment* env;
	RTSPClient* rtsp_client;
  	MediaSession* media_session;
	char* sdp;
	int audio_stream;
	int video_stream;
	
	unsigned char* configData; //required for MP4V-ES "VOL header"
	int configDataLen;
#ifdef WITH_VBUFFER
	unsigned char* vbuffer;
	int vbufferlen;
#endif
	int extraPacketHeaderSize;	// H264 (and some other formats) need a couple extra bytes at the beginning of each packet.
	unsigned char* audio_packet;
	unsigned char* video_packet;
	timestamp_t last_pts;
	bool need_audio;
	bool need_video;
	int nstream;
	char blocking_flag;
	bool eof;
	timestamp_t clip_end;
	bool is_clip_end;
	float duration;
	timestamp_t time_left;
	const char* audio_codec_name;
	const char* video_codec_name;
	timeval first_sync_time;
	audio_format audio_fmt;	
	video_format video_fmt;
	demux_datasink *sinks[MAX_STREAMS];
	int nsinks;

//xxxbo for h264
	int gb_first_sync;
};
	
class rtsp_demux : public abstract_demux {
  public:
	rtsp_demux(rtsp_context_t* context, timestamp_t clip_begin, timestamp_t clip_end);
	~rtsp_demux();
	static rtsp_context_t* supported(const net::url& url);
	
	void add_datasink(demux_datasink *parent, int stream_index);
	void remove_datasink(int stream_index);
  	int audio_stream_nr() { return m_context->audio_stream; };
	int video_stream_nr() { return m_context->video_stream; };  
	int nstreams() { return m_context->nstream; };
	double duration(){ return 0.0; };
	audio_format& get_audio_format() { return m_context->audio_fmt; };
	video_format& get_video_format() { return m_context->video_fmt; };
	void seek(timestamp_t time);
	void read_ahead(timestamp_t time);
	void cancel();
	timestamp_t get_clip_end();
	timestamp_t get_clip_begin();
	timestamp_t get_start_time() { return m_clip_begin; };
	// These next two should be protected, but I don't know how to make a static function a friend.
	void after_reading_audio(unsigned sz, unsigned truncated, struct timeval pts, unsigned duration);
	void after_reading_video(unsigned sz, unsigned truncated, struct timeval pts, unsigned duration);
  protected:
	unsigned long run();
  private:	
	void _set_position(timestamp_t time);
	void _cancel();
	lib::critical_section m_critical_section;
	rtsp_context_t* m_context;
  	timestamp_t m_clip_begin;
	timestamp_t m_clip_end;
	timestamp_t m_seektime;
  	bool m_seektime_changed; // true if either m_clip_begin or m_seektime has changed
};


}
} //end namespaces

#endif
