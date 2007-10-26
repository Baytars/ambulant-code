print "STEP ONE - Python code loaded into python plugin in ambulant in Safari"
import sys
print sys.path
import ambulant
print "STEP TWO - Ambulant version is", ambulant.get_version()

def initialize(*args):
    print "STEP THREE - Initialize called"
    print "ARGS", args
    import objc
    import WebKit
    print "STEP FOUR - pyobjc objc and WebKit imported!"