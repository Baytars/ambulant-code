import sys
import os
import subprocess
import urllib
import urlparse
import posixpath

NOCHECK=False
NORUN=False

WINDOWS_UNZIP="e:\\ufs\\jack\\bin\\unzip.exe"
WINDOWS_TAR='"c:\\Program Files\\GnuWin32\\bin\\tar.exe"'

class TPP:
    
    DEFAULT_BUILD_COMMAND="cd %s && ./configure && make && make install"
    DEFAULT_EXTRACT_COMMAND="tar xf %s"
    
    def __init__(self, name, url=None, downloadedfile=None, extractcmd=None, checkcmd=None, buildcmd=None):
        self.name = name

        self.url = url
        if url and not downloadedfile:
            _, _, path, _, _, _ = urlparse.urlparse(url)
            downloadedfile = posixpath.basename(path)
        self.downloadedfile = downloadedfile
        self.checkcmd = checkcmd
        if not extractcmd:
            extractcmd = self.DEFAULT_EXTRACT_COMMAND % self.downloadedfile
        self.extractcmd = extractcmd
        self.buildcmd = buildcmd
        if not buildcmd:
            buildcmd = self.DEFAULT_BUILD_COMMAND
        self.buildcmd = buildcmd
        self.output = None
        
    def begin(self):
        self.output = open("log.%s.txt" % self.name, "w")
        
    def end(self):
        self.output.close()
        self.output = None
        
    def _command(self, cmd, force=False):
        if not cmd:
            return True
        print >>self.output, "+ run:", cmd
        if NORUN and not force:
            print >>self.output, "+ dry run, skip execution"
            return True
        self.output.flush()
        sts = subprocess.call(cmd, shell=True, stdout=self.output, stderr=subprocess.STDOUT)
        print >>self.output, "+ run status:", sts
        return sts == 0
        
    def check(self):
        if not self.checkcmd:
            print >>self.output, "+ skip availability check"
            return True
        if NOCHECK:
            print >>self.output, "+ dry run, pretend failure:", self.checkcmd
            return False
        return self._command(self.checkcmd, force=True)
        
    def download(self):
        print >>self.output, "+ download:", self.url
        try:
            urllib.urlretrieve(self.url, self.downloadedfile)
        except IOError, arg:
            print >>self.output, "+ download status: error:", arg
            return False
        else:
            print >>self.output, "+ download status: success"
            return True
            
    def extract(self):
        return self._command(self.extractcmd)
        
    def build(self):
        return self._command(self.buildcmd)
        
    def run(self):
        self.begin()
        ok = True
        if self.check():
            print >>self.output, "+ already installed"
            self.end()
            return True
        ok = self.download()
        if ok:
            ok = self.extract()
        if ok:
            ok = self.build()
        if ok:
            print >>self.output, "+ installed"
        else:
            print >>self.output, "+ not installed"
        self.end()
        return ok

class WinTPP(TPP):

    DEFAULT_BUILD_COMMAND=None
    DEFAULT_EXTRACT_COMMAND=WINDOWS_UNZIP + " %s"
    
def appendPath(varname, value):
    """Append a value to a colon-separated environment variable, if
    it isn't in there already.
    """
    oldvalue = os.getenv(varname)
    if not oldvalue:
        os.putenv(varname, value)
        return
    
    oldlist = oldvalue.split(':')
    if not value in oldlist:
        oldlist.append(value)
    os.putenv(varname, ':'.join(oldlist))
    
# Locate ambulant base directory
dir=os.getcwd()
while dir != '/':
    dir = os.path.dirname(dir)
    if os.path.exists(os.path.join(dir, 'configure.in')):
        break
if dir == '/':
    print 'ERROR: cannot find Ambulant toplevel directory'
    sys.exit(1)
print '+ Ambulant toplevel directory:', dir
AMBULANT_DIR=dir
COMMON_INSTALLDIR=os.path.join(os.getcwd(), "installed")

#
# Common flags for MacOSX 10.4
#
MAC104_COMMON_CFLAGS="-arch i386 -arch ppc -isysroot /Developer/SDKs/MacOSX10.4u.sdk"
MAC104_COMMON_CONFIGURE="./configure --prefix='%s' CFLAGS='%s' CC=gcc-4.0 CXX=g++-4.0   " % (COMMON_INSTALLDIR, MAC104_COMMON_CFLAGS)

