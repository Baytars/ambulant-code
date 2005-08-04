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
 
//#include <math.h>
//#include <map>

#include "ambulant/config/config.h"
#include "ambulant/net/demux_datasource.h"
#include "ambulant/net/ffmpeg_audio.h"

// Minimum delay
#define MIN_EVENT_DELAY 1

// How many video frames we would like to buffer at most
#define MAX_VIDEO_FRAMES 300

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif 

using namespace ambulant;
using namespace net;

// **************************** demux_audio_datasource ******************************

demux_audio_datasource *
demux_audio_datasource::new_demux_audio_datasource(
  		const net::url& url, 
		abstract_demux *thread)
{
	
	int stream_index;
	
	AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::new_demux_audio_datasource()");
	// Find the index of the audio stream
	stream_index = thread->audio_stream_nr();
	assert(stream_index >= 0);
	
	if (stream_index >= thread->nstreams()) {
		lib::logger::get_logger()->error(gettext("%s: no more audio streams"), url.get_url().c_str());
		return NULL;
	} 

	AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::new_demux_audio_datasource() looking for the right codec");
	
	return new demux_audio_datasource(url, thread, stream_index);
}

demux_audio_datasource::demux_audio_datasource(const net::url& url, abstract_demux *thread, int stream_index)
:	m_url(url),
	m_stream_index(stream_index),
//	m_fmt(audio_format("ffmpeg")),
	m_src_end_of_file(false),
	m_event_processor(NULL),
	m_thread(thread),
	m_client_callback(NULL)
{	
	//AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::demux_audio_datasource: rate=%d, channels=%d", context->streams[m_stream_index]->codec.sample_rate, context->streams[m_stream_index]->codec.channels);
	// XXX ignoring the codec for now but i'll have to look into this real soon
	//m_fmt.parameters = (void *)&context->streams[m_stream_index]->codec;
	m_thread->add_datasink(this, stream_index);
}

demux_audio_datasource::~demux_audio_datasource()
{
	AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::~demux_audio_datasource(0x%x)", (void*)this);
	stop();
}

void
demux_audio_datasource::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::stop(0x%x)", (void*)this);
	if (m_thread) {
		abstract_demux *tmpthread = m_thread;
		m_thread = NULL;
		m_lock.leave();
		tmpthread->remove_datasink(m_stream_index);
		m_lock.enter();
	}
	m_thread = NULL;
	AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::stop: thread stopped");
	//if (m_con) delete m_con;
	//m_con = NULL; // owned by the thread
	if (m_client_callback) delete m_client_callback;
	m_client_callback = NULL;
	m_lock.leave();
}	

