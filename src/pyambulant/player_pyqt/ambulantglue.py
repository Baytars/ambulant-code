import ambulant

class jaja:
    pass
    
class Glue(ambulant.gui_player, ambulant.factories):

    def __init__(self, filename, widget):
        ambulant.gui_player.__init__(self)
        self.widget = widget

        #
        # Initialize the gui_player infrastructure
        #
        self.init_factories()
        # XXX The init_*_factory methods should have been called
        # by init_factories, but they aren't???
        self.init_window_factory()
        self.init_playable_factory()
        self.init_datasource_factory()
        self.init_parser_factory()
        self.init_plugins()
        
        #
        # Parse the document, create and initialize the player
        #
        self.document = ambulant.create_from_file(self, filename)
        player = ambulant.create_smil2_player(self.document, self, None)
        player.initialize()
        self.set_player(player)
        
    def qwidget_to_ambulant(self, widget):
        """Hack: convert QWidget to CObject (which Ambulant can handle)"""
        print 'QWidget', widget
        import sip
        widget_address = sip.unwrapinstance(widget)
        print 'address', hex(widget_address)
        widget_vptr = sip.voidptr(widget_address)
        print 'voidptr', widget_vptr
        widget_cobj = widget_vptr.ascobject()
        print 'cobject', widget_cobj
        return widget_cobj
    
    #
    # Initialization methods - create the various factories
    #
    
    def init_window_factory(self):
        qwa = self.qwidget_to_ambulant(self.widget)
        wf = ambulant.create_qt_window_factory_unsafe(qwa, 20, self)
        self.set_window_factory(wf)
        
    def init_playable_factory(self):
        pf = ambulant.get_global_playable_factory()
        qt_pf = ambulant.create_qt_playable_factory(self)
        pf.add_factory(qt_pf)
        self.set_playable_factory(pf)
        
    def init_datasource_factory(self):
        df = ambulant.datasource_factory()
        self.set_datasource_factory(df)
        
    def init_parser_factory(self):
        pf = ambulant.get_parser_factory()
        self.set_parser_factory(pf)
                
class GlueWindowFactory(ambulant.pycppbridge):
    def __init__(self, widget):
        self.widget = widget
        
    def new_window(self, *args):
        print 'new_window', `args`
        return None
        
    def new_background_renderer(self, *args):
        print 'new_background_renderer', `args`
        return None
        