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

using namespace ambulant;
using namespace net;

#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)


ambulant::net::rtsp_demux::rtsp_demux(rtsp_context_t* context)
:	m_context(context)
{
}

rtsp_context_t*
ambulant::net::rtsp_demux::supported(const net::url& url) 
{
	rtsp_context_t* context = new rtsp_context_t;
	context->rtsp_client = NULL;
	context->media_session = NULL;
	context->sdp = NULL;
	context->audio_stream = -1;
	context->video_stream = -1;
	context->nstream = 0;
	context->blocking_flag = 0;
	context->audio_packet = NULL;
	context->video_packet = NULL;
	
	
	// setup the basics.
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	if (!scheduler) {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to create scheduler");
	    lib::logger::get_logger()->error("RTSP Connection Failed");		
		return NULL;
	}
	
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
	if (!env) {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to create UsageEnvironment");
		lib::logger::get_logger()->error("RTSP Connection Failed");		
		return NULL;
	}
	// setup a rtp session
	int verbose;
	context->rtsp_client = RTSPClient::createNew(*env, verbose, "AmbulantPlayer");
	if (!context->rtsp_client) {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to create  a RTSP Client");
		lib::logger::get_logger()->error("RTSP Connection Failed");		
		return NULL;
	}
	
	const char* ch_url = url.get_url().c_str();
	if (ch_url) {
		context->sdp = context->rtsp_client->describeURL(ch_url);
		if (!context->sdp) {
			lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to get dsp description from rtsp server");
			lib::logger::get_logger()->error("RTSP Connection Failed");		
			return NULL;
		}
	} else {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to get url");
		lib::logger::get_logger()->error("Wrong URL !");		
		return NULL;
	}
	
	context->media_session = MediaSession::createNew(*env, context->sdp);
	if (!context->media_session) {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to create  a MediaSession");
		lib::logger::get_logger()->error("RTSP Connection Failed");		
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
			}
		} else if (strcmp(subsession->mediumName(), "video") == 0) {
			desired_buf_size = 200000;
			if (context->video_stream < 0) {
				context->video_stream = context->nstream;
			}
		}
		context->nstream++;
		if (!subsession->initiate()) {
			lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to initiate subsession");
			lib::logger::get_logger()->error("RTSP Connection Failed");
			return NULL;
		}
		
		int rtp_sock_num = subsession->rtpSource()->RTPgs()->socketNum();
		int buf_size = increaseReceiveBufferTo(*env, rtp_sock_num, desired_buf_size);
		
		if(!context->rtsp_client->setupMediaSubsession(*subsession, false, false)) {
			lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) failed to send setup command to subsesion");
			lib::logger::get_logger()->error("RTSP Connection Failed");
			return NULL;
		}
	}
		
}

unsigned long 
ambulant::net::rtsp_demux::run() 
{
	m_context->blocking_flag = 0;
	if(!m_context->rtsp_client->playMediaSession(*m_context->media_session)) {
		lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url) play failed");
		lib::logger::get_logger()->error("playing RTSP connection failed");
		return 1;
	}
	
	while(!m_context->eof) {
	MediaSubsession* subsession;
	MediaSubsessionIterator iter(*m_context->media_session);
	// Only audio/video session need to apply for a job !
	while ((subsession = iter.next()) != NULL) {
		if (strcmp(subsession->mediumName(), "audio") == 0) {
			subsession->readSource()->getNextFrame(m_context->audio_packet, MAX_RTP_FRAME_SIZE, after_reading_audio, m_context,  on_source_close ,NULL);
		} else if (strcmp(subsession->mediumName(), "video") == 0) {
			subsession->readSource()->getNextFrame(m_context->video_packet, MAX_RTP_FRAME_SIZE, after_reading_video, m_context, on_source_close,NULL);
		}
	}
	TaskScheduler& scheduler = m_context->media_session->envir().taskScheduler();
	scheduler.doEventLoop(m_context->blocking_flag);
	}
}

