print "STEP ONE - Python code loaded into python plugin in ambulant in Safari"
import sys
##print sys.path
import ambulant
print "STEP TWO - Ambulant version is", ambulant.get_version()
import pyamplugin_webkitscripting.state
embedder = None
def set_extra_data(idd):
	global embedder
	embedder = idd
	
keeper_hack = []

def initialize(apiversion, factories, gui_player):
    print 'pyamplugin_webkitscripting: initialize() called'
    if not gui_player:
    	# This is the initial initialize call, before a document
    	# is opened. Ignore, we'll get another one later.
    	return

    keeper_hack.append(factories)
    keeper_hack.append(gui_player)
    
    print "pyamplugin_webkitscripting: WebPlugInContainer is", embedder
    embedder.webPlugInContainerShowStatus_("AmbulantWebKitPlugin: glue loaded")
    webframe = embedder.webFrame()
    print "pyamplugin_webkitscripting: WebFrame is", webframe
    domdocument = webframe.DOMDocument()

    sf = pyamplugin_webkitscripting.state.MyStateComponentFactory(domdocument)
    gsf = factories.get_state_component_factory()
    if not gsf:
        gsf = ambulant.get_global_state_component_factory()
        factories.set_state_component_factory(gsf)
    gsf.add_factory(sf)

##    statenode = domdocument.getElementById_("smilstate")
##    print "smilstate element is", statenode
##    firstlist = statenode.getElementsByTagName_("varone")
##    first = firstlist.item_(0)
##    print "first variable is", first
##    firstvalue = first.firstChild()
##    print "first variable value is", firstvalue
##    print "value was", firstvalue.nodeValue()
##    firstvalue.setNodeValue_("Een")
##    print "after setting, it is", firstvalue.nodeValue()
##    
    
