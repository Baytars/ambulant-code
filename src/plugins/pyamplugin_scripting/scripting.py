import ambulant

class MyScriptComponentFactory(ambulant.script_component_factory):
    def __init__(self):
        pass
        
    def new_script_component(self, uri):
        print 'new_script_component, uri=', uri
        if url == "http://www.ambulantplayer.org/components/pyscript":
            return MyScriptComponent()
        return None
        
class MyScriptComponent(ambulant.script_component):
    def declare_state(self, state):
        print 'declare_state, node=', node
        
    def bool_expression(self, expr):
        print 'bool_expression, expr=', expr
        return False
        
    def set_value(self, var, expr):
        print 'set_value, var=', var, 'expr=', expr
        
    def send(self, submission):
        print 'send, submission=', submission
        
    def string_expression(self, expr):
        print 'string_expression, expr=', expr
        return ''
        