static void 
after_reading_audio(void* data, unsigned sz, unsigned truncated, struct timeval pts, unsigned duration)
{
	rtsp_context_t* context = (rtsp_context_t*) data;
	if (context) {
		double rpts = pts.tv_sec +  (pts.tv_usec / 1000000.0);
		if(context->sinks[context->audio_stream])
			context->sinks[context->audio_stream]->data_avail(rpts, (uint8_t*) data, sz);
		context->blocking_flag = "1";
		//XXX Do we need to free data here ?
	}
}	

static void 
after_reading_video(void* data, unsigned sz, unsigned truncated, struct timeval pts, unsigned duration)
{
	rtsp_context_t* context = (rtsp_context_t*) data;
	if (context) {
		double rpts = pts.tv_sec +  (pts.tv_usec / 1000000.0);
		if(context->sinks[context->video_stream]) 
			context->sinks[context->video_stream]->data_avail(rpts, (uint8_t*) data, sz);
		context->blocking_flag = "1";
		//XXX Do we need to free data here ?
	}
}

static void 
on_source_close(void* data) 
{
	rtsp_context_t* context = (rtsp_context_t*) data;
	if (context) {
		context->eof = true;
		context->blocking_flag = "1";
	}
}

//====================================== proof of concept ffmpeg datasources ==================
live_ffmpeg_audio_datasource *
live_ffmpeg_audio_datasource::new_live_ffmpeg_audio_datasource(
  		const net::url& url, 
  		AVFormatContext *context,
		rtsp_demux *thread)
{
	AVCodec *codec;
	AVCodecContext *codeccontext;
	int stream_index;
	
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::new_live_ffmpeg_audio_datasource()");
	ffmpeg_init();
	// Find the index of the audio stream
	for (stream_index=0; stream_index < context->nb_streams; stream_index++) {
		if (context->streams[stream_index]->codec.codec_type == CODEC_TYPE_AUDIO)
			break;
	}
	
	
	if (stream_index >= context->nb_streams) {
		lib::logger::get_logger()->error(gettext("%s: no more audio streams"), url.get_url().c_str());
		return NULL;
	} 

	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::live_new_ffmpeg_audio_datasource() looking for the right codec");
	codeccontext = &context->streams[stream_index]->codec; 
	codec = avcodec_find_decoder(codeccontext->codec_id);
	
	if( !codec) {
		lib::logger::get_logger()->error(gettext("%s: Audio codec %d(%s) not supported"), repr(url).c_str(), codeccontext->codec_id, codeccontext->codec_name);
		return NULL;
	} else {
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::new_live_ffmpeg_audio_datasource(): codec found!");
	}

	
	if((!codec) || (avcodec_open(codeccontext,codec) < 0) ) {
		lib::logger::get_logger()->error(gettext("%s: Cannot open audio codec %d(%s)"), repr(url).c_str(), codeccontext->codec_id, codeccontext->codec_name);
		return NULL;
	} else {
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::new_live_ffmpeg_audio_datasource(): succesfully opened codec");
	}
	
	return new live_ffmpeg_audio_datasource(url, context, thread, stream_index);
}

live_ffmpeg_audio_datasource::live_ffmpeg_audio_datasource(const net::url& url, AVFormatContext *context,
	detail::ffmpeg_demux *thread, int stream_index)
:	m_url(url),
	m_con(context),
	m_stream_index(stream_index),
	m_fmt(audio_format("ffmpeg")),
	m_src_end_of_file(false),
	m_event_processor(NULL),
	m_thread(thread),
	m_client_callback(NULL)
{	
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::live_ffmpeg_audio_datasource: rate=%d, channels=%d", context->streams[m_stream_index]->codec.sample_rate, context->streams[m_stream_index]->codec.channels);
	m_fmt.parameters = (void *)&context->streams[m_stream_index]->codec;
	m_thread->add_datasink(this, stream_index);
}

