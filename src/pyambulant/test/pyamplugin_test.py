# Test plugin.
# prints lines for the form
# TEST $timestamp $event $args
# $timestamp is delta-t from previous line (or 0)
# event is a word denoting what happened
# args (which may contain spaces) is any additional information
#
import sys
sys.stderr = sys.stdout = open('AM_TEST-output.txt', 'w')
import ambulant
import time

# First check: the ambulant glue works.

print 'TEST', 0, 'test_framework_imported', ambulant.get_version()

class Reporting_feedback(ambulant.player_feedback):
    def __init__(self):
        self.old_time = time.time()
        
    def document_loaded(self):
        t = time.time()
        print 'TEST', t-self.old_time, 'document_loaded'

    def document_started(self):
        t = time.time()
        print 'TEST', t-self.old_time, 'document_started'

    def document_stopped(self):
        t = time.time()
        print 'TEST', t-self.old_time, 'document_stopped'
        sys.exit()

    def node_started(self, node):
        t = time.time()
        print 'TEST', t-self.old_time, 'node_started', node.get_sig()

    def node_stopped(self, node):
        t = time.time()
        print 'TEST', t-self.old_time, 'node_stopped', node.get_sig()

    def node_focussed(self, node):
        pass # We are not interested in these, for now.
        ##t = time.time()
        ##if node:
        ##    print 'TEST', t-self.old_time, 'node_focussed', node.get_sig()
        ##else:
        ##    print 'TEST', t-self.old_time, 'node_unfocussed'

#
# Unfortunately we need a helper object: when initialize() is called the
# gui_player.player has not been instantiated, so we cannot set our
# Reporting_feedback object. Luckily, gui_player.embedder.starting() is called
# at that point, so we can hook up there.

class GlueEmbedder(ambulant.embedder):
    def __init__(self, orig_embedder):
        self.orig_embedder = orig_embedder
        
    # Most calls we simply forward to the original embedder, only for
    # starting() we actually need to do something
    
    def close(self, *args):
        return self.orig_embedder.close(*args)
        
    def open(self, *args):
        return self.orig_embedder.open(*args)
        
    def done(self, *args):
        return self.orig_embedder.done(*args)
        
    def starting(self, player):
        feedback = Reporting_feedback()
        player.set_feedback(feedback)
        return self.orig_embedder.starting(player)
        
    def aux_open(self, *args):
        return self.orig_embedder.aux_open(*args)
        
    def show_file(self, *args):
        return self.orig_embedder.show_file(*args)

#
# Main routine for the plugin.
#
# Hook into the document, if this is indeed a call to the plugin to open a document
#
def initialize(apiversion, factories, gui_player):
    # if gui_player is None there is no document open yet. We will be called again.
    if not gui_player:
        print 'TEST', 0, 'test_framework_initialized'
        return
    embedder = GlueEmbedder(gui_player.get_embedder())
    gui_player.set_embedder(embedder)