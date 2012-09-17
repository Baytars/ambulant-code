/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2012 Kees Blom <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:basesrc-ambulantsrc
 *
 * FIXME:Describe ambulantsrc here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m ambulantsrc ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <gst/base/gstbasesrc.h>

#include "gstambulantsrc.h"

GST_DEBUG_CATEGORY_STATIC (gst_ambulantsrc_debug);
#define GST_CAT_DEFAULT gst_ambulantsrc_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */

static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ( "ANY" 
		      ) 
    );


GST_BOILERPLATE (GstAmbulantSrc, gst_ambulantsrc, GstBaseSrc,
    GST_TYPE_BASE_SRC);


static void gst_ambulantsrc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_ambulantsrc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static gboolean gst_ambulantsrc_start (GstBaseSrc * basesrc);
static gboolean gst_ambulantsrc_stop (GstBaseSrc * basesrc);
static GstCaps* gst_ambulantsrc_get_caps (GstBaseSrc * bsrc);
static gboolean gst_ambulantsrc_set_caps (GstBaseSrc * bsrc, GstCaps * caps);

/* GstBaseSrc virtual methods we need to override */

/* ask the subclass to create a buffer with offset and size */
static GstFlowReturn gst_ambulantsrc_create (GstBaseSrc * bsrc, guint64 offset, guint length,
					     GstBuffer ** buffer);
/* given a buffer, return start and stop time when it should be pushed
 * out. The base class will sync on the clock using these times. */
static void gst_ambulantsrc_get_times (GstBaseSrc *src, GstBuffer *buffer,
                                 GstClockTime *start, GstClockTime *end);
/* get the total size of the resource in bytes */
static gboolean gst_ambulantsrc_get_size (GstBaseSrc *src, guint64 *size);

void read_header(GstAmbulantSrc* asrc)
{
  if(!asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);

  if (asrc != NULL) {
    if (fscanf(stdin, "Time: %8lu\nSize: %8lu\nW: %5u\nH: %5u\n",
	       &asrc->timestamp, &asrc->datasize, &asrc->W, &asrc->H) < 0) {
      asrc->eos = TRUE;
    }
  }
}

void read_buffer(GstAmbulantSrc* asrc)
{
  if(!asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);

  if (asrc->eos) {
    return;
  }
  if (asrc != NULL && asrc->datasize != 0) {
    if (asrc->databuffer == NULL) {
      asrc->databuffer = g_malloc(asrc->datasize);
    } else {
      asrc->databuffer = g_realloc(asrc->databuffer, asrc->datasize);
    }
    clearerr(stdin);
    size_t n_bytes = fread (asrc->databuffer,1,asrc->datasize,stdin);
    if (n_bytes != asrc->datasize) {
      asrc->eos = TRUE;
    }
  }
}
/* GObject virtual method implementations */

static void
gst_ambulantsrc_base_init (gpointer gclass)
{
//fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);

  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple(element_class,
    "Ambulant Source",
    "Source",
    "Read data produced by 'ambulant-recorder-plugin' from 'stdin' (RGB 24bpp)"
    "and push these as buffers in a gstreamer pipeline",
    "Kees Blom <<Kees.Blom@cwi.nl>>");
   gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_template));
}

/* initialize the ambulantsrc's class */
static void
gst_ambulantsrc_class_init (GstAmbulantSrcClass * klass)
{
//fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);

  GObjectClass *gobject_class;
  GstBaseSrcClass *gstbasesrc_class;

  gobject_class = (GObjectClass *) klass;
  gstbasesrc_class = (GstBaseSrcClass *) klass;

  gobject_class->set_property = gst_ambulantsrc_set_property;
  gobject_class->get_property = gst_ambulantsrc_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));
  gstbasesrc_class->start = gst_ambulantsrc_start;
  gstbasesrc_class->stop = gst_ambulantsrc_stop;

  gstbasesrc_class->get_caps = gst_ambulantsrc_get_caps;
//gstbasesrc_class->set_caps = gst_ambulantsrc_set_caps; // disabled, SEGV
  gstbasesrc_class->get_size = gst_ambulantsrc_get_size;
  gstbasesrc_class->get_times = gst_ambulantsrc_get_times;
  gstbasesrc_class->create = gst_ambulantsrc_create;
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_ambulantsrc_init (GstAmbulantSrc * asrc,
    GstAmbulantSrcClass * gclass)
{
  asrc->silent = TRUE;
  if(!asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);
  asrc->eos = FALSE;
  asrc->gstbuffer = NULL;
  read_header(asrc);
  read_buffer(asrc);
  if ( ! asrc->eos) {
    GstBaseSrc* bsrc = (GstBaseSrc*) asrc;
    gst_base_src_set_blocksize (bsrc, asrc->datasize);
    gst_base_src_set_live (bsrc, TRUE);
  }
}

