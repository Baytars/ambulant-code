#!/bin/sh
# create-xo-installer - create an autoinstall file for Ambulant on an OLPC XO-machine
set -x
#rm -fr tmp # commented out for incremental (i.o. clean) build
# make sure third-party-packages are installed in the default place (needed for live)
(cd ../../third_party_packages;python ../scripts/build-third-party-packages.py linux)
if [ ! -e tmp ]; then mkdir tmp; fi
cd tmp
if [ ! -e usr ]; then mkdir usr; fi
# copy everything (possibly updated) ./installed to ./usr
(cd ../../../third_party_packages/installed;tar cf - .)|(cd usr;tar xf -)
DEST=$PWD
PATH=$DEST/usr/bin:$PATH
(cd ../../..;[ -e $PWD/configure ] || ./autogen.sh && ./configure CFLAGS=-fabi-version=0 CXXFLAGS=-fabi-version=0 --prefix=/usr --without-qt --with-python --with-python-plugin && make DESTDIR=$DEST install)
cp ../../pyambulant/player_pygtk/player_pygtk.py tmp/usr/bin
cp ../../pyambulant/player_pygtk/ambulantglue.py tmp/usr/bin
zip -r -y ../Ambulant.zip `../install.sh -files`
cd ..
cat ./install.sh Ambulant.zip > ambulant-xo-installer.sh
chmod +x ambulant-xo-installer.sh
rm Ambulant.zip
