#!/bin/sh
set -x # uncomment for debug
if [ $OSTYPE = linux-gnu ]
then AMBULANT_EXTRAS=$HOME/share/ambulant
elif  [ $OSTYPE = cygwin ]
then AMBULANT_EXTRAS=/cygdrive/c/Program\ Files/AmbulantPlayer-2.0/Extras
fi

AMBULANTPLAYER=AmbulantPlayer 

testall() {
       for i in $* ; do
		   if [ ! -e $i ]
		   then return
		   fi
		   echo $i
		   $AMBULANTPLAYER $i
	   done
}

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


testall `ls -b "$AMBULANT_EXTRAS"/Welcome/*.smil`
testall `ls -b "$AMBULANT_EXTRAS"/DemoPresentation/*.smil`

testzip http://ambulantplayer.org/Demos/smilText.zip
testzip http://ambulantplayer.org/Demos/PanZoom.zip
testzip http://ambulantplayer.org/Demos/VideoTests.zip
testzip http://ambulantplayer.org/Demos/Birthday.zip
testzip http://ambulantplayer.org/Demos/Euros.zip
testzip http://ambulantplayer.org/Demos/Flashlight.zip
testzip http://ambulantplayer.org/Demos/News.zip

testall http://ambulantplayer.org/Demos/smilText/NYC-sT.smil
testall http://ambulantplayer.org/Demos/smilText/NYC-sT-rtsp.smil
testall http://ambulantplayer.org/Demos/PanZoom/Fruits-4s.smil
testall http://ambulantplayer.org/Demos/VideoTests/VideoTests.smil
testall http://ambulantplayer.org/Demos/VideoTests/VideoTests-rtsp.smil
testall http://ambulantplayer.org/Demos/Birthday/HappyBirthday.smil
testall http://ambulantplayer.org/Demos/Birthday/HappyBirthday-rtsp.smil
testall http://ambulantplayer.org/Demos/Euros/Euros.smil
testall http://ambulantplayer.org/Demos/Euros/Euros-rtsp.smil 
testall http://ambulantplayer.org/Demos/Flashlight/Flashlight-US.smil
testall http://ambulantplayer.org/Demos/Flashlight/Flashlight-US-rtsp.smil
testall http://ambulantplayer.org/Demos/News/DanesV2-Desktop.smil
testall http://ambulantplayer.org/Demos/News/DanesV2-Desktop-rtsp.smil

echo $0 " finished"