live_ffmpeg_audio_datasource::~live_ffmpeg_audio_datasource()
{
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::~live_ffmpeg_audio_datasource(0x%x)", (void*)this);
	stop();
}

void
live_ffmpeg_audio_datasource::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::stop(0x%x)", (void*)this);
	if (m_thread) {
		detail::ffmpeg_demux *tmpthread = m_thread;
		m_thread = NULL;
		m_lock.leave();
		tmpthread->remove_datasink(m_stream_index);
		m_lock.enter();
	}
	m_thread = NULL;
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::stop: thread stopped");
	//if (m_con) delete m_con;
	m_con = NULL; // owned by the thread
	if (m_client_callback) delete m_client_callback;
	m_client_callback = NULL;
	m_lock.leave();
}	

void 
live_ffmpeg_audio_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	
	if (m_client_callback != NULL) {
		delete m_client_callback;
		m_client_callback = NULL;
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::start(): m_client_callback already set!");
	}
	if (m_buffer.buffer_not_empty() || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (callbackk) {
			assert(evp);
			AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::start: trigger client callback");
			evp->add_event(callbackk, 0, ambulant::lib::event_processor::med);
		} else {
			lib::logger::get_logger()->debug("Internal error: live_ffmpeg_audio_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		m_client_callback = callbackk;
		m_event_processor = evp;
	}
	m_lock.leave();
}
 
void 
live_ffmpeg_audio_datasource::readdone(int len)
{
	m_lock.enter();
	m_buffer.readdone(len);
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource.readdone : done with %d bytes", len);
//	restart_input();
	m_lock.leave();
}

void 
live_ffmpeg_audio_datasource::data_avail(int64_t pts, uint8_t *inbuf, int sz)
{
	// XXX timestamp is ignored, for now
	m_lock.enter();
	m_src_end_of_file = (sz == 0);
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource.data_avail: %d bytes available", sz);
	if(sz && !m_buffer.buffer_full()){
		uint8_t *outbuf = (uint8_t*) m_buffer.get_write_ptr(sz);
		if (outbuf) {
			memcpy(outbuf, inbuf, sz);
			m_buffer.pushdata(sz);
			// XXX m_src->readdone(sz);
		} else {
			lib::logger::get_logger()->debug("Internal error: live_ffmpeg_audio_datasource::data_avail: no room in output buffer");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
			m_buffer.pushdata(0);
		}
	}

	if ( m_client_callback && (m_buffer.buffer_not_empty() || m_src_end_of_file ) ) {
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::data_avail(): calling client callback (%d, %d)", m_buffer.size(), m_src_end_of_file);
		assert(m_event_processor);
		m_event_processor->add_event(m_client_callback, 0, ambulant::lib::event_processor::med);
		m_client_callback = NULL;
		//m_event_processor = NULL;
	} else {
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::data_avail(): No client callback!");
	}
	m_lock.leave();
}


bool 
live_ffmpeg_audio_datasource::end_of_file()
{
	m_lock.enter();
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool 
live_ffmpeg_audio_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_buffer.buffer_not_empty()) return false;
	return m_src_end_of_file;
}

bool 
live_ffmpeg_audio_datasource::buffer_full()
{
	m_lock.enter();
	bool rv = m_buffer.buffer_full();
	m_lock.leave();
	return rv;
}	

char* 
live_ffmpeg_audio_datasource::get_read_ptr()
{
	m_lock.enter();
	char *rv = m_buffer.get_read_ptr();
	m_lock.leave();
	return rv;
}

int 
live_ffmpeg_audio_datasource::size() const
{
		return m_buffer.size();
}	


audio_format&
live_ffmpeg_audio_datasource::get_audio_format()
{
#if 0
	if (m_con) {
		// Refresh info on audio format
		m_fmt.samplerate = m_con->sample_rate;
		m_fmt.bits = 16; // XXXX
		m_fmt.channels = m_con->channels;
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::select_decoder: rate=%d, bits=%d,channels=%d",m_fmt.samplerate, m_fmt.bits, m_fmt.channels);
	}
#endif
	return m_fmt;
}

