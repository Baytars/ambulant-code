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

 
#ifndef AMBULANT_NET_DATASOURCE_H
#define AMBULANT_NET_DATASOURCE_H

#include "ambulant/config/config.h"

#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/net/url.h"

namespace ambulant {

namespace net {

	
#ifdef AMBULANT_HAS_LONG_LONG
  typedef long long int timestamp_t; // microseconds
#else
typedef INT64 timestamp_t;
#endif
	
/// This struct completely describes an audio format.
/// If name is "" the format is linear samples encoded
/// in host order, and samplerate, channels and bits
/// are meaningful. If name is nonempty it is some encoded
/// format, parameters points to format-specific data
/// and samplerate/channels/bits are not meaningful.
struct audio_format {
	std::string name;	///< Name of the format, or empty for linear samples
	void *parameters;	///< For a named format, pointer to parameters
	int samplerate;		///< For linear samples: the samplerate
	int channels;		///< For linear samples: the number of channels
	int bits;			///< For linear samples: the numer of bits per sample.
	
	/// Default constructor: creates unknown audio_format.
	audio_format()
	:   name("unknown"),
		parameters(NULL),
		samplerate(0),
		channels(0),
		bits(0) {};
		
	/// Constructor for linear samples.
	audio_format(int s, int c, int b)
	:   name(""),
		parameters(NULL),
		samplerate(s),
		channels(c),
		bits(b) {};
	
	/// Constructor for named audio_format.
	audio_format(std::string &n, void *p=(void *)0)
	:   name(n),
		parameters(p),
		samplerate(0),
		channels(0),
		bits(0) {};
		
	/// Constructor for named audio format.
	audio_format(const char *n, void *p=(void *)0)
	:   name(n),
		parameters(p),
		samplerate(0),
		channels(0),
		bits(0) {};
};

struct video_format {
	std::string name;	///< Name of the format, or empty for linear samples
	void *parameters;	///< For a named format, pointer to parameters
	int framerate;		///< For linear samples: the samplerate
	int width;			/// The width of the video
	int height;			///	The height of the video
	
	/// Default constructor: creates unknown audio_format.
	video_format()
	:   name("unknown"),
		parameters(NULL),
		framerate(0),
		width(0),
		height(0) {};
		
	/// Constructor for linear samples.
	video_format(int r, int w, int h)
	:   name(""),
		parameters(NULL),
		framerate(r),
		width(w),
		height(h) {};
	
	/// Constructor for named video_format.
	video_format(std::string &n, void *p=(void *)0)
	:   name(n),
		parameters(p),
		framerate(0),
		width(0),
		height(0) {};
		
	/// Constructor for named video_format.
	video_format(const char *n, void *p=(void *)0)
	:   name(n),
		parameters(p),
		framerate(0),
		width(0),
		height(0) {};
};

#ifdef __OBJC__
// This is a workaround for a problem when using gcc 3.3 to compile
// ObjC++
;
#endif

// This class describes the range of audio formats supported by a consumer.
// It always contains at least one supported format.
// The design assumes that support for various sample rates, channels and
// bits are independent variables. In addition, an audio_format_choices
// can support both various linear formats and named formats.
class audio_format_choices {
  public:
  
	/// Constructor using a single audio_format.
	audio_format_choices(audio_format &fmt);
	
	/// Constructor using a linear sample format. 
	audio_format_choices(int samplerate, int channels, int bits);
	
	/// Constructor using a named format.
	audio_format_choices(std::string &name);
	
	/// Return the best (highest quality) format.
	const audio_format& best() const;
	
	/// Add support for an additional samplerate.
	void add_samplerate(int samplerate);
	
	/// Add support for an additional number of channels.
	void add_channels(int channels);
	
	/// Add support for an addition number of bits per sample.
	void add_bits(int bits);
	
	/// Add support for an additional named format.
	void add_named_format(std::string &name);
	
	/// Return true if the audio_format argument matches any of the supported formats.
	bool contains(audio_format& fmt) const;
	
