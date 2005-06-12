# This script generates a Python interface for an Apple Macintosh Manager.
# It uses the "bgen" package to generate C code.
# The function specifications are generated by scanning the mamager's header file,
# using the "scantools" package (customized for this particular manager).

#error missing SetActionFilter

import string
from bgen import *

# Declarations that change for each manager
MODNAME = 'ambulant'                         # The name of the module

# The following is *usually* unchanged but may still require tuning
MODPREFIX = 'PyAm'                        # The prefix for module-wide routines
INPUTFILE = 'ambulantgen.py' # The file generated by the scanner
OUTPUTFILE = MODNAME + "module.cpp"       # The file generated by this program


# Create the type objects

includestuff = """
#include "ambulant/config/config.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/event.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/parser_factory.h"
#include "ambulant/lib/sax_handler.h"
#include "ambulant/lib/system.h"
#include "ambulant/lib/timer.h"

#include "ambulant/common/player.h"

/*
** Parse/generate CFRange records
*/
PyObject *bool_New(bool itself)
{
    if (itself) {
        Py_RETURN_TRUE;
    } 
    Py_RETURN_FALSE;
}

int
bool_Convert(PyObject *v, bool *p_itself)
{
    int istrue = PyObject_IsTrue(v);
    if (istrue < 0) return 0;
    *p_itself = (istrue > 0);
    return 1;
}

"""

finalstuff = """
#if 0
/* Routines to convert any CF type to/from the corresponding CFxxxObj */
PyObject *CFObj_New(CFTypeRef itself)
{
        if (itself == NULL)
        {
                PyErr_SetString(PyExc_RuntimeError, "cannot wrap NULL");
                return NULL;
        }
        if (CFGetTypeID(itself) == CFArrayGetTypeID()) return CFArrayRefObj_New((CFArrayRef)itself);
        if (CFGetTypeID(itself) == CFDictionaryGetTypeID()) return CFDictionaryRefObj_New((CFDictionaryRef)itself);
        if (CFGetTypeID(itself) == CFDataGetTypeID()) return CFDataRefObj_New((CFDataRef)itself);
        if (CFGetTypeID(itself) == CFStringGetTypeID()) return CFStringRefObj_New((CFStringRef)itself);
        if (CFGetTypeID(itself) == CFURLGetTypeID()) return CFURLRefObj_New((CFURLRef)itself);
        /* XXXX Or should we use PyCF_CF2Python here?? */
        return CFTypeRefObj_New(itself);
}
int CFObj_Convert(PyObject *v, CFTypeRef *p_itself)
{

        if (v == Py_None) { *p_itself = NULL; return 1; }
        /* Check for other CF objects here */

        if (!CFTypeRefObj_Check(v) &&
                !CFArrayRefObj_Check(v) &&
                !CFMutableArrayRefObj_Check(v) &&
                !CFDictionaryRefObj_Check(v) &&
                !CFMutableDictionaryRefObj_Check(v) &&
                !CFDataRefObj_Check(v) &&
                !CFMutableDataRefObj_Check(v) &&
                !CFStringRefObj_Check(v) &&
                !CFMutableStringRefObj_Check(v) &&
                !CFURLRefObj_Check(v) )
        {
                /* XXXX Or should we use PyCF_Python2CF here?? */
                PyErr_SetString(PyExc_TypeError, "CF object required");
                return 0;
        }
        *p_itself = ((CFTypeRefObject *)v)->ob_itself;
        return 1;
}
#endif
"""

initstuff = """
"""

variablestuff="""
"""

