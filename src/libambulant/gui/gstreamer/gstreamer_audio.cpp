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

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/gui/gstreamer/gstreamer_audio.h"
#include "ambulant/net/posix_datasource.h"
#include "ambulant/common/region_info.h"

#include <stdlib.h>

using namespace ambulant;
using namespace gui::gstreamer;

static void
mutex_acquire(void* player);
static void
mutex_release(void* player);
static void
pipeline_store(void* player, GstElement* p);

extern "C" {

/* from sanbox/Nokia770/AudioPlayer/mp3player.c */

static void
mp3player_eos_cb (GstElement * bin, gpointer user_data);
static void
mp3player_error_cb (GstElement * bin, GstElement * error_element,
		    GError * error,  const gchar * debug_msg,
		    gpointer user_data);

int
gst_mp3_player(const char* uri, GstElement** gst_player_p, gstreamer_player* gstreamer_player, gboolean* player_done_p)
{
  GMainLoop *loop;
  GstElement *source=NULL,*sink=NULL, *pipeline=NULL;
  char **files = NULL;
  const char* id = "gst_mp3_player";
  AM_DBG g_print ("%s: %s\n", id, "starting");

  /* create elements */
  pipeline = (GstElement*)gst_pipeline_new ("mp3-player");
#ifdef  WITH_NOKIA770
  source   = gst_element_factory_make ("gnomevfssrc", "source"); 
  sink     = gst_element_factory_make ("dspmp3sink", "sink"); 
#else //WITH_NOKIA770
  source   = gst_element_factory_make ("playbin", "playbin"); 
  if (gst_player_p) *gst_player_p = pipeline;
#endif//WITH_NOKIA770
  if (gst_player_p) *gst_player_p = pipeline;

  if ( !( pipeline && source
#ifdef  WITH_NOKIA770
	  && sink
#endif//WITH_NOKIA770
	  ) ) {
    g_print ("%s:", "gst_mp3_player");
    if ( ! pipeline) g_print (" %s() failed", "get_pipeline_new");
#ifdef  WITH_NOKIA770
    if ( ! source)   g_print (" %s=%s(%s) failed", "source", 
			      "gst_element_factory_make", "gnomevfssrc");
    if ( ! sink)   g_print (" %s=%s(%s) failed", "sink", 
			      "gst_element_factory_make", "dspmp3sink");
#else //WITH_NOKIA770
    if ( ! source)   g_print (" %s=%s(%s) failed", "source", 
			      "gst_element_factory_make", "playbin");
#endif//WITH_NOKIA770
    g_print ("\n");
    return -1;
  }
  AM_DBG g_print ("%s: %s\n", id, "set the source audio file");
  /* set the source audio file */
#ifdef  WITH_NOKIA770
  g_object_set (G_OBJECT(source), "location", uri, NULL);
#else //WITH_NOKIA770
  g_object_set (G_OBJECT(source), "uri", uri, NULL);
#endif//WITH_NOKIA770

  /* put all elements  to the main pipeline */
  gst_bin_add_many (GST_BIN(pipeline), source,
#ifdef  WITH_NOKIA770
		    sink,
#endif//WITH_NOKIA770
		    NULL);

#ifdef  WITH_NOKIA770
  /* link the elements */
  gst_element_link (source, sink);
#endif//WITH_NOKIA770


  /* wait for start */
  gst_element_set_state (pipeline, GST_STATE_PAUSED);
//gst_element_set_state (GST_ELEMENT(pipeline), GST_STATE_PAUSED);

  AM_DBG g_print ("%s: %s\n", id, "add call-back message handler for eos");
  /* add call-back message handlers to check for eos and errors */
  g_signal_connect (GST_BIN(pipeline), "eos",
		    G_CALLBACK (mp3player_eos_cb), player_done_p);
  AM_DBG g_print ("%s: %s\n", id, "add call-back message handler for error");
  g_signal_connect (GST_BIN(pipeline), "error",
		    G_CALLBACK (mp3player_error_cb), player_done_p);

  AM_DBG g_print ("%s: %s\n", id, "wait for start");

  pipeline_store(gstreamer_player, pipeline);
  mutex_release(gstreamer_player);

  AM_DBG g_print ("%s: %s\n", id, "iterate");
   /* iterate */
  AM_DBG if ( ! *player_done_p) g_print ("Now playing %s ...", uri);
  while ( ! *player_done_p) {
    mutex_acquire(gstreamer_player); 
    gst_bin_iterate (GST_BIN(pipeline));
    mutex_release(gstreamer_player);
  }
  AM_DBG if (player_done_p) g_print (" done !\n"); 

  mutex_acquire(gstreamer_player); // to be released by the caller

  /* stop the pipeline */
  gst_element_set_state (GST_ELEMENT(pipeline), GST_STATE_NULL);

  /* cleanup */
  gst_object_unref (GST_OBJECT(pipeline));

  return 0;
}

static void
mp3player_eos_cb (GstElement *pipeline,gpointer user_data)
{
  const char* id = "mp3player_eos_cb";
  gboolean *p_player_done = (gboolean *) user_data;

  AM_DBG g_print ("%s: %s\n", id, "called");
  *p_player_done = TRUE;
}

static void
mp3player_error_cb (GstElement *pipeline, GstElement *error_element,
		    GError *error, const gchar *debug_msg,
gpointer user_data)
{
  const char* id = "mp3player_error_cb";
  gboolean *p_player_done = (gboolean *) user_data;

  AM_DBG g_print ("%s: %s\n", id, "called");
  if (error)
    g_printerr ("Error: %s\n", error->message);

  *p_player_done = TRUE;
}
}

