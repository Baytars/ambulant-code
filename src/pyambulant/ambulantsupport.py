# This script generates a Python interface for an Apple Macintosh Manager.
# It uses the "bgen" package to generate C code.
# The function specifications are generated by scanning the mamager's header file,
# using the "scantools" package (customized for this particular manager).

import string
from bgen import *
from bgenCxxSupport import *
# Declarations that change for each manager
MODNAME = 'ambulant'                         # The name of the module

# The following is *usually* unchanged but may still require tuning
MODPREFIX = 'PyAm'                        # The prefix for module-wide routines
INPUTFILE = 'ambulantgen.py' # The file generated by the scanner
PY2CXXFILE = MODNAME + "module.cpp"       # The Python to C++ glue code
CXX2PYFILE = MODNAME + "interface.cpp"   # The C++ to Python glue code
CXX2PYDECLFILE = MODNAME + "interface.h"     # The C++ to Python declarations

# Create the type objects

includestuff = """
#define WITH_EXTERNAL_DOM 1
#include "ambulant/config/config.h"
#include "ambulant/version.h"
"""
execfile("ambulantincludegen.py")

includestuff = includestuff + """

#include "ambulantinterface.h"
#include "ambulantutilities.h"
#include "ambulantmodule.h"

extern PyObject *audio_format_choicesObj_New(ambulant::net::audio_format_choices *itself);
extern int audio_format_choicesObj_Convert(PyObject *v, ambulant::net::audio_format_choices *p_itself);
"""

finalstuff = """
// Declare initambulant as a C external:

extern "C" void initambulant(); 
"""

initstuff = """
PyEval_InitThreads();
"""

variablestuff="""
"""

print "=== Defining simple types ==="

bool = OpaqueByValueType("bool", "bool")
size_t = Type("size_t", "l")
unsigned_int = Type("unsigned int", "l")
progress_type = Type("ambulant::lib::transition_info::progress_type", "d")
std_string = StdStringType()
xml_string = StdStringType("ambulant::lib::xml_string")
const_xml_string_ref = StdStringType("const ambulant::lib::xml_string&")
q_name_pair = StdPairType(xml_string, xml_string, "ambulant::lib::q_name_pair")
const_q_name_pair_ref = StdPairType(xml_string, xml_string, 
    "const ambulant::lib::q_name_pair&", "ambulant::lib::q_name_pair")
duration = StdPairType(bool, double, "ambulant::common::duration")

InBuffer = VarInputBufferType('char', 'size_t', 'l')
return_stringptr = Type("const char *", "s")  # ONLY FOR RETURN VALUES!!
# output_stringptr = Type("char *", "s")  # BE CAREFUL!
output_malloc_buf = MallocHeapOutputBufferType("char", "size_t", "l")

# Ambulant-specific
q_attributes_list = OpaqueByRefType("ambulant::lib::q_attributes_list", "ambulant_attributes_list")
region_dim = OpaqueByRefType("ambulant::common::region_dim", "ambulant_region_dim")
net_url = OpaqueByRefType("ambulant::net::url", "ambulant_url")
rect = OpaqueByRefType("ambulant::lib::rect", "ambulant_rect")
point = OpaqueByRefType("ambulant::lib::point", "ambulant_point")
const_lib_point_ref = OpaqueByRefType("const ambulant::lib::point&", "ambulant_point")
const_lib_rect_ref = OpaqueByRefType("const ambulant::lib::rect&", "ambulant_rect")
size = OpaqueByRefType("ambulant::lib::size", "ambulant_size")
zindex_t = Type("ambulant::common::zindex_t", "l")
cookie_type = Type("ambulant::common::playable::cookie_type", "l")
const_cookie_type = cookie_type
color_t = Type("ambulant::lib::color_t", "l") # XXXX Split into RGB
event_priority = Type("ambulant::lib::event_priority", "l")
timestamp_t = Type("ambulant::net::timestamp_t", "L")
time_type = Type("ambulant::lib::timer::time_type", "l")
tiling = Type("ambulant::common::tiling", "l")
fit_t = Type("ambulant::common::fit_t", "l")
sound_alignment = Type("ambulant::common::sound_alignment", "l")

