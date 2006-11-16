// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 
#include <math.h>
#include <map>

#include "ambulant/net/ffmpeg_common.h" 
#include "ambulant/net/datasource.h"
#include "ambulant/lib/logger.h"
#include "ambulant/net/url.h"

// WARNING: turning on AM_DBG globally for the ffmpeg code seems to trigger
// a condition that makes the whole player hang or collapse. So you probably
// shouldn't do it:-)
//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif 


// This construction is needed to get the CVS version of ffmpeg to work:
// AVStream.codec got changed from AVCodecContext to AVCodecContext*
#if LIBAVFORMAT_BUILD > 4628
	#define am_get_codec_var(codec,var) codec->var
	#define am_get_codec(codec) codec
#else
	#define am_get_codec_var(codec,var) codec.var
	#define am_get_codec(codec) &codec
#endif

using namespace ambulant;
using namespace net;

void 
ambulant::net::ffmpeg_init()
{
	static bool is_inited = false;
	if (is_inited) return;
	avcodec_init();
	av_register_all();
	is_inited = true;
}

// Static initializer function shared among ffmpeg classes
ffmpeg_codec_id* ambulant::net::ffmpeg_codec_id::m_uniqueinstance = NULL;

ffmpeg_codec_id*
ffmpeg_codec_id::instance()
{
	if (m_uniqueinstance == NULL) {
        m_uniqueinstance = new ffmpeg_codec_id();
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_codec_id::instance() returning 0x%x", (void*) m_uniqueinstance);
	}
    return m_uniqueinstance;
}

void
ffmpeg_codec_id::add_codec(const char* codec_name, CodecID id)
{
  std::string str(codec_name);
  m_codec_id.insert(std::pair<std::string, CodecID>(str, id));
}

CodecID
ffmpeg_codec_id::get_codec_id(const char* codec_name) 
{
	std::string str(codec_name);
	std::map<std::string, CodecID>::iterator i;
	i = m_codec_id.find(str);
	if (i != m_codec_id.end()) {
		AM_DBG lib::logger::get_logger()->debug("get_codec_id(%s) -> %d", str.c_str(), i->second);
		return i->second;
	}
	
	return CODEC_ID_NONE;
}

ffmpeg_codec_id::ffmpeg_codec_id()
{
	
	// XXX Lots of formats missing here obviously
	// Live.com formats
	add_codec("MPA", CODEC_ID_MP3);
	add_codec("MPA-ROBUST", CODEC_ID_MP3);
	add_codec("X_MP3-DRAFT-00", CODEC_ID_MP3);
	add_codec("X-MP3-DRAFT-00", CODEC_ID_MP3);
	add_codec("MPV", CODEC_ID_MPEG2VIDEO);
	add_codec("L16", CODEC_ID_PCM_S16LE);
	add_codec("MP4V-ES", CODEC_ID_MPEG4);
	add_codec("MPEG4-GENERIC", CODEC_ID_MPEG4AAC);
	add_codec("X-QT", CODEC_ID_MP3); //XXXX
}

// **************************** ffmpeg_demux *****************************


ffmpeg_demux::ffmpeg_demux(AVFormatContext *con, timestamp_t clip_begin, timestamp_t clip_end)
:   m_con(con),
	m_nstream(0),
	m_clip_begin(clip_begin),
	m_clip_end(clip_end),
	m_clip_begin_set(false)
{
#if LIBAVFORMAT_BUILD > 4609
	
	assert(m_clip_begin >= 0);
	if ( m_clip_begin > 0 
#if 1
	// Bug in ffmpeg 49.2.0: seeking in mov/mp4 files fails. Reported 5-Dec-05.
	&& !strstr(con->iformat->name, "mp4")
#endif
		) {
		assert (m_con);
		assert (m_con->iformat);
#if LIBAVFORMAT_BUILD > 4628
		int seekresult = av_seek_frame(m_con, -1, m_clip_begin, 0);
#else
		int seekresult = av_seek_frame(m_con, -1, m_clip_begin);
#endif
		if (seekresult < 0) {
			lib::logger::get_logger()->debug("ffmpeg_demux: av_seek_frame() returned %d", seekresult);
//#if LIBAVFORMAT_BUILD > 4628
//			// ffmpeg has discarded data if av_seek_frame() failed
//			av_seek_frame(m_con, -1, 0, AVSEEK_FLAG_BYTE);
//#endif
		}
	} 
#endif
	
	m_audio_fmt = audio_format("ffmpeg");
	m_audio_fmt.bits = 16;
	int audio_idx = audio_stream_nr();
	if ( audio_idx >= 0) {
		m_audio_fmt.parameters = (void *) am_get_codec(con->streams[audio_idx]->codec);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::supported: audio_codec_name=%s", am_get_codec_var(m_con->streams[audio_idx]->codec, codec_name));
		m_audio_fmt.samplerate = am_get_codec_var(con->streams[audio_idx]->codec, sample_rate);
		m_audio_fmt.channels = am_get_codec_var(con->streams[audio_idx]->codec, channels);
		m_audio_fmt.bits = 16;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::ffmpeg_demux(): samplerate=%d, channels=%d, m_con->codec (0x%x)", m_audio_fmt.samplerate, m_audio_fmt.channels, m_audio_fmt.parameters  );
	} 
	m_video_fmt = video_format("ffmpeg");
	int video_idx = video_stream_nr();
	if (video_idx >= 0) {
		m_video_fmt.parameters = (void *) am_get_codec(m_con->streams[video_stream_nr()]->codec);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::supported: video_codec_name=%s", am_get_codec_var(m_con->streams[video_stream_nr()]->codec, codec_name));
	} else {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::supported: No Video stream ?");
		m_video_fmt.parameters = NULL;
	}
	memset(m_sinks, 0, sizeof m_sinks);
}

