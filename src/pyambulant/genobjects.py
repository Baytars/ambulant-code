FORMAT="""class %(pname)sObjectDefinition(MyGlobalObjectDefinition):
    %(bname)s
%(pname)s_object = %(pname)sObjectDefinition('%(pname)s', '%(pname)sObj', '%(cname)s*')
%(pname)s_methods = []
module.addobject(%(pname)s_object)
"""

OBJECTS=[
	"lib/node.h",
	("Node", "ambulant::lib::node", None),
	("NodeContext", "ambulant::lib::node_context", None),
	
	"lib/document.h",
	("Document", "ambulant::lib::document", "NodeContext_Type"),
	
	"lib/event.h",
	("Event", "ambulant::lib::event", None),
	
	"lib/event_processor.h",
	("EventProcessor", "ambulant::lib::event_processor", None),
	
	"lib/parser_factory.h",
	("ParserFactory", "ambulant::lib::parser_factory", None),
	
	"lib/sax_handler.h",
	("XmlParser", "ambulant::lib::xml_parser", None),
	
	"lib/system.h",
	("System", "ambulant::lib::system", None),
	
	"lib/timer.h",
	("TimerEvents", "ambulant::lib::timer_events", None),
	("AbstractTimer", "ambulant::lib::abstract_timer", None),
	
	"common/layout.h",
	("Alignment", "ambulant::common::alignment", None),
	("AnimationNotification", "ambulant::common::animation_notification", None),
	("GuiWindow", "ambulant::common::gui_window", None),
	("GuiEvents", "ambulant::common::gui_events", None),
	("Renderer", "ambulant::common::renderer", "GuiEvents_Type"),
	("BgRenderer", "ambulant::common::bgrenderer", "GuiEvents_Type"),
	("Surface", "ambulant::common::surface", None),
	("WindowFactory", "ambulant::common::window_factory", None),
	("SurfaceTemplate", "ambulant::common::surface_template", "AnimationNotification_Type"),
	("SurfaceFactory", "ambulant::common::surface_factory", None),
	("LayoutManager", "ambulant::common::layout_manager", None),
	
	"common/playable.h",
	("Playable", "ambulant::common::playable", None), # XXX Refcounted
	("PlayableNotification", "ambulant::common::playable_notification", None), 
	("PlayableFactory", "ambulant::common::playable_factory", None),
	
	"common/player.h",
	("PlayerFeedback", "ambulant::common::player_feedback", None),
	("Player", "ambulant::common::player", None),
	
	"common/region_info.h",
	("RegionInfo", "ambulant::common::region_info", None),
	("AnimationDestination", "ambulant::common::animation_destination", "RegionInfo_Type"),
	
	"net/datasource.h",
	("Datasource", "ambulant::net::datasource", None), # XXX Refcounted
	("AudioDatasource", "ambulant::net::audio_datasource", "Datasource_Type"), # XXX Refcounted
	("VideoDatasource", "ambulant::net::video_datasource", None), # XXX Refcounted
	("RawDatasourceFactory", "ambulant::net::raw_datasource_factory", None),
	("AudioDatasourceFactory", "ambulant::net::audio_datasource_factory", None),
	("VideoDatasourceFactory", "ambulant::net::video_datasource_factory", None),
	("AudioParserFinder", "ambulant::net::audio_parser_finder", None),
	("AudioFilterFinder", "ambulant::net::audio_filter_finder", None),
]

out = open('ambulantobjgen.py', 'w')
for item in OBJECTS:
	if type(item) == type(""):
		print >>out, "# From %s:" % item
		inc = '#include "ambulant/%s"' % item
		print >>out, 'includestuff = includestuff + \'' + inc + '\\n\''
		continue
	pname, cname, bname = item
	if bname:
		bname = 'basetype = "%s"' % bname
	else:
		bname = 'pass'
	print >>out, FORMAT % locals()
	