# This is a bit of a hack. These types are opaque, really.
renderer_private_data_ptr = Type("ambulant::common::renderer_private_data *", "l")
renderer_private_id = Type("ambulant::common::renderer_private_id", "l")

# Our (opaque) objects

class MyGlobalObjectDefinition(CxxMixin, PEP253Mixin, GlobalObjectDefinition):

    def __init__(self, name, prefix, itselftype):
        GlobalObjectDefinition.__init__(self, name, prefix, itselftype)
        self.constructors = []
        
    def add(self, g, dupcheck=0):
        if g.name == self.name:
            g.setselftype(self.objecttype, self.itselftype)
            self.constructors.append(g)
            print "Adding constructor for", self.name
        else:
            GlobalObjectDefinition.add(self, g, dupcheck)
            
    def outputCheckNewArg(self):
        Output('if (itself == NULL)')
        OutLbrace()
        Output('Py_INCREF(Py_None);')
        Output('return Py_None;')
        OutRbrace()
        CxxMixin.outputCheckNewArg(self)
        # XXX Add refcount, if needed
        
    def outputCheckConvertArg(self):
        Output('if (v == Py_None)')
        OutLbrace()
        Output('*p_itself = NULL;')
        Output('return 1;')
        OutRbrace()
        CxxMixin.outputCheckConvertArg(self)
        
    def outputStructMembers(self):
        GlobalObjectDefinition.outputStructMembers(self)
        # XXX Output("bool owned;")
        
    def outputInitStructMembers(self):
        GlobalObjectDefinition.outputInitStructMembers(self)
        # XXX init owned, if needed
        
    def outputCleanupStructMembers(self):
        # XXX For refcounted objects decref
        # XXX For owned objects delete
        pass

    def outputCompare(self):
        Output()
        Output("static int %s_compare(%s *self, %s *other)", self.prefix, self.objecttype, self.objecttype)
        OutLbrace()
        Output("if ( self->ob_itself > other->ob_itself ) return 1;")
        Output("if ( self->ob_itself < other->ob_itself ) return -1;")
        Output("return 0;")
        OutRbrace()

    def outputHash(self):
        Output()
        Output("static int %s_hash(%s *self)", self.prefix, self.objecttype)
        OutLbrace()
        Output("return (int)self->ob_itself;")
        OutRbrace()

    def output_tp_newBody(self):
        Output("PyObject *_self;")
        Output()
        Output("if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;")
        Output("((%s *)_self)->ob_itself = NULL;", self.objecttype)
        ##Output("((%s *)self)->ob_freeit = CFRelease;", self.objecttype)
        Output("return _self;")

    def output_tp_initBody(self):
        Output("%s itself;", self.itselftype)
        Output("const char *kw[] = {\"itself\", 0};")
        Output()
        for con in self.constructors:
            con.outputConstructorBody()
        Output("if (PyArg_ParseTupleAndKeywords(_args, _kwds, \"O&\", kw, %s_Convert, &itself))",
                self.prefix)
        OutLbrace()
        Output("((%s *)_self)->ob_itself = itself;", self.objecttype)
        Output("return 0;")
        OutRbrace()
        Output("return -1;")
        
# Create the generator groups and link them
module = CxxModule(MODNAME, MODPREFIX, includestuff, finalstuff, initstuff, variablestuff)
functions = []

print "=== generating object definitions ==="

execfile("ambulantobjgen.py")

print "=== declaring more types ==="

# XXXX Temporarily disabled
methods_none_playable_factory = []

audio_format = TupleType("ambulant::net::audio_format",
        (std_string, "mime_type"),
        (std_string, "name"),
        (FakeType("(void *)0"), "parameters"),
        (int, "samplerate"),
        (int, "channels"),
        (int, "bits"))
audio_format_ref = TupleType("ambulant::net::audio_format&",
        (std_string, "mime_type"),
        (std_string, "name"),
        (FakeType("(void *)0"), "parameters"),
        (int, "samplerate"),
        (int, "channels"),
        (int, "bits"))
video_format = TupleType("ambulant::net::video_format",
        (std_string, "mime_type"),
        (std_string, "name"),
        (FakeType("(void*)0"), "parameters"),
        (timestamp_t, "frameduration"),
        (int, "width"),
        (int, "height"))