typedef lib::no_arg_callback<gui::gstreamer::gstreamer_audio_renderer> readdone_callback;
	
// ************************************************************


// ************************************************************

gstreamer_audio_renderer::gstreamer_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory)
:	common::renderer_playable(context, cookie, node, evp),
	m_player(NULL),
	m_pipeline(NULL),
	m_is_playing(false),
	m_is_paused(false),
	m_read_ptr_called(false),
	m_volcount(0)
#ifdef USE_SMIL21
	,
	m_intransition(NULL),
	m_outtransition(NULL),
	m_transition_engine(NULL)
#endif                                   
{
        init_player(node);
}

gstreamer_audio_renderer::gstreamer_audio_renderer(
    common::playable_notification *context,
    common::playable_notification::cookie_type cookie,
    const lib::node *node,
    lib::event_processor *evp,
	common::factories* factory,
	net::audio_datasource *ds)
:	common::renderer_playable(context, cookie, node, evp),
	m_player(NULL),
	m_pipeline(NULL),
	m_is_playing(false),
	m_is_paused(false),
	m_read_ptr_called(false)
#ifdef USE_SMIL21
	,
	m_intransition(NULL),
	m_outtransition(NULL),
	m_transition_engine(NULL)
#endif                                   
{
        init_player(node);
}

gstreamer_audio_renderer::~gstreamer_audio_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer::~gstreamer_audio_renderer(0x%x) m_url=%s",  this, m_url.get_url().c_str());		
	if (m_is_playing) {
	//	m_lock.leave();
		//TBD stop it
		m_player->stop();
	//	m_lock.enter();
	}	
#ifdef USE_SMIL21
	if (m_transition_engine) {
		delete m_transition_engine;
		m_transition_engine = NULL;
	}
#endif                                   
	m_is_playing = false;
	if (m_player) delete m_player;
	m_player = NULL;
	m_lock.leave();
}

static void
pipeline_store(void* obj, GstElement*pipeline) {
	gstreamer_audio_renderer* rend = (gstreamer_audio_renderer*) obj;
	if (rend) rend->m_pipeline = pipeline;
}

void
gstreamer_audio_renderer::init_player(const lib::node *node) {
	assert (node);
	m_url = node->get_url("src");
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer::init_player(0x%x) url=",  this, m_url.get_url().c_str());
	_init_clip_begin_end();
	m_player = new gstreamer_player(m_url.get_url().c_str(), this);
	m_player->init();
	m_context->started(m_cookie, 0);
	//KB	if (gst_player failed)m_context->stopped(m_cookie, 0);
}
#ifdef USE_SMIL21

void
gstreamer_audio_renderer::set_intransition(const lib::transition_info* info) {
 	if (m_transition_engine)
		delete m_transition_engine;
	m_intransition = info;
	m_transition_engine = new smil2::audio_transition_engine();
	m_transition_engine->init(m_event_processor, false, info);
}

void
gstreamer_audio_renderer::start_outtransition(const lib::transition_info* info) {
 	if (m_transition_engine)
		delete m_transition_engine;
	m_outtransition = info;
	m_transition_engine = new smil2::audio_transition_engine();
	m_transition_engine->init(m_event_processor, true, info);
}
#endif

bool
gstreamer_audio_renderer::is_supported(const lib::node *node)
{
	if ( ! node)
    		return false;
	std::string mimetype(node->get_url("src").guesstype());
	
#ifdef  WITH_NOKIA770
	if (mimetype == "audio/mpeg") // .mp3
		return true;
#else //WITH_NOKIA770
	if (mimetype == "audio/wav") // .wav
		return true;
#endif//WITH_NOKIA770
	return false;
}

