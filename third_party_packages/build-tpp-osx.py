import sys
import os
import subprocess
import urllib
import urlparse
import posixpath

NORUN=True

class TPP:
	def __init__(self, name, url, downloadedfile=None, extractcmd=None, checkcmd=None, buildcmd=None):
		self.name = name

		self.url = url
		if not downloadedfile:
			_, _, path, _, _, _ = urlparse.urlparse(url)
			downloadedfile = posixpath.basename(path)
		self.downloadedfile = downloadedfile
		self.checkcmd = checkcmd
		if not extractcmd:
			extractcmd = "tar xf %s" % self.downloadedfile
		self.extractcmd = extractcmd
		self.buildcmd = buildcmd
		if not buildcmd:
			buildcmd = "cd %s && ./configure && make && make install"
		self.buildcmd = buildcmd
		self.output = None
		
	def begin(self):
		self.output = open("log.%s.txt" % self.name, "w")
		
	def end(self):
		self.output.close()
		self.output = None
		
	def _command(self, cmd):
		print >>self.output, "+ run:", cmd
		if NORUN:
			print >>self.output, "+ dry run, skip execution"
			return True
		sts = subprocess.call(cmd, stdout=self.output, stderr=subprocess.STDOUT)
		print >>self.output, "+ run status:", sts
		return sts == 0
		
	def check(self):
		if not self.checkcmd:
			print >>self.output, "+ skip availability check"
			return True
		if NORUN:
			print >>self.output, "+ dry run, pretend failure:", self.checkcmd
			return False
		return self._command(self.checkcmd)
		
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

AMBULANT_DIR="%s/src/ambulant" % os.getenv("HOME")
COMMON_INSTALLDIR=os.path.join(os.getcwd(), "installed")
COMMON_CFLAGS="-arch i386 -arch x86_64"
COMMON_CONFIGURE="./configure --prefix='%s' CFLAGS='%s'	" % (COMMON_INSTALLDIR, COMMON_CFLAGS)

third_party_packages=[
	TPP("expat", 
		url="http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?use_mirror=autoselect",
		checkcmd="pkg-config --atleast-version=2.0.0 expat",
		buildcmd=
			"cd expat-2.0.1 && "
			"%s && "
			"make && "
			"make install" % COMMON_CONFIGURE
		),
	TPP("xerces-c",
		url="http://apache.mirror.transip.nl/xerces/c/3/sources/xerces-c-3.0.1.tar.gz",
		checkcmd="pkg-config --atleast-version=3.0.0 xerces-c",
		buildcmd=
			"cd xxxx && "
			"%s CXXFLAGS='%s' --disable-dependency-tracking && "
			"make && "
			"make install" % (COMMON_CONFIGURE, COMMON_CFLAGS)
		),
	TPP("faad2",
		url="http://downloads.sourceforge.net/project/faac/faad2-src/faad2-2.7/faad2-2.7.tar.gz?use_mirror=autoselect",
		checkcmd="test -f %s/lib/libfaad.a" % COMMON_INSTALLDIR,
		buildcmd=
			"cd faad2 && "
			"%s --disable-dependency-tracking && "
			"make && "
			"make install" % COMMON_CONFIGURE
		),
	TPP("ffmpeg",
		url="http://ffmpeg.org/releases/ffmpeg-0.5.tar.bz2",
		checkcmd="pkg-config --atleast-version=52.20.0 libavformat",
		buildcmd=
			"mkdir ffmpeg-0.5-universal && "
			"cd ffmpeg-0.5-universal && "
			"%s/third_party_packages/ffmpeg-osx-fatbuild.sh %s/ffmpeg-0.5 all" % 
				(AMBULANT_DIR, os.getcwd())
		),
	TPP("SDL",
		url="http://www.libsdl.org/tmp/SDL-1.3.tar.gz",
		checkcmd="pkg-config --atleast-version=1.3.0 sdl",
		buildcmd=
			"cd SDL-1.3.0-4703 && "
			"./configure --prefix='%s' "
				"CFLAGS='%s -framework ForceFeedback' "
				"LDFLAGS='-framework ForceFeedback' &&"
			"make && "
			"make install" % (COMMON_INSTALLDIR, COMMON_CFLAGS)
		),
	TPP("live",
		url="http://www.live555.com/liveMedia/public/live555-latest.tar.gz",
		checkcmd="test -f ./live/liveMedia/libliveMedia.a",
		buildcmd=
			"cd live && "
			"tar xf %s/third_party_packages/live-osx-fatbuild-patches.tar && "
			"./genMakefiles macosx3264 && "
			"make" % AMBULANT_DIR
		),
	TPP("gettext",
		url="http://ftp.gnu.org/pub/gnu/gettext/gettext-0.17.tar.gz",
		checkcmd="test -f %s/lib/libintl.a" % COMMON_INSTALLDIR,
		buildcmd=
			"cd gettext-0.17 && "
			"%s --disable-csharp && "
			"make && "
			"make install" % COMMON_CONFIGURE
		),
	TPP("libxml2",
		url="ftp://xmlsoft.org/libxml2/libxml2-2.7.5.tar.gz",
		checkcmd="pkgconfig --atleast-version=2.6.9 libxml-2.0",
		buildcmd=
			"cd libxml2-2.7.5 && "
			"%s --disable-dependency-tracking && "
			"make && "
			"make install" % COMMON_CONFIGURE
		)
]

def main():
	allok = True
	for pkg in third_party_packages:
		print "+ processing:", pkg.name
		ok = pkg.run()
		if ok:
			print "+ ok:", pkg.name
		else:
			print "+ failed:", pkg.name
			allok = False
	return allok
	
if __name__ == '__main__':
	sts = main()
	sys.exit(sts)