ffmpeg_demux::~ffmpeg_demux()
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::~ffmpeg_demux()");
	if (m_con) av_close_input_file(m_con);
	m_con = NULL;
}

timestamp_t
ffmpeg_demux::get_clip_end()
{	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::get_clip_end(): %lld", m_clip_end);
	return m_clip_end;
}

timestamp_t
ffmpeg_demux::get_clip_begin()
{	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::get_clip_begin(): %lld", m_clip_begin);
	return m_clip_begin;
}

AVFormatContext *
ffmpeg_demux::supported(const net::url& url)
{
	ffmpeg_init();
	// Setup struct to allow ffmpeg to determine whether it supports this
	AVInputFormat *fmt;
	AVProbeData probe_data;
	std::string url_str(url.get_url());
	std::string ffmpeg_name = url_str;
	if (url.is_local_file())
		ffmpeg_name = url.get_file();
	
#if 0
	// There appears to be some support for RTSP in ffmpeg, but it doesn'
	// seem to work yet. Disable it so we don't get confused by error messages.
	if (url_str.substr(0, 5) == "rtsp:") return NULL;
#endif
	probe_data.filename = ffmpeg_name.c_str();
	probe_data.buf = NULL;
	probe_data.buf_size = 0;
	fmt = av_probe_input_format(&probe_data, 0);
	if (!fmt && url.is_local_file()) {
		// get the file extension
		std::string short_name = ffmpeg_name.substr(ffmpeg_name.length()-3,3);
		if (short_name == "mov" || short_name == "mp4")
			// ffmpeg's id for QuickTime/MPEG4/Motion JPEG 2000 format
			short_name = "mov,mp4,m4a,3gp,3g2,mj2";
		fmt = av_find_input_format(short_name.c_str());
		
	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::supported(%s): (%s) av_probe_input_format: 0x%x", url_str.c_str(), ffmpeg_name.c_str(), (void*)fmt);
	AVFormatContext *ic = NULL;
	int err = av_open_input_file(&ic, ffmpeg_name.c_str(), fmt, 0, 0);
	if (err) {
		lib::logger::get_logger()->trace("ffmpeg_demux::supported(%s): av_open_input_file returned error %d, ic=0x%x", url_str.c_str(), err, (void*)ic);
		if (ic) av_close_input_file(ic);
		return NULL;
	}
	//err = av_read_play(ic);
 	err = av_find_stream_info(ic);
	if (err < 0) {
		lib::logger::get_logger()->trace("ffmpeg_demux::supported(%s): av_find_stream_info returned error %d, ic=0x%x", url_str.c_str(), err, (void*)ic);
		if (ic) av_close_input_file(ic);
		return NULL;
	}
	
	AM_DBG dump_format(ic, 0, ffmpeg_name.c_str(), 0);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::supported: rate=%d, channels=%d", am_get_codec_var(ic->streams[0]->codec,sample_rate), am_get_codec_var(ic->streams[0]->codec,channels));
	assert(ic);
	return ic;
}

void
ffmpeg_demux::cancel()
{
	if (is_running())
		stop();
	release();
}

int 
ffmpeg_demux::audio_stream_nr() 
{
	int stream_index;
	for (stream_index=0; stream_index < m_con->nb_streams; stream_index++) {
		if (am_get_codec_var(m_con->streams[stream_index]->codec, codec_type) == CODEC_TYPE_AUDIO)
			return stream_index;
	}
	
	return -1;
}

int 
ffmpeg_demux::video_stream_nr() 
{
	int stream_index;
	for (stream_index=0; stream_index < m_con->nb_streams; stream_index++) {
		if (am_get_codec_var(m_con->streams[stream_index]->codec, codec_type) == CODEC_TYPE_VIDEO) {
			return stream_index;
		}
	}
	
	return -1;
}

double
ffmpeg_demux::duration()
{
	// XXX this is a double now, later this should retrun a long long int
	// XXX Note that this code knows that m_clip_{begin,end} and m_con->duration
	// are both microseconds
	timestamp_t dur = m_con->duration;
	if (m_clip_end > 0 && m_clip_end < dur)
		dur = m_clip_end;
	dur -= m_clip_begin;
 	return dur / (double)AV_TIME_BASE;
	
}
	
int 
ffmpeg_demux::nstreams()
{
	return m_con->nb_streams;
}

audio_format&
ffmpeg_demux::get_audio_format()
{
	 return m_audio_fmt; 
}

video_format& 
ffmpeg_demux::get_video_format() 
{ 
	m_lock.enter();
	if (m_video_fmt.width == 0) m_video_fmt.width = am_get_codec_var(m_con->streams[video_stream_nr()]->codec, width);
	if (m_video_fmt.height == 0) m_video_fmt.height = am_get_codec_var(m_con->streams[video_stream_nr()]->codec, height);
	m_lock.leave();
	return m_video_fmt; 
	
}

void 
ffmpeg_demux::add_datasink(demux_datasink *parent, int stream_index)
{
	m_lock.enter();
	assert(stream_index >= 0 && stream_index < MAX_STREAMS);
	assert(m_sinks[stream_index] == 0);
	m_sinks[stream_index] = parent;
	m_nstream++;
	m_lock.leave();
}

void
ffmpeg_demux::seek(timestamp_t time)
{
	m_lock.enter();
	m_clip_begin = time;
	m_clip_begin_set = false;
	m_lock.leave();
}

void
ffmpeg_demux::remove_datasink(int stream_index)
{
	m_lock.enter();
	assert(stream_index >= 0 && stream_index < MAX_STREAMS);
	assert(m_sinks[stream_index] != 0);
	if (m_sinks[stream_index])
		// signal EOF
		m_sinks[stream_index]->data_avail(0, 0, 0);
	m_sinks[stream_index] = 0;
	m_nstream--;
	m_lock.leave();
	if (m_nstream <= 0) cancel();
}

unsigned long
ffmpeg_demux::run()
{
	m_lock.enter();
	int pkt_nr;
#if LIBAVFORMAT_BUILD > 4628
	int streamnr = video_stream_nr();
#endif
	timestamp_t pts;
	pkt_nr = 0;
	assert(m_con);
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: started");
	while (!exit_requested()) {
		AVPacket pkt1, *pkt = &pkt1;
		
		pkt->pts = 0;
		// Read a packet
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run:  started");
		m_lock.leave();
#if LIBAVFORMAT_BUILD > 4609
		int ret = av_read_frame(m_con, pkt);
#else
		int ret = av_read_packet(m_con, pkt);
#endif
		m_lock.enter();
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: av_read_packet returned ret= %d, (%lld, 0x%x, %d)", ret, pkt->pts ,pkt->data, pkt->size);
		if (ret < 0) break;
		pkt_nr++;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: av_read_packet number : %d",pkt_nr);
		// Find out where to send it to
		assert(pkt->stream_index >= 0 && pkt->stream_index < MAX_STREAMS);
		demux_datasink *sink = m_sinks[pkt->stream_index];
		if (sink == NULL) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: Drop data for stream %d (%lld, 0x%x, %d)", pkt->stream_index, pkt->pts ,pkt->data, pkt->size);
		} else {
			AM_DBG lib::logger::get_logger ()->debug ("ffmpeg_parser::run sending data to datasink (stream %d) (%lld, 0x%x, %d)",pkt->stream_index, pkt->pts ,pkt->data, pkt->size);
			/* XXXX not needed for pkt_datasource
			// Wait until there is room in the buffer
			//while (sink->buffer_full() && !exit_requested()) {
			while (sink && sink->buffer_full() && !exit_requested()) {
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: waiting for buffer space for stream %d", pkt->stream_index);
				m_lock.leave();
				sleep(1);   // This is overdoing it
				m_lock.enter();
				sink = m_sinks[pkt->stream_index];
			}
			*/
			if (sink && !exit_requested()) {
				
				pts = 0;
				
#if LIBAVFORMAT_BUILD > 4628
				if (streamnr > -1) {
					if (pkt->dts != AV_NOPTS_VALUE) {
            			pts = (timestamp_t) round(( (double) m_con->streams[streamnr]->time_base.num* 1000000.0 /m_con->streams[streamnr]->time_base.den)*pkt->dts);
					}
				}
#elif LIBAVFORMAT_BUILD > 4609
				if (pkt->pts != (int64_t)AV_NOPTS_VALUE) {
					pts = pkt->pts;							
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: ffmpeg 0.4.9 pts = %lld", pts);					
				}
#else
				if (pkt->pts != (int64_t)AV_NOPTS_VALUE) {
					int num = m_con->pts_num;
					int den = m_con->pts_den;
					pts = (timestamp_t) round(((double) pkt->pts * (((double) num)*1000000)/den));
				}
#endif
				
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: calling %d.data_avail(%lld, 0x%x, %d, %d) pts=%lld", pkt->stream_index, pkt->pts, pkt->data, pkt->size, pkt->duration, pts);
				
				sink->data_avail(pts, pkt->data, pkt->size);

			}
		}
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: freeing pkt (number %d)",pkt_nr);
		av_free_packet(pkt);
	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: final data_avail(0, 0)");
	int i;
	for (i=0; i<MAX_STREAMS; i++)
		if (m_sinks[i])
			m_sinks[i]->data_avail(0, 0, 0);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: returning");
	m_lock.leave();
	return 0;
}