void 
demux_audio_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	assert(m_thread);
	if (m_thread && !m_thread->is_running()) {
		m_thread->start();
	}
	if (m_client_callback != NULL) {
		delete m_client_callback;
		m_client_callback = NULL;
		AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::start(): m_client_callback already set!");
	}
	if (m_buffer.buffer_not_empty() || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (callbackk) {
			assert(evp);
			AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::start: trigger client callback");
			evp->add_event(callbackk, MIN_EVENT_DELAY, ambulant::lib::event_processor::med);
		} else {
			lib::logger::get_logger()->debug("Internal error: demux_audio_datasource::start(): no client callback!");
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
demux_audio_datasource::readdone(int len)
{
	m_lock.enter();
	m_buffer.readdone(len);
	AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource.readdone : done with %d bytes", len);
//	restart_input();
	m_lock.leave();
}

void 
demux_audio_datasource::seek(timestamp_t time)
{
	m_lock.enter();
	assert(m_thread);
	assert(m_thread->is_running()); // the thread should be running before we can seek it
	assert(m_thread);
	m_thread->seek(time);
	m_lock.leave();
}

void 
demux_audio_datasource::read_ahead(timestamp_t time)
{
	m_lock.enter();
	assert(m_thread);
	assert(!m_thread->is_running());
	assert(m_thread);
	
	m_thread->seek(time);
	m_thread->start();
	m_lock.leave();
}


void 
demux_audio_datasource::data_avail(timestamp_t pts, uint8_t *inbuf, int sz)
{
	// XXX timestamp is ignored, for now
	m_lock.enter();
	m_src_end_of_file = (sz == 0);
	AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource.data_avail: %d bytes available (ts = %lld)", sz, pts);
	if(sz && !m_buffer.buffer_full()){
		uint8_t *outbuf = (uint8_t*) m_buffer.get_write_ptr(sz);
		if (outbuf) {
			memcpy(outbuf, inbuf, sz);
			m_buffer.pushdata(sz);
			// XXX m_src->readdone(sz);
		} else {
			lib::logger::get_logger()->debug("Internal error: demux_audio_datasource::data_avail: no room in output buffer");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
			m_buffer.pushdata(0);
		}
	}

	if ( m_client_callback && (m_buffer.buffer_not_empty() || m_src_end_of_file ) ) {
		AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::data_avail(): calling client callback (%d, %d)", m_buffer.size(), m_src_end_of_file);
		assert(m_event_processor);
		m_event_processor->add_event(m_client_callback, MIN_EVENT_DELAY, ambulant::lib::event_processor::med);
		m_client_callback = NULL;
		//m_event_processor = NULL;
	} else {
		AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::data_avail(): No client callback!");
	}
	m_lock.leave();
}


bool 
demux_audio_datasource::end_of_file()
{
	m_lock.enter();
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool 
demux_audio_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_buffer.buffer_not_empty()) return false;
	return m_src_end_of_file;
}

bool 
demux_audio_datasource::buffer_full()
{
	m_lock.enter();
	bool rv = m_buffer.buffer_full();
	m_lock.leave();
	return rv;
}	

char* 
demux_audio_datasource::get_read_ptr()
{
	m_lock.enter();
	char *rv = m_buffer.get_read_ptr();
	m_lock.leave();
	return rv;
}

int 
demux_audio_datasource::size() const
{
	const_cast <demux_audio_datasource*>(this)->m_lock.enter();
	int rv = m_buffer.size();
	const_cast <demux_audio_datasource*>(this)->m_lock.leave();
	return rv;
}	

timestamp_t
demux_audio_datasource::get_clip_end()
{	
	m_lock.enter();
	assert(m_thread);
	timestamp_t clip_end = m_thread->get_clip_end();
	AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::get_clip_end: clip_end=%d", clip_end);
	m_lock.leave();
	return  clip_end;
}

timestamp_t
demux_audio_datasource::get_clip_begin()
{
	m_lock.enter();
	assert(m_thread);
	timestamp_t clip_begin = m_thread->get_clip_begin();
	AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::get_clip_begin: clip_begin=%d", clip_begin);
	m_lock.leave();
	return  clip_begin;
}

audio_format&
demux_audio_datasource::get_audio_format()
{
    assert(m_thread);
	return m_thread->get_audio_format();
}

common::duration
demux_audio_datasource::get_dur()
{
	common::duration rv(false, 0.0);
	m_lock.enter();
	assert(m_thread);
	if (m_thread->duration() >= 0) {
		rv = common::duration(true, m_thread->duration());
		AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::get_dur: duration=%f", rv.second);
	}
	m_lock.leave();
	return rv;
}

// **************************** demux_video_datasource *****************************

demux_video_datasource *
demux_video_datasource::new_demux_video_datasource(
  		const net::url& url, 
		abstract_demux *thread)
{
	
	int stream_index;
	
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::new_demux_video_datasource()");
	// Find the index of the audio stream
	stream_index = thread->video_stream_nr();
	assert(stream_index >= 0);
	
	if (stream_index >= thread->nstreams()) {
		lib::logger::get_logger()->error(gettext("%s: no more audio streams"), url.get_url().c_str());
		return NULL;
	} 

	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::new_demux_video_datasource() looking for the right codec");
	
	
	return new demux_video_datasource(url, thread, stream_index);
}

demux_video_datasource::demux_video_datasource(const net::url& url, abstract_demux *thread, int stream_index)
:	m_url(url),
	m_stream_index(stream_index),
//	m_fmt(audio_format(0,0,0)),
	m_src_end_of_file(false),
	m_event_processor(NULL),
	m_thread(thread),
	m_client_callback(NULL),
	m_audio_src(NULL),
	m_frame_nr(0)
{
    assert(m_thread);
	m_thread->add_datasink(this, stream_index);
	int audio_stream_idx = m_thread->audio_stream_nr();
	if (audio_stream_idx >= 0) 
		m_audio_src = new demux_audio_datasource(m_url, m_thread, audio_stream_idx);
	
}

demux_video_datasource::~demux_video_datasource()
{
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::~demux_video_datasource(0x%x)", (void*)this);
	stop();
}

void
demux_video_datasource::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::stop(0x%x)", (void*)this);
	if (m_thread) {
		abstract_demux *tmpthread = m_thread;
		m_thread = NULL;
		m_lock.leave();
		tmpthread->remove_datasink(m_stream_index);
		m_lock.enter();
	}
	m_thread = NULL;
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::stop: thread stopped");
	//if (m_con) delete m_con;
	//m_con = NULL; // owned by the thread
	if (m_client_callback) delete m_client_callback;
	m_client_callback = NULL;
	m_lock.leave();
	
}	



