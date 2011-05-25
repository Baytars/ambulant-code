#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant
# Mac 10.6 version
#
set -e
set -x

# An optional parameter is the branch name, which also sets destination directory
BRANCH=
case x$1 in
x)	;;
*)	BRANCH=$1
esac

# Tunable parameters, to some extent
AMBULANTVERSION=2.3
ARCH=`uname -p`
HGARGS=""
HGCLONEARGS="http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant"
DESTINATION=sen5@ambulantplayer.org:/var/www/AmbulantPlayerOrg/nightlybuilds
BUILDHOME=$HOME/tmp/ambulant-nightly
TODAY=`date +%Y%m%d`

# The rest should be automatic
case x$BRANCH in
x)	;;
*)
	TODAY=$TODAY-$BRANCH
	DESTINATION=$DESTINATION/$BRANCH
esac
BUILDDIR=ambulant-build-$TODAY
DESTDIR=ambulant-install-$TODAY
BUILD3PPARGS=linux
CONFIGOPTS="--with-qt --with-gtk --with-xerces --with-xerces-plugin --with-npambulant"
MAKEOPTS=
VERSIONSUFFIX=.$TODAY
DESTINATION_SRC=$DESTINATION/src
DESTINATION_NPAMBULANT=$DESTINATION/linux-intel-firefoxplugin

echo
echo ==========================================================
echo Ambulant nightly build for Linux, $ARCH, $USER@`hostname`, `date`
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
rm -rf $BUILDDIR
rm -rf $DESTDIR
touch .empty
echo If the following command fails you have no SSH key that matches the destination
scp .empty $DESTINATION/.empty

ls -t | tail -n +6 | grep ambulant- | xargs chmod -R a+w .empty
ls -t | tail -n +6 | grep ambulant- | xargs rm -rf
hg $HGARGS clone $HGCLONEARGS $BUILDDIR
###
### We are building a binary distribution, so we want to completely ignore any
### library installed system-wide (in /usr/local, basically)
###
##export PKG_CONFIG_LIBDIR=$BUILDHOME/$BUILDDIR/third_party_packages/installed/lib/pkgconfig

#
# Prepare the tree
#
cd $BUILDDIR
case x$BRANCH in
x)	;;
*)
	hg up -r $BRANCH
esac
sh autogen.sh
cd third_party_packages
python ../scripts/build-third-party-packages.py $BUILD3PPARGS
cd ..


#
# Build source distribution, upload
#
./configure $CONFIGOPTS
make $MAKEOPTS distcheck
make $MAKEOPTS dist
mv ambulant-$AMBULANTVERSION.tar.gz ambulant-$AMBULANTVERSION$VERSIONSUFFIX.tar.gz
scp ambulant-$AMBULANTVERSION$VERSIONSUFFIX.tar.gz $DESTINATION_SRC

#
# configure, make, make install
#
./configure $CONFIGOPTS
make $MAKEOPTS
make $MAKEOPTS DESTDIR=$BUILDHOME/$DESTDIR install

#
# Create installer package, upload
#
# XXX TODO

#
# Build plugin installer, upload
#
cd src/npambulant
make installer
mv npambulant-$AMBULANTVERSION-linux-$ARCH.xpi npambulant-$AMBULANTVERSION$VERSIONSUFFIX-linux-$ARCH.xpi
scp npambulant-$AMBULANTVERSION$VERSIONSUFFIX-linux-$ARCH.xpi $DESTINATION_NPAMBULANT
cd ../..
#
# Delete old installers, remember current
#
# XXX TODO
