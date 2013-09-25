/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2012 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

#ifndef AMBULANT_NET_FFMPEG_VIDEO_H
#define AMBULANT_NET_FFMPEG_VIDEO_H

#include "ambulant/config/config.h"
#include "ambulant/net/datasource.h"

#include <queue>

// Needed for avcodec.h:
#ifndef INT64_C
#define INT64_C(x) x ## LL
#endif
#ifndef UINT64_C
#define UINT64_C(x) x ## ULL
#endif

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

// See if we must use swscale, or can use the older method
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 59, 0)
#error Cannot use ffmpeg older than release 1.0
#endif

#ifdef WITH_FFMPEG_SWSCALE
#include "libswscale/swscale.h"
#else
// Compatible routines are available, but no declarations. We provide our own.
#define SWS_FAST_BILINEAR 1
void *sws_getCachedContext(void*,int, int, int, int, int, int, int, void*, void*, double*);
int sws_scale(void*, uint8_t* srcSlice[], int srcStride[], int srcSliceY,
	int srcSliceH, uint8_t* dst[], int dstStride[]);
void sws_freeContext(struct SwsContext *swsContext);
#endif // WITH_FFMPEG_SWSCALE

}


namespace ambulant
{

namespace net
{

class ffmpeg_video_datasource_factory : public video_datasource_factory {
  public:
	~ffmpeg_video_datasource_factory() {};
	video_datasource* new_video_datasource(const net::url& url, timestamp_t clip_begin, timestamp_t clip_end);
};

typedef std::pair<timestamp_t, char*> ts_pointer_pair;

typedef std::queue<ts_pointer_pair> sorted_frames;

class ffmpeg_video_decoder_datasource:
	virtual public video_datasource,
	virtual public lib::ref_counted_obj {
  public:
	static bool supported(const video_format& fmt);

	//ffmpeg_video_decoder_datasource(const net::url& url, datasource *src);
	ffmpeg_video_decoder_datasource(pkt_datasource *src, video_format fmt, net::url url);

	~ffmpeg_video_decoder_datasource();

	bool has_audio();
	int width();
	int height();
	timestamp_t frameduration();
	audio_datasource *get_audio_datasource();

	void start_frame(lib::event_processor *evp, lib::event *callback, timestamp_t timestamp);
	void stop();

	bool end_of_file();
	char* get_frame(timestamp_t now, timestamp_t *timestamp, size_t *size);
	void frame_processed_keepdata(timestamp_t timestamp, char *data);
	void frame_processed(timestamp_t timestamp);
	void read_ahead(timestamp_t clip_begin);
	void seek(timestamp_t time);
	void set_clip_end(timestamp_t clip_end);
	void start_prefetch(lib::event_processor *evp);
	void data_avail();
	bool buffer_full();
	timestamp_t get_clip_end() { return m_src->get_clip_end(); };
	timestamp_t get_clip_begin() { return m_src->get_clip_begin(); };
	timestamp_t get_start_time() { return m_src->get_start_time(); };
	timestamp_t get_buffer_time() { return m_frames.size() * frameduration(); };
	void set_pixel_layout(pixel_order l) { m_pixel_layout = l; };
	common::duration get_dur();
    long get_bandwidth_usage_data(const char **resource) { return m_src->get_bandwidth_usage_data(resource); }
        void set_is_live (bool is_live) { m_src->set_is_live(is_live); m_is_live = is_live; }
        bool get_is_live () { return m_is_live; }
        

  private:
	bool _select_decoder(const char* file_ext);
	bool _select_decoder(video_format &fmt);
	bool _end_of_file();
	bool _clip_end();
	bool _buffer_full();
	void _pop_top_frame();
	void _need_fmt_uptodate();

	pkt_datasource* m_src;
    net::url m_url;
	AVCodecContext *m_con;
	struct SwsContext *m_img_convert_ctx;

	bool m_con_owned;	// True if we have to close/free m_con
	video_format m_fmt;
	lib::event_processor *m_event_processor;
	sorted_frames  m_frames;
	int m_size;		// NOTE: this assumes all decoded frames are the same size!
	lib::event *m_client_callback;  // This is our calllback to the client
	timestamp_t m_pts_last_frame;
	timestamp_t m_oldest_timestamp_wanted;
	timestamp_t m_video_clock;
	int m_frame_count;
	int m_dropped_count;
	int m_dropped_count_before_decoding;
	lib::critical_section m_lock;
	timestamp_t m_elapsed;
	bool m_start_input;		// True when m_src->start_frame() is needed
    bool m_complete_frame_seen; // True whenever a real frame has been seen
    bool m_is_live; // True for live streams
	pixel_order m_pixel_layout;	// Per-pixel format receiver wants.

    static lib::critical_section s_lock;
};

}	// end namespace net
}	// end namespace ambulant

#endif // AMBULANT_NET_FFMPEG_VIDEO_H
