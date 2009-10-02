#
# Examine a framework and recursively search for external
# dynamic libraries. Copy all of these into the framework.
#
import sys
import os
import shutil
import subprocess
import re

OTOOL_MATCHER=re.compile(r'(.*) \(compatibility version .*, current version .*\)')

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


# XXX Needs work for iPhone (because of different relative paths)

class Internalizer:
	def __init__(self, bundle, (run_dir, framework_dir, plugin_dir, safe_prefixes)):
		self.bundle_dir = bundle
		self.run_dir = os.path.join(bundle, run_dir)
		self.framework_dir = os.path.join(bundle, framework_dir)
		self.plugin_dir = os.path.join(bundle, plugin_dir)
		self.safe_prefixes = safe_prefixes
		self.safe_prefixes.append(bundle) # XXX good idea?
		
		self.todo = {}
		self.done = {}
		self.used = {}
		
		for dirpath, dirnames, filenames in os.walk(self.run_dir):
			for name in filenames:
				name = os.path.join(dirpath, name)
				self.add(name)
		for dirpath, dirnames, filenames in os.walk(self.plugin_dir):
			for name in filenames:
				name = os.path.join(dirpath, name)
				self.add(name)
		for dirpath, dirnames, filenames in os.walk(self.framework_dir):
			for name in filenames:
				name = os.path.join(dirpath, name)
				self.add(name)
		
		self.norun = False
		
	def add(self, src, copy=False):
		print "* add?", src
		while os.path.islink(src):
			src = os.path.realpath(src)
		if src in self.todo or src in self.done:
			return
		if not self.is_loadable(src):
			return
		print '* add', src
		if copy:
			dstname = os.path.basename(src)
			if dstname in self.used:
				print '** destname', dstname, 'in use'
				import pdb ; pdb.set_trace()
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
		print
		print '* process', src, dst
		libraries = self.get_libs(src)
		must_change = []
		for lib in libraries:
			if self.must_copy(lib):
				self.add(lib, copy=True)
				must_change.append(lib)
		if dst:
			self.copy(src, dst)
			self.set_name(dst)
			src = dst
		for lib in must_change:
			self.modify_reference(src, lib)
		
	def copy(self, src, dst):
		if '.framework/' in src:
			print '** Warning: About to copy from framework:', src
		dstfilename = os.path.join(self.framework_dir, dst)
		if self.norun:
			print 'copy', src, dstfilename
		else:
			shutil.copy(src, dstfilename)
		
	def get_libs(self, src):
		proc = subprocess.Popen(['otool', '-L', src],
			stdout=subprocess.PIPE)
		rv = []
		for line in proc.stdout.readlines():
			line = line.strip()
			matches = OTOOL_MATCHER.match(line)
			if matches:
				rv.append(matches.group(1))
		return rv
		
	def set_name(self, dst):
		dstfilename = os.path.join(self.framework_dir, dst)
		dstfileid = os.path.join("@loader_path/../Frameworks/", os.path.basename(dst))
		if self.norun:
			print 'setname', dstfilename, dstfileid
		else:
			assert 0
			
	def modify_reference(self, dst, lib):
		reallib = os.path.realpath(lib)
		libid = os.path.join("@loader_path/../Frameworks/", os.path.basename(reallib))
		if self.norun:
			print 'modify_lib_reference', dst, lib, libid
			
	def must_copy(self, lib):
		for prefix in self.safe_prefixes:
			if  os.path.commonprefix([prefix, lib]) == prefix:
				print '* no_copy', lib
				return False
		print '* must_copy', lib
		return True
		
	def is_loadable(self, file):
		proc = subprocess.Popen(['file', file], 
				stdout=subprocess.PIPE,
				stderr=open('/dev/null', 'w'))
		line = proc.stdout.read()
		rv = ('Mach-O executable' in line or
			'Mach-O 64-bit executable' in line or
			'Mach-O bundle' in line or
			'Mach-O 64-bit bundle' in line or
			'Mach-O dynamically linked shared library' in line or
			'Mach-O 64-bit dynamically linked shared library' in line)
		sts = proc.wait()
		return rv
		
def main():
	if len(sys.argv) != 2:
		print 'Usage: %s bundlepath '% sys.argv[0]
		print 'Recursively slurp dylibs used in a bundle.'
		sys.exit(1)
	internalizer = Internalizer(os.path.realpath(sys.argv[1]), MACOSX_BUNDLE_DIRS)
	internalizer.norun = True
	internalizer.run()
		
if __name__ == '__main__':
	main()
	