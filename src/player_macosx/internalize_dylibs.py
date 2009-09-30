#
# Examine a framework and recursively search for external
# dynamic libraries. Copy all of these into the framework.
#

class Internalizer:
	def __init__(self, bundle):
		self.bundle_dir = bundle
		self.run_dir = os.path.join(bundle, 'Contents/MacOS')
		self.framework_dir = os.path.join(bundle, 'Contents/Frameworks')
		self.plugin_dir = os.path.join(bundle, 'Contents/PlugIns')
		
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
		
	def add(self, src, copy=False):
		if src in self.todo or src in self.done:
			return
		if not self.is_loadable(src):
			return
		if copy:
			dstname = os.path.basename(src)
			if dstname in self.used:
				assert 0
			self.used[dstname] = True
		else:
			dstname = None
		self.todo[src] = dstname
		
	def run(self):
		while self.todo:
			src, dst = self.todo.items[0]
			del self.todo[src]
			self.process(src, dst)
			self.done[src] = dst
		
	def process(self, src, dst):
		libraries = self.get_libs(src)
		must_change = []
		for lib in libraries:
			if self.must_copy(lib):
				self.add(lib, copy=True)
				must_change.append(lib)
		self.copy(src, dst)
		self.set_name(dst)
		for lib in must_change:
			self.modify_reference(dst, lib)
		
	def copy(self, src, dst):
		dstfilename = os.path.join(self.framework_dir, dst)
		shutil.copy(src, dstfilename)
		
	def get_libs(self, src):
		assert 0
		
	def set_name(self, dst):
		dstfilename = os.path.join(self.framework_dir, dst)
		dstfileid = os.path.join("@loader_path/../Frameworks/", dst)
		
		