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

#include "ambulant/net/rtsp_datasource.h"
#include "ambulant/lib/logger.h"
#include "GroupsockHelper.hh"

using namespace ambulant;
using namespace net;

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define MIN_VIDEO_PACKET_SIZE 1024


ambulant::net::rtsp_demux::rtsp_demux(rtsp_context_t* context, timestamp_t clip_begin, timestamp_t clip_end)
:	m_context(context),
	m_clip_begin(clip_begin),
	m_clip_end(clip_end),
	m_clip_begin_set(false)
{
	m_context->audio_fmt.parameters = (void*) m_context->audio_codec_name;
	m_context->video_fmt.parameters = (void*) m_context->video_codec_name;
}


void 
ambulant::net::rtsp_demux::add_datasink(detail::datasink *parent, int stream_index)
{
	assert(stream_index >= 0 && stream_index < MAX_STREAMS);
	assert(m_context->sinks[stream_index] == 0);
	m_context->sinks[stream_index] = parent;
	m_context->nstream++;
}

void
ambulant::net::rtsp_demux::remove_datasink(int stream_index)
{
	assert(stream_index >= 0 && stream_index < MAX_STREAMS);
	assert(m_context->sinks[stream_index] != 0);
	m_context->sinks[stream_index] = 0;
	m_context->nstream--;
	if (m_context->nstream <= 0) cancel();
}


rtsp_context_t*
ambulant::net::rtsp_demux::supported(const net::url& url) 
{
	rtsp_context_t* context = new rtsp_context_t;
	context->first_sync_time.tv_sec = 0;
	context->first_sync_time.tv_usec = 0;
	context->rtsp_client = NULL;
	context->media_session = NULL;
	context->sdp = NULL;
	context->audio_stream = -1;
	context->video_stream = -1;
	context->nstream = 0;
	context->blocking_flag = 0;
	context->audio_packet = NULL;
	//context->audio_packet = (unsigned char*) malloc(MAX_RTP_FRAME_SIZE);
	//if (!context->audio_packet) {
	//	return NULL;		
	//}
	context->video_packet = NULL;
	//context->video_packet = (unsigned char*) malloc(MAX_RTP_FRAME_SIZE);
	//if (!context->video_packet) {
	//		return NULL;		
	//}
	context->video_buffer = NULL;
	context->video_buffer_size = 0;
	context->last_pts = 0;
	context->audio_codec_name = NULL;
	context->video_codec_name = NULL;
	context->audio_fmt = audio_format("live");
	context->video_fmt = video_format("live");
	context->eof = false;
	context->need_video = true;
	context->need_audio = true;
	
	memset(context->sinks, 0, sizeof context->sinks);
	
	// setup the basics.
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	if (!scheduler) {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to create scheduler");
	    //lib::logger::get_logger()->error("RTSP Connection Failed");		
		return NULL;
	}
	
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
	if (!env) {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to create UsageEnvironment");
		//lib::logger::get_logger()->error("RTSP Connection Failed");		
		return NULL;
	}
	// setup a rtp session
	int verbose = 0;
	context->rtsp_client = RTSPClient::createNew(*env, verbose, "AmbulantPlayer");
	if (!context->rtsp_client) {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to create  a RTSP Client");
		//lib::logger::get_logger()->error("RTSP Connection Failed");		
		return NULL;
	}
	
	const char* ch_url = url.get_url().c_str();
	if (ch_url) {
		context->sdp = context->rtsp_client->describeURL(ch_url);
		if (!context->sdp) {
			lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to get dsp description from rtsp server");
			//lib::logger::get_logger()->error("RTSP Connection Failed");		
			return NULL;
		}
	} else {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to get url");
		lib::logger::get_logger()->error("Wrong RTSP URL !");		
		return NULL;
	}
	
	context->media_session = MediaSession::createNew(*env, context->sdp);
	if (!context->media_session) {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to create  a MediaSession");
		//lib::logger::get_logger()->error("RTSP Connection Failed");		
		return NULL;
	}	
	
	// next set up the rtp subsessions.
	
	unsigned int desired_buf_size;
	MediaSubsession* subsession;
	MediaSubsessionIterator iter(*context->media_session);
	// Only audio/video session need to apply for a job !
	while ((subsession = iter.next()) != NULL) {
		if (strcmp(subsession->mediumName(), "audio") == 0) {
			desired_buf_size = 100000;
			if (context->audio_stream < 0) {
				
				context->audio_stream = context->nstream;
				context->audio_codec_name = subsession->codecName();
				lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url), audio codecname :%s ",context->audio_codec_name);
				context->audio_fmt.channels = subsession->numChannels() + 1;
				context->audio_fmt.bits = 16;
				//context->fmt.samplerate = subsession->rtpSource()->timestampFrequency();
				
			}
		} else if (strcmp(subsession->mediumName(), "video") == 0) {
			desired_buf_size = 200000;
			if (context->video_stream < 0) {
				context->video_stream = context->nstream;
				context->video_codec_name = subsession->codecName();
				lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url), video codecname :%s ",context->video_codec_name);
				context->video_fmt.framerate = subsession->videoFPS();
				context->video_fmt.width = subsession->videoWidth();
				context->video_fmt.height = subsession->videoHeight();
			}
		}
		context->nstream++;
		if (!subsession->initiate()) {
			lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to initiate subsession");
			//lib::logger::get_logger()->error("RTSP Connection Failed");
			return NULL;
		}
		
		int rtp_sock_num = subsession->rtpSource()->RTPgs()->socketNum();
		int buf_size = increaseReceiveBufferTo(*env, rtp_sock_num, desired_buf_size);
		
		if(!context->rtsp_client->setupMediaSubsession(*subsession, false, false)) {
			lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to send setup command to subsesion");
			//lib::logger::get_logger()->error("RTSP Connection Failed");
			return NULL;
		}
	}
	
	return context;
		
}
timestamp_t
ambulant::net::rtsp_demux::get_clip_end()
{
	return m_clip_end;
}

