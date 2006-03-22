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

#define AM_DBG
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
lock_release(void* rend);

static void
pipeline_store(void* rend, GstElement* p);

extern "C" {

/* from sanbox/Nokia770/AudioPlayer/mp3player.c */

static void
mp3player_eos_cb (GstElement * bin, gpointer user_data);
static void
mp3player_error_cb (GstElement * bin, GstElement * error_element,
		    GError * error,  const gchar * debug_msg,
		    gpointer user_data);

int
gst_mp3_player(const char* uri, GstElement** gst_player_p, gstreamer_audio_renderer* rend)
{
  GMainLoop *loop;
  GstElement *source, *pipeline;
  char **files = NULL;
  gboolean gst_mp3_player_done = FALSE;

  /* create elements */
  pipeline = (GstElement*)gst_pipeline_new ("mp3-player");
  source   = gst_element_factory_make ("playbin", "playbin");
  
  if (gst_player_p) *gst_player_p = source;

  if ( !( pipeline && source) ) {
    g_print ("%s:", "gst_mp3_player");
    if ( ! pipeline) g_print (" %s() failed", "get_pipeline_new");
    if ( ! source)   g_print (" %s=%s(%s) failed", "source", 
			      "gst_element_factory_make", "filesrc");

    g_print ("\n");
    return -1;
  }

  /* set the source audio file */
  g_object_set (G_OBJECT(source), "uri",uri, NULL);

  /* put all elements  to the main pipeline */
  gst_bin_add_many (GST_BIN(pipeline), source, NULL);

  /* link the elements */

  /* add call-back message handlers to check for eos and errors */
  g_signal_connect (GST_BIN(pipeline), "eos",
		    G_CALLBACK (mp3player_eos_cb), &gst_mp3_player_done);
  g_signal_connect (GST_BIN(pipeline), "error",
		    G_CALLBACK (mp3player_error_cb), &gst_mp3_player_done);

  /* wait for start */
  gst_element_set_state (GST_ELEMENT(pipeline), GST_STATE_PAUSED);

  lock_release(rend);
  pipeline_store(rend, pipeline);

   /* iterate */
  if ( ! gst_mp3_player_done) g_print ("Now playing %s ...", uri);
  while ( ! gst_mp3_player_done) {
     gst_bin_iterate (GST_BIN(pipeline));
     sleep(1);
  }
  /* stop the pipeline */
  gst_element_set_state (GST_ELEMENT(pipeline), GST_STATE_NULL);
  if ( ! gst_mp3_player_done) g_print (" done !"); 
  g_print ("\n");


  /* cleanup */
  gst_object_unref (GST_OBJECT(pipeline));

  return 0;
}

static void
mp3player_eos_cb (GstElement *pipeline,gpointer user_data)
{
  gboolean *p_gst_mp3_player_done = (gboolean *) user_data;

  *p_gst_mp3_player_done = TRUE;
}

static void
mp3player_error_cb (GstElement *pipeline, GstElement *error_element,
		    GError *error, const gchar *debug_msg,
gpointer user_data)
{
  gboolean *p_gst_mp3_player_done = (gboolean *) user_data;

  if (error)
    g_printerr ("Error: %s", error->message);

  *p_gst_mp3_player_done = TRUE;
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
		m_lock.leave();
		//TBD stop it
		m_lock.enter();
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

void
gstreamer_audio_renderer::lock_release() {
	m_lock.leave();
}

static void
pipeline_store(void* obj, GstElement*pipeline) {
	gstreamer_audio_renderer* rend = (gstreamer_audio_renderer*) obj;
	if (rend) rend->m_pipeline = pipeline;
}

static void
lock_release(void* obj) {
	gstreamer_audio_renderer* rend = (gstreamer_audio_renderer*) obj;
	if (rend) rend->lock_release();
}

void
gstreamer_audio_renderer::init_player(const lib::node *node) {
	assert (node);
	m_url = node->get_url("src");
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer::gstreamer_audio_renderer(0x%x) url=",  this, m_url.get_url().c_str());
	_init_clip_begin_end();
	m_player = new gstreamer_player(m_url.get_url().c_str(), this);
	m_lock.enter(); // thread will release the lock
	m_player->start();
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
	if (m_is_playing) {
		m_lock.leave();
		// XXX Should we call stopped_callback?
		m_context->stopped(m_cookie, 0);
		m_lock.enter();
	}
	m_is_playing = false;
	m_lock.leave();
}

void
gstreamer_audio_renderer::pause()
{
	m_lock.enter();
	gst_element_set_state (m_player->gst_player(), GST_STATE_PAUSED);
	m_is_paused = true;
	m_lock.leave();
}

void
gstreamer_audio_renderer::resume()
{
	m_lock.enter();
	gst_element_set_state (m_player->gst_player(), GST_STATE_PLAYING);
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
       if (m_player) {
              double microsec = 1e6;
	      guint64 where_guint64;
	      where += (m_clip_begin / microsec);
	      where_guint64 = llrint(where)* GST_SECOND;	      
	      lib::logger::get_logger()->trace("gstreamer_audio_renderer: seek() where=%f, where_guint64=%lu", where, where_guint64);
	      if ( ! gst_element_seek(m_player->gst_player(), (GstSeekType) (GST_SEEK_METHOD_SET | GST_FORMAT_TIME | GST_SEEK_FLAG_FLUSH), where_guint64)) {
	             lib::logger::get_logger()->trace("gstreamer_audio_renderer: seek() failed.");
	      }
       }
}

common::duration 
gstreamer_audio_renderer::get_dur()
{
	gint64 length = -1;
	GstFormat fmtTime = GST_FORMAT_TIME;
	common::duration rv(false, 0.0);

	m_lock.enter();
	if (m_dest) {
	       gst_element_query(m_player->gst_player(), GST_QUERY_TOTAL, &fmtTime, &length);
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
}

gstreamer_player::~gstreamer_player() {
	delete m_uri;
}

unsigned long
gstreamer_player::run() {
        gst_mp3_player (m_uri, &m_gst_player, m_audio_renderer);
}

unsigned long
gstreamer_player::init() {
        start();
	return 0;
}

GstElement*
gstreamer_player::gst_player() {
	return m_gst_player;
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
	common::playable *rv;
	lib::xml_string tag = node->get_qname().second;
    AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "audio") {
		rv = new gui::gstreamer::gstreamer_audio_renderer(context, cookie, node, evp, m_factory);
		AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory: node 0x%x: returning gstreamer_audio_renderer 0x%x", (void *)node, (void *)rv);
	} else {
		AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory: no GStreamer renderer for tag \"%s\"", tag.c_str());
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
	common::playable *rv;
	lib::xml_string tag = node->get_qname().second;
    AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory:new_aux_audio_playable: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	rv = new gui::gstreamer::gstreamer_audio_renderer(context, cookie, node, evp, m_factory, src);
	AM_DBG lib::logger::get_logger()->debug("gstreamer_renderer_factory: node 0x%x: returning gstreamer_audio_renderer 0x%x", (void *)node, (void *)rv);
	return rv;
}
