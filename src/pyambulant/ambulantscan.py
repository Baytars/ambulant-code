# Scan an Apple header file, generating a Python file of generator calls.

import sys
import os
import re
from bgenlocations import TOOLBOXDIR, BGENDIR
sys.path.append(BGENDIR)
from scantools import Scanner

AMBULANT="/usr/local/include/ambulant/"
DO_SCAN=True

def main():
    input = [
        AMBULANT+ "lib/node.h",
#        AMBULANT+ "lib/document.h",
#        AMBULANT+ "lib/event.h",
#        AMBULANT+ "lib/event_processor.h",
#        AMBULANT+ "lib/parser_factory.h",
#        AMBULANT+ "lib/sax_handler.h",
#        AMBULANT+ "lib/system.h",
#        AMBULANT+ "lib/timer.h",
#        AMBULANT+ "common/layout.h",
#        AMBULANT+ "common/playable.h",
#        AMBULANT+ "common/player.h",
#        AMBULANT+ "common/region_info.h",
#        AMBULANT+ "net/datasource.h",
            ]
    if DO_SCAN:
        output = "ambulantgen.py"
        defsoutput = None
        scanner = MyScanner(input, output, defsoutput)
        scanner.scan()
        scanner.gentypetest("ambulanttypetest.py")
        scanner.close()
        print "=== Testing definitions output code ==="
        if defsoutput: execfile(defsoutput, {}, {})
    print "=== Done scanning and generating, now importing the generated code ==="
    exec "import ambulantsupport"
    print "=== Done.  It's up to you to compile it now! ==="