  private:
	const audio_format m_best;
	std::set<int> m_samplerate;
	std::set<int> m_channels;
	std::set<int> m_bits;
	std::set<std::string> m_named_formats;
};

/// The interface to an object that supplies data to a consumer.
/// The consumer calls start() whenever it wants
/// data. This call returns immedeately and later the datasource arranges
/// that the callback is done, when data is available. The consumer then
/// calls size(), get_read_ptr() and end_of_file() to get available data size,
/// pointer and status. Whenever the consumer has consumed some bytes it calls
/// read_done().
class datasource : virtual public ambulant::lib::ref_counted {  	
  public:
	virtual ~datasource() {};

	/// Called by the client to indicate it wants more data.
	/// When the data is available (or end of file reached) exactly one
	/// callback is scheduled through the event_processor.
	virtual void start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback) = 0;
	
	/// Called by the client to indicate it wants no more data.
	virtual void stop() = 0;

	/// Return true if all data has been consumed.
    virtual bool end_of_file() = 0;
	
	/// Return a pointer to the current data.
	/// Should only be called from the callback routine.
	virtual char* get_read_ptr() = 0;
	
	/// Return the number of bytes available at get_read_ptr().
	virtual int size() const = 0;		

	/// Called by the client to signal it has consumed len bytes.
    virtual void readdone(int len) = 0;
};

/// Interface to an object that supplies audio data to a consumer.
/// Audio_datasource extends the datasource protocol with methods to obtain
/// information on the way the audio data is encoded. 
class audio_datasource : virtual public datasource {
  public:
	virtual ~audio_datasource() {};
		  
	/// Returns the native format of the audio data.
	virtual audio_format& get_audio_format() = 0;
	// Tells the datasource to start reading data starting from time t.
	virtual void read_ahead(timestamp_t time) = 0; 
	// Return the duration of the audio data, if known.
	virtual std::pair<bool, double> get_dur() = 0;
};

/// Interface to an object that supplies video data to a consumer.
// Video_datasource is *not* a subclass of datasource: it does not deliver a stream
// of bytes (like datasource and audio_datasource) but a stream of images.
// It also has an ad-hoc method to determine whether audio is available too, and obtain
// a datasource for that.
class video_datasource : virtual public lib::ref_counted_obj {
  public:
  	virtual ~video_datasource() {};

	// Return the duration of the audio data, if known.
	virtual std::pair<bool, double> get_dur() = 0;

	/// Returns true if the video stream contains audio data too.
	virtual bool has_audio() = 0;
	
	/// Returns an audio_datasource object for the audio data.
	virtual audio_datasource *get_audio_datasource() = 0;
	/// Called by the client to indicate it wants a new frame.
	/// When the data is available (or end of file reached) exactly one
	/// callback is scheduled through the event_processor.
	/// The client is not interested in any frames with times earlier
	/// than the given timestamp.
	virtual void start_frame(lib::event_processor *evp, lib::event *callback, timestamp_t pts) = 0;
	
	/// Called by the client to indicate it wants no more data.
  	virtual void stop() = 0;
	
	/// Return true if all data has been consumed.
  	virtual bool end_of_file() = 0;
  	
	/// Return the current video frame.
	/// Should only be called from the callback routine.
	/// The timestamp of the frame and the size of the data are also returned.
  	virtual char* get_frame(timestamp_t now, timestamp_t *timestamp, int *size) = 0; 

	/// Returns the width of the image returned by get_frame.
	virtual int width() = 0;
	
	/// Returns the height of the image returned by get_frame.
	virtual int height() = 0;
	
	/// Called by the client to indicate all frames up to timestamp are consumed.
	/// If keepdata is set the actual storage for a frame with an exact
	/// timestamp match is not freed.
  	virtual void frame_done(timestamp_t timestamp, bool keepdata) = 0;
	

};

/// Interface to create a datasource for a given URL.
class raw_datasource_factory {
  public: 
    virtual ~raw_datasource_factory() {}; 	
	