void
ambulant::net::rtsp_demux::seek(timestamp_t time)
{
	
	m_clip_begin = time;
	m_clip_begin_set = false;
}

bool
ambulant::net::rtsp_demux::set_position(timestamp_t time)
{
	float time_sec;
	
	time_sec = time / 1000000.0;
	MediaSubsession* subsession;
	MediaSubsessionIterator iter(*m_context->media_session);
	while (( subsession = iter.next() ) != NULL) {
		m_context->rtsp_client->playMediaSubsession(*subsession, time_sec);
	}
}


unsigned long 
ambulant::net::rtsp_demux::run() 
{
	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() called (%d)", m_context->need_audio);
	m_context->blocking_flag = 0;
	if (m_context->media_session) {
		if(!m_context->rtsp_client->playMediaSession(*m_context->media_session)) {
			lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() play failed");
			lib::logger::get_logger()->error("playing RTSP connection failed");
			return 1;
		}
	} else {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() m_context->media_session = NULL RTSP play failed !");
		lib::logger::get_logger()->error("playing RTSP connection failed");
		return 1;
	}
	
	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() starting the loop ");
	

	while(!m_context->eof) {
		if (!m_clip_begin_set) {
			set_position(m_clip_begin);
			m_clip_begin_set = true;
		}
		MediaSubsession* subsession;
		MediaSubsessionIterator iter(*m_context->media_session);
		// Only audio/video session need to apply for a job !
		while ((subsession = iter.next()) != NULL) {
			if (strcmp(subsession->mediumName(), "audio") == 0) {
				if(m_context->need_audio) {
					assert(!m_context->audio_packet);
					m_context->audio_packet = (unsigned char*) malloc(MAX_RTP_FRAME_SIZE);
					AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() Calling getNextFrame for an audio frame");
					m_context->need_audio = false;
					subsession->readSource()->getNextFrame(m_context->audio_packet, MAX_RTP_FRAME_SIZE, after_reading_audio, m_context,  on_source_close ,m_context);
				}
			} else if (strcmp(subsession->mediumName(), "video") == 0) {
				if (m_context->need_video) {
					assert(!m_context->video_packet);
					m_context->video_packet = (unsigned char*) malloc(MAX_RTP_FRAME_SIZE);
					//std::cout << " MAX_RTP_FRAME_SIZE = " << MAX_RTP_FRAME_SIZE;
					AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() Calling getNextFrame for an video frame");
					m_context->need_video = false;
					AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() video_packet 0x%x", m_context->video_packet);
					subsession->readSource()->getNextFrame(m_context->video_packet, MAX_RTP_FRAME_SIZE, after_reading_video, m_context, on_source_close,m_context);
				}
			} else {
				AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() not interested in this data");
			}
		}
		
		AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() blocking_flag: 0x%x, %d, need_audio %d", &m_context->blocking_flag, m_context->blocking_flag, m_context->need_audio);		
		TaskScheduler& scheduler = m_context->media_session->envir().taskScheduler();
		scheduler.doEventLoop(&m_context->blocking_flag);
		m_context->blocking_flag = 0;
	}
	
}

