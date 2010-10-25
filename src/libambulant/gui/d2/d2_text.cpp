// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2010 Stichting CWI,
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/*
 * @$Id$
 */

#include "ambulant/gui/d2/d2_text.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"
#include "ambulant/smil2/test_attrs.h"

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace d2 {

inline D2D1_RECT_F d2_rectf(lib::rect r) {
	return D2D1::RectF(r.left(), r.top(), r.right(), r.bottom());
}

extern const char d2_text_playable_tag[] = "text";
extern const char d2_text_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererDirect2D");
extern const char d2_text_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererText");

IDWriteFactory *ambulant::gui::d2::d2_text_renderer::s_write_factory = NULL;

common::playable_factory *
create_d2_text_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirect2D"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererText"), true);
	return new common::single_playable_factory<
		d2_text_renderer,
		d2_text_playable_tag,
		d2_text_playable_renderer_uri,
		d2_text_playable_renderer_uri2,
		d2_text_playable_renderer_uri2>(factory, mdp);
}


d2_text_renderer::d2_text_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp)
:	d2_renderer<renderer_playable_dsall>(context, cookie, node, evp, factory, mdp),
	m_text_format(NULL),
	m_brush(NULL)

{
	if (s_write_factory == NULL) {
		HRESULT hr;
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(s_write_factory),
			reinterpret_cast<IUnknown**>(&s_write_factory));
		if (!SUCCEEDED(hr)) {
			lib::logger::get_logger()->error("Cannot create DirectWrite factory: error 0x%x", hr);
		}
	}
}

void
d2_text_renderer::init_with_node(const lib::node *node)
{
	lib::textptr font_name("Helvetica");
	float font_size = 14.0;
	m_text_color = 0;

	smil2::params *params = smil2::params::for_node(node);
	if (params) {
		font_name = params->get_str("font-family");
		font_size = params->get_float("font-size", 14.0);
		m_text_color = params->get_color("color", 0);
		delete params;
	}
	if (m_text_format) {
		m_text_format->Release();
		m_text_format = NULL;
	}
	if (m_brush) {
		m_brush->Release();
		m_brush = NULL;
	}
	assert(s_write_factory);
	if (!s_write_factory) return;
	HRESULT hr;
	hr = s_write_factory->CreateTextFormat(
		font_name.wstr(),
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		font_size,
		L"",
		&m_text_format);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->error("Cannot create DirectWrite TextFormat: error 0x%x", hr);
	}
}

d2_text_renderer::~d2_text_renderer()
{
	m_lock.enter();
	m_lock.leave();
}

void
d2_text_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	recreate_d2d();

	m_lock.enter();
	if (!m_data || !m_text_format || !m_brush) {
		m_lock.leave();
		return;
	}

	ID2D1RenderTarget *rt = m_d2player->get_rendertarget();
	assert(rt);

	rect destrect = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("d2_text_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, destrect.left(), destrect.top(), destrect.right(), destrect.bottom());

	destrect.translate(m_dest->get_global_topleft());
	
	lib::textptr text_data((char *)m_data);

	rt->DrawText(
		text_data.c_wstr(),
		wcslen(text_data.c_wstr()),
		m_text_format,
		d2_rectf(destrect),
		m_brush);

#ifdef JNK
	d2Rect d2_dstrect = [view d2RectForAmbulantRect: &dstrect];
	// Set the text matrix
	d2ContextSetTextMatrix(ctx, d2AffineTransformIdentity);
	// Set the color
	double alfa = 1.0;
#ifdef WITH_SMIL30
	const common::region_info *ri = m_dest->get_info();
	if (ri) alfa = ri->get_mediaopacity();
#endif
	d2Float components[] = {redf(m_text_color), greenf(m_text_color), bluef(m_text_color), alfa};
	CGColorSpaceRef genericColorSpace = CGColorSpaceCreateDeviceRGB();
	CGContextSetFillColorSpace(ctx, genericColorSpace);
	CGContextSetFillColor(ctx, components);
	// Set the font
	AM_DBG lib::logger::get_logger()->debug("d2_text: select font %s, size %f", m_font_name, m_font_size);
	CGContextSelectFont(ctx, m_font_name, m_font_size, kCGEncodingMacRoman);
	// Calculate sizes
	float lineheight = m_font_size;
	// XXXX These calculations assume COCOA_USE_BOTLEFT
	float x = CGRectGetMinX(cg_dstrect);
	float y = CGRectGetMaxY(cg_dstrect) - lineheight;
	float w = CGRectGetWidth(cg_dstrect);
	size_t lbegin, lend;
	const char *cdata = (char *)m_data;
	lbegin = 0;
	lend = 0;
	while(_calc_fit(ctx, w, lbegin, lend) ) {
		AM_DBG lib::logger::get_logger()->debug("d2_text: draw line at (%f, %f)", x, y);
		CGContextSetTextPosition(ctx, x, y);
		AM_DBG{ CGAffineTransform mtx = CGContextGetTextMatrix(ctx); lib::logger::get_logger()->debug("cg_text: textmatrix: (%f, %f) (%f, %f) (%f, %f)", mtx.a, mtx.b, mtx.c, mtx.d, mtx.tx, mtx.ty); }
		AM_DBG{ CGAffineTransform mtx = CGContextGetCTM(ctx); lib::logger::get_logger()->debug("cg_text: matrix: (%f, %f) (%f, %f) (%f, %f)", mtx.a, mtx.b, mtx.c, mtx.d, mtx.tx, mtx.ty); }
		CGContextSetTextDrawingMode(ctx, kCGTextFill);
		CGContextShowText(ctx, cdata+lbegin, lend-lbegin);
		lbegin = lend;
		y -= lineheight;
	}
	CGColorSpaceRelease(genericColorSpace);
#endif // JNK
	m_lock.leave();
}

