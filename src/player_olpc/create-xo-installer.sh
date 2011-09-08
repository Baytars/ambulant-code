#!/bin/sh
# create-xo-installer - create an autoinstall file for Ambulant on an OLPC XO-machine
# to be run on Fedora 9 to get binary version compatibility with factory fresh XO-1
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
# The C-compiler flag -fabi-version=0 was added to achieve binary compatibilty with the
# libraries on factory installed XO-1 machines 
(cd ../../..;[ -e $PWD/configure ] || ./autogen.sh && ./configure CFLAGS=-fabi-version=0 CXXFLAGS=-fabi-version=0 --prefix=/usr --without-qt --with-python --with-python-plugin && make DESTDIR=$DEST install)
PYTHON_VERSION=`python -c "import sys;version=sys.version_info;print str(version[0])+'.'+str(version[1])"`
cp ../../pyambulant/player_pygtk/player_pygtk.py ./usr/bin
cp ../../pyambulant/player_pygtk/ambulantglue.py ./usr/lib/python$PYTHON_VERION/site-packages
zip -r -y ../Ambulant.zip `../install.sh -files`
cd ..
cat ./install.sh Ambulant.zip > ambulant-xo-installer.sh
chmod +x ambulant-xo-installer.sh
rm Ambulant.zip
