#
# Examine a directory and recursively search for external
# shared libraries. Copy all of these to the directory,
# and modify the rpath headers.
#
# Note: currently specifically geared towards packaging
# the Ambulant firefox plugin.
#
import sys
import os
import shutil
import subprocess
import re

# RE that matches dylib references in MacOSX otool output:
OTOOL_MATCHER=re.compile(r'(.*) \(compatibility version .*, current version .*\)')
# RE that matches shared library reference in Linux objdump output:
NEEDED_MATCHER=re.compile(r'NEEDED\s+(.*)')
# RE that matches rpath directory in Linux objdump output:
RPATH_MATCHER=re.compile(r'RPATH\s+(.*)')


MACOSX_BUNDLE_DIRS = (
	'Contents/MacOS',
	'Contents/Frameworks',
	'Contents/PlugIns',
	[
		'/System/',
		'/lib/',
		'/usr/lib/',
		'@loader_path/',
		'@executable_path/',
	]
)

DEFAULT_RPATH=[
	'/lib',
	'/lib64',
	'/usr/lib',
	'/usr/lib64',
]

LINUX_BUNDLE_DIRS = (
	'.',
	'npambulant',
	'npambulant/plugins',
	DEFAULT_RPATH + ['$ORIGIN', '${ORIGIN}']
)

# XXX Needs work for iPhone (because of different relative paths)

class Internalizer:
	def __init__(self, bundle, (run_dir, framework_dir, plugin_dir, safe_prefixes)):
		self.bundle_dir = bundle
		self.run_dir = os.path.join(bundle, run_dir)
		self.framework_dir = None
		self.plugin_dir = None
		if framework_dir:
			self.framework_dir = os.path.join(bundle, framework_dir)
			self.destination_dir = self.framework_dir
		else:
			self.destination_dir = self.run_dir
		if plugin_dir:
			self.plugin_dir = os.path.join(bundle, plugin_dir)
		self.safe_prefixes = safe_prefixes
		self.safe_prefixes.append(bundle) # XXX good idea?
		
		self.todo = {}
		self.done = {}
		self.used = {}
				
		self.norun = False
		self.verbose = False
		self.work_done = False
		
		self.rpath = []
		env_rpath = os.getenv('LD_LIBRARY_PATH')
		if env_rpath:
			self.rpath += env_rpath.split(':')
		self.rpath += DEFAULT_RPATH
		
	def add_standard(self):
		for dirpath, dirnames, filenames in os.walk(self.run_dir):
			for name in filenames:
				name = os.path.join(dirpath, name)
				self.add(name)
		if self.plugin_dir:
			for dirpath, dirnames, filenames in os.walk(self.plugin_dir):
				for name in filenames:
					name = os.path.join(dirpath, name)
					self.add(name)
		if self.framework_dir:
			for dirpath, dirnames, filenames in os.walk(self.framework_dir):
				for name in filenames:
					name = os.path.join(dirpath, name)
					self.add(name)

	def add(self, src, copy=False):
		if 0:
			while os.path.islink(src):
				src = os.path.realpath(src)
		if src in self.todo or src in self.done:
			return
		if not self.is_loadable(src):
			return
		if self.verbose: print '* add', src
		if copy:
			dstname = os.path.basename(src)
			if dstname in self.used:
				print '** destname', dstname, 'in use'
				return
			self.used[dstname] = True
		else:
			dstname = None
		self.todo[src] = dstname
		
	def run(self):
		while self.todo:
			src, dst = self.todo.items()[0]
			self.process(src, dst)
			del self.todo[src]
			self.done[src] = dst
		
	def process(self, src, dst):
		if self.verbose: print '* process', src, dst
		origin = os.path.dirname(src)
		libraries, rpath = self.get_libs_rpath(src)
		must_change = []
		for lib in libraries:
			libpath = self.find_library(lib, rpath, origin)
			if not libpath:
				print '** Warning: cannot find', lib, 'referenced in', src
			elif self.must_copy(libpath):
				self.add(libpath, copy=True)
				must_change.append(lib)
		if dst:
			self.copy(src, dst)
			self.set_name(dst)
			src = os.path.join(self.destination_dir, dst)
		self.modify_rpath(src)
		for lib in must_change:
			self.modify_reference(src, lib)
		
	def copy(self, src, dst):
		if '.framework/' in src:
			print '** Warning: About to copy from framework:', src
		dstfilename = os.path.join(self.destination_dir, dst)
		if self.verbose:
			print 'copy', src, dstfilename
		if not self.norun:
			dstdir = os.path.dirname(dstfilename)
			if not os.path.exists(dstdir):
				os.mkdir(dstdir)
			shutil.copy(src, dstfilename)
		self.work_done = True
		
	def find_library(self, lib, rpath, origin):
		if os.path.isabs(lib):
			return lib
		#
		# The following is not 100% correct. We add every path found to our
		# global rpath, but it should really be added only to the rpath for
		# submodules loaded by this module.
		#
		if rpath:
			self.rpath += rpath.split(':')
		searchdirs = self.rpath
		for d in searchdirs:
			if not d: continue
			if d.startswith('$ORIGIN') or d.startswith('${ORIGIN}'):
				if d.startswith('$ORIGIN'):
					real_d = origin + d[7:]
				elif d.startswith('${ORIGIN}'):
					real_d = orgin + d[9:]
				if os.path.exists(os.path.join(real_d, lib)):
					return os.path.join('$ORIGIN', lib)
				continue
			trylib = os.path.join(d, lib)
			if os.path.exists(trylib):
				return trylib
		return None
		
	def get_libs_rpath(self, src):
		if 0:
			proc = subprocess.Popen(['otool', '-L', src],
				stdout=subprocess.PIPE)
			rv = []
			for line in proc.stdout.readlines():
				line = line.strip()
				matches = OTOOL_MATCHER.match(line)
				if matches:
					rv.append(matches.group(1))
			return rv, None
		else:
			proc = subprocess.Popen(['objdump', '-p', src],
				stdout = subprocess.PIPE)
			rv = []
			rpath = None
			for line in proc.stdout.readlines():
				line = line.strip()
				matches = NEEDED_MATCHER.match(line)
				if matches:
					rv.append(matches.group(1))
				matches = RPATH_MATCHER.match(line)
				if matches:
					if rpath:
						print '* Warning: Multiple RPATH directives in', src
					rpath = matches.group(1)
			return rv, rpath
			
		
	def set_name(self, dst):
		if 0:
			dstfilename = os.path.join(self.destination_dir, dst)
			dstfileid = os.path.join("@loader_path/../Frameworks/", os.path.basename(dst))
			if self.verbose:
				print 'setname', dstfilename, dstfileid
			if not self.norun:
				subprocess.check_call(['install_name_tool', '-id', dstfileid, dstfilename])
			self.work_done = True
			
	def modify_reference(self, dst, lib):
		if 0:
			reallib = os.path.realpath(lib)
			libid = os.path.join("@loader_path/../Frameworks/", os.path.basename(reallib))
			if self.verbose:
				print 'modify_lib_reference', dst, lib, libid
			if not self.norun:
				subprocess.check_call(['install_name_tool', '-change', lib, libid, dst])
			if not self.verbose and self.norun:
				print 'Warning: %s has reference to %s' % (dst, lib)
			self.work_done = True
			
	def modify_rpath(self, lib):
		origin = '$ORIGIN'
		libdir = os.path.dirname(lib)

		relpath = os.path.relpath(self.destination_dir, libdir)
		if relpath and relpath != '.':
			origin = os.path.join(origin, relpath)
		if self.verbose:
			print "chrpath -r '%s' %s", (origin, lib)
		if not self.norun:
			try:
				proc = subprocess.Popen(['chrpath', '-r', origin, lib])
			except OSError, arg:
				print '%s: Cannot execute "chrpath", did you install it?' % sys.argv[0]
				print '%s: (install through yum/apt/... or google for "chrpath")' % sys.argv[0]
				sys.exit(1)
			sts = proc.wait()
			if sts:
				print 'Warning: chrpath exit status', sts, 'for', lib
		# self.work_done = True
			
			
	def must_copy(self, lib):
		for prefix in self.safe_prefixes:
			if  os.path.commonprefix([prefix, lib]) == prefix:
				return False
		return True
		
	def is_loadable(self, file):
		proc = subprocess.Popen(['objdump', '-a', file], 
				stdout=open('/dev/null', 'w'),
				stderr=subprocess.STDOUT)
		sts = proc.wait()
		return sts == 0
		