#
# Common flags for MacOSX 10.6
#
MAC106_COMMON_CFLAGS="-arch i386 -arch x86_64"
MAC106_COMMON_CONFIGURE="./configure --prefix='%s' CFLAGS='%s'  " % (COMMON_INSTALLDIR, MAC106_COMMON_CFLAGS)

#
# Common flags for iOS 3.0
#

# Initial iPhone support for iPhoneOS 3.0, unstable, unfinished.
# for now: --prefix=installed/arm; export IPHONEOS_DEPLOYMENT_TARGET=3.0; export ACOSX_DEPLOYMENT_TARGET=10.4; PATH=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin:$PATH
# other mods needed a couple of lines down
# packages not yet covered: ffmpeg, SDL and live555, can be build using these separate build procedures.
# ffmpeg: see: http://lists.mplayerhq.hu/pipermail/ffmpeg-devel/2009-October/076618.html
# SDL: use Xcode 3.0 with SDL-1.3.0-4429/Xcode-iPhoneOS/SDL/SDLiPhoneOS.xcodeproj
# live: use config.iphone30 as in http://cache.gmane.org//gmane/comp/multimedia/live555/devel/5394-001.bin
IPHONE30_COMMON_CONFIGURE="./configure --prefix='%s/arm' --host=arm-apple-darwin9 CC=arm-apple-darwin9-gcc-4.2.1  CXX=arm-apple-darwin9-g++-4.2.1 LD=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/ld CPP=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/cpp CFLAGS=-isysroot\ /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS3.0.sdk" % COMMON_INSTALLDIR
IPHONE30_COMMON_CFLAGS="-arch armv6 -isysroot /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iphoneOS3.0.sdk"
IPHONE30_COMMON_CONFIGURE="./configure --host=arm-apple-darwin9 --prefix='%s'/arm CFLAGS='%s' CC=gcc-4.2 CXX=g++-4.2    " % (COMMON_INSTALLDIR, IPHONE30_COMMON_CFLAGS)

#
# Common flags for Linux
#
LINUX_COMMON_CONFIGURE="./configure --prefix='%s'" % COMMON_INSTALLDIR

#
# Common Win32 flags
#
WIN32_COMMON_CONFIG="Release"
#WIN32_COMMON_CONFIG="Debug"

if os.path.sep == '/':
    #
    # Assume we are running on some Unix variant. Adapt PATH and PKG_CONFIG_PATH
    #
    appendPath("PATH", os.path.join(COMMON_INSTALLDIR, "bin"))
    appendPath("PKG_CONFIG_PATH", os.path.join(COMMON_INSTALLDIR, "lib/pkgconfig"))
    # for iPhone comment prev 2 lines, uncomment next 2 lines
    # appendPath("PATH", os.path.join(COMMON_INSTALLDIR, "arm/bin"))
    # appendPath("PKG_CONFIG_PATH", os.path.join(COMMON_INSTALLDIR, "arm/lib/pkgconfig"))
else:
    #
    # Assume we are running on Windows. Check that vcvars32.bat has been run.
    #
    if not os.getenv("DevEnvDir"):
        print "** This script needs the Visual Studio environment vars to be able to run"
        print '** Run "call ....\\VC\\bin\\vcvars32.bat" from your VC9 dir first.'
        sys.exit(1)
    
