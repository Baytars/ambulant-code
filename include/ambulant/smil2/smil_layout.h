/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2010 Stichting CWI,
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 * @$Id$
 */

#ifndef AMBULANT_SMIL2_REGION_BUILDER_H
#define AMBULANT_SMIL2_REGION_BUILDER_H

#include "ambulant/config/config.h"
#include "ambulant/common/layout.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/document.h"
#include <cassert>

namespace ambulant {
namespace common {
class schema;
}}

namespace ambulant {
namespace lib {
class document;
}}

namespace ambulant {

namespace smil2 {

class region_node;

class regpoint_node :
	public common::animation_notification,
	public common::animation_destination
{
  public:
	regpoint_node(const lib::node *n);
	virtual ~regpoint_node() {}
	
	const lib::node* dom_node() { return m_node; }
	std::string get_name() const { return m_node->get_attribute("id"); };
	const char *get_regalign() const { return m_node->get_attribute("regAlign"); }
	void forward_animation_notifications(animation_notification *an);

	rect get_rect(const lib::rect *default_rect = NULL) const {assert(0); return lib::rect(); };
	fit_t get_fit() const  {assert(0); return fit_default; };
	color_t get_bgcolor() const  {assert(0); return 0; };
	double get_bgopacity() const  {assert(0); return 0; };
	bool get_transparent() const  {assert(0); return true; };
	zindex_t get_zindex() const  {assert(0); return 0; };
	bool get_showbackground() const  {assert(0); return false; };
	bool is_subregion() const  {assert(0); return false; };
	double get_soundlevel() const  {assert(0); return 1.0; };
	sound_alignment get_soundalign() const  {assert(0); return sa_default; };
	tiling get_tiling() const  {assert(0); return tiling_default; };
	const char *get_bgimage() const  {assert(0); return NULL; };
	rect get_crop_rect(const size& srcsize) const  {assert(0); return lib::rect(); };
	double get_mediaopacity() const  {assert(0); return 1.0;  };
	double get_mediabgopacity() const  {assert(0); return 1.0; };
	bool is_chromakey_specified() const  {assert(0); return false; };
	lib::color_t get_chromakey() const  {assert(0); return 0; };
	lib::color_t get_chromakeytolerance() const  {assert(0); return 0; };
	double get_chromakeyopacity() const  {assert(0); return 1.0; };

	region_dim get_region_dim(const std::string& which, bool fromdom = false) const;
	color_t get_region_color(const std::string& which, bool fromdom = false) const { assert(0); return 0; };
	zindex_t get_region_zindex(bool fromdom = false) const { assert(0); return 0; };
	double get_region_soundlevel(bool fromdom = false) const { assert(0); return 0; };
	sound_alignment get_region_soundalign(bool fromdom = false) const { assert(0); return sa_default; };
	const region_dim_spec& get_region_panzoom(bool fromdom = false) const { assert(0); static region_dim_spec s; return s; };
	double get_region_opacity(const std::string& which, bool fromdom = false) const { assert(0); return 1.0; };
	void set_region_dim(const std::string& which, const region_dim& rd);
	void set_region_color(const std::string& which, lib::color_t clr) { assert(0); };
	void set_region_zindex(common::zindex_t z) { assert(0); };
	void set_region_soundlevel(double level) { assert(0); };
	void set_region_soundalign(sound_alignment sa) { assert(0); };
	void set_region_panzoom(const region_dim_spec& rds) { assert(0); };
	void set_region_opacity(const std::string& which, double level) { assert(0); };

	void animated();
  private:
	bool fix_from_dom_node();
	const lib::node* m_node;
	region_dim_spec m_rds;
	region_dim_spec m_display_rds;
	std::set<animation_notification*> m_an_clients;
};

class smil_layout_manager :
	public common::layout_manager,
	public lib::avt_change_notification
{
  public:
	smil_layout_manager(common::factories *factory, lib::document *doc);
	~smil_layout_manager();

	void load_bgimages(common::factories *factories);

	common::surface *get_surface(const lib::node *node);
	common::alignment *get_alignment(const lib::node *node);
	common::animation_notification *get_animation_notification(const lib::node *node);
	common::animation_destination *get_animation_destination(const lib::node *node);
	common::surface_template *get_region(const lib::node *n);
#ifdef WITH_SMIL30
	void avt_value_changed_for(const lib::node *n);
#endif // WITH_SMIL30

  private:
	lib::node *get_document_layout(lib::document *doc);
	void build_layout_tree(lib::node *layout_root, lib::document *doc);

	region_node *get_region_node_for(const lib::node *n, bool nodeoverride);
	common::surface *get_default_rendering_surface(const lib::node *n);

	void build_surfaces(common::window_factory *wf);
	void build_body_regions(lib::document *doc);
	common::surface_template *create_top_surface(common::window_factory *wf,
		const region_node *rn, common::bgrenderer *bgrenderer);

	const common::schema *m_schema;
	common::surface_factory *m_surface_factory;

	lib::node *m_layout_section;
	region_node *m_layout_tree;
	std::vector<region_node *>m_default_region_subregions;

	std::vector<common::surface_template*> m_rootsurfaces;
	std::map<std::string, region_node*> m_id2region;
	std::map<std::string, regpoint_node*> m_id2regpoint;
	std::map<std::string, std::list<region_node*> > m_name2region;
	std::map<const lib::node*, region_node*> m_node2region;
	bool m_uses_bgimages;
};

} // namespace smil2

} // namespace ambulant

#endif // AMBULANT_SMIL2_REGION_BUILDER_H
