# This script generates a Python interface for an Apple Macintosh Manager.
# It uses the "bgen" package to generate C code.
# The function specifications are generated by scanning the mamager's header file,
# using the "scantools" package (customized for this particular manager).

import string
from bgen import *
from bgenCxxSupport import CxxMethodGenerator, CxxMixin
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
"""
execfile("ambulantincludegen.py")

includestuff = includestuff + """

#include "ambulantinterface.h"
#include "ambulantutilities.h"
#include "ambulantmodule.h"

"""

finalstuff = """
// Declare initambulant as a C external:

extern "C" void initambulant(); 
"""

initstuff = """
"""

variablestuff="""
"""

bool = OpaqueByValueType("bool", "bool")
size_t = Type("size_t", "l")
unsigned_int = Type("unsigned int", "l")
std_string = OpaqueByRefType("std::string", "cxx_std_string")

InBuffer = VarInputBufferType('char', 'size_t', 'l')

# Ambulant-specific
net_url = OpaqueByRefType("ambulant::net::url", "ambulant_url")
screen_rect_int = OpaqueByRefType("ambulant::lib::screen_rect_int", "ambulant_screen_rect")
const_lib_screen_rect_int_ref = screen_rect_int
rect = OpaqueByRefType("ambulant::lib::rect", "ambulant_rect")
point = OpaqueByRefType("ambulant::lib::point", "ambulant_point")
size = OpaqueByRefType("ambulant::lib::size", "ambulant_size")
zindex_t = Type("ambulant::common::zindex_t", "l")
cookie_type = Type("ambulant::common::playable::cookie_type", "l")
const_cookie_type_ref = cookie_type
color_t = Type("ambulant::lib::color_t", "l") # XXXX Split into RGB
event_priority = Type("ambulant::lib::event_processor::event_priority", "l")
timestamp_t = Type("ambulant::net::timestamp_t", "L")
time_type = Type("ambulant::lib::abstract_timer::time_type", "l")
tiling = Type("ambulant::common::tiling", "l")
fit_t = Type("ambulant::common::fit_t", "l")
sound_alignment = Type("ambulant::common::sound_alignment", "l")

# Our (opaque) objects

class MyGlobalObjectDefinition(CxxMixin, PEP253Mixin, GlobalObjectDefinition):
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
        Output("PyObject *self;")
        Output
        Output("if ((self = type->tp_alloc(type, 0)) == NULL) return NULL;")
        Output("((%s *)self)->ob_itself = NULL;", self.objecttype)
        ##Output("((%s *)self)->ob_freeit = CFRelease;", self.objecttype)
        Output("return self;")

    def output_tp_initBody(self):
        Output("%s itself;", self.itselftype)
        Output("char *kw[] = {\"itself\", 0};")
        Output()
        Output("if (PyArg_ParseTupleAndKeywords(args, kwds, \"O&\", kw, %s_Convert, &itself))",
                self.prefix)
        OutLbrace()
        Output("((%s *)self)->ob_itself = itself;", self.objecttype)
        Output("return 0;")
        OutRbrace()
        Output("return -1;")

# Create the generator groups and link them
module = Module(MODNAME, MODPREFIX, includestuff, finalstuff, initstuff, variablestuff)
functions = []

execfile("ambulantobjgen.py")

# Some type synonyms
node_interface_ptr = node_ptr
lib_node_ptr = node_ptr
const_node_interface_ptr = const_node_ptr
methods_node_interface = methods_node

lib_event_processor_ptr = event_processor_ptr
abstract_event_processor_ptr = event_processor_ptr
methods_abstract_event_processor = methods_event_processor

lib_event_ptr = event_ptr
lib_timer_ptr = abstract_timer_ptr
lib_abstract_timer_ptr = abstract_timer_ptr
lib_screen_rect_int = screen_rect_int
lib_point = point
lib_size = size
lib_color_t = color_t
lib_rect = rect
common_zindex_t = zindex_t

# Do the type tests
execfile("ambulanttypetest.py")

# Create the generator classes used to populate the lists
Function = FunctionGenerator
Method = CxxMethodGenerator
ConstMethod = Method

# Create and populate the lists

execfile(INPUTFILE)


# add the populated lists to the generator groups
# (in a different wordl the scan program would generate this)
for f in functions: module.add(f)

for name, object in locals().items():
    if name[-7:] == '_object':
        methodlist_name = 'methods_' + name[:-7]
        methodlist = locals()[methodlist_name]
        for f in methodlist:
            object.add(f)

# ADD add forloop here

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
        
Function = NoFunctionGenerator
Method = BackMethodGenerator
ConstMethod = ConstBackMethodGenerator

const_cookie_type_ref = Type("ambulant::common::playable::cookie_type&", "l")
const_lib_screen_rect_int_ref = OpaqueByRefType("ambulant::lib::screen_rect_int&", "ambulant_screen_rect")

module = BackModule("pyambulant", includestuff, finalstuff)
functions = []
execfile("ambulantobjgen.py")

gui_window_object.baseconstructors = "ambulant::common::gui_window(0)"

#import pdb ; pdb.set_trace()
execfile(INPUTFILE)

for name, object in locals().items():
    if name[-7:] == '_object':
        methodlist_name = 'methods_' + name[:-7]
        methodlist = locals()[methodlist_name]
        for f in methodlist:
            object.add(f)

# Generate the interface
SetOutputFileName(CXX2PYDECLFILE)
module.generateDeclaration()
# Generate the code
module.includestuff = """
#include "ambulantinterface.h"
#include "ambulantutilities.h"
#include "ambulantmodule.h"

"""
SetOutputFileName(CXX2PYFILE)
module.generate()