third_party_packages={
    'mac10.6' : [
        TPP("expat", 
            url="http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?use_mirror=autoselect",
            checkcmd="pkg-config --atleast-version=2.0.0 expat",
            buildcmd=
                "cd expat-2.0.1 && "
                "patch < %s/third_party_packages/expat.patch && "
                "autoconf && "
                "%s && "
                "make $(MAKEFLAGS) && "
                "make install" % (AMBULANT_DIR, MAC106_COMMON_CONFIGURE)
            ),
        TPP("xerces-c",
            url="http://apache.proserve.nl/xerces/c/3/sources/xerces-c-3.1.1.tar.gz",
            checkcmd="pkg-config --atleast-version=3.0.0 xerces-c",
            buildcmd=
                "cd xerces-c-3.1.1 && "
                "%s CXXFLAGS='%s' --disable-dependency-tracking && "
                "make $(MAKEFLAGS) && "
                "make install" % (MAC106_COMMON_CONFIGURE, MAC106_COMMON_CFLAGS)
            ),
        TPP("faad2",
            url="http://downloads.sourceforge.net/project/faac/faad2-src/faad2-2.7/faad2-2.7.tar.gz?use_mirror=autoselect",
            checkcmd="test -f %s/lib/libfaad.a" % COMMON_INSTALLDIR,
            buildcmd=
                "cd faad2-2.7 && "
                "%s --disable-dependency-tracking && "
                "make $(MAKEFLAGS) && "
                "make install" % MAC106_COMMON_CONFIGURE
            ),
##      TPP("ffmpeg",
##          url="http://ffmpeg.org/releases/ffmpeg-0.5.tar.bz2",
##          checkcmd="pkg-config --atleast-version=52.47.0 libavformat",
##          buildcmd=
##              "mkdir ffmpeg-0.5-universal && "
##              "cd ffmpeg-0.5-universal && "
##              "%s/third_party_packages/ffmpeg-osx-fatbuild.sh %s/ffmpeg-0.5 all" % 
##                  (AMBULANT_DIR, os.getcwd())
##          ),
        TPP("ffmpeg",
##          url="http://homepages.cwi.nl/~jack/ambulant/ffmpeg-export-2010-01-22.tgz",
            url="http://sourceforge.net/projects/ambulant/files/ffmpeg%20for%20Ambulant/ffmpeg-export-2010-01-22.tar.gz/download",
            checkcmd="pkg-config --atleast-version=52.47.0 libavformat",
            buildcmd=
                "mkdir ffmpeg-export-universal && "
                "cd ffmpeg-export-universal && "
                "%s/third_party_packages/ffmpeg-osx-fatbuild.sh %s/ffmpeg-export-2010-01-22 all" % 
                    (AMBULANT_DIR, os.getcwd())
            ),
        TPP("SDL",
            url="http://www.libsdl.org/tmp/SDL-1.3.tar.gz",
            checkcmd="pkg-config --atleast-version=1.3.0 sdl",
            buildcmd=
                "cd SDL-1.3.0-* && "
                "./configure --prefix='%s' --disable-dependency-tracking "
                    "CFLAGS='%s -framework ForceFeedback' "
                    "LDFLAGS='%s -framework ForceFeedback' &&"
                "make $(MAKEFLAGS) && "
                "make install" % (COMMON_INSTALLDIR, MAC106_COMMON_CFLAGS, MAC106_COMMON_CFLAGS)
            ),
        TPP("live",
            url="http://www.live555.com/liveMedia/public/live555-latest.tar.gz",
            checkcmd="test -f ./live/liveMedia/libliveMedia.a",
            buildcmd=
                "cd live && "
                "tar xf %s/third_party_packages/live-osx-fatbuild-patches.tar && "
                "./genMakefiles macosx3264 && "
                "make $(MAKEFLAGS) " % AMBULANT_DIR
            ),
        TPP("gettext",
            url="http://ftp.gnu.org/pub/gnu/gettext/gettext-0.17.tar.gz",
            checkcmd="test -f %s/lib/libintl.a" % COMMON_INSTALLDIR,
            buildcmd=
                "cd gettext-0.17 && "
                "%s --disable-csharp && "
                "make $(MAKEFLAGS) && "
                "make install" % MAC106_COMMON_CONFIGURE
            ),
        TPP("libxml2",
            url="ftp://xmlsoft.org/libxml2/libxml2-2.7.5.tar.gz",
            checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
            buildcmd=
                "cd libxml2-2.7.5 && "
                "%s --disable-dependency-tracking && "
                "make $(MAKEFLAGS) && "
                "make install" % MAC106_COMMON_CONFIGURE
            )
        ],
    'mac10.4' : [
        TPP("expat", 
            url="http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?use_mirror=autoselect",
            checkcmd="pkg-config --atleast-version=2.0.0 expat",
            buildcmd=
                "cd expat-2.0.1 && "
                "patch < %s/third_party_packages/expat.patch && "
                "autoconf && "
                "%s && "
                "make $(MAKEFLAGS) && "
                "make install" % (AMBULANT_DIR, MAC104_COMMON_CONFIGURE)
            ),
        TPP("xerces-c",
            url="http://apache.proserve.nl/xerces/c/3/sources/xerces-c-3.1.1.tar.gz",
            checkcmd="pkg-config --atleast-version=3.0.0 xerces-c",
            buildcmd=
                "cd xerces-c-3.1.1 && "
                "%s CXXFLAGS='%s' --disable-dependency-tracking --without-curl && "
                "make $(MAKEFLAGS) && "
                "make install" % (MAC104_COMMON_CONFIGURE, MAC104_COMMON_CFLAGS)
            ),
        TPP("faad2",
            url="http://downloads.sourceforge.net/project/faac/faad2-src/faad2-2.7/faad2-2.7.tar.gz?use_mirror=autoselect",
            checkcmd="test -f %s/lib/libfaad.a" % COMMON_INSTALLDIR,
            buildcmd=
                "cd faad2-2.7 && "
                "%s --disable-dependency-tracking && "
                "make $(MAKEFLAGS) && "
                "make install" % MAC104_COMMON_CONFIGURE
            ),
##      TPP("ffmpeg",
##          url="http://ffmpeg.org/releases/ffmpeg-0.5.tar.bz2",
##          checkcmd="pkg-config --atleast-version=52.47.0 libavformat",
##          buildcmd=
##              "mkdir ffmpeg-0.5-universal && "
##              "cd ffmpeg-0.5-universal && "
##              "%s/third_party_packages/ffmpeg-osx-fatbuild.sh %s/ffmpeg-0.5 all" % 
##                  (AMBULANT_DIR, os.getcwd())
##          ),
        TPP("ffmpeg",
            url="http://sourceforge.net/projects/ambulant/files/ffmpeg%20for%20Ambulant/ffmpeg-export-2010-01-22.tar.gz/download",
            checkcmd="pkg-config --atleast-version=52.47.0 libavformat",
            buildcmd=
                "mkdir ffmpeg-export-universal && "
                "cd ffmpeg-export-universal && "
                "%s/third_party_packages/ffmpeg-osx-fatbuild.sh %s/ffmpeg-export-2010-01-22 all" % 
                    (AMBULANT_DIR, os.getcwd())
            ),
##      TPP("SDL",
##          url="http://www.libsdl.org/release/SDL-1.2.13.tar.gz",
##          checkcmd="sdl-config",
##          buildcmd=
##              "cd SDL-1.2.13 && "
##              "./configure --prefix='%s' "
##                  "--disable-dependency-tracking "
##                  "--disable-video-x11 "
##                  "CC=gcc-4.0 CXX=g++-4.0 "
##                  "CFLAGS='%s' "
##                  "LDFLAGS='%s' &&"
##              "make $(MAKEFLAGS) && "
##              "make install" % (COMMON_INSTALLDIR, MAC104_COMMON_CFLAGS, MAC104_COMMON_CFLAGS)
##          ),
        TPP("SDL",
            url="http://www.libsdl.org/tmp/SDL-1.3.tar.gz",
            checkcmd="pkg-config --atleast-version=1.3.0 sdl",
            buildcmd=
                "cd SDL-1.3.0-* && "
                "./configure --prefix='%s' "
                    "--disable-dependency-tracking "
                    "CC=gcc-4.0 CXX=g++-4.0 "
                    "CFLAGS='%s' "
                    "LDFLAGS='%s -framework ForceFeedback' &&"
                "make $(MAKEFLAGS) && "
                "make install" % (COMMON_INSTALLDIR, MAC104_COMMON_CFLAGS, MAC104_COMMON_CFLAGS)
            ),
        TPP("live",
            url="http://www.live555.com/liveMedia/public/live555-latest.tar.gz",
            checkcmd="test -f ./live/liveMedia/libliveMedia.a",
            buildcmd=
                "cd live && "
                "tar xf %s/third_party_packages/live-osx-fatbuild-patches.tar && "
                "./genMakefiles macosxfat && "
                "make $(MAKEFLAGS) " % AMBULANT_DIR
            ),
        TPP("gettext",
            url="http://ftp.gnu.org/pub/gnu/gettext/gettext-0.17.tar.gz",
            checkcmd="test -f %s/lib/libintl.a" % COMMON_INSTALLDIR,
            buildcmd=
                "cd gettext-0.17 && "
                "%s --disable-csharp && "
                "make $(MAKEFLAGS) && "
                "make install" % MAC104_COMMON_CONFIGURE
            ),
        TPP("libxml2",
            url="ftp://xmlsoft.org/libxml2/libxml2-2.7.5.tar.gz",
            checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
            buildcmd=
                "cd libxml2-2.7.5 && "
                "%s --disable-dependency-tracking && "
                "make $(MAKEFLAGS) && "
                "make install" % MAC104_COMMON_CONFIGURE
            )
        ],
    'iPhone30' : [
        TPP("expat", 
            url="http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?use_mirror=autoselect",
            checkcmd="pkg-config --atleast-version=2.0.0 expat",
            buildcmd=
                "cd expat-2.0.1 && "
                "patch < %s/third_party_packages/expat.patch && "
                "autoconf && "
                "%s && "
                "make clean;make $(MAKEFLAGS) && "
                "make install" % (AMBULANT_DIR, IPHONE30_COMMON_CONFIGURE)
            ),
##      TPP("xerces-c",
##          url="http://apache.proserve.nl/xerces/c/3/sources/xerces-c-3.1.1.tar.gz",
##          checkcmd="pkg-config --atleast-version=3.0.0 xerces-c",
##          buildcmd=
##              "cd xerces-c-3.1.1 && "
##              "%s CXXFLAGS='%s' --disable-dependency-tracking --without-curl && "
##          "make clean;make $(MAKEFLAGS) && "
##              "make install" % (IPHONE30_COMMON_CONFIGURE, IPHONE30_COMMON_CFLAGS)
##          ),
        TPP("faad2",
            url="http://downloads.sourceforge.net/project/faac/faad2-src/faad2-2.7/faad2-2.7.tar.gz?use_mirror=autoselect",
            checkcmd="test -f %s/lib/arm/libfaad.a" % COMMON_INSTALLDIR,
            buildcmd=
                "cd faad2-2.7 && "
                "%s --disable-dependency-tracking && "
                "make clean;make $(MAKEFLAGS) && "
                "make install" % IPHONE30_COMMON_CONFIGURE
            ),
##      TPP("ffmpeg",
##          url="http://ffmpeg.org/releases/ffmpeg-0.5.tar.bz2",
##          checkcmd="pkg-config --atleast-version=52.47.0 libavformat",
##          buildcmd=
##              "mkdir ffmpeg-0.5-universal && "
##              "cd ffmpeg-0.5-universal && "
##              "%s/third_party_packages/ffmpeg-osx-fatbuild.sh %s/ffmpeg-0.5 all" % 
##                  (AMBULANT_DIR, os.getcwd())
##          ),
        TPP("ffmpeg",
            url="http://sourceforge.net/projects/ambulant/files/ffmpeg%20for%20Ambulant/ffmpeg-export-2010-01-22.tar.gz/download",
            checkcmd="pkg-config --atleast-version=52.47.0 libavformat",
            buildcmd="./configure --enable-cross-compile --arch=arm --target-os=darwin --cc=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/gcc --as='gas-preprocessor.pl  \
                /Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/gcc' --sysroot=/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS3.0.sdk --cpu=arm1176jzf-s --extra-cflags='-arch  \
                armv6 -I../installed/arm/include' --extra-ldflags='-arch armv6 -L../installed/arm/lib' --enable-libfaad --prefix=../installed/arm --enable-gpl"
                "make clean;make $(MAKEFLAGS); make install"
            ),
##      TPP("SDL",
##          url="http://www.libsdl.org/release/SDL-1.2.13.tar.gz",
##          checkcmd="sdl-config",
##          buildcmd=
##              "cd SDL-1.2.13 && "
##              "./configure --prefix='%s' "
##                  "--disable-dependency-tracking "
##                  "--disable-video-x11 "
##                  "CC=gcc-4.0 CXX=g++-4.0 "
##                  "CFLAGS='%s' "
##                  "LDFLAGS='%s' &&"
##              "make clean;make $(MAKEFLAGS) && "
##              "make install" % (COMMON_INSTALLDIR, IPHONE30_COMMON_CFLAGS, IPHONE30_COMMON_CFLAGS)
##          ),
##      TPP("SDL",
##          url="http://www.libsdl.org/tmp/SDL-1.3.tar.gz",
##          checkcmd="pkg-config --atleast-version=1.3.0 sdl",
##          buildcmd=
##              "cd SDL-1.3.0-* && "
##          "./configure --prefix='%s'/arm "
##                  "--disable-dependency-tracking "
##                  "CC=gcc-4.0 CXX=g++-4.0 "
##                  "CFLAGS='%s' "
##                  "LDFLAGS='%s -framework ForceFeedback' &&"
##              "make clean;make $(MAKEFLAGS) && "
##              "make install" % (COMMON_INSTALLDIR, IPHONE30_COMMON_CFLAGS, IPHONE30_COMMON_CFLAGS)
##          ),
        TPP("live",
            url="http://www.live555.com/liveMedia/public/live555-latest.tar.gz",
            checkcmd="test -f ./live/liveMedia/libliveMedia.a",
            buildcmd=
                "cd live && "
                "tar xf %s/third_party_packages/live-osx-fatbuild-patches.tar && "
                "./genMakefiles macosxfat && "
                "make clean;make $(MAKEFLAGS) " % AMBULANT_DIR
            ),
        TPP("gettext",
            url="http://ftp.gnu.org/pub/gnu/gettext/gettext-0.17.tar.gz",
            checkcmd="test -f %s/lib/libintl.a" % COMMON_INSTALLDIR,
            buildcmd=
                "cd gettext-0.17 && "
                "%s --disable-csharp && "
                "make clean;make $(MAKEFLAGS) && "
                "make install" % IPHONE30_COMMON_CONFIGURE
            ),
#       TPP("libxml2",
#           url="ftp://xmlsoft.org/libxml2/libxml2-2.7.5.tar.gz",
#           checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
#           buildcmd=
#               "cd libxml2-2.7.5 && "
#               "%s --disable-dependency-tracking && "
#               "make clean;make $(MAKEFLAGS) && "
#               "make install" % IPHONE30_COMMON_CONFIGURE
#           )
        ],
    'linux' : [
        TPP("libtool", 
            url="http://ftp.gnu.org/gnu/libtool/libtool-2.2.6a.tar.gz",
            checkcmd="test -f %s/lib/libltdl.a" % COMMON_INSTALLDIR,
            buildcmd=
                "cd libtool-2.2.6 && "
                        "%s --enable-ltdl-install &&"
                "make $(MAKEFLAGS) && "
                "make install" % LINUX_COMMON_CONFIGURE
            ),
        TPP("expat", 
            url="http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?use_mirror=autoselect",
            checkcmd="pkg-config --atleast-version=2.0.0 expat",
            buildcmd=
                "set -x;cd expat-2.0.1 && "
                "if [ ! -e expat.pc.in ] ; then patch < %s/third_party_packages/expat.patch; fi && "
                "autoconf && "
                "%s && "
                "make $(MAKEFLAGS) && "
                "make install" % (AMBULANT_DIR, LINUX_COMMON_CONFIGURE)
            ),
        TPP("xerces-c",
            url="http://apache.proserve.nl/xerces/c/3/sources/xerces-c-3.1.1.tar.gz",
            checkcmd="pkg-config --atleast-version=3.0.0 xerces-c",
            buildcmd=
                "cd xerces-c-3.1.1 && "
                "%s && "
                "make $(MAKEFLAGS) && "
                "make install" % (LINUX_COMMON_CONFIGURE)
            ),
        TPP("faad2",
            url="http://downloads.sourceforge.net/project/faac/faad2-src/faad2-2.7/faad2-2.7.tar.gz?use_mirror=autoselect",
            checkcmd="test -f %s/lib/libfaad.a" % COMMON_INSTALLDIR,
            buildcmd=
                "cd faad2-2.7 && "
                "%s && "
                "make $(MAKEFLAGS) && "
                "make install" % LINUX_COMMON_CONFIGURE
            ),
## xulrunner-sdk is only needed for building npambulant firefox plugin
        TPP("xulrunner-sdk",
            url="http://releases.mozilla.org/pub/mozilla.org/xulrunner/releases/1.9.2/sdk/xulrunner-1.9.2.en-US.linux-i686.sdk.tar.bz2",
            checkcmd="test -d xulrunner-sdk",
            buildcmd="test -d xulrunner-sdk"
            ),
## ogg-vorbis-theora decoding are default enabled on most Linux installations
##      TPP("libogg",
##          url=" http://downloads.xiph.org/releases/ogg/libogg-1.1.4.tar.gz",
##          checkcmd="test -f %s/lib/libogg-1.1.4.a" % COMMON_INSTALLDIR,
##          buildcmd=
##              "cd libogg-1.1.4  && "
##              "%s && "
##              "make $(MAKEFLAGS) && "
##              "make install" % LINUX_COMMON_CONFIGURE
##          ),
##      TPP("libvorbis",
##          url=" http://downloads.xiph.org/releases/vorbis/libvorbis-1.2.3.tar.gz",
##          checkcmd="test -f %s/lib/libvorbis-1.2.3.a" % COMMON_INSTALLDIR,
##          buildcmd=
##              "cd libvorbis-1.2.3  && "
##              "%s && "
##              "make $(MAKEFLAGS) && "
##              "make install" % LINUX_COMMON_CONFIGURE
##          ),
##      TPP("libtheora",
##          url=" http://downloads.xiph.org/releases/theora/libtheora-1.1.1.tar.bz2",
##          checkcmd="test -f %s/lib/libtheora-1.1.1.a" % COMMON_INSTALLDIR,
##          buildcmd=
##              "cd libtheora-1.1.1  && "
##              "%s && "
##              "make $(MAKEFLAGS) && "
##              "make install" % LINUX_COMMON_CONFIGURE
##          ),
## liboil and libschroedinger needed for dirac video decoding not yet supported
##      TPP("liboil",
##          url="http://liboil.freedesktop.org/download/liboil-0.3.17.tar.gz",
##          checkcmd="test -f %s/lib/liboil-0.3.a" % COMMON_INSTALLDIR,
##          buildcmd=
##              "cd liboil-0.3.17  && "
##              "%s && "
##              "make $(MAKEFLAGS) && "
##              "make install" % LINUX_COMMON_CONFIGURE
##          ),
##      TPP("libschroedinger",
##          url="http://diracvideo.org/download/schroedinger/schroedinger-1.0.7.tar.gz",
##          checkcmd="test -f %s/lib/libschroedinger-1.0.a" % COMMON_INSTALLDIR,
##          buildcmd=
##              "cd schroedinger-1.0.7  && "
##              "%s --disable-gstreamer && "
##              "make $(MAKEFLAGS) && "
##              "make install" % LINUX_COMMON_CONFIGURE
##          ),
##      TPP("ffmpeg",
##          url="http://ffmpeg.org/releases/ffmpeg-0.5.tar.bz2",
##          checkcmd="pkg-config --atleast-version=52.47.0 libavformat",
##          buildcmd=
##              "cd ffmpeg-0.5 && "
##              "%s --enable-gpl --enable-libfaad --enable-swscale --enable-shared --extra-cflags=-I%s/include --extra-ldflags=-L%s/lib&&"
##              "make $(MAKEFLAGS) && "
##              "make install " % 
##                  (LINUX_COMMON_CONFIGURE, COMMON_INSTALLDIR, COMMON_INSTALLDIR)
##          ),
        TPP("ffmpeg",
##          url="http://homepages.cwi.nl/~jack/ambulant/ffmpeg-export-2010-01-22.tgz",
            url="http://sourceforge.net/projects/ambulant/files/ffmpeg%20for%20Ambulant/ffmpeg-export-2010-01-22.tar.gz/download",
            checkcmd="pkg-config --atleast-version=52.47.0 libavformat",
            buildcmd=
                "cd ffmpeg-export-2010-01-22 && "
                "%s --enable-gpl --enable-libfaad --enable-shared --disable-bzlib --extra-cflags=-I%s/include --extra-ldflags=-L%s/lib&&"
                "make install " % 
                    (LINUX_COMMON_CONFIGURE, COMMON_INSTALLDIR, COMMON_INSTALLDIR)
            ),
        TPP("SDL",
            url="http://www.libsdl.org/tmp/SDL-1.3.tar.gz",
            checkcmd="pkg-config --atleast-version=1.3.0 sdl",
            buildcmd=
                "cd SDL-1.3.0-* && "
                "%s &&"
                "make $(MAKEFLAGS) && "
                "make install" % (LINUX_COMMON_CONFIGURE)
            ),
        TPP("live",
            url="http://www.live555.com/liveMedia/public/live555-latest.tar.gz",
            checkcmd="test -f ./live/liveMedia/libliveMedia.a",
            buildcmd=
                "cd live && "
                        "( grep fPIC config.linux >/dev/null || patch -i %s/third_party_packages/live.patch config.linux ) &&"
                "./genMakefiles linux && "
                "make $(MAKEFLAGS) " % (AMBULANT_DIR)
            ),
        TPP("gettext",
            url="http://ftp.gnu.org/pub/gnu/gettext/gettext-0.17.tar.gz",
            checkcmd="test -d %s/lib/gettext -o -d /usr/lib/gettext" % COMMON_INSTALLDIR,
            buildcmd=
                "cd gettext-0.17 && "
                "%s --disable-csharp && "
                "make $(MAKEFLAGS) && "
                "make install" % LINUX_COMMON_CONFIGURE
            ),
        TPP("libxml2",
            url="ftp://xmlsoft.org/libxml2/libxml2-2.7.5.tar.gz",
            checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
            buildcmd=
                "cd libxml2-2.7.5 && "
                "%s && "
                "make $(MAKEFLAGS) && "
                "make install" % LINUX_COMMON_CONFIGURE
            )
        ],
    'win32' : [
        WinTPP("expat-jpeg-lpng-mp3lib-zlib",
            url="https://sourceforge.net/projects/ambulant/files/Third%20Party%20Packages%2Cwin32_wm5/tpp-win-20081123/tpp-win-20081123.zip/download",
            checkcmd="if not exist expat exit 1",
            buildcmd='move "INTO third_party_packages"\\* .',
            # Real building is done by FINAL
            ),
            
        WinTPP("xerces-c",
            url="http://apache.proserve.nl/xerces/c/3/sources/xerces-c-3.1.1.zip",
            checkcmd="if not exist xerces-c-3.1.1\\Build\\Win32\\VC9\\%s\\xerces-c_3.lib exit 1" % WIN32_COMMON_CONFIG,
            buildcmd=
                "cd xerces-c-3.1.1\\projects\\Win32\\VC9\\xerces-all && "
                "devenv xerces-c-3.1.1\\projects\\Win32\\VC9\\xerces-all.sln /build %s /project XercesLib" % WIN32_COMMON_CONFIG
            ),
            
        WinTPP("xulrunner-sdk",
            url="http://releases.mozilla.org/pub/mozilla.org/xulrunner/releases/1.9.2/sdk/xulrunner-1.9.2.en-US.win32.sdk.zip",
            checkcmd="if not exist xulrunner-sdk\\sdk\\include\\npapi.h exit 1",
            # No build needed
            ),

        WinTPP("ffmpeg",
            url="https://sourceforge.net/projects/ambulant/files/ffmpeg%20for%20Ambulant%2C%20win32/20100122-win32-prebuilt/ffmpeg-20100122-win32-prebuilt.zip/download",
            checkcmd="if not exist ffmpeg-20100122-win32-prebuilt\\libavformat\\avformat-52.dll exit 1",
            # No build needed
            ),

        WinTPP("SDL",
            url="http://www.libsdl.org/tmp/SDL-1.2.14.zip",
            checkcmd="if not exist SDL-1.2.14\\VisualC\\SDL\\%s\\SDL.dll exit 1" % WIN32_COMMON_CONFIG,
            buildcmd=
                "cd SDL-1.2.14 && "
                "%s VisualC.zip &&"
                "devenv VisualC\\SDL.sln /build %s" % (WINDOWS_UNZIP, WIN32_COMMON_CONFIG)
            ),

        WinTPP("live",
            url="http://www.live555.com/liveMedia/public/live555-latest.tar.gz",
            extractcmd="%s xfz live555-latest.tar.gz" % WINDOWS_TAR,
            checkcmd="if not exist live\\liveMedia\\COPYING exit 1",
            # Build is done by FINAL
            ),
            
        WinTPP("libxml2",
            url="ftp://xmlsoft.org/libxml2/libxml2-2.7.7.tar.gz",
            extractcmd="%s xfz libxml2-2.7.7.tar.gz" % WINDOWS_TAR,
            checkcmd="if not exist libxml2-2.7.7\\xml2-config.in exit 1",
            # Build is done by FINAL
            ),
            
        WinTPP("FINAL",
            # The FINAL step builds some packages and copies everything to
            # where Ambulant expects it (bin\\win32 and lib\\win32)
            buildcmd="devenv ..\\projects\\vc9\\third_party_packages.sln /build %s" % WIN32_COMMON_CONFIG
            ),
        ],
    
}

def main():
    if len(sys.argv) != 2 or sys.argv[1] not in third_party_packages:
        print "Usage: %s platform" % sys.argv[0]
        print "Platform is one of:", ' '.join(third_party_packages.keys())
        return 2
    allok = True
    final_package = None
    for pkg in third_party_packages[sys.argv[1]]:
        if pkg.name == "FINAL":
            # Do this package last
            final_package=pkg
            continue
        print "+ processing:", pkg.name
        ok = pkg.run()
        if ok:
            print "+ ok:", pkg.name
        else:
            print "+ failed:", pkg.name
            allok = False
    if allok and final_package:
        print "+ processing FINAL package"
        ok = pkg.run()
        if ok:
            print "+ ok: FINAL package"
        else:
            print "+ failed: FINAL package"
            allok = False
    elif final_package:
        print "+ skipped: FINAL package, due to earlier errors"
    if not allok:
        return 1
    return 0
    
if __name__ == '__main__':
    sts = main()
    sys.exit(sts)
