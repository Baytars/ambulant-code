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
    print "WebPlugInContainer is", embedder
    embedder.webPlugInContainerShowStatus_("AmbulantWebKitPlugin: glue loaded")
    webframe = embedder.webFrame()
    print "WebFrame is", webframe
    domdocument = webframe.DOMDocument()
    print "DOMDocument is", domdocument
    print "Contents:", dir(domdocument)
    statenode = domdocument.getElementById_("smilstate")
    print "smilstate element is", statenode
    firstlist = statenode.getElementsByTagName_("varone")
    first = firstlist.item_(0)
    print "first variable is", first
    firstvalue = first.firstChild()
    print "first variable value is", firstvalue
    print "value was", firstvalue.nodeValue()
    firstvalue.setNodeValue_("Een")
    print "after setting, it is", firstvalue.nodeValue()
    
    
