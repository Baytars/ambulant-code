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

datasource* 
rtsp_datasource_factory::new_video_datasource(const net::url& url)
{
#ifdef WITH_FFMPEG_AVFORMAT
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource(%s)", repr(url).c_str());
	AVFormatContext *context = detail::ffmpeg_demux::supported(url);
	if (!context) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource: no support for %s", repr(url).c_str());
		return NULL;
	}
	detail::ffmpeg_demux *thread = new detail::ffmpeg_demux(context);
	video_datasource *ds = ffmpeg_video_datasource::new_ffmpeg_video_datasource(url, context, thread);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource (ds = 0x%x)", (void*) ds);

	if (ds == NULL) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource: could not allocate ffmpeg_video_datasource");
		thread->cancel();
		return NULL;
	}
	//thread->start();
	return ds;
#else
	return NULL;	
#endif // WITH_FFMPEG_AVFORMAT
}

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

audio_datasource* 
live_audio_datasource_factory::new_audio_datasource(const net::url& url, audio_format_choices fmts)
{
#ifdef WITH_FFMPEG_AVFORMAT
	
	AM_DBG lib::logger::get_logger()->debug("live_audio_datasource_factory::new_audio_datasource(%s)", repr(url).c_str());
	rtsp_context_t *context = rtsp_demux::supported(url);
	if (!context) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: no support for %s", repr(url).c_str());
		return NULL;
	}
	detail::ffmpeg_demux *thread = new detail::ffmpeg_demux(context);
	audio_datasource *ds = ffmpeg_audio_datasource::new_live_audio_datasource(url, context, thread);
	if (ds == NULL) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource: could not allocate ffmpeg_video_datasource");
		thread->cancel();
		return NULL;
	}
	thread->start();
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: parser ds = 0x%x", (void*)ds);
	// XXXX This code should become generalized in datasource_factory
	if (fmts.contains(ds->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: matches!");
		return ds;
	}
	audio_datasource *dds = new ffmpeg_decoder_datasource(ds);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: decoder ds = 0x%x", (void*)dds);
	if (dds == NULL) {
		int rem = ds->release();
		assert(rem == 0);
		return NULL;
	}
	if (fmts.contains(dds->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: matches!");
		return dds;
	}
	audio_datasource *rds = new ffmpeg_resample_datasource(dds, fmts);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: resample ds = 0x%x", (void*)rds);
	if (rds == NULL)  {
		int rem = dds->release();
		assert(rem == 0);
		return NULL;
	}
	if (fmts.contains(rds->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: matches!");
		return rds;
	}
	lib::logger::get_logger()->error(gettext("%s: unable to create audio resampler"));
	int rem = rds->release();
	assert(rem == 0);
#endif // WITH_FFMPEG_AVFORMAT
	return NULL;	
}


live_ffmpeg_audio_datasource *
live_ffmpeg_audio_datasource::new_live_ffmpeg_audio_datasource(
  		const net::url& url, 
  		rtsp_context_t *context,
		rtsp_demux *thread)
{
	int i = 0;
	char* codec_name;
	if (context->audio_stream) {
		while ((subsession = iter.next()) != NULL) {
			if (i == context->audio_stream) {
				codec_name = subsession->codecName()
			}
		i++	
		}
	} else {
		lib::logger::get_logger()->error(gettext("%s: no more audio streams"), url.get_url().c_str());
		return NULL;
	}
	
	AM_DBG lib::logger::get_logger()->debuf("live_audio_datasource::new_live_audio_datasource(): audio codec : %s", codec_name);
	AVCodec *codec = avcodec_find_decoder_by_name(codec_name);
	AVCodecContext *codeccontext = avcodec_alloc_context();
	int stream_index;
	
	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::new_live_ffmpeg_audio_datasource()");
	ffmpeg_init();
	// Find the index of the audio stream
	stream_index = context->audio_stream;


	AM_DBG lib::logger::get_logger()->debug("live_ffmpeg_audio_datasource::live_new_ffmpeg_audio_datasource() looking for the right codec");
	
	
	
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



#endif // WITH_FFMPEG_AVFORMAT