video_format_ref = TupleType("ambulant::net::video_format&",
        (std_string, "mime_type"),
        (std_string, "name"),
        (FakeType("(void*)0"), "parameters"),
        (timestamp_t, "frameduration"),
        (int, "width"),
        (int, "height"))
const_audio_format_ref = TupleType("const ambulant::net::audio_format&",
        (std_string, "mime_type"),
        (std_string, "name"),
        (FakeType("(void *)0"), "parameters"),
        (int, "samplerate"),
        (int, "channels"),
        (int, "bits"))
        
class audio_format_choicesObjectDefinition(MyGlobalObjectDefinition):
    baseclass = None
    argref = "*"
    argconst = "const "
    def output_tp_newBody(self):
        Output("PyObject *_self;")
        Output()
        Output("if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;")
        #Output("((%s *)_self)->ob_itself = NULL;", self.objecttype)
        Output("return _self;")

    def outputCheckConvertArg(self):
        CxxMixin.outputCheckConvertArg(self)

    def outputCompare(self):
        Output()
        Output("#define %s_compare NULL", self.prefix)

    def outputHash(self):
        Output()
        Output("#define %s_hash NULL", self.prefix)

audio_format_choices_object = audio_format_choicesObjectDefinition('audio_format_choices', 'audio_format_choicesObj', 'ambulant::net::audio_format_choices')
methods_audio_format_choices = []
module.addobject(audio_format_choices_object)

class OpaqueByFunnyRefType(OpaqueByRefType):
    def mkvalueArgs(self, name):
        return "%s(&%s)" % (self.new, name)

audio_format_choices = OpaqueByFunnyRefType('ambulant::net::audio_format_choices', 'audio_format_choicesObj')
audio_format_choices_ptr = OpaqueByValueType('ambulant::net::audio_format_choices*', 'audio_format_choicesObj')
const_audio_format_choices_ptr = OpaqueByValueType('const ambulant::net::audio_format_choices*', 'audio_format_choicesObj')

# Some type synonyms
node_interface_ptr = node_ptr
lib_node_ptr = node_ptr
const_node_interface_ptr = const_node_ptr
methods_node_interface = methods_node

lib_event_processor_ptr = event_processor_ptr
ambulant_lib_event_processor_ptr = event_processor_ptr
lib_transition_info_ptr = transition_info_ptr

common_duration = duration
common_surface_ptr = surface_ptr
common_playable_ptr = playable_ptr
common_region_info_ptr = region_info_ptr
common_gui_window_ptr = gui_window_ptr
common_gui_events_ptr = gui_events_ptr
common_bgrenderer_ptr = bgrenderer_ptr
lib_document_ptr = document_ptr
lib_event_ptr = event_ptr
ambulant_lib_event_ptr = event_ptr
lib_timer_ptr = timer_ptr
lib_point = point
lib_size = size
lib_color_t = color_t
lib_rect = rect
common_zindex_t = zindex_t
common_embedder_ptr = embedder_ptr
playable_notification_cookie_type = cookie_type
common_playable_notification_cookie_type = cookie_type
common_playable_notification_ptr = playable_notification_ptr
net_audio_datasource_ptr = audio_datasource_ptr
ambulant_net_url = net_url
url = net_url
const_ambulant_net_url_ref = net_url
posix_datasource_ptr = datasource_ptr
stdio_datasource_ptr = datasource_ptr
lib_global_parser_factory_ptr = global_parser_factory_ptr
lib_node_factory_ptr = node_factory_ptr
net_datasource_factory_ptr = datasource_factory_ptr
common_factories_ptr = factories_ptr

print "=== Testing availability of support for all needed C types ==="

# Do the type tests
execfile("ambulanttypetest.py")

print "=== Populating method and function lists ==="

class AllowThreadMixin:
    def beginallowthreads(self):
        Output("PyThreadState *_save = PyEval_SaveThread();")
        
    def endallowthreads(self):
        Output("PyEval_RestoreThread(_save);")
        
class Function(AllowThreadMixin, FunctionGenerator):
    pass
class Method(AllowThreadMixin, CxxMethodGenerator):
    pass
ConstMethod = Method
ConstructorMethod = CxxConstructorGenerator

# Create and populate the lists

execfile(INPUTFILE)

print "=== Adding methods to objects, resolving duplicates ==="

