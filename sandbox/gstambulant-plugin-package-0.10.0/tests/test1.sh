#!/bin/sh
# set -x
GST_PLUGIN_PATH=/usr/local/lib/gstreamer-0.10 # change this appropriately when a --prefix 'configure' option value is used
# TBD get the caps from tests/input and use these in the sink pad of the plugin
echo $srcdir
gst-launch --gst-debug-level=2 ambulantsrc  ! ffmpegcolorspace ! sdlvideosink < $srcdir/input
if  [ ! $? ]
then   gst-launch --gst-debug-level=2 ambulantsrc ! ffmpegcolorspace ! autovideosink < $srcdir/input
fi
if  [ ! $? ]
then gst-launch --gst-debug-level=2 ambulantsrc  ! ffmpegcolorspace ! videoscale ! ximagesink sync=false < $srcdir/input
fi