static void 
after_reading_audio(void* data, unsigned sz, unsigned truncated, struct timeval pts, unsigned duration)
{

	AM_DBG lib::logger::get_logger()->debug("after_reading_audio: called data = 0x%x, sz = %d, truncated = %d", data, sz, truncated);
	rtsp_context_t* context = NULL;
	if (data) {
		context = (rtsp_context_t*) data;
	} 
	AM_DBG lib::logger::get_logger()->debug("after_reading_audio: audio data available (client data: 0x%x", data);
	if (context) {
		timestamp_t rpts = (pts.tv_sec* 1000000 )+  pts.tv_usec;
		if(context->sinks[context->audio_stream]) {
			AM_DBG lib::logger::get_logger()->debug("after_reading_audio: calling data_avail");
			context->sinks[context->audio_stream]->data_avail(rpts, (uint8_t*) context->audio_packet, sz);
			AM_DBG lib::logger::get_logger()->debug("after_reading_audio: calling data_avail done");
		}
		assert (context->audio_packet);
		free(context->audio_packet);
		context->audio_packet = NULL;
		//XXX Do we need to free data here ?
	}
	context->blocking_flag = ~0;
	context->need_audio = true;
}	

static void 
after_reading_video(void* data, unsigned sz, unsigned truncated, struct timeval pts, unsigned duration)
{
	AM_DBG lib::logger::get_logger()->debug("after_reading_video: called data = 0x%x, sz = %d, truncated = %d", data, sz, truncated);
	rtsp_context_t* context = (rtsp_context_t*) data;
	if (context) {
		if (context->first_sync_time.tv_sec == 0 && context->first_sync_time.tv_usec == 0 ) {
			context->first_sync_time.tv_sec = pts.tv_sec;
			context->first_sync_time.tv_usec = pts.tv_usec; 
		}
		timestamp_t rpts =  (pts.tv_sec - context->first_sync_time.tv_sec) * 1000000  +  (timestamp_t) (pts.tv_usec - context->first_sync_time.tv_usec);
		AM_DBG lib::logger::get_logger()->debug("after_reading_audio: called timestamp %lld, sec = %d, usec =  %d", rpts, pts.tv_sec, pts.tv_usec);
		
		
		//~ if (rpts == context->last_pts) {
			//~ context->video_buffer = (unsigned char*) realloc(context->video_buffer, context->video_buffer_size + sz);
			//~ if (context->video_buffer) {
				//~ if (sz > 0)
					//~ memcpy((context->video_buffer + context->video_buffer_size), context->video_packet, sz);
				//~ context->video_buffer_size += sz;
				//~ lib::logger::get_logger()->debug("after_reading_video: stored !! (I)(sz = %d, buf_sz = %d", sz, context->video_buffer_size);
			//~ } else {
				//~ lib::logger::get_logger()->debug("after_reading_video: Out of memory (buf_sz = %d", context->video_buffer_size);
			//~ }
		
		//~ } else {	
			//~ if(context->video_buffer) {			
				//~ if(context->sinks[context->video_stream]) 
					//~ context->sinks[context->video_stream]->data_avail(rpts, (uint8_t*) context->video_buffer , context->video_buffer_size);
			//~ }
			//~ if (context->video_buffer)
				//~ free(context->video_buffer);
			//~ context->last_pts = rpts;
			//~ context->video_buffer = NULL;
			//~ context->video_buffer_size = 0;
			
			//~ if (sz > 0 )
				//~ context->video_buffer = (unsigned char*) malloc(sz);
		
			//~ if (context->video_buffer) {
				//~ memcpy(context->video_buffer + context->video_buffer_size, context->video_packet, sz);
				//~ context->video_buffer_size += sz;
				//~ lib::logger::get_logger()->debug("after_reading_video: stored !! (II)(sz = %d, buf_sz = %d", sz, context->video_buffer_size);
			//~ } else {
				//~ lib::logger::get_logger()->debug("after_reading_video: Out of memory (buf_sz = %d", context->video_buffer_size);
			//~ }
		//~ }
		
		 if(context->video_packet) {			
			if(context->sinks[context->video_stream]) 
				context->sinks[context->video_stream]->data_avail(rpts, (uint8_t*) context->video_packet , sz);
		}
		context->need_video = true;
		assert(context->video_packet);
		free(context->video_packet);
		context->video_packet  = NULL;
		context->blocking_flag = ~0;
		//XXX Do we need to free data here ?
	}
}

static void 
on_source_close(void* data) 
{
	rtsp_context_t* context = (rtsp_context_t*) data;
	if (context) {
		context->eof = true;
		context->blocking_flag = ~0;
	}
}