static void
gst_ambulantsrc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAmbulantSrc *asrc = GST_AMBULANTSRC (object);
  if(!asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);

  switch (prop_id) {
    case PROP_SILENT:
      asrc->silent = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_ambulantsrc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstAmbulantSrc *asrc = GST_AMBULANTSRC (object);
  if(!asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, asrc->silent);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */
static GstCaps* gst_ambulantsrc_get_caps (GstBaseSrc * bsrc)
{
  GstAmbulantSrc *asrc = GST_AMBULANTSRC (bsrc);
  GstCaps* caps =  gst_static_pad_template_get_caps(&src_template);
  gchar* s = gst_caps_to_string(caps);
  if(!asrc->silent)fprintf(stderr,"%s=%s\n", __PRETTY_FUNCTION__,s);

  free(s);
  return caps;
}
/* this function handles the link with other elements */
static gboolean
gst_ambulantsrc_set_caps (GstBaseSrc* bsrc, GstCaps * caps)
{
  GstAmbulantSrc *asrc = GST_AMBULANTSRC (bsrc);
  gchar* s = gst_caps_to_string(caps);
  if(!asrc->silent)fprintf(stderr,"%s=%s\n", __PRETTY_FUNCTION__,s);

  free(s);
  return TRUE;
}

static void gst_ambulantsrc_get_times (GstBaseSrc *src, GstBuffer *buffer,
					GstClockTime *start, GstClockTime *end)
{
  GstAmbulantSrc *asrc = GST_AMBULANTSRC(src);
  if(!asrc->silent)fprintf(stderr,"%s timestamp=%lu\n", __PRETTY_FUNCTION__, asrc->timestamp);

  if (start != NULL) {
    *start = asrc->timestamp * 1000000; // millis to nanos
  }
  if (end != NULL) {
    *end =  GST_CLOCK_TIME_NONE;
  }
}

/* get the total size of the resource in bytes */
static gboolean gst_ambulantsrc_get_size (GstBaseSrc *src, guint64 *size)
{
  GstAmbulantSrc *asrc = GST_AMBULANTSRC(src);
  if(!asrc->silent)fprintf(stderr,"%s=%ld\n", __PRETTY_FUNCTION__, asrc->datasize);

  return 0; // size of datafile undetermined
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
ambulantsrc_init (GstPlugin * ambulantsrc)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template ambulantsrc' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_ambulantsrc_debug, "ambulantsrc",
      0, "Template ambulantsrc");
  return gst_element_register (ambulantsrc, "ambulantsrc", GST_RANK_NONE,
      GST_TYPE_AMBULANTSRC);
}


static gboolean
gst_ambulantsrc_start (GstBaseSrc * basesrc)
{
  GstAmbulantSrc *asrc = GST_AMBULANTSRC (basesrc);
  if(!asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);

  // TBD GstAmbulantSrc *src;

  // TBD src = GST_AMBULANTSRC (basesrc);

  return TRUE;
}

static gboolean
gst_ambulantsrc_stop (GstBaseSrc * basesrc)
{
  GstAmbulantSrc *asrc = GST_AMBULANTSRC (basesrc);
  if(!asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);

  GST_OBJECT_LOCK (asrc);

  GST_OBJECT_UNLOCK (asrc);

  return TRUE;
}

static GstFlowReturn
gst_ambulantsrc_create (GstBaseSrc * bsrc, guint64 offset, guint length, GstBuffer ** buffer)
{
  GstAmbulantSrc *asrc = GST_AMBULANTSRC (bsrc);
  if(!asrc->silent)fprintf(stderr,"%s(bsrc=0x%p,offset=%lu,length=%u,buffer=0x%p)\n", __PRETTY_FUNCTION__,bsrc, offset, length, buffer);

  if (buffer == NULL) {
    return GST_FLOW_OK;
  }
  if (asrc->eos) {
    return GST_FLOW_UNEXPECTED; // end of stream
  }
  if (asrc->gstbuffer != NULL) {
    gst_buffer_unref (asrc->gstbuffer);
  }
  asrc->gstbuffer = gst_buffer_new();
  GST_BUFFER_DATA (asrc->gstbuffer) = asrc->databuffer;
  GST_BUFFER_SIZE (asrc->gstbuffer) = asrc->datasize;
  GST_BUFFER_TIMESTAMP (asrc->gstbuffer) = asrc->timestamp * 1000000; // millis to nanos
  GST_BUFFER_DURATION (asrc->gstbuffer) = GST_CLOCK_TIME_NONE;
  GST_BUFFER_OFFSET (asrc->gstbuffer) = offset;
  gst_buffer_ref(asrc->gstbuffer);
  *buffer = asrc->gstbuffer;

  read_header(asrc);
  read_buffer(asrc);

  return GST_FLOW_OK;

}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "gstambulantsrc"
#endif

/* gstreamer looks for this structure to register 'ambulantsrc'
 *
 * exchange the string 'Template ambulantsrc' with your ambulantsrc description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "ambulantsrc",
    "Template ambulantsrc",
    ambulantsrc_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