std::pair<bool, double>
live_ffmpeg_audio_datasource::get_dur()
{
	std::pair<bool, double> rv(false, 0.0);
	m_lock.enter();
	if (m_con && m_con->duration >= 0) {
		rv = std::pair<bool, double>(true, m_con->duration / (double)AV_TIME_BASE);
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::get_dur: duration=%f", rv.second);
	}
	m_lock.leave();
	return rv;
}

// **************************** ffmpeg_video_datasource *****************************




live_ffmpeg_video_datasource *
live_ffmpeg_video_datasource::new_live_ffmpeg_video_datasource(const net::url& url, AVFormatContext *context, detail::ffmpeg_demux *thread)
{
	AVCodec *codec;
	AVCodecContext *codeccontext;
	int stream_index;
	
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::new_live_ffmpeg_video_datasource() called");
	
	if (!thread) {
		lib::logger::get_logger()->error(gettext("live_ffmpeg_video_datasource::new_live_ffmpeg_video_datasource(): Cannot start video reader thread"));
		return NULL;
	}

	ffmpeg_init();

	// Find the index of the video stream
	for (stream_index=0; stream_index < context->nb_streams; stream_index++) {
		if (context) {
			if (context->streams[stream_index]->codec.codec_type == CODEC_TYPE_VIDEO)
				break;
		} else {
			lib::logger::get_logger()->debug(gettext("live_ffmpeg_video_datasource::new_ffmpeg_video_datasource(): AVFormatContext is NULL "));
			return NULL;
		}
	}
	if (stream_index >= context->nb_streams) {
		lib::logger::get_logger()->error(gettext("%s: no video streams in file"), url.get_url().c_str());
		return NULL;
	}
	
	codeccontext = &context->streams[stream_index]->codec; 
	codec = avcodec_find_decoder(codeccontext->codec_id);
	
	if( !codec) {
		lib::logger::get_logger()->error(gettext("%s: Video codec %d(%s) not supported"), repr(url).c_str(), codeccontext->codec_id, codeccontext->codec_name);
		return NULL;
	}
	
	if((!codec) || (avcodec_open(codeccontext,codec) < 0) ) {
		lib::logger::get_logger()->error(gettext("%s: Video codec %d(%s): cannot open"), repr(url).c_str(), codeccontext->codec_id, codeccontext->codec_name);
		return NULL;
	}

	return new live_ffmpeg_video_datasource(url, context, thread, stream_index);
}

live_ffmpeg_video_datasource::live_ffmpeg_video_datasource(const net::url& url, AVFormatContext *context,
	detail::ffmpeg_demux *thread, int stream_index)
:	m_url(url),
	m_con(context),
	m_stream_index(stream_index),
	m_src_end_of_file(false),
	m_event_processor(NULL),
	m_thread(thread),
	m_client_callback(NULL),
    m_thread_started(false),
	m_pts_last_frame(0.0),
	m_last_p_pts(0.0)
{	
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::ffmpeg_video_datasource() (this = 0x%x)", (void*)this);
	m_thread->add_datasink(this, m_stream_index);
	
}

live_ffmpeg_video_datasource::~live_ffmpeg_video_datasource()
{
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::~live_ffmpeg_video_datasource(0x%x)", (void*)this);
	stop();
}

void
live_ffmpeg_video_datasource::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::stop(0x%x)", (void*)this);
	if (m_thread) {
		detail::ffmpeg_demux *tmpthread = m_thread;
		m_thread = NULL;
		m_lock.leave();
		tmpthread->remove_datasink(m_stream_index);
		m_lock.enter();
	}
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::stop: thread stopped");
	//if (m_con) delete m_con;
	m_con = NULL; // owned by the thread
	if (m_client_callback) delete m_client_callback;
	m_client_callback = NULL;
	// And delete any frames left
	while ( m_frames.size() > 0 ) {
		std::pair<double, char*> element = m_frames.front();
		free(element.second);
		m_frames.pop();
	}
	if (m_old_frame.second) {
		free(m_old_frame.second);
		m_old_frame.second = NULL;
	}
	m_lock.leave();
}	

