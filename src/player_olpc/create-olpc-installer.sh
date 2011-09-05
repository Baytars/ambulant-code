#!/bin/sh
set -x
#rm -fr tmp # commented out for incremental build
if [ ! -e tmp ]; then mkdir tmp; fi
cd tmp
python ../../../scripts/build-third-party-packages.py linux
mv installed usr
DEST=$PWD/usr
(cd ../../..;./configure  --prefix=/usr --without-qt --with-python --with-python-plugin; \
make DESTDIR=$DEST install)
cp ../../pyambulant/player_pygtk/player_pygtk.py usr/bin
cp ../../pyambulant/player_pygtk/ambulantglue.py usr/bin
zip -r --symlinks Ambulant.zip usr
cat install.sh Ambulant.zip > ambulant-installer.sh
chmod +x ambulant-installer.sh
#ffmpeg still missing