import ambulant

class MyScriptComponentFactory(ambulant.script_component_factory):
    def __init__(self):
        pass
        
    def new_script_component(self, uri):
        print 'new_script_component, uri=', uri
        if uri == "http://www.ambulantplayer.org/components/pyscript":
            return MyScriptComponent()
        return None
        
class MyScriptComponent(ambulant.script_component):
    def __init__(self):
        print 'MyScriptComponent()'
        self.scope = {}
        # What dowe want to export to scope???
        
    def declare_state(self, state):
        print 'declare_state, node=', state
        statements = state.get_trimmed_data()
        exec statements in self.scope, self.scope
        #import pdb
        #pdb.set_trace()
        
    def bool_expression(self, expr):
        print 'bool_expression, expr=', expr
        rv = eval(expr, self.scope, self.scope)
        print 'bool_expression returning', rv
        rv = not not rv
        print 'bool_expression returning casted', rv
        return rv
        
    def set_value(self, var, expr):
        print 'set_value, var=', var, 'expr=', expr
        exec "%s = %s" % (var, expr) in self.scope, self.scope
        
    def send(self, submission):
        print 'send, submission=', submission
        
    def string_expression(self, expr):
        print 'string_expression, expr=', expr
        rv = eval(expr, self.scope, self.scope)
        print 'string_expression returning', rv
        rv = str(rv)
        print 'string_expression returning casted', rv
        return rv
        