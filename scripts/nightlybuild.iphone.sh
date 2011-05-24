#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant
# iPhone version
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
BUILDDIR=ambulant-iphone-build-$TODAY
DESTDIR=ambulant-iphone-install-$TODAY
VERSIONSUFFIX=.$TODAY
DESTINATION_IPHONE=$DESTINATION/iphone/

echo
echo ==========================================================
echo Ambulant nightly build for MacOSX, $USER@`hostname`, `date`
echo ==========================================================
echo

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

ls -t | tail -n +6 | grep ambulant- | xargs chmod -R a+w
ls -t | tail -n +6 | grep ambulant- | xargs rm -rf
hg $HGARGS clone $HGCLONEARGS $BUILDDIR
#
# We are building a binary distribution, so we want to completely ignore any
# library installed system-wide (in /usr/local, basically)
#
export PKG_CONFIG_LIBDIR=$BUILDHOME/$BUILDDIR/build-iOS-Fat/third_party_packages/installed/lib/pkgconfig
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
#
# Build CG player
#
cd projects/xcode32
xcodebuild -project iAmbulant.xcodeproj \
	-target iAmbulant \
	-configuration Distribution \
	-sdk iphoneos4.2 \
	build
##	DSTROOT=$BUILDHOME/$DESTDIR \
##	INSTALL_PATH=/Applications \
cd ../..
#
# Create installer IPA file and upload
#
sh installers/mkiphonedist.sh iAmbulant-$AMBULANTVERSION.$TODAY.ipa projects/xcode32/build/Distribution-iphoneos/iAmbulant.app
scp iAmbulant-$AMBULANTVERSION.$TODAY.ipa $DESTINATION_IPHONE
#
# Delete old installers, remember current
#
# XXX TODO