bool = OpaqueByValueType("bool", "bool")
##Boolean = Type("Boolean", "l")
##CFTypeID = Type("CFTypeID", "l") # XXXX a guess, seems better than OSTypeType.
##CFHashCode = Type("CFHashCode", "l")
##CFIndex = Type("CFIndex", "l")
##CFRange = OpaqueByValueType('CFRange', 'CFRange')
##CFOptionFlags = Type("CFOptionFlags", "l")
##CFStringEncoding = Type("CFStringEncoding", "l")
##CFComparisonResult = Type("CFComparisonResult", "l")  # a bit dangerous, it's an enum
##CFURLPathStyle = Type("CFURLPathStyle", "l") #  a bit dangerous, it's an enum
##
##char_ptr = stringptr
##return_stringptr = Type("char *", "s")  # ONLY FOR RETURN VALUES!!
##
##CFAllocatorRef = FakeType("(CFAllocatorRef)NULL")
##CFArrayCallBacks_ptr = FakeType("&kCFTypeArrayCallBacks")
##CFDictionaryKeyCallBacks_ptr = FakeType("&kCFTypeDictionaryKeyCallBacks")
##CFDictionaryValueCallBacks_ptr = FakeType("&kCFTypeDictionaryValueCallBacks")
### The real objects
##CFTypeRef = OpaqueByValueType("CFTypeRef", "CFTypeRefObj")
##CFArrayRef = OpaqueByValueType("CFArrayRef", "CFArrayRefObj")
##CFMutableArrayRef = OpaqueByValueType("CFMutableArrayRef", "CFMutableArrayRefObj")
##CFArrayRef = OpaqueByValueType("CFArrayRef", "CFArrayRefObj")
##CFMutableArrayRef = OpaqueByValueType("CFMutableArrayRef", "CFMutableArrayRefObj")
##CFDataRef = OpaqueByValueType("CFDataRef", "CFDataRefObj")
##CFMutableDataRef = OpaqueByValueType("CFMutableDataRef", "CFMutableDataRefObj")
##CFDictionaryRef = OpaqueByValueType("CFDictionaryRef", "CFDictionaryRefObj")
##CFMutableDictionaryRef = OpaqueByValueType("CFMutableDictionaryRef", "CFMutableDictionaryRefObj")
##CFStringRef = OpaqueByValueType("CFStringRef", "CFStringRefObj")
##CFMutableStringRef = OpaqueByValueType("CFMutableStringRef", "CFMutableStringRefObj")
##CFURLRef = OpaqueByValueType("CFURLRef", "CFURLRefObj")
##OptionalCFURLRef  = OpaqueByValueType("CFURLRef", "OptionalCFURLRefObj")
##CFPropertyListRef = OpaqueByValueType("CFPropertyListRef", "CFTypeRefObj")
# ADD object type here

# Our (opaque) objects

class CxxMethodGenerator(FunctionGenerator):
    def __init__(self, returntype, name, *argumentlist, **conditionlist):
        FunctionGenerator.__init__(self, returntype, name, *argumentlist, **conditionlist)
        self.callname = "_self->ob_itself->" + self.name

class CxxMixin:
    def outputCheck(self):
        Output("extern PyTypeObject %s;", self.typename)
        Output()
        Output("inline bool %s_Check(PyObject *x)", self.prefix)
        OutLbrace()
        Output("return ((x)->ob_type == &%s || PyObject_TypeCheck((x), &%s));",
               self.typename, self.typename)
        OutRbrace()
        Output()

class MyGlobalObjectDefinition(CxxMixin, PEP253Mixin, GlobalObjectDefinition):
    def outputCheckNewArg(self):
        Output('if (itself == NULL)')
        OutLbrace()
        Output('Py_INCREF(Py_None);')
        Output('return Py_None;')
        OutRbrace()
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

# From lib/node.h:

class NodeObjectDefinition(MyGlobalObjectDefinition):
    pass
Node_object = NodeObjectDefinition('Node', 'NodeObj', 'ambulant::lib::node_interface*')
Node_methods = []
module.addobject(Node_object)

class NodeContextObjectDefinition(MyGlobalObjectDefinition):
    pass
NodeContext_object = NodeContextObjectDefinition('NodeContext', 'NodeContextObj', 'ambulant::lib::node_context*')
NodeContext_methods = []
module.addobject(NodeContext_object)

# From lib/document.h:
class DocumentObjectDefinition(MyGlobalObjectDefinition):
    basetype = "NodeContext_Type"
    pass
Document_object = DocumentObjectDefinition('Document', 'DocumentObj', 'ambulant::lib::document*')
Document_methods = []
module.addobject(Document_object)

# From lib/event.h:
class EventObjectDefinition(MyGlobalObjectDefinition):
    pass
Event_object = EventObjectDefinition('Event', 'EventObj', 'ambulant::lib::event*')
Event_methods = []
module.addobject(Event_object)