bool
live_ffmpeg_video_datasource::has_audio()
{		
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::has_audio");

	if (get_audio_stream_nr() >= 0) {
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::has_audio TRUE");
		return true;
	}
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::has_audio FALSE");

	return false;	
}

audio_datasource *
live_ffmpeg_video_datasource::get_audio_datasource()
{	
	AVCodec *codec;
	AVCodecContext *codeccontext;
	net::audio_datasource *audio_ds;
	
	int stream_index = get_audio_stream_nr();

	if (stream_index < 0 ) 
		return NULL;
	
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::get_audio_stream_nr() looking for the right codec");
	codeccontext = &m_con->streams[stream_index]->codec; 
	codec = avcodec_find_decoder(codeccontext->codec_id);
	
	if( !codec) {
		lib::logger::get_logger()->error(gettext("%s: Audio codec %d(%s): not supported"), repr(m_url).c_str(), codeccontext->codec_id, codeccontext->codec_name);
		return NULL;
	} else {
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::get_audio_stream_nr(): codec found !");
	}

	if((!codec) || (avcodec_open(codeccontext,codec) < 0) ) {
		lib::logger::get_logger()->error(gettext("%s: Audio codec %d(%s): cannot open"), repr(m_url).c_str(), codeccontext->codec_id, codeccontext->codec_name);
		return NULL;
	} else {
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::get_audio_stream_nr(): succesfully opened codec");
	}
	
	audio_ds = new live_ffmpeg_audio_datasource(m_url, m_con, m_thread, stream_index);
	
	return new ffmpeg_decoder_datasource(audio_ds);
	
}