void 
demux_video_datasource::read_ahead(timestamp_t time)
{
	m_lock.enter();
	assert(m_thread);
	assert(!m_thread->is_running());
	assert(m_thread);
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::read_ahead: (this = 0x%x), clip_begin=%d", (void*) this, time);

	m_thread->seek(time);
	m_thread->start();
	m_lock.leave();
}

void 
demux_video_datasource::start_frame(ambulant::lib::event_processor *evp, 
	ambulant::lib::event *callbackk, timestamp_t timestamp)
{
	m_lock.enter();
	assert(m_thread);
	if(!m_thread->is_running()) {
		m_thread->start();
	}
	
	
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::start_frame: (this = 0x%x), callback = 0x%x", (void*) this, (void*) callbackk);

	if (m_client_callback != NULL) {
		delete m_client_callback;
		m_client_callback = NULL;
		lib::logger::get_logger()->error("demux_video_datasource::start(): m_client_callback already set!");
	}
	if (m_frames.size() > 0 /* XXXX Check timestamp! */ || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (callbackk) {
			assert(evp);
			AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::start: trigger client callback");
			evp->add_event(callbackk, MIN_EVENT_DELAY, ambulant::lib::event_processor::med);
		} else {
			lib::logger::get_logger()->debug("Internal error: demux_video_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::start: remembering callback");
		m_client_callback = callbackk;
		m_event_processor = evp;
	}
	m_lock.leave();
}


void 
demux_video_datasource::frame_done(timestamp_t pts, bool keepdata)
{
	// Note: we ignore pts and always discard a single frame.
	m_lock.enter();
	assert(pts == 0);
	assert(m_frames.size() > 0);
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource.frame_done(%d)", pts);
	ts_frame_pair element = m_frames.front();

	if (m_old_frame.second.data) {
		free(m_old_frame.second.data);
		AM_DBG  lib::logger::get_logger()->debug("demux_video_datasource::frame_done: free(0x%x)", m_old_frame.second.data);
		m_old_frame.second.data = NULL;
	}
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::frame_done(%d): removing frame with ts=%d", pts, element.first);
	AM_DBG  lib::logger::get_logger()->debug("demux_video_datasource::frame_done: %lld 0x%x %d", element.first, element.second.data, element.second.size);
	m_old_frame = element;
	m_frames.pop();
	if (!keepdata) {
		free(m_old_frame.second.data);
		m_old_frame.second.data = NULL;
	}
	m_lock.leave();
}

void
write_data(long long int frame_nr, char* data, int sz)
{
	char filename[50];
	sprintf(filename,"%3.8lld.frm",frame_nr);
	AM_DBG lib::logger::get_logger()->debug("write_data: filename : %s", filename);
	FILE* out = fopen(filename,"w+");
	if (out) {
		fwrite(data,sz,1,out);
		fclose(out);
	}
}


void 
demux_video_datasource::data_avail(timestamp_t pts, uint8_t *inbuf, int sz)
{
	m_lock.enter();

	m_src_end_of_file = (sz == 0);
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::data_avail(): recieving data sz=%d ,pts=%lld", sz, pts);
	if(sz > 0) {
		//m_frame_nr++;
		//write_data(m_frame_nr, (char*) inbuf, sz);
		char* frame_data = (char*) malloc(sz+1);
		assert(frame_data);
		memcpy(frame_data, inbuf, sz);
		video_frame vframe;
		vframe.data = frame_data;
		vframe.size = sz;
		m_frames.push(ts_frame_pair(pts, vframe));
		AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::data_avail(): %lld 0x%x %d", pts, vframe.data, vframe.size);
	}		
	if ( m_frames.size() || _end_of_file()  ) {
		if ( m_client_callback ) {
			AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::data_avail(): calling client callback (eof=%d)", m_src_end_of_file);
			assert(m_event_processor);
			m_event_processor->add_event(m_client_callback, MIN_EVENT_DELAY, ambulant::lib::event_processor::med);
			m_client_callback = NULL;
			//m_event_processor = NULL;
		} else {
			AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::data_avail(): No client callback");
		}
	}		
	m_lock.leave();
}


bool 
demux_video_datasource::end_of_file()
{
	m_lock.enter();
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool 
demux_video_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_frames.size()) {
		AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::_end_of_file() returning false (still %d frames in local buffer)",m_frames.size());
		return false;
	}
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::_end_of_file() no frames in buffer returning %d", m_src_end_of_file);
	return m_src_end_of_file;
}