# From lib/event_processor.h:
class EventProcessorObjectDefinition(MyGlobalObjectDefinition):
    pass
EventProcessor_object = EventProcessorObjectDefinition('EventProcessor', 'EventProcessorObj', 'ambulant::lib::event_processor*')
EventProcessor_methods = []
module.addobject(EventProcessor_object)

### From lib/mtsync.h:
##class CriticalSectionObjectDefinition(MyGlobalObjectDefinition):
##    pass
##CriticalSection_object = CriticalSectionObjectDefinition('CriticalSection', 'CriticalSectionObj', 'ambulant::lib::critical_section*')
##CriticalSection_methods = []
##module.addobject(CriticalSection_object)
##
##class ConditionObjectDefinition(MyGlobalObjectDefinition):
##    pass
##Condition_object = ConditionObjectDefinition('Condition', 'ConditionObj', 'ambulant::lib::condition*')
##Condition_methods = []
##module.addobject(Condition_object)

# From lib/parser_factory.h:
class ParserFactoryObjectDefinition(MyGlobalObjectDefinition):
    pass
ParserFactory_object = ParserFactoryObjectDefinition('ParserFactory', 'ParserFactoryObj', 'ambulant::lib::parser_factory*')
ParserFactory_methods = []
module.addobject(ParserFactory_object)

# From lib/sax_handler.h:
class XmlParserObjectDefinition(MyGlobalObjectDefinition):
    pass
XmlParser_object = XmlParserObjectDefinition('XmlParser', 'XmlParserObj', 'ambulant::lib::xml_parser*')
XmlParser_methods = []
module.addobject(XmlParser_object)

# From lib/system.h:
class SystemObjectDefinition(MyGlobalObjectDefinition):
    pass
System_object = SystemObjectDefinition('System', 'SystemObj', 'ambulant::lib::system*')
System_methods = []
module.addobject(System_object)

# From lib/timer.h:
class TimerEventsObjectDefinition(MyGlobalObjectDefinition):
    pass
TimerEvents_object = TimerEventsObjectDefinition('TimerEvents', 'TimerEventsObj', 'ambulant::lib::timer_events*')
TimerEvents_methods = []
module.addobject(TimerEvents_object)

class AbstractTimerObjectDefinition(MyGlobalObjectDefinition):
    pass
AbstractTimer_object = AbstractTimerObjectDefinition('AbstractTimer', 'AbstractTimerObj', 'ambulant::lib::abstract_timer*')
AbstractTimer_methods = []
module.addobject(AbstractTimer_object)


# From common/player.h:
class PlayerObjectDefinition(MyGlobalObjectDefinition):
    pass
Player_object = PlayerObjectDefinition('Player', 'PlayerObj', 'ambulant::common::player*')
Player_methods = []
module.addobject(Player_object)


# Create the generator classes used to populate the lists
Function = FunctionGenerator
Method = CxxMethodGenerator

# Create and populate the lists

execfile(INPUTFILE)


# add the populated lists to the generator groups
# (in a different wordl the scan program would generate this)
for f in functions: module.add(f)
for f in Player_methods: Player_object.add(f)
##for f in CFTypeRef_methods: CFTypeRef_object.add(f)
##for f in CFArrayRef_methods: CFArrayRef_object.add(f)
##for f in CFMutableArrayRef_methods: CFMutableArrayRef_object.add(f)
##for f in CFDictionaryRef_methods: CFDictionaryRef_object.add(f)
##for f in CFMutableDictionaryRef_methods: CFMutableDictionaryRef_object.add(f)
##for f in CFDataRef_methods: CFDataRef_object.add(f)
##for f in CFMutableDataRef_methods: CFMutableDataRef_object.add(f)
##for f in CFStringRef_methods: CFStringRef_object.add(f)
##for f in CFMutableStringRef_methods: CFMutableStringRef_object.add(f)
##for f in CFURLRef_methods: CFURLRef_object.add(f)