bool
gstreamer_audio_renderer::is_paused()
{
	m_lock.enter();
	bool rv;
	rv = m_is_paused;
	m_lock.leave();
	return rv;
}

bool
gstreamer_audio_renderer::is_stopped()
{
	m_lock.enter();
	bool rv;
	rv = !m_is_playing;
	m_lock.leave();
	return rv;
}

bool
gstreamer_audio_renderer::is_playing()
{
	m_lock.enter();
	bool rv;
	rv = m_is_playing;
	m_lock.leave();
	return rv;
}


void
gstreamer_audio_renderer::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer::stop(0x%x)",(void*)this);
	if (m_player && m_is_playing) {
		m_lock.leave();
		m_player->stop_player();
		m_lock.enter();
		// XXX Should we call stopped_callback?
		m_context->stopped(m_cookie, 0);
	}
	m_is_playing = false;
	m_lock.leave();
}

void
gstreamer_audio_renderer::pause()
{
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer.pause(0x%x)", (void *)this);
	m_lock.enter();
	if (m_player)
		m_player->pause();
	m_is_paused = true;
	m_lock.leave();
}

void
gstreamer_audio_renderer::resume()
{
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer.resume(0x%x)", (void *)this);
	m_lock.enter();
	if (m_player)
		m_player->play();
	m_is_playing = true;
	m_is_paused = false;
	m_lock.leave();
}

void
gstreamer_audio_renderer::start(double where)
{
	double microsec = 1e6;
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer.start(0x%x): url=%s, where=%f", (void *)this, m_url.get_url().c_str(),where);
	pause();
	seek(where);
	resume(); // turn on playing
}

void
gstreamer_audio_renderer::seek(double where)
{
       lib::logger::get_logger()->trace("gstreamer_audio_renderer: seek(%f)", where);
       m_lock.enter();
       if (m_player) {
              double microsec = 1e6;
	      guint64 where_guint64;
	      where += (m_clip_begin / microsec);
	      where_guint64 = llrint(where)* GST_SECOND;	      
	      lib::logger::get_logger()->trace("gstreamer_audio_renderer: seek() where=%f, where_guint64=%lu", where, where_guint64);
	      m_player->mutex_acquire();
	      if ( ! gst_element_seek(m_player->gst_player(), (GstSeekType) (GST_SEEK_METHOD_SET | GST_FORMAT_TIME | GST_SEEK_FLAG_FLUSH), where_guint64)) {
	             lib::logger::get_logger()->trace("gstreamer_audio_renderer: seek() failed.");
	      }
	      m_player->mutex_release();
       }
       m_lock.leave();
}

common::duration 
gstreamer_audio_renderer::get_dur()
{
	gint64 length = -1;
	GstFormat fmtTime = GST_FORMAT_TIME;
	common::duration rv(false, 0.0);

	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer.get_dur(0x%x)", (void *)this);
	m_lock.enter();
	if (m_player) {
	        m_player->mutex_acquire();
		gst_element_query(m_player->gst_player(), GST_QUERY_TOTAL, &fmtTime, &length);
		m_player->mutex_release();
	}
	m_lock.leave();

	if (length != -1) {
	        double nanosec = 1e9, microsec = 1e6;
		double dur = double(length) / nanosec;
		double clip_begin = m_clip_begin / microsec;
		double clip_end = m_clip_end / microsec;

		if (clip_end > 0 && dur > clip_end)
	               dur = clip_end;
		if (clip_begin > 0)
	               dur -= clip_begin;
		lib::logger::get_logger()->trace("gstreamer_audio_renderer: get_dur() clip_begin=%f clip_end=%f dur=%f", clip_begin, clip_end, dur);
		rv = common::duration(true, dur);
  	}
	return rv;
}

/* gstreamer_player **********************************************************/

gstreamer_player::gstreamer_player(const char* uri, gstreamer_audio_renderer* rend)
  : m_gst_player(NULL),
    m_audio_renderer(NULL),
    m_uri(NULL) {
	m_uri = strdup(uri);
	m_audio_renderer = rend;
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player(0x%x) uri=%s", (void*)this, uri);
}

gstreamer_player::~gstreamer_player() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::~gstreamer_player()(0x%x) m_uri=%s", (void*)this, m_uri);
	stop_player();
	if (m_uri) free(m_uri);
	m_uri = NULL;
	// static mutex needs to exists during the lifetime of the program
	//  if (pthread_mutex_destroy(&s_mutex) < 0) {
	//  	    lib::logger::get_logger()->fatal("gstreamer_player: pthread_mutex_destroy failed: %s", strerror(errno));
        // }
}

