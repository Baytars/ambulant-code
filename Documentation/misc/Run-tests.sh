#!/bin/sh
# set -x # uncomment for debug
AMBULANT_PLAYER=AmbulantPlayer 
if [ $OSTYPE = linux-gnu ]
then AMBULANT_EXTRAS=$HOME/share/ambulant
elif  [ $OSTYPE = cygwin ]
then AMBULANT_EXTRAS=/cygdrive/c/Program\ Files/AmbulantPlayer-2.0/Extras
	[3~[3~[3~[3~[3~AMBULANT_PLAYER=/cygwin/c/Program Files[D[D[D[D[D[D\ Files/AmbulantPlayer-2.0/AmbulantPlayer.exe
fi

AMBULANT_PLAYER=AmbulantPlayer 

# test all sepcified files/urls 
testall() {
       for i in $* ; do
		   echo $i
		   "$AMBULANT_PLAYER" $i
	   done
}

# get specified zip file from web, unpack and test all contained .smil files
testzip() {
	dir=`basename $1 .zip` 
	echo wget:  $1 $dir
	wget $1
	zipfile=$dir.zip
	mkdir -p $dir
	cd $dir
	unzip -qq ../$zipfile
#	echo *.smil
	testall *.smil
	cd ..
	rm -fr $zipfile $dir
}


(cd "$AMBULANT_EXTRAS/Welcome";testall *.smil)
(cd "$AMBULANT_EXTRAS/DemoPresentation";testall *.smil)

testzip http://www.ambulantplayer.org/Demos/smilText.zip
testzip http://www.ambulantplayer.org/Demos/PanZoom.zip
testzip http://www.ambulantplayer.org/Demos/VideoTests.zip
testzip http://www.ambulantplayer.org/Demos/Birthday.zip
testzip http://www.ambulantplayer.org/Demos/Euros.zip
testzip http://www.ambulantplayer.org/Demos/Flashlight.zip
testzip http://www.ambulantplayer.org/Demos/News.zip

testall http://www.ambulantplayer.org/Demos/smilText/NYC-sT.smil
testall http://www.ambulantplayer.org/Demos/smilText/NYC-sT-rtsp.smil
testall http://www.ambulantplayer.org/Demos/PanZoom/Fruits-4s.smil
testall http://www.ambulantplayer.org/Demos/VideoTests/VideoTests.smil
testall http://www.ambulantplayer.org/Demos/VideoTests/VideoTests-rtsp.smil
testall http://www.ambulantplayer.org/Demos/Birthday/HappyBirthday.smil
testall http://www.ambulantplayer.org/Demos/Birthday/HappyBirthday-rtsp.smil
testall http://www.ambulantplayer.org/Demos/Euros/Euros.smil
testall http://www.ambulantplayer.org/Demos/Euros/Euros-rtsp.smil 
testall http://www.ambulantplayer.org/Demos/Flashlight/Flashlight-US.smil
testall http://www.ambulantplayer.org/Demos/Flashlight/Flashlight-US-rtsp.smil
testall http://www.ambulantplayer.org/Demos/News/DanesV2-Desktop.smil
testall http://www.ambulantplayer.org/Demos/News/DanesV2-Desktop-rtsp.smil

echo $0 " finished"
