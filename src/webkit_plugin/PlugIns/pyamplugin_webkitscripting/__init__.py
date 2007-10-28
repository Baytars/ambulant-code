print "STEP ONE - Python code loaded into python plugin in ambulant in Safari"
import sys
print sys.path
import ambulant
print "STEP TWO - Ambulant version is", ambulant.get_version()

embedder = None
def set_extra_data(idd):
	global embedder
	embedder = idd
	
def initialize(*args):
    print "STEP THREE - Initialize called"
    print "ARGS", args
    import objc
    import WebKit
    print "STEP FOUR - pyobjc objc and WebKit imported!"
    print "embedder is", embedder
    embedder.webPlugInContainerShowStatus_("AmbulantWebKitPlugin: glue loaded")
	