	/// Create a new datasource to read the given URL.
	/// Returns NULL if this factory cannot create such a datasource.
  	virtual datasource* new_raw_datasource(const net::url& url) = 0;
};

/// Interface to create an audio_datasource for a given URL.
/// This class is the client API used to create an audio_datasource for
/// a given URL, with an extra parameter specifying which audio encodings
/// the client is able to handle.
class audio_datasource_factory  {
  public: 
    virtual ~audio_datasource_factory() {}; 	

	/// Create a new audio_datasource to read the given URL.
	/// The fmt parameter describes the audio formats the client can handle,
	/// the actual format can then be obtained from the audio_datasource returned.
	/// Returns NULL if this factory cannot create such a datasource.
  	virtual audio_datasource* new_audio_datasource(const net::url& url, audio_format_choices fmt) = 0;
};

/// Factory for implementations where the audio_datasource
/// does only parsing, using a datasource to obtain raw data. The audio_format_choices
/// is only a hint, it may be the case that the audio_datasource returns
/// incompatible data.
class audio_parser_finder {
  public:
	virtual ~audio_parser_finder() {};
	
	/// Create an audio parser for the given datasource.
	virtual audio_datasource* new_audio_parser(const net::url& url, audio_format_choices hint, datasource *src) = 0;
};

/// Factory for implementations where the audio_datasource
/// does only conversion of the audio data provided by the source to the format
/// wanted by the client.
class audio_filter_finder  {
  public:
    virtual ~audio_filter_finder() {};
	
	/// Create a filter that converts audio data from src to a format compatible with fmts.
  	virtual audio_datasource* new_audio_filter(audio_datasource *src, audio_format_choices fmts) = 0;
};

/// Interface to create a video_datasource for a given URL.
class video_datasource_factory  {
  public: 
    virtual ~video_datasource_factory() {};

	/// Create a new video_datasource to read the given URL.
  	virtual video_datasource* new_video_datasource(const net::url& url) = 0;
};

/// Implementation of all datasource factories.
/// A datasource implementation registers its factory function with
/// an object of this class. Subsequently, when a client needs a new datasource
/// it will try the various factories in turn.
/// In addition, for audio_datasources, it will also try to obtain a raw datasource
/// and stack a parser and filter onto it. 
class datasource_factory :
	public raw_datasource_factory,
	public audio_datasource_factory,
	public video_datasource_factory
{
  public:
	datasource_factory() {};
  	~datasource_factory();
  
	/// Client interface: obtain a datasource for the given URL.
  	datasource* new_raw_datasource(const net::url& url);
	
	/// Client interface: obtain an audio_datasource for the given URL and format.
	audio_datasource* new_audio_datasource(const net::url& url, audio_format_choices fmt);
	
	/// Client interface: obtain a video datasource for the given URL.
  	video_datasource* new_video_datasource(const net::url& url);
	
	/// Semi-private interface: obtain an audio filter datasource.
	audio_datasource* new_filter_datasource(const net::url& url, audio_format_choices fmt, audio_datasource* ds);
	
	/// Provider interface: add a raw_datasource_factory.
  	void add_raw_factory(raw_datasource_factory *df);
	
	/// Provider interface: add an audio_datasource_factory.
	void add_audio_factory(audio_datasource_factory *df);
	
	/// Provider interface: add an audio_parser_finder.
	void add_audio_parser_finder(audio_parser_finder *df);
	
	/// Provider interface: add an audio_filter_finder.
	void add_audio_filter_finder(audio_filter_finder *df);
	
	/// Provider interface: add a video_datasource_factory.
	void add_video_factory(video_datasource_factory *df);
		
  private:
	std::vector<raw_datasource_factory*> m_raw_factories;
	std::vector<audio_datasource_factory*> m_audio_factories;
	std::vector<audio_parser_finder*> m_audio_parser_finders;
	std::vector<audio_filter_finder*> m_audio_filter_finders;
	std::vector<video_datasource_factory*> m_video_factories;
};

/// convenience function: read a whole document through any raw datasource.
int read_data_from_url(const net::url &url, datasource_factory *df, char **result);

} // end namespace net

} //end namespace ambulant


#endif
