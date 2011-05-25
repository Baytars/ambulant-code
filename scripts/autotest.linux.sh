#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant and test
# Linux Ubuntu 10.10 version
#
set -e
set -x
AMBULANTVERSION=2.3
ARCH=`uname -p`
HGARGS=""
HGCLONEARGS="http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant"
BUILDHOME=$HOME/tmp/ambulant-nightly
TODAY=`date +%Y%m%d`
BUILDDIR=ambulant-build-$TODAY
DESTDIR=ambulant-install-$TODAY
BUILD3PPARGS=linux
#X CONFIGOPTS="--with-qt --with-gtk --with-xerces --with-xerces-plugin --with-npambulant"
CONFIGOPTS="--prefix=$PWD/$DESTDIR --with-qt --with-gtk --with-xerces --with-xerces-plugin --without-npambulant -with-python --with-python-plugin"
MAKEOPTS=
VERSIONSUFFIX=.$TODAY
DESTINATION=sen5@ambulantplayer.org:/var/www/AmbulantPlayerOrg/nightlybuilds
DESTINATION_SRC=$DESTINATION/src
DESTINATION_NPAMBULANT=$DESTINATION/linux-intel-firefoxplugin

echo
echo ==========================================================
echo Ambulant nightly test for Linux, $ARCH, $USER@`hostname`, `date`
echo ==========================================================
echo

#
# Check that chrpath exists (which it often doesn't)
#
chrpath -v >/dev/null 2>&1
case $? in
0)
	;;
*)
	echo $0: chrpath not found, therefore nightly build will fail. Please install it first.
	exit 1
	;;
esac
#
# Check out a fresh copy of Ambulant
#
mkdir -p $BUILDHOME
cd $BUILDHOME
if [ -e $BUILDDIR ] ; then chmod -R +w $BUILDDIR ; fi
rm -rf $BUILDDIR
rm -rf $DESTDIR
ls -t | tail -n +6 | grep ambulant- | xargs rm -rf
hg $HGARGS clone $HGCLONEARGS $BUILDDIR
###
### We are building a binary distribution, so we want to completely ignore any
### library installed system-wide (in /usr/local, basically)
###
# export PKG_CONFIG_PATH=/usr/lib/pkgconfig:/usr/lib64/pkgconfig
export PKG_CONFIG_PATH=$BUILDHOME/$BUILDDIR/third_party_packages/installed/lib/pkgconfig:$PKG_CONFIG_PATH
#
# Prepare the tree
#
cd $BUILDDIR
hg pull;hg update #X
sh autogen.sh
cd third_party_packages
python ../scripts/build-third-party-packages.py $BUILD3PPARGS
cd ..
LIVE_DIR=$PWD/third_party_packages/live
LIVE_INCLUDES="-I$LIVE_DIR/BasicUsageEnvironment\ -I$LIVE_DIR/UsageEnvironment\ -I$LIVE_DIR/groupsock\ -I$LIVE_DIR/liveMedia"
LIVE_LIBRARIES="-L$LIVE_DIR/BasicUsageEnvironment\ -L$LIVE_DIR/UsageEnvironment\ -L$LIVE_DIR/groupsock\ -L$LIVE_DIR/liveMedia"
#
# Build source distribution, upload
#
#X cd ~/Ambulant/ambulant #X
#
# configure, make, make install
#
./configure $CONFIGOPTS

make $MAKEOPTS
which Xvfb >/dev/null 2>&1 
if [ $? -eq 0 ]
then
# for 'make check' to work during 'make distcheck', it is necessary to specify
# third party packages include files for compile (-Iflags), binaries 
# for linkage (-Lflag) and for running (LD_LIBRARY_PATH environment variable)
# Xvfb is needed to enable X server protocol from a cron-driven script
    ((Xvfb  :1& DISPLAY=:1 DISTCHECK_CONFIGURE_FLAGS=--with-python\ --with-python-plugin\ CFLAGS=$LIVE_INCLUDES\ LDFLAGS=$LIVE_LIBRARIES\ PKG_CONFIG_PATH=$BUILDHOME/$BUILDDIR/third_party_packages/installed/lib/pkgconfig LD_LIBRARY_PATH=$BUILDHOME/$BUILDDIR/third_party_packages/installed/lib:$LD_LIBRARY_PATH make $MAKEOPTS distcheck); killall -15 Xvfb) #X
else 
    echo "Xvfb not installed, skipping make check"
fi
make $MAKEOPTS dist
mv ambulant-$AMBULANTVERSION.tar.gz ambulant-$AMBULANTVERSION$VERSIONSUFFIX.tar.gz
#X scp ambulant-$AMBULANTVERSION$VERSIONSUFFIX.tar.gz $DESTINATION_SRC
make $MAKEOPTS DESTDIR=$BUILDHOME/$DESTDIR install

#
# Create installer package, upload
#
# XXX TODO

#
# Build plugin installer, upload
#
#X cd src/npambulant
#X make installer
#X mv npambulant-$AMBULANTVERSION-linux-$ARCH.xpi npambulant-$AMBULANTVERSION$VERSIONSUFFIX-linux-$ARCH.xpi
#X scp npambulant-$AMBULANTVERSION$VERSIONSUFFIX-linux-$ARCH.xpi $DESTINATION_NPAMBULANT
cd ../..
#
# Delete old installers, remember current
#
# XXX TODO
