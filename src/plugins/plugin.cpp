#include "ambulant/plugin/plugin.h"
#include "ambulant/common/renderer.h"


using namespace ambulant;

void 
plugin::initialize(common::global_playable_factory* rf, net::datasource_factory df)
{
	//rf->add_factory(new /*playable_factory(df)*/);
}
	
common::playable* 
plugin::basic_plugin_factory::new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);
{
	common::playable *rv;
	lib::xml_string tag = node->get_qname().second;
    AM_DBG lib::logger::get_logger()->trace("sdl_renderer_factory: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "audio") /*or any other tag ofcourse */ {
		rv = new plugin::basic_plugin(context, cookie, node, evp, m_datasource_factory);
		AM_DBG lib::logger::get_logger()->trace("sdl_renderer_factory: node 0x%x: returning sdl_active_audio_renderer 0x%x", (void *)node, (void *)rv);
	} else {
		AM_DBG lib::logger::get_logger()->trace("sdl_renderer_factory: no SDL renderer for tag \"%s\"", tag.c_str());
        return NULL;
	}
	return rv;
	
}


plugin::basic_plugin::basic_plugin(
	common::playable_notification *context,
    common::playable_notification::cookie_type cookie,
    const lib::node *node,
    lib::event_processor *evp,
	net::datasource_factory *df) 
:	common::playable_imp(context, cookie, node, evp)
{
	
}

void
plugin::basic_plugin::data_avail()
{
}

void
plugin::basic_plugin::start(double t)
{
}


void 
plugin::basic_plugin::stop()
{
}

void 
plugin::basic_plugin::pause()
{
}

void 
plugin::basic_plugin::resume()
{
}