bool 
demux_video_datasource::buffer_full()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::buffer_full() (this=0x%x, count=%d)", (void*) this, m_frames.size(), MAX_VIDEO_FRAMES);
	bool rv = (m_frames.size() > MAX_VIDEO_FRAMES);
	m_lock.leave();
	return rv;
}	

timestamp_t
demux_video_datasource::get_clip_end()
{
	m_lock.enter();
	assert(m_thread);
	timestamp_t clip_end = m_thread->get_clip_end();
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::get_clip_end: clip_end=%d", clip_end);
	m_lock.leave();
	return  clip_end;
}

timestamp_t
demux_video_datasource::get_clip_begin()
{
	m_lock.enter();
	assert(m_thread);
	timestamp_t clip_begin = m_thread->get_clip_begin();
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::get_clip_begin: clip_begin=%d", clip_begin);
	m_lock.leave();
	return  clip_begin;
}

char*
demux_video_datasource::get_frame(timestamp_t now, timestamp_t *timestamp, int *sizep)
{
	
	// We ignore now here and always return a the oldest frame in the queue.
	m_lock.enter();
	assert(now == 0);
//	assert (m_frames.size() > 0 || _end_of_file());
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::get_frame() (this=0x%x, size=%d", (void*) this, m_frames.size());
	if (m_frames.size() == 0) {
		*sizep = 0;
		*timestamp = 0;
		m_lock.leave();
		return NULL;
	}
	ts_frame_pair frame = m_frames.front();
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::get_frame(): ts=%lld 0x%x %d", frame.first, frame.second.data, frame.second.size);
	char *rv = (char*) frame.second.data;
	*sizep = frame.second.size;
	*timestamp = frame.first;
	m_lock.leave();
	return rv;
}

int 
demux_video_datasource::size() const
{
	const_cast <demux_video_datasource*>(this)->m_lock.enter();
	int rv = m_frames.size();
	const_cast <demux_video_datasource*>(this)->m_lock.leave();
	return rv;
}

video_format&
demux_video_datasource::get_video_format()
{
    m_lock.enter();
    assert(m_thread);
	video_format rv = m_thread->get_video_format();
	m_lock.leave();
	return rv;
}

int
demux_video_datasource::width()
{
	m_lock.enter();
	assert(m_thread);
	video_format fmt = m_thread->get_video_format();
	m_lock.leave();
	return fmt.width;
}

int
demux_video_datasource::height()
{
	m_lock.enter();
	assert(m_thread);
	video_format fmt = m_thread->get_video_format();
	m_lock.leave();
	return fmt.height;
}

bool 
demux_video_datasource::has_audio()
{
	m_lock.enter();
	assert(m_thread);
	if (m_thread->audio_stream_nr() >= 0) {
		m_lock.leave();
		return true;
	} else {
		m_lock.leave();
		return false;
	}
}


audio_datasource*
demux_video_datasource::get_audio_datasource()
{
	m_lock.enter();
	if (m_audio_src) {
		//XXX a factory should take care of getting a decoder ds.
		audio_datasource *dds = new ffmpeg_decoder_datasource(m_audio_src);
		AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::get_audio_datasource: decoder ds = 0x%x", (void*)dds);
		if (dds == NULL) {
			int rem = m_audio_src->release();
			assert(rem == 0);
			m_lock.leave();
			return NULL;
		}
		m_lock.leave();
		return dds;
	}
	m_lock.leave();
	return NULL;
}

common::duration
demux_video_datasource::get_dur()
{
	common::duration rv(false, 0.0);
	m_lock.enter();
	assert(m_thread);
	if (m_thread->duration() >= 0) {
		rv = common::duration(true, m_thread->duration());
		AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::get_dur: duration=%f", rv.second);
	}
	m_lock.leave();
	return rv;
}