GstElement*
gstreamer_player::gst_player() {
	return m_gst_player;
}

unsigned long
gstreamer_player::run() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::run(0x%x)m_uri=%s", (void*)this, m_uri);
	gst_mp3_player (m_uri, &m_gst_player, this, &m_player_done);
	m_gst_player = NULL;
	mutex_release(); // lock was acquired by gst_mp3_player()
	// inform the scheduler that the gstreamer player has terminated
	m_audio_renderer->stop();
}

unsigned long
gstreamer_player::init() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::init(0x%x)m_uri=%s", (void*)this, m_uri);
	mutex_acquire(); // lock will be released by gst_mp3_player()
	m_player_done = FALSE;
	start();	     // starts run() in separate thread
	mutex_acquire(); // wait until gst_mp3_player() is initialized
	mutex_release();
	return 0;
}

void
gstreamer_player::stop_player() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::stop_player(0x%x)m_uri=%s", (void*)this, m_uri);
	mutex_acquire(); 
	if (m_gst_player)
		mp3player_eos_cb(m_gst_player, &m_player_done);
	mutex_release(); 
}

void
gstreamer_player::pause() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::pause(0x%x)m_uri=%s", (void*)this, m_uri);
	mutex_acquire(); 
	if (m_gst_player)
		gst_element_set_state (m_gst_player, GST_STATE_PAUSED);
	mutex_release(); 
}

void
gstreamer_player::play() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::play(0x%x)m_uri=%s", (void*)this, m_uri);
	mutex_acquire(); 
	if (m_gst_player)
		gst_element_set_state (m_gst_player, GST_STATE_PLAYING);
	mutex_release(); 
}

void
gstreamer_player::mutex_acquire() {
	if ( ! s_initialized) {
		if (pthread_mutex_init(&s_mutex, NULL) < 0) {
                	lib::logger::get_logger()->fatal("gstreamer_player:::mutex_acquire() pthread_mutex_init failed: %s", strerror(errno));
		}
		s_initialized = true;
	}
	if (pthread_mutex_lock(&s_mutex) < 0) {
                lib::logger::get_logger()->fatal("gstreamer_player::mutex_acquire(): pthread_mutex_lock failed: %s", strerror(errno));
	}
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::mutex_acquire(0x%x): pthread_mutex_lock called.", this);
}

void
gstreamer_player::mutex_release() {
	if ( ! s_initialized) {
                lib::logger::get_logger()->fatal("gstreamer_player::mutex_release() called while not initialized");
	}
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::mutex_release(0x%x): pthread_mutex_unlock called.", this);
	if (pthread_mutex_unlock(&s_mutex) < 0) {
             	lib::logger::get_logger()->fatal("gstreamer_player::mutex_release() pthread_mutex_unlock failed: %s", strerror(errno));
	}
}


static void
mutex_acquire(void* obj) {
        gstreamer_player* player = (gstreamer_player*) obj;
        if (player) {
	        player->mutex_acquire();
        }
}

static void
mutex_release(void* obj) {
        gstreamer_player* player = (gstreamer_player*) obj;
        if (player) {
	        player->mutex_release();
        }
}

/* gstreamer_renderer_factory *********************************************/

gstreamer_renderer_factory::~gstreamer_renderer_factory()
{
}

common::playable *
gstreamer_renderer_factory::new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp)
{
	common::playable *rv = NULL;
	lib::xml_string tag = node->get_qname().second;
	AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory::new_playable: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "audio" && gui::gstreamer::gstreamer_audio_renderer::is_supported(node)) {
		rv = new gui::gstreamer::gstreamer_audio_renderer(context, cookie, node, evp, m_factory);
		AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory::new_playable: node 0x%x: returning gstreamer_audio_renderer 0x%x", (void *)node, (void *)rv);
	} else {
		AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory::new_playable: no GStreamer renderer for tag \"%s\"", tag.c_str());
                return NULL;
	}
	return rv;
}

common::playable*
gstreamer_renderer_factory::new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src)
{
	common::playable *rv = NULL;
	lib::xml_string tag = node->get_qname().second;
	AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory::new_aux_playable: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "audio" && gui::gstreamer::gstreamer_audio_renderer::is_supported(node)) {
		rv = new gui::gstreamer::gstreamer_audio_renderer(context, cookie, node, evp, m_factory);
		AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory::new_aux_playable: node 0x%x: returning gstreamer_audio_renderer 0x%x", (void *)node, (void *)rv);
	} else {
		AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory::new_aux_playable: no GStreamer renderer for tag \"%s\"", tag.c_str());
                return NULL;
	}
	return rv;
}