# add the populated lists to the generator groups
# (in a different wordl the scan program would generate this)
for f in functions: module.add(f)

for name, object in locals().items():
    if name[-7:] == '_object':
        methodlist_name = 'methods_' + name[:-7]
        methodlist = locals()[methodlist_name]
        for f in methodlist:
            object.add(f)

# Resolve duplicates
module.resolveduplicates()

# ADD add forloop here

print "=== Generating Python->C++ interface module ==="

# generate output (open the output file as late as possible)
SetOutputFileName(PY2CXXFILE)
module.generate()

# Now we start to get really gross. We want to reuse as much as possible
# when generating the C++ wrapper classes around Python objects. So, we
# redefine MyGlobalObjectDefinition, create a new module based on the
# wrapper module, reread the object definitions file
# (which will then create a whole new set of objects based on the new baseclass
# and reinitialize the methodlists to be empty)
# and call generate on this new module. Poof! We have the interface
# the other way...
from bgenBackSupport import *
class BackVarInputBufferType(VarInputBufferType):
    
    def getAuxDeclarations(self, name):
        return []
        
    def mkvalueArgs(self, name):
        return "%s__in__, (int)%s__len__" % (name, name)

InBuffer = BackVarInputBufferType('char', 'long', 'l')

includestuff = """
#define WITH_EXTERNAL_DOM 1
"""
finalstuff = ""
execfile("ambulantincludegen.py")

class MyGlobalObjectDefinition(BackObjectDefinition):
    pass
    
class NoFunctionGenerator(FunctionGenerator):
    def generate(self):
        pass
        
    def setClass(self, name):
        pass
        
    def generateDeclaration(self):
        pass
        
    def checkreturnvar(self):
        pass
        
    def checkgenerate(self):
        return False
        
    def generateAttributeExistenceTest(self):
        pass
        
Function = NoFunctionGenerator
Method = BackMethodGenerator
ConstMethod = BackMethodGenerator
ConstructorMethod = NoFunctionGenerator

module = BackModule("pyambulant", includestuff, finalstuff)
functions = []
print "=== generating object definitions for callbacks ==="

execfile("ambulantobjgen.py")

print "=== declaring more types for callbacks ==="
# Some type synonyms
node_interface_ptr = node_ptr
lib_node_ptr = node_ptr
const_node_interface_ptr = const_node_ptr
methods_node_interface = methods_node

lib_event_processor_ptr = event_processor_ptr

lib_event_ptr = event_ptr
lib_timer_ptr = timer_ptr
lib_point = point
lib_size = size
lib_color_t = color_t
lib_rect = rect
common_zindex_t = zindex_t
net_audio_datasource_ptr = audio_datasource_ptr

gui_window_object.baseconstructors.append("ambulant::common::gui_window(0)")

# We do not want callback support for audio_format__choices.

del audio_format_choices_object

print "=== Populating method and function lists for callbacks ==="
#import pdb ; pdb.set_trace()
execfile(INPUTFILE)

print "=== Adding methods to objects, callbacks ==="

for name, object in locals().items():
    if name[-7:] == '_object':
        methodlist_name = 'methods_' + name[:-7]
        methodlist = locals()[methodlist_name]
        for f in methodlist:
            object.add(f)

# Dummy versions of methods we cannot support:
node_context_object.othermethods = [
    "const custom_test_map* get_custom_tests() const { return NULL; }",
]
node_object.othermethods = [
    "void get_children(const_node_list& l) const {}", # XXX for now
    "void append_data(const char *data, size_t len) { abort(); }", # XXX for now
    "void set_attributes(const char **attrs) { abort(); }", # XXX for now
]
node_factory_object.othermethods = [
	"ambulant::lib::node *new_node(const char *local_name, const char **attrs = 0, const ambulant::lib::node_context *ctx = 0) { abort(); };",
	"ambulant::lib::node *new_node(const ambulant::lib::xml_string& local_name, const char **attrs = 0, const ambulant::lib::node_context *ctx = 0) { abort(); };",
]
parser_factory_object.othermethods = [
    "ambulant::lib::xml_parser* new_parser(ambulant::lib::sax_content_handler*, ambulant::lib::sax_error_handler*) { abort(); }", # XXX for now
]
xml_parser_object.othermethods = [
    "bool parse(const char*, long unsigned int, bool) { abort(); }", # XXX for now
    "bool parse(const char*, unsigned int, bool) { abort(); }", # XXX for now
    "void set_content_handler(ambulant::lib::sax_content_handler*) { abort(); }", #XXXX
    "void set_error_handler(ambulant::lib::sax_error_handler*) { abort(); }", #XXXX
]
embedder_object.othermethods = [
    "void show_file(const ambulant::net::url& url) { system_embedder::show_file(url); }"
]