### Manual generators for getting data out of strings
##
##getasstring_body = """
##int size = CFStringGetLength(_self->ob_itself)+1;
##char *data = malloc(size);
##
##if( data == NULL ) return PyErr_NoMemory();
##if ( CFStringGetCString(_self->ob_itself, data, size, 0) ) {
##        _res = (PyObject *)PyString_FromString(data);
##} else {
##        PyErr_SetString(PyExc_RuntimeError, "CFStringGetCString could not fit the string");
##        _res = NULL;
##}
##free(data);
##return _res;
##"""
##
##f = ManualGenerator("CFStringGetString", getasstring_body);
##f.docstring = lambda: "() -> (string _rv)"
##CFStringRef_object.add(f)
##
##getasunicode_body = """
##int size = CFStringGetLength(_self->ob_itself)+1;
##Py_UNICODE *data = malloc(size*sizeof(Py_UNICODE));
##CFRange range;
##
##range.location = 0;
##range.length = size;
##if( data == NULL ) return PyErr_NoMemory();
##CFStringGetCharacters(_self->ob_itself, range, data);
##_res = (PyObject *)PyUnicode_FromUnicode(data, size-1);
##free(data);
##return _res;
##"""
##
##f = ManualGenerator("CFStringGetUnicode", getasunicode_body);
##f.docstring = lambda: "() -> (unicode _rv)"
##CFStringRef_object.add(f)
##
### Get data from CFDataRef
##getasdata_body = """
##int size = CFDataGetLength(_self->ob_itself);
##char *data = (char *)CFDataGetBytePtr(_self->ob_itself);
##
##_res = (PyObject *)PyString_FromStringAndSize(data, size);
##return _res;
##"""
##
##f = ManualGenerator("CFDataGetData", getasdata_body);
##f.docstring = lambda: "() -> (string _rv)"
##CFDataRef_object.add(f)
##
### Manual generator for CFPropertyListCreateFromXMLData because of funny error return
##fromxml_body = """
##CFTypeRef _rv;
##CFOptionFlags mutabilityOption;
##CFStringRef errorString;
##if (!PyArg_ParseTuple(_args, "l",
##                      &mutabilityOption))
##        return NULL;
##_rv = CFPropertyListCreateFromXMLData((CFAllocatorRef)NULL,
##                                      _self->ob_itself,
##                                      mutabilityOption,
##                                      &errorString);
##if (errorString)
##        CFRelease(errorString);
##if (_rv == NULL) {
##        PyErr_SetString(PyExc_RuntimeError, "Parse error in XML data");
##        return NULL;
##}
##_res = Py_BuildValue("O&",
##                     CFTypeRefObj_New, _rv);
##return _res;
##"""
##f = ManualGenerator("CFPropertyListCreateFromXMLData", fromxml_body)
##f.docstring = lambda: "(CFOptionFlags mutabilityOption) -> (CFTypeRefObj)"
##CFTypeRef_object.add(f)
##
### Convert CF objects to Python objects
##toPython_body = """
##_res = PyCF_CF2Python(_self->ob_itself);
##return _res;
##"""
##
##f = ManualGenerator("toPython", toPython_body);
##f.docstring = lambda: "() -> (python_object)"
##CFTypeRef_object.add(f)
##
##toCF_body = """
##CFTypeRef rv;
##CFTypeID typeid;
##
##if (!PyArg_ParseTuple(_args, "O&", PyCF_Python2CF, &rv))
##        return NULL;
##typeid = CFGetTypeID(rv);
##
##if (typeid == CFStringGetTypeID())
##        return Py_BuildValue("O&", CFStringRefObj_New, rv);
##if (typeid == CFArrayGetTypeID())
##        return Py_BuildValue("O&", CFArrayRefObj_New, rv);
##if (typeid == CFDictionaryGetTypeID())
##        return Py_BuildValue("O&", CFDictionaryRefObj_New, rv);
##if (typeid == CFURLGetTypeID())
##        return Py_BuildValue("O&", CFURLRefObj_New, rv);
##
##_res = Py_BuildValue("O&", CFTypeRefObj_New, rv);
##return _res;
##"""
##f = ManualGenerator("toCF", toCF_body);
##f.docstring = lambda: "(python_object) -> (CF_object)"
##module.add(f)

# ADD add forloop here

# generate output (open the output file as late as possible)
SetOutputFileName(OUTPUTFILE)
module.generate()
