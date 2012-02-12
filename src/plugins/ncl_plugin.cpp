// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/lib/logger.h"
#include "ambulant/version.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/common/gui_player.h"
#include "ambulant/smil2/test_attrs.h"
using namespace ::ambulant;
using namespace ::ambulant::common;
using namespace ::ambulant::lib;

#include "util/functions.h"
using namespace ::br::pucrio::telemidia::util;

#include "gingalssm/IPresentationEngineManager.h"
using namespace ::br::pucrio::telemidia::ginga::lssm;

#include "cm/IComponentManager.h"
using namespace ::br::pucrio::telemidia::ginga::core::cm;

#include "player/INCLPlayer.h"
using namespace ::br::pucrio::telemidia::ginga::core::player;

#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

class ncl_plugin_factory : public common::playable_factory {
  public:

	ncl_plugin_factory(common::factories* factory, common::playable_factory_machdep *mdp)
	:	m_factory(factory),
		m_mdp(mdp)
	{}
	~ncl_plugin_factory() {};

	bool supports(common::renderer_select *rs);

	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);

	common::playable *new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src) { return NULL; }
  private:
	common::factories *m_factory;
	common::playable_factory_machdep *m_mdp;
};

class ncl_plugin : public common::playable_imp
{
  public:
  ncl_plugin(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp);

	~ncl_plugin();

	void data_avail();
	void start(double where);
	void seek(double where) {};
	//void stop();
	bool stop();
	void pause(pause_display d=display_show);
	void resume();

  private:
	const lib::node* m_node;
	net::url m_url;
	const char* m_file;
	// In Ginga, a Presentation Engine Manager steers all presentations
	IPresentationEngineManager* m_pem;
	IComponentManager* m_cm; // Needed to get one
	INCLPlayer* m_player;
 };


bool
ncl_plugin_factory::supports(common::renderer_select *rs)
{
	const char *renderer_uri = rs->get_renderer_uri();
	// Only use this plugin if explicitly requested. The tag does not matter.
	if (renderer_uri != NULL && strcmp(renderer_uri, AM_SYSTEM_COMPONENT("RendererNCLPlugin")) == 0)
		return true;
	return false;
}

common::playable*
ncl_plugin_factory::new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp)
{
	common::playable *rv;

	lib::xml_string tag = node->get_qname().second;
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin_factory::new_playable(0x%x): node 0x%x:	inspecting %s\n", (void*)this, (void *)node, tag.c_str());
	if ( tag == "ref" || tag == "video" ) /*or any other valid tag, of course */ {
		rv = new ncl_plugin(context, cookie, node, evp, m_factory, m_mdp);
		AM_DBG lib::logger::get_logger()->debug("ncl_plugin_factory::new_playable(0x%x): node 0x%x: returning ncl_plugin 0x%x", (void*)this, (void *)node, (void *)rv);
	} else {
	  AM_DBG lib::logger::get_logger()->debug("ncl_plugin_factory::new_playable(0x%x) : plugin does not support \"%s\"", (void*)this, tag.c_str());
		return NULL;
	}
	return rv;
}

ncl_plugin::ncl_plugin(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories* factory,
	common::playable_factory_machdep *mdp)
:	common::playable_imp(context, cookie, node, evp, factory, mdp)
{
	m_node = node;
	m_url  = node->get_url("src");
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin_factory::new_playable(0x%x) : src=\"%s\"", (void*)this, repr(m_url).c_str());
	m_file = m_url.get_path().c_str();
	m_cm  = IComponentManager::getCMInstance(); 	
	m_pem = ((PEMCreator*)(m_cm->getObject("PresentationEngineManager")))(0, 0, 0, 0, 0, false);
//X	if (fileExists(m_file) /* || isRemoteDoc */) {
		m_pem->setIsLocalNcl(true, NULL);
		if (m_pem->openNclFile(m_file)) {
			m_pem->startPresentation(m_file, "");
			m_player = m_pem->getNclPlayer(m_file); // m_url.get_url());
		}
//X	}
}

ncl_plugin::~ncl_plugin() {
	if (m_pem != NULL) {
	  	m_player->stop();
		delete m_pem;
	}
}

void
ncl_plugin::data_avail()
{
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin::data_avail(0x%x): ", (void*) this);
}

void
ncl_plugin::start(double t)
{
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin::start(0x%x): t=%f", (void*) this, t);
	m_pem->startPresentation(m_file, "");
}


bool
ncl_plugin::stop()
{
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin::stop(0x%x): ", (void*) this);
	m_player->stop();
	return true;
}

void
ncl_plugin::pause(pause_display d)
{
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin::pause(0x%x): ", (void*) this);
	m_player->pause();
}

void
ncl_plugin::resume()
{
	AM_DBG lib::logger::get_logger()->debug("ncl_plugin::resume(0x%x): ", (void*) this);
	m_player->resume();
}


static ambulant::common::factories *
bug_workaround(ambulant::common::factories* factory)
{
	return factory;
}

extern "C"
#ifdef AMBULANT_PLATFORM_WIN32
__declspec(dllexport)
#endif
void initialize(
	int api_version,
	ambulant::common::factories* factory,
	ambulant::common::gui_player *player)
{
	if ( api_version != AMBULANT_PLUGIN_API_VERSION ) {
		lib::logger::get_logger()->warn(gettext("%s: built for plugin-api version %d, current %d. Skipping."),"ncl_plugin", AMBULANT_PLUGIN_API_VERSION, api_version);
		return;
	}
	if ( !ambulant::check_version() ) {
		lib::logger::get_logger()->warn(gettext("%s: built for different Ambulant version (%s)"),"ncl_plugin", AMBULANT_VERSION);
	}
	factory = bug_workaround(factory);
	lib::logger::get_logger()->debug("ncl_plugin: loaded.");
	common::global_playable_factory *pf = factory->get_playable_factory();
	if (pf) {
		ncl_plugin_factory *bpf = new ncl_plugin_factory(factory, NULL);
	pf->add_factory(bpf);
		lib::logger::get_logger()->trace("ncl_plugin: registered");
	}
}