void
d2_text_renderer::recreate_d2d()
{
	if (m_brush) return;
	m_lock.enter();
	HRESULT hr = S_OK;
	ID2D1RenderTarget *rt = m_d2player->get_rendertarget();
	assert(rt);

	double alfa = 1.0;
#ifdef WITH_SMIL30
	const common::region_info *ri = m_dest->get_info();
	if (ri) alfa = ri->get_mediaopacity();
#endif
// CreateSolidColorBrush
	hr = rt->CreateSolidColorBrush(D2D1::ColorF(redf(m_text_color), greenf(m_text_color), bluef(m_text_color), alfa), &m_brush);
	if (!SUCCEEDED(hr)) lib::logger::get_logger()->trace("CreateSolidColorBrush: error 0x%x", hr);
	m_lock.leave();
}


void
d2_text_renderer::discard_d2d()
{
#ifdef JNK
	if (m_d2bitmap) {
//		m_d2bitmap->Release();
		m_d2bitmap = NULL;
	}
#endif // JNK
}

#ifdef JNK
bool
d2_text_renderer::_calc_fit(d2ContextRef ctx, float width, size_t& lbegin, size_t& lend)
{
	const char *cdata = (const char *)m_data;
	// Find beginning point
	if (lbegin > 0)
		while (lbegin < m_data_size && isspace(cdata[lbegin])) lbegin++;
	if (cdata[lbegin] == '\0' || lbegin >= m_data_size) return false;
	lend = lbegin+1;
	int lendcand = lend;
	do {
		while (cdata[lendcand] != '\0' && lendcand < m_data_size && !isspace(cdata[lendcand])) lendcand++;
		if (!_fits(ctx, width, cdata+lbegin, lendcand-lbegin))
			return true;
		lend = lendcand;
		if (cdata[lend] == '\r' || cdata[lend] == '\n')
			return true;
		while (isspace(cdata[lendcand]) && lendcand < m_data_size) lendcand++;
	} while(cdata[lendcand] != '\0' && lendcand < m_data_size);
	return true;
}

bool
d2_text_renderer::_fits(d2ContextRef ctx, float maxwidth, const char *data, size_t datalen)
{
	CGPoint beginpos = CGContextGetTextPosition(ctx);
	CGContextSetTextDrawingMode(ctx, kCGTextInvisible);
	CGContextShowText(ctx, data, datalen);
	CGPoint endpos = CGContextGetTextPosition(ctx);
	float width = endpos.x - beginpos.x;
	return width <= maxwidth;
}
#endif // JNK

} // namespace d2

} // namespace gui

} //namespace ambulant
