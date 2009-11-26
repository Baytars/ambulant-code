# Scan an Apple header file, generating a Python file of generator calls.

import sys
import os
import re
from bgenlocations import TOOLBOXDIR, BGENDIR
sys.path.append(BGENDIR)
from bgenCxxSupport import CxxScanner

AMBULANT="../../include/ambulant/"
DO_SCAN=True

def main():
    input = [
        AMBULANT+ "version.h",
        AMBULANT+ "lib/node.h",
        AMBULANT+ "lib/document.h",
        AMBULANT+ "lib/event.h",
        AMBULANT+ "lib/event_processor.h",
        AMBULANT+ "lib/parser_factory.h",
        AMBULANT+ "lib/sax_handler.h",
        AMBULANT+ "lib/system.h",
        AMBULANT+ "lib/timer.h",
        AMBULANT+ "lib/transition_info.h",
        AMBULANT+ "common/embedder.h",
        AMBULANT+ "common/factory.h",
        AMBULANT+ "common/gui_player.h",
        AMBULANT+ "common/layout.h",
        AMBULANT+ "common/playable.h",
        AMBULANT+ "common/player.h",
        AMBULANT+ "common/region_info.h",
        AMBULANT+ "common/state.h",
        AMBULANT+ "gui/none/none_gui.h",
        AMBULANT+ "gui/qt/qt_factory.h",
        AMBULANT+ "gui/gtk/gtk_factory.h",
        AMBULANT+ "gui/SDL/sdl_factory.h",
        AMBULANT+ "net/datasource.h",
        AMBULANT+ "net/posix_datasource.h",
        AMBULANT+ "net/stdio_datasource.h",
        AMBULANT+ "net/ffmpeg_factory.h",
        AMBULANT+ "net/rtsp_factory.h",
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

       
class MyScanner(CxxScanner):
    silent = 1

    def makeblacklistnames(self):
        return [
            # node.h
#            "set_attributes",       # string list, too difficult
            "get_read_ptr", # Does not translate to Python
            "get_frame", # Doable, with size in the arglist
#            "wakeup", # XXX private/protected
#            "wait_event", # XXX private/protected
            "flag_event", # Constructor for unsupported type
            "sax_error", # Constructor for unsupported type
#            "audio_format_choices", # Constructor for unsupported type
            "none_playable", # XXX Constructor for unsupported type
            "none_background_renderer", # XXX Constructor for unsupported type
            "none_playable_factory",  # XXX Constructor for unsupported type
            "event_processor_impl", # Constructor for unsupported type
            "load_test_attrs",
            "create_from_tree", # Ifdeffed out, for the time being
            "gdk_pixmap_bitblt",
            "create_gtk_window_factory",
            # For get_screenshot(), the Python->C++ bridge works, but not the
            # reverse. This showed up on 64bit machines (some of the casts were
            # wrong. Need to fix later.
            "get_screenshot",  
            "single_playable_factory",
           
        ]

    def makeblacklisttypes(self):
        return [
            "event_processor_impl",  # Concrete version of event_processor, ignore.
            "audio_datasource", #XXX
            "video_datasource", #XXX
            "abstract_demux", #XXX
            "demux_datasink", #XXX
            "demux_datasink_ptr", #XXX
            "posix_datasource", # We want only the factory and (abstract) interface
            "posix_datasource_ptr", # Ditto
            "posix_datasource_factory", # Ditto
            "stdio_datasource", # We want only the factory and (abstract) interface
            "stdio_datasource_ptr", # Ditto
            "stdio_datasource_factory", # Ditto
            "filter_datasource_impl", # XXXX
            "filter_datasource_impl_ptr", # XXXX
            "live_audio_datasource_factory",
            "live_video_datasource_factory",
            "raw_filter_finder", # XXXX
            "raw_filter_finder_ptr", # XXXX
            "ts_packet_t", # XXXX Lazy: should do this one.
            "audio_datasource_mixin", #  XXXX Multiple inheritance is difficult!
            "Where_we_get_our", # Parser trips over a comment:-)
            "flag_event",  # Holds a reference to a bool, not useful for Python
            "flag_event_ptr",  # Holds a reference to a bool, not useful for Python
            "const_custom_test_map_ptr", # We don't do maps for now
            "node_list",     # We don't do lists, for now
            "delta_timer", # XXX for now
            "iterator", # Not needed in Python
            "const_iterator", # Not needed in Python
            "char_ptr_ptr",
            "sax_content_handler",
            "sax_content_handler_ptr",
            "sax_error_handler_ptr",
            "sax_error",
            "sax_error_ptr",
            "none_playable",
            "none_playable_ptr",
            "none_background_renderer",
            "none_background_renderer_ptr",
            "tile_positions",    # We don't do lists, for now
            "region_dim_spec",   # XXXX Not sure this is needed?
            "const_region_dim_spec_ref",  # XXXX Not sure this is needed?
            # Next couple are gtk. For now I've excluded them because they cause
            # errors, but it could well be they are needed and need to be handled.
            "QWidget_ptr",
            "GdkPixmap_ptr",
            "GtkWidget_ptr",
            "gtk_ambulant_widget",
            "gtk_ambulant_widget_ptr",
            "gtk_window_factory",
            "gtk_window_factory_ptr",
            "ambulant_gtk_window",
            "ambulant_gtk_window_ptr",
            "gtk_renderer_factory",
            "gtk_renderer_factory_ptr",
            "gtk_video_factory",
            "gtk_video_factory_ptr",
            "event_processor_observer", # XXX Lazy right now, do later
            "event_processor_observer_ptr",
            "single_playable_factory",
            "single_playable_factory_ptr"
            
        ]

    def makegreylist(self):
        return [
            ('#ifdef WITH_QT', [
                'create_qt_window_factory_unsafe',
                'create_qt_video_factory',
                'create_qt_fill_playable_factory',
                'create_qt_image_playable_factory',
                'create_qt_smiltext_playable_factory',
                'create_qt_text_playable_factory',
                'create_qt_html_playable_factory',
                'create_qt_video_playable_factory',
                ]
            ),
            ('#ifdef WITH_GTK', [
                'create_gtk_window_factory_unsafe',
                'create_gtk_video_factory',
                'create_gtk_fill_playable_factory',
                'create_gtk_image_playable_factory',
                'create_gtk_smiltext_playable_factory',
                'create_gtk_text_playable_factory',
                'create_gtk_video_playable_factory',
                ]
            ),
            ('#ifdef WITH_SDL', [
                'create_sdl_playable_factory',
                ]
            ),
            ('#ifdef WITH_FFMPEG', [
                'get_ffmpeg_raw_datasource_factory',
                'get_ffmpeg_video_datasource_factory',
                'get_ffmpeg_audio_datasource_factory',
                'get_ffmpeg_audio_parser_finder',
                'get_ffmpeg_audio_filter_finder',
                'get_ffmpeg_audio_decoder_finder',
                ]
            ),
            ('#ifdef WITH_LIVE', [
                'create_live_video_datasource_factory',
                'create_live_audio_datasource_factory',
                ]
            ),
            ('#ifndef AMBULANT_PLATFORM_WIN32', [
                'create_posix_datasource_factory',
                
                ]
            ),
            ('#ifdef WITH_AMBULANT_TEST', [
                'new_default_subsurface',
                
                ]
            ),
            ('#ifdef WITH_SMIL30', [
                'apply_avt',
                'set_state',
                'get_state',
                ]
            ),
        ]

    def makerepairinstructions(self):
        return [
        	# Assume an int pointer is an out-param
        	(
        	  [
        	  	('int_ptr', '*', 'InMode'),
        	  ], [
        	    ('int', '*', 'OutMode')
        	  ]
        	),
            # Assume a pair (const char *, size_t) is an input buffer
            (
              [
                ('char_ptr', '*', 'InMode+ConstMode'),
                ('size_t', '*', 'InMode')
              ],[
                ('InBuffer', '*', 'InMode+ConstMode'),
               ]
            ),
            
            # Assume a const char * is an input string
            (
              [
                ('char_ptr', '*', 'InMode+ConstMode'),
              ],[
                ('stringptr', '*', '*'),
               ]
            ),
            
            # Assume a const uint8_t * is an input string
            (
              [
                ('uint8_t_ptr', '*', 'InMode+ConstMode'),
              ],[
                ('stringptr', '*', '*'),
               ]
            ),
            
            # Handle const char * return values as strings too
            (
              [
                ('const_char_ptr', '*', 'ReturnMode'),
              ],[
                ('return_stringptr', '*', '*'),
               ]
            ),
            
            # And also handle char **, size_t * (malloced buffer plus size)
            # The InOutMode is a hack: it makes sure that the buffer is set to
            # NULL.
            (
              [
                ('char_ptr_ptr', '*', 'InMode'),
                ('size_t_ptr', '*', 'InMode'),
              ],[
                ('output_malloc_buf', '*', 'InOutMode'),
               ]
            ),
            
            # get_fit_rect got one output parameter wrong.
            ('get_fit_rect',
              [
                ('lib_rect_ptr', 'out_src_rect', 'InMode'),
              ], [
                ('lib_rect', 'out_src_rect', 'OutMode'),
              ]
            ),
            ('create_qt_window_factory_unsafe',
              [
                ('void_ptr', '*', 'InMode'),
              ], [
                ('pycobject', '*', 'InMode'),
              ]
            ),
            ('create_gtk_window_factory_unsafe',
              [
                ('void_ptr', '*', 'InMode'),
              ], [
                ('pygobject', '*', 'InMode'),
              ]
            ),

        ]        
if __name__ == "__main__":
    main()