class CxxScanner(Scanner):
    def __init__(self, input=None, output=None, defsoutput=None):
        Scanner.__init__(self, input, output, defsoutput)
        self.initnamespaces()
        
    def initnamespaces(self):
        self.namespaces = []
        self.in_class_defn = 0

    def pythonizename(self, name):        
        if '<' in name or '>' in name:
            self.error("Use of templates in typename or functionname not supported: %s", name)
        name = re.sub("\*", " ptr", name)
        name = re.sub("&", " ref", name)
        name = re.sub("::", " ", name)
        name = name.strip()
        name = re.sub("[ \t]+", "_", name)
        return name
        
    def modifyarg(self, type, name, mode):
        if type[:6] == "const_":
            type = type[6:]
            # Note that const ref and const ptr parameters stay InMode
            if type[-4:] == '_ref':
                type = type[:-4]
                mode = mode + "+RefMode"
        elif type[-4:] == '_ref':
            type = type[:-4]
            mode = "OutMode+RefMode"
        elif type in self.inherentpointertypes:
            mode = "OutMode"
        if type[-4:] == "_far":
            type = type[:-4]
        return type, name, mode

    def initpatterns(self):
        self.head_pat = r"^\s*(virtual|extern)\s+"
        self.tail_pat = r"[;={}]"
        self.type_pat = r"(virtual|extern)" + \
                        r"\s+" + \
                        r"(?P<type>[a-zA-Z0-9_*:& \t]*[a-zA-Z0-9_*&])" + \
                        r"\s+"
        self.name_pat = r"(?P<name>[a-zA-Z0-9_]+)\s*"
        self.args_pat = r"\((?P<args>([^\(;=\)]+|\([^\(;=\)]*\))*)\)"
        self.whole_pat = self.type_pat + self.name_pat + self.args_pat
        self.sym_pat = r"^[ \t]*(?P<name>[a-zA-Z0-9_]+)[ \t]*=" + \
                       r"[ \t]*(?P<defn>[-0-9_a-zA-Z'\"\(][^\t\n,;}]*),?"
        self.asplit_pat = r"^(?P<type>.*[^a-zA-Z0-9_])(?P<name>[a-zA-Z0-9_]+)(?P<array>\[\])?$"
        self.comment1_pat = r"(?P<rest>.*)//.*"
        # note that the next pattern only removes comments that are wholly within one line
        self.comment2_pat = r"(?P<rest1>.*)/\*.*\*/(?P<rest2>.*)"
        self.namespace_pat = r"^\s*namespace\s+(?P<name>[a-zA-Z0-9_:]+)\s+{"
        self.klass_pat = r"^\s*class\s+(?P<name>[a-zA-Z0-9_:]+)\s+[{:]"

    def donamespace(self, match):
        if self.in_class_defn:
            self.report("Cannot do namespace inside class")
        name = match.group("name")
        self.namespaces.append(name)
        if self.debug:
            self.report("      %d: namespace %s" % (len(self.namespaces), name))
        
    def doclass(self, match):
        name = match.group("name")
        self.namespaces.append(name)
        self.in_class_defn += 1
        if self.debug:
            self.report("      %d: namespace %s" % (len(self.namespaces), name))
            
    def dobeginscope(self, count):
        for i in range(count):
            self.namespaces.append("<scope>")
            if self.in_class_defn:
                self.in_class_defn += 1
        
    def doendscope(self, count):
        for i in range(count):
            if self.in_class_defn:
                self.in_class_defn -= 1
            if self.debug:
                self.report("      %d: leaving %s" % (len(self.namespaces), self.namespaces[-1]))
            del self.namespaces[-1]
            count -= 1

    def scan(self):
        if not self.scanfile:
            self.error("No input file has been specified")
            return
        inputname = self.scanfile.name
        self.report("scanfile = %r", inputname)
        if not self.specfile:
            self.report("(No interface specifications will be written)")
        else:
            self.report("specfile = %r", self.specfile.name)
            self.specfile.write("# Generated from %r\n\n" % (inputname,))
        if not self.defsfile:
            self.report("(No symbol definitions will be written)")
        else:
            self.report("defsfile = %r", (self.defsfile.name,))
            self.defsfile.write("# Generated from %r\n\n" % (os.path.split(inputname)[1],))
            self.writeinitialdefs()
        self.alreadydone = []
        try:
            while 1:
                try: line = self.getline()
                except EOFError: break
                if self.debug:
                    self.report("LINE: %r" % (line,))
                match = self.comment1.match(line)
                if match:
                    line = match.group('rest')
                    if self.debug:
                        self.report("\tafter comment1: %r" % (line,))
                match = self.comment2.match(line)
                while match:
                    line = match.group('rest1')+match.group('rest2')
                    if self.debug:
                        self.report("\tafter comment2: %r" % (line,))
                    match = self.comment2.match(line)
                if self.defsfile:
                    match = self.sym.match(line)
                    if match:
                        if self.debug:
                            self.report("\tmatches sym.")
                        self.dosymdef(match)
                        continue
                match = self.head.match(line)
                if match:
                    if self.debug:
                        self.report("\tmatches head.")
                    line = self.dofuncspec()
                    # XXX Need to check for { and }
                    beginscopecount = line.count('{')
                    endscopecount = line.count('}')
                    if beginscopecount > endscopecount:
                        self.dobeginscope(beginscopecount-endscopecount)
                    elif beginscopecount < endscopecount:
                        self.doendscope(endscopecount-beginscopecount)
                    continue
                match = self.namespace.match(line)
                if match:
                    if self.debug:
                        self.report("\tmatches namespace.")
                    self.donamespace(match)
                    continue
                match = self.klass.match(line)
                if match:
                    if self.debug:
                        self.report("\tmatches class.")
                    self.doclass(match)
                    continue
                beginscopecount = line.count('{')
                endscopecount = line.count('}')
                if beginscopecount > endscopecount:
                    self.dobeginscope(beginscopecount-endscopecount)
                elif beginscopecount < endscopecount:
                    self.doendscope(endscopecount-beginscopecount)
                continue
        except EOFError:
            self.error("Uncaught EOF error")
        self.reportusedtypes()

    def destination(self, type, name, arglist):
        if self.in_class_defn:
            classname = self.namespaces[-1]
            classname = self.pythonizename(classname)
            return "CxxMethodGenerator", "methods_%s" % classname
        return "FunctionGenerator", "functions"
       
class MyScanner(CxxScanner):
    def makeblacklistnames(self):
        return [
            # node.h
            "find_nodes_with_name",
            "has_graph_data",
            "create_idmap",
            "to_string",
            "to_trimmed_string",
            "set_attributes",       # string list, too difficult
            
        ]

    def makeblacklisttypes(self):
        return [
            "q_attributes_list",
            "q_attributes_list_ref",
            "const_q_name_pair",
            "const_q_name_pair_ref",
            "node_list", # XXX For now
            "xml_string", # XXX For now
            "const_xml_string_ref", # XXX For now
            "const_custom_test_map_ptr", # XXX For now 
            "const_q_attributes_list_ref",  # XXX For now
        ]

    def makegreylist(self):
        return []

    def makerepairinstructions(self):
        return [
            ('set_attribute',
              [
                ('char_ptr', '*', '*'),
                ('char_ptr', '*', '*')
              ],[
                ('stringptr', '*', '*'),
                ('stringptr', '*', '*')
              ]
            ),
            ('locate_node',
              [
                ('char_ptr', '*', '*')
              ],[
                ('stringptr', '*', '*')
              ]
            ),
            ('get_url',
              [
                ('char_ptr', '*', '*')
              ],[
                ('stringptr', '*', '*')
              ]
            ),
            (
              [
                ('char_ptr', 'data', 'InMode'),
                ('size_t', '*', 'InMode')
              ],[
                ('InBuffer', '*', 'InMode'),
               ]
            ),

        ]        
if __name__ == "__main__":
    main()
