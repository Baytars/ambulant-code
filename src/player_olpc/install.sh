#!/bin/sh
# install.sh - first part of an autoinstall file for Ambulant on an OLPC XO-machine
# Initial script for (un)install Ambulant on olpc
FILES="\
usr/lib/ambulant \
usr/lib/libambulant_ffmpeg.so \
usr/lib/libambulant_ffmpeg.so.0 \
usr/lib/libambulant_ffmpeg.so.0.0.0 \
usr/lib/libambulant_gtk.so \
usr/lib/libambulant_gtk.so.0 \
usr/lib/libambulant_gtk.so.0.0.0 \
usr/lib/libambulant_live.so \
usr/lib/libambulant_live.so.0 \
usr/lib/libambulant_live.so.0.0.0 \
usr/lib/libambulant_sdl.so \
usr/lib/libambulant_sdl.so.0 \
usr/lib/libambulant_sdl.so.0.0.0 \
usr/lib/libambulant.so \
usr/lib/libambulant.so.0 \
usr/lib/libambulant.so.0.0.0 \
usr/lib/libexpat.so \
usr/lib/libexpat.so.1 \
usr/lib/libexpat.so.1.5.2 \
usr/lib/libxerces-c-3.1.so \
usr/lib/libxerces-c.so \
usr/lib/libfaad.so \
usr/lib/libfaad.so.2 \
usr/lib/libfaad.so.2.0.0 \
usr/lib/libltdl.so \
usr/lib/libltdl.so.7 \
usr/lib/libltdl.so.7.2.0 \
usr/lib/libSDL-1.3.so.0 \
usr/lib/libSDL-1.3.so.0.0.0 \
usr/lib/libSDL.so \
usr/lib/libavcodec.so \
usr/lib/libavcodec.so.52 \
usr/lib/libavcodec.so.52.48.0 \
usr/lib/libavdevice.so \
usr/lib/libavdevice.so.52 \
usr/lib/libavdevice.so.52.2.0 \
usr/lib/libavformat.so \
usr/lib/libavformat.so.52 \
usr/lib/libavformat.so.52.47.0 \
usr/lib/libavutil.so \
usr/lib/libavutil.so.50 \
usr/lib/libavutil.so.50.7.0 \
usr/lib/libswscale.so \
usr/lib/libswscale.so.0 \
usr/lib/libswscale.so.0.9.0 \
usr/lib/python2.7/site-packages/ambulant-2.2-py2.7.egg-info \
usr/lib/python2.7/site-packages/ambulant.so \
usr/bin/AmbulantPlayer_gtk \
usr/bin/AmbulantPlayer \
usr/bin/ambulantplayerglue.py \
usr/bin/player_pygtk.py \
usr/share/ambulant \
usr/share/ambulant/*"
#DBG set -x
#DBG echo n=$# 

UNINSTALL=NO
HELP=NO
PRINTFILES=NO
NAME=ambulant-xo-installer.sh

case $# in
0)
	;;
1)
	if [ $1 = "-uninstall" ] ; then UNINSTALL=YES
	elif [ $1 = "-files" ] ; then PRINTFILES=YES
	else HELP=YES
	fi
	;;
esac
if [ $HELP = "YES" ] ; then echo "Usage: $NAME  [-uninstall -files -help ]"; exit
fi
if [ $PRINTFILES = "YES" ] ; then echo $FILES; exit
fi

PYTHON_VERSION=`python -c "import sys;version=sys.version_info;print str(version[0])+'.'+str(version[1])"`

if [ $UNINSTALL = "YES" ] ;
then cd /;sudo rm -fr $FILES usr/lib/python$PYTHON_VERSION/site-packages/ambulant*;
else 
    ZIP=$PWD/ambulant-xo-installer.sh
    cd /;sudo unzip $ZIP $FILES
    sudo cp usr/lib/python2.7/site-packages/ambulant-2.2-py2.7.egg-info \
	usr/lib/python2.7/site-packages/ambulant.so \
	usr/lib/python$PYTHON_VERSION/site-packages
fi
exit
X=<<EOF
# anything may follow
