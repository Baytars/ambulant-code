import ambulant
import Foundation
import traceback

DEBUG=True

NS_XFORMS="http://www.w3.org/2002/xforms"
NS_XPATH="http://www.w3.org/TR/1999/REC-xpath-19991116"

class MyStateComponentFactory(ambulant.state_component_factory):
    def __init__(self, domdocument):
        self.domdocument = domdocument
        
    def new_state_component(self, uri):
        print 'new_state_component, uri=', uri
        if uri == NS_XPATH:
            return MyStateComponent(self.domdocument)
        return None

class MainThreadCaller(Foundation.NSObject):
    
    def initWithArgs_(self, args):
        self = self.init()
        self.args = args
        self.rv = None
        return self

    def callWait_(self, sender):
        self.performSelectorOnMainThread_withObject_waitUntilDone_(
            self.call_, self.args, True)
        return self.rv
    
    def call_(self, (func, args, kwargs)):
        try:
            self.rv = func(*args, **kwargs)
        except:
            print 'webkitpluginstate: HELP! Exception!'
            traceback.print_exc()
 
def callWait(func, *args, **kwargs):
    """call a function on the main thread (sync)"""
    pool = Foundation.NSAutoreleasePool.alloc().init()
    obj = MainThreadCaller.alloc().initWithArgs_((func, args, kwargs))
    return obj.callWait_(None)

class MyStateComponent(ambulant.state_component):
    def __init__(self, domdocument):
        if DEBUG: print 'MyStateComponent()'
        self.globscope = {}
        self.domdocument = domdocument
        if DEBUG: print 'DOMDocument is', self.domdocument
        if DEBUG: print 'DOMDocument has', dir(self.domdocument)
        self.statenode = None
        self.nsresolver = None
        # What do we want to export to scope???
        
    def register_state_test_methods(self, stm):
        if DEBUG: print 'register_state_test_methods, stm=', stm
        # Export things to the scripts
        for name in dir(stm):
            if name[:5] == 'smil_':
                self.globscope[name] = getattr(stm, name)
        
    def declare_state(self, state):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        if DEBUG: print 'declare_state, node=', state
        if 0:
            # "correct" way, which does not work:
            src = state.get_attribute_1("src")
            if not src:
                print "webkitpluginstate: only state with src attribute allowed"
                return
            if src[0] != "#":
                print "webkitpluginstate: only #id allowed for src attribute on state"
                return
            if DEBUG: print "state id is", src[1:]
            statecontainer = self.domdocument.getElementById_(src[1:])
        else:
            # Hack, which does work, maybe
            elements = self.domdocument.getElementsByTagNameNS__(NS_XFORMS, "instance")
            statecontainer = elements.item_(0)
            if not statecontainer:
                import pdb ; pdb.set_trace()
        if DEBUG: print "state container node is", statecontainer
        ch = statecontainer.firstChild()
        while ch and ch.nodeType() != 1:
            if DEBUG: print 'webkitpluginstate: Skip', ch
            ch = ch.nextSibling()
        if not ch:
            print "webkitpluginstate: state container",src,"is empty"
            return
        self.statenode = ch
        ch = ch.nextSibling()
        while ch:
            print 'webkitpluginstate: examine', ch
            if ch.nodeType() == 1:
                print "webkitpluginstate: state container", src, "has more than one child"
                self.statenode = None
                return
            ch = ch.nextSibling()
        if DEBUG: print "state node is", self.statenode
        
    def bool_expression(self, expr):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        if DEBUG: print 'bool_expression, expr=', expr
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
        pool = Foundation.NSAutoreleasePool.alloc().init()
        return callWait(self._set_value, var, expr)
    
    def _set_value(self, var, expr):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        if DEBUG: print 'set_value', (var, expr)
        value = self.string_expression(expr)
        nodelist = self.statenode.getElementsByTagName_(var)
        node = nodelist.item_(0)
        if not node:
            print 'webkitpluginstate: set_value: no such node:', var
            return
        valuenode = node.firstChild()
        if not valuenode:
            print 'webkitpluginstate: set_value: not yet imp: node has no data yet:', node
            return
        valuenode.setNodeValue_(value)
        
    def new_value(self, ref, where, name, expr):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        if DEBUG: print 'NOTIMPLEMENTED: new_value, statement=', (ref, where, name, expr)
##        exec stmt in self.scope, self.globscope
        
    def del_value(self, ref):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        print 'NOT IMPLEMENTED: del_value, ref=', ref
        
    def send(self, submission):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        print 'NOT IMPLEMENTED: send, submission=', submission
        
    def string_expression(self, expr):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        return callWait(self._string_expression, expr)

    def _string_expression(self, expr):
        #pool = Foundation.NSAutoreleasePool.alloc().init()
        if DEBUG: print 'string_expression, expr=', expr
        if not self.nsresolver:
            self.nsresolver = self.domdocument.createNSResolver_(self.statenode)
        rv = self.domdocument.evaluate_____(expr, self.statenode, self.nsresolver, 0, None)
        if DEBUG: print 'string_expression returned', rv
##       import pdb ; pdb.set_trace()
        if rv.resultType() == 1:
            return str(rv.numberValue())
        if rv.resultType() == 2:
            return str(rv.booleanValue())
        if rv.resultType() == 3:
            return rv.stringValue()
        if rv.resultType() == 4: # node iterators
            node = rv.iterateNext()
            if not node:
                print 'string_expression: does not match a node:', expr
                return ''
            if rv.iterateNext():
                print 'string_expression: matches multiple nodes:', expr
            valuenode = node.firstChild()
            if not valuenode:
                return ''
            value = valuenode.nodeValue()
            return value
        print 'string_expression: XPath returned unknown type, resultType=', rv.resultType()
        
