import ambulant

class MyStateComponentFactory(ambulant.state_component_factory):
    def __init__(self, domdocument):
        self.domdocument = domdocument
        
    def new_state_component(self, uri):
        print 'new_state_component, uri=', uri
        if uri == "http://www.w3.org/TR/1999/REC-xpath-19991116":
            return MyStateComponent(self.domdocument)
        return None
        
class MyStateComponent(ambulant.state_component):
    def __init__(self, domdocument):
        print 'MyStateComponent()'
        self.globscope = {}
        self.domdocument = domdocument
        self.statenode = None
        # What dowe want to export to scope???
        
    def register_state_test_methods(self, stm):
        print 'register_state_test_methods, stm=', stm
        # Export things to the scripts
        for name in dir(stm):
            if name[:5] == 'smil_':
                self.globscope[name] = getattr(stm, name)
        
    def declare_state(self, state):
        print 'declare_state, node=', state
        src = state.get_attribute_1("src")
        if not src:
            print "webkitpluginstate: only state with src attribute allowed"
            return
        if src[0] != "#":
            print "webkitpluginstate: only #id allowed for src attribute on state"
            return
        print "state id is", src[1:]
        self.statenode = self.domdocument.getElementById_(src[1:])
        print "state node is", self.statenode
        print "dir", dir(self.statenode)
        
    def bool_expression(self, expr):
        print 'bool_expression, expr=', expr
        strexpr = self.string_expression(expr)
        if not strexpr:
            return False
        try:
            number = int(strexpr)
        except:
            pass
        else:
            return not not number
        return True
        
    def set_value(self, var, expr):
        print 'set_value', (var, expr)
        value = self.string_expression(expr)
        nodelist = self.statenode.getElementsByTagName_(var)
        node = nodelist.item_(0)
        if not node:
            print 'set_value: no such node:', var
            return
        valuenode = node.firstChild()
        if not valuenode:
            print 'set_value: not yet imp: node has no data yet:', node
            return
        valuenode.setNodeValue_(value)
        
    def new_value(self, ref, where, name, expr):
        print 'set_value, statement=', (ref, where, name, expr)
##        exec stmt in self.scope, self.globscope
        
    def del_value(self, ref):
        print 'del_value, ref=', ref
        
    def send(self, submission):
        print 'send, submission=', submission
        
    def string_expression(self, expr):
        print 'string_expression, expr=', expr
        rv = expr
##        rv = eval(expr, self.scope, self.globscope)
##        print 'string_expression returning', rv
##        rv = str(rv)
##        print 'string_expression returning casted', rv
        return rv
        