void 
live_ffmpeg_video_datasource::start_frame(ambulant::lib::event_processor *evp, 
	ambulant::lib::event *callbackk, double timestamp)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::start_frame: (this = 0x%x)", (void*) this);

	if (m_client_callback != NULL) {
		delete m_client_callback;
		m_client_callback = NULL;
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::start(): m_client_callback already set!");
	}
	if (m_frames.size() > 0 /* XXXX Check timestamp! */ || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (callbackk) {
			assert(evp);
			AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::start: trigger client callback");
			evp->add_event(callbackk, 0, ambulant::lib::event_processor::med);
		} else {
			lib::logger::get_logger()->debug("Internal error: live_ffmpeg_video_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		m_client_callback = callbackk;
		m_event_processor = evp;
	}
	if (!m_thread_started) {
		m_thread->start();
		m_thread_started = true;
	}
	m_lock.leave();
}
 
void 
live_ffmpeg_video_datasource::frame_done(double timestamp, bool keepdata)
{
	m_lock.enter();
	if (m_frames.size() == 0) {
		lib::logger::get_logger()->debug("Internal error: live_ffmpeg_video_datasource.readdone: frame_done() called with no current frames");
		lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource.frame_done(%f)", timestamp);
	while( m_frames.size() > 0 ) {
		std::pair<double, char*> element = m_frames.front();
		if (element.first > timestamp)
			break;
		if (m_old_frame.second) {
			free(m_old_frame.second);
			m_old_frame.second = NULL;
		}
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::frame_done(%f): removing frame with ts=%f", timestamp, element.first);
		m_old_frame = element;
		m_frames.pop();
		if (!keepdata) {
			free(m_old_frame.second);
			m_old_frame.second = NULL;
		}
	}
	m_lock.leave();
}


int 
live_ffmpeg_video_datasource::width()
{
	return m_con->streams[m_stream_index]->codec.width;
}

int 
live_ffmpeg_video_datasource::height()
{
	return m_con->streams[m_stream_index]->codec.height;
}

//~ #undef AM_DBG
//~ #define AM_DBG
void 
live_ffmpeg_video_datasource::data_avail(int64_t ipts, uint8_t *inbuf, int sz)
{
	m_lock.enter();
	int got_pic;
	AVFrame *frame = avcodec_alloc_frame();
	AVPicture picture;
	int len, dummy2;
	int pic_fmt, dst_pic_fmt;
	int width,height;
	int num, den;
	int framerate;
	int framebase;
	double pts, pts1;
    unsigned char* ptr;
	double frame_delay;
	
	got_pic = 0;
	
	m_src_end_of_file = (sz == 0);
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource.data_avail: %d bytes available", sz);
	if(sz) {	
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource.data_avail:start decoding (0x%x) ", m_con->streams[m_stream_index]->codec);
		assert(&m_con->streams[m_stream_index]->codec != NULL);
		ptr = inbuf;
		
		while (sz > 0) {
				/*AM_DBG*/ lib::logger::get_logger()->debug("live_ffmpeg_video_datasource.data_avail: decoding picture(s),  %d byteas of data ", sz);
				len = avcodec_decode_video(&m_con->streams[m_stream_index]->codec, frame, &got_pic, ptr, sz);	
				if (len >= 0) {
					assert(len <= sz);
					ptr +=len;	
					sz -= len;
					if (got_pic) {
						/*AM_DBG*/ lib::logger::get_logger()->debug("live_ffmpeg_video_datasource.data_avail: decoded picture, used %d bytes, %d left", len, sz);
						// Setup the AVPicture for the format we want, plus the data pointer
						width = m_con->streams[m_stream_index]->codec.width;
						height = m_con->streams[m_stream_index]->codec.height;
						m_size = width * height * 4;
						char *framedata = (char*) malloc(m_size);
						assert(framedata != NULL);
						dst_pic_fmt = PIX_FMT_RGBA32;
						dummy2 = avpicture_fill(&picture, (uint8_t*) framedata, dst_pic_fmt, width, height);
						// The format we have is already in frame. Convert.
						pic_fmt = m_con->streams[m_stream_index]->codec.pix_fmt;
						img_convert(&picture, dst_pic_fmt, (AVPicture*) frame, pic_fmt, width, height);
						
						// And convert the timestamp
#ifdef	WITH_FFMPEG_0_4_9					
						num = 0;
						den = 0;
#else /*WITH_FFMPEG_0_4_9*/
						num = m_con->pts_num;
						den = m_con->pts_den;
#endif/*WITH_FFMPEG_0_4_9*/
						framerate = m_con->streams[m_stream_index]->codec.frame_rate;
						framebase = m_con->streams[m_stream_index]->codec.frame_rate_base;
					
					
						pts = 0;
						
						if (ipts != AV_NOPTS_VALUE) {
#ifdef	WITH_FFMPEG_0_4_9					
							pts = (double) ipts / AV_TIME_BASE;							
#else /*WITH_FFMPEG_0_4_9*/							
							pts = (double) ipts * ((double) num)/den;
#endif/*WITH_FFMPEG_0_4_9*/
						}
						AM_DBG lib::logger::get_logger()->debug("pts seems to be : %f",pts);
						pts1= pts;
						
						if (m_con->streams[m_stream_index]->codec.has_b_frames && frame->pict_type != FF_B_TYPE) {
							pts = m_last_p_pts;
							m_last_p_pts = pts1;
							AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource.data_avail:frame has B frames but this frame is no B frame  (this=0x%x) ", this);
							AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource.data_avail:pts set to %f, remember %f", pts, m_last_p_pts);
						}
						
						if (pts != 0 ) {
							m_pts_last_frame = pts;
						} else {
							if (framerate != 0) {
								frame_delay = (double) framebase/framerate;
							} else {
								frame_delay = 0;
							}
							pts = m_pts_last_frame + frame_delay;
							m_pts_last_frame = pts;			
							//~ if( frame.repeat_pict) {
								//~ pts += frame.repeat_pict * (frame_delay * 0.5);
							//~ }
							AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource.data_avail:pts was 0, set to %f", pts);
						}
						
						AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource.data_avail: timestamp=%lld num=%d, den=%d",pts, num,den);
						
						AM_DBG {
							switch(frame->pict_type) {
								case FF_B_TYPE:
									lib::logger::get_logger()->debug("BBBBB live_ffmpeg_video_datasource.data_avail: B-frame, timestamp = %f", pts); 
									break;
								case FF_P_TYPE:framebase/framerate;
									lib::logger::get_logger()->debug("PPPPP live_ffmpeg_video_datasource.data_avail: P-frame, timestamp = %f", pts); 
									break;
								case FF_I_TYPE:
									lib::logger::get_logger()->debug("IIIII live_ffmpeg_video_datasource.data_avail: I-frame, timestamp = %f", pts); 
									break;
								default:
									lib::logger::get_logger()->debug("live_ffmpeg_video_datasource.data_avail: I-frame, timestamp = %f", pts); 
							}
						}
						// And store the data.
						std::pair<double, char*> element(pts, framedata);
						m_frames.push(element);
					} else {
						AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource.data_avail: incomplete picture, used %d bytes, %d left", len, sz);
					}
				} else {
						lib::logger::get_logger()->error(gettext("%s: error decoding video frame"), m_url.get_url().c_str());
					}
		}
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource.data_avail:done decoding (0x%x) ", m_con->streams[m_stream_index]->codec);

  	}
	if ( m_frames.size() || m_src_end_of_file  ) {
	  if ( m_client_callback ) {
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::data_avail(): calling client callback (eof=%d)", m_src_end_of_file);
		assert(m_event_processor);
		m_event_processor->add_event(m_client_callback, 0, ambulant::lib::event_processor::med);
		m_client_callback = NULL;
		//m_event_processor = NULL;
	  } else {
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::data_avail(): No client callback!");
	  }
  	}
	av_free(frame);
	m_lock.leave();
}

bool 
live_ffmpeg_video_datasource::end_of_file()
{
	m_lock.enter();
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool 
live_ffmpeg_video_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_frames.size() > 0) return false;
	return m_src_end_of_file;
}

bool 
live_ffmpeg_video_datasource::buffer_full()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::buffer_full() (this=0x%x, count=%d)", (void*) this, m_frames.size());
	bool rv = (m_frames.size() > MAX_VIDEO_FRAMES);
	m_lock.leave();
	return rv;
}	