def main():
	norun = False
	verbose = False
	check = False
	instlibdir = None
	reallibdir = None
	try:
		opts, args = getopt.getopt(sys.argv[1:], 'vncs:')
		for o, v in opts:
			if o == '-v':	
				verbose = True
			if o == '-n':
				norun = True
			if o == '-c':
				check = True
			if o == '-s':
				if not ':' in v:
					raise getopt.error
				instlibdir, reallibdir = v.split(':')
		if len(args) != 1:
			raise getopt.error
	except getopt.error:
		print 'Usage: %s [-vnc] [-s instlibdir:reallibdir] bundlepath '% sys.argv[0]
		print 'Recursively slurp dylibs used in a bundle.'
		print '-n\tNo-run, only print actions, do not do the work'
		print '-v\tVerbose, print actions as well as doing them'
		print '-c\tCheck, do nothing, print nothing, return nonzero exit status if there was work'
		print '-s\tSet library directory substitution (for uninstalled libraries)'
		sys.exit(1)
	internalizer = Internalizer(os.path.realpath(sys.argv[1]), LINUX_BUNDLE_DIRS)
	if norun:
		internalizer.norun = True
		internalizer.verbose = True
	elif verbose:
		internalizer.verbose = True
	elif check:
		internalizer.norun = True
		
	internalizer.add_standard()
	internalizer.run()
	
	if check:
		if internalizer.work_done:
			sys.exit(1)
		
if __name__ == '__main__':
	main()
	
