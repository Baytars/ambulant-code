#!/bin/sh
set -e -x

VERSION=2.3
if [ ! -f include/ambulant/version.h ]; then
	echo "Please run only in a toplevel ambulant directory"
	exit 1
fi
echo '+ May need: sudo sudo apt-get install dpkg-dev debhelper devscripts fakeroot'
if [ -d ambulant-debiandist-tmp ]; then
	rm -rf ambulant-debiandist-tmp
fi
mkdir ambulant-debiandist-tmp
cd ambulant-debiandist-tmp
hg clone .. ambulant-$VERSION
rm -rf ambulant-$VERSION/.hg
tar cfz ambulant_$VERSION.orig.tar.gz ambulant-$VERSION
cd ambulant-$VERSION
debuild -us -uc