int 
live_ffmpeg_video_datasource::get_audio_stream_nr()
{

	int stream_index;
	
	ffmpeg_init();
	// Find the index of the audio stream
	for (stream_index=0; stream_index < m_con->nb_streams; stream_index++) {
		if (m_con->streams[stream_index]->codec.codec_type == CODEC_TYPE_AUDIO)
			break;
	}
	
	if (stream_index >= m_con->nb_streams) {
		AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::get_audio_stream_nr(): no audio streams");
		return -1;
	} 
	return stream_index;
}




char* 
live_ffmpeg_video_datasource::get_frame(double *timestamp, int *size)
{
	m_lock.enter();
	if( m_frames.size() > 0 ) {
	//assert(m_frames.size() > 0);
	std::pair<double, char*> element = m_frames.front();
	char *rv = element.second;
	*timestamp = element.first;
	*size = m_size;
	m_lock.leave();
	return rv;
	}
	
	return NULL;
}


std::pair<bool, double>
live_ffmpeg_video_datasource::get_dur()
{
	std::pair<bool, double> rv(false, 0.0);
	m_lock.enter();
	if (m_con && m_con->duration >= 0) {
		rv = std::pair<bool, double>(true, m_con->duration / (double)AV_TIME_BASE);
		lib::logger::get_logger()->debug("live_ffmpeg_video_datasource::get_dur: duration=%f", rv.second);
	}
	m_lock.leave();
	return rv;
}

#endif // WITH_FFMPEG_AVFORMAT