renderer_object.othermethods = [
    "void redraw(const ambulant::lib::rect&, ambulant::common::gui_window*) { abort(); }", # XXX
    "void user_event(const ambulant::lib::point&, int) { abort(); }", # XXXX
    "void transition_freeze_end(ambulant::lib::rect) { abort(); }", # XXX
]
bgrenderer_object.othermethods = [
    "void redraw(const ambulant::lib::rect&, ambulant::common::gui_window*) { abort(); }", # XXX
    "void user_event(const ambulant::lib::point&, int) { abort(); }", # XXXX
    "void transition_freeze_end(ambulant::lib::rect) { abort(); }", # XXX
]
surface_object.othermethods = [
    "ambulant::lib::rect get_fit_rect(const ambulant::lib::size&, ambulant::lib::rect*, const ambulant::common::alignment*) const { abort(); }", # XXX
    "ambulant::common::tile_positions get_tiles(ambulant::lib::size s, ambulant::lib::rect r) const { return surface::get_tiles(s, r); }",
]
surface_template_object.othermethods = [
    "void animated() { abort(); }", # XXX
]
animation_destination_object.othermethods = [
    "std::string get_name() const { return region_info::get_name(); }",
    "ambulant::lib::rect get_rect() const { return region_info::get_rect(); }",
    "ambulant::common::fit_t get_fit() const { return region_info::get_fit(); }",
    "ambulant::lib::color_t get_bgcolor() const { return region_info::get_bgcolor(); }",
    "bool get_transparent() const { return region_info::get_transparent(); }",
    "ambulant::common::zindex_t get_zindex() const { return region_info::get_zindex(); }",
    "bool get_showbackground() const { return region_info::get_showbackground(); }",
    "bool is_subregion() const { return region_info::is_subregion(); }",
    "double get_soundlevel() const { return region_info::get_soundlevel(); }",
    "ambulant::common::sound_alignment get_soundalign() const { return region_info::get_soundalign(); }",
    "ambulant::common::tiling get_tiling() const { return region_info::get_tiling(); }",
    "const char* get_bgimage() const { return region_info::get_bgimage(); }",
#    "ambulant::common::region_dim get_region_dim(const std::string&, bool = false) const { abort(); }",
#    "void set_region_dim(const std::string&, const ambulant::common::region_dim&) { abort(); }",
]
global_playable_factory_object.othermethods = [
    "ambulant::common::playable* new_playable(ambulant::common::playable_notification*, int, const ambulant::lib::node*, ambulant::lib::event_processor*) { abort(); }", # XXX
    "ambulant::common::playable* new_aux_audio_playable(ambulant::common::playable_notification *context, int, const ambulant::lib::node *node, ambulant::lib::event_processor *evp, ambulant::net::audio_datasource *src) { abort(); }", # XXX
]
datasource_object.othermethods = [
    "long add_ref() { return 1; }",
    "long release() { return 1;}",
    "long get_ref_count() const { return 1; }",
    "char *get_read_ptr() { abort(); return NULL;}", # XXX
]

print "=== Generating C++->Python callback interfaces (.h file) ==="

# Generate the interface
SetOutputFileName(CXX2PYDECLFILE)
module.generateDeclaration()

print "=== Generating C++->Python callback implementation (.cpp file) ==="

# Generate the code
module.includestuff = """
#include "ambulantinterface.h"
#include "ambulantutilities.h"
#include "ambulantmodule.h"

extern PyObject *audio_format_choicesObj_New(const ambulant::net::audio_format_choices *itself);
extern int audio_format_choicesObj_Convert(PyObject *v, ambulant::net::audio_format_choices *p_itself);
"""
SetOutputFileName(CXX2PYFILE)
module.generate()
