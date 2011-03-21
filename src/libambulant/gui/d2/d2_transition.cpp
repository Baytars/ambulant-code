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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/*
 * @$Id$
 */
#ifdef	WITH_D2
#undef	WITH_D2
#endif//WITH_D2
#include "ambulant/gui/d2/d2_transition.h"
#include "ambulant/gui/d2/d2_player.h"
#include "ambulant/gui/d2/d2_window.h"
#include "ambulant/lib/logger.h"


#include <wincodec.h>
#include <d2d1.h>
#include <d2d1helper.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace common;

namespace gui {

namespace d2 {

inline D2D1_RECT_F d2_rectf(lib::rect r) {
	return D2D1::RectF((float) r.left(), (float) r.top(), (float) r.right(), (float) r.bottom());
}
inline D2D1_RECT_U d2_rectu(lib::rect r) {
	return D2D1::RectU((UINT32) r.left(), (UINT32) r.top(), (UINT32) r.right(), (UINT32) r.bottom());
}
inline D2D1_SIZE_F d2_sizef(lib::rect r) {
	return D2D1::SizeF((float) r.width(), (float) r.height());
}
inline D2D1_SIZE_U d2_sizeu(lib::rect r) {
	return D2D1::SizeU((UINT32) r.width(), (UINT32) r.height());
}

#ifdef	WITH_D2
// Helper functions to setup and finalize transitions
static CGLayer*
setup_transition (bool outtrans, AmbulantView *view)
{
	CGLayer* rv = NULL;
	if (outtrans) {
		rv = [view getTransitionTmpSurface];
//		 *rv = NULL; //[view getTransitionOldSource];
//		[[view getTransitionSurface] lockFocus];
		return rv;
	} else {
		rv = [view getTransitionSurface];
//		return [view getTransitionNewSource];
//		rv = UIGraphicsGetImageFromCurrentImageContext().CGImage;
//		CALayer* cal = 
	}
	return rv;
}
#endif//WITH_D2
static void
finalize_transition(bool outtrans, common::surface *dst)
{
#ifdef	WITH_D2
	if (outtrans) {
		cg_window *window = (cg_window *)dst->get_gui_window();
		AmbulantView *view = (AmbulantView *)window->view();
//XX	[[view getTransitionSurface] unlockFocus];

		const lib::rect& dstrect_whole = dst->get_clipped_screen_rect();
		CGRect cg_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
		[[view getTransitionNewSource] drawInRect: cg_dstrect_whole
			fromRect: cg_dstrect_whole
			operation: NSCompositeSourceOver
			fraction: 1.0f];
	}
#endif//WITH_D2
}

#ifdef	WITH_D2
// Helper function: add a clockwise defined rectangle to the path of a CGContext
// This is used for out transitions to reverse the effect of the counter-clockwise defined 
// clipping paths by enclosing them in a clockwise defined rectangle for the whole region
// using the non-zero winding rule
void
add_clockwise_rectangle (CGContextRef ctx, CGRect cg_rect)
{	
	CGFloat cg_minX = CGRectGetMinX(cg_rect), cg_maxX = CGRectGetMaxX(cg_rect),
	cg_minY = CGRectGetMinY(cg_rect), cg_maxY = CGRectGetMaxY(cg_rect);
	CGContextMoveToPoint(ctx, cg_minX, cg_minY);
	CGContextAddLineToPoint(ctx, cg_minX, cg_maxY);
	CGContextAddLineToPoint(ctx, cg_maxX, cg_maxY);
	CGContextAddLineToPoint(ctx, cg_maxX, cg_minY);
	CGContextAddLineToPoint(ctx, cg_minX, cg_minY);
}
#endif//WITH_D2
	
void
d2_transition_blitclass_fade::update()
{
	AM_DBG lib::logger::get_logger()->debug("d2_transition_blitclass_fade::update(%f)", m_progress);
	gui_window *window = m_dst->get_gui_window();
	d2_window *cwindow = (d2_window *)window;
	d2_player* d2_player = cwindow->get_d2_player();

	ID2D1Layer* layer = NULL;
	ID2D1Bitmap* bitmap = NULL;
	D2D1_LAYER_PARAMETERS layer_params = D2D1::LayerParameters();
	ID2D1RenderTarget* rt = (ID2D1RenderTarget*) d2_player->get_rendertarget();
	ID2D1BitmapRenderTarget* brt = d2_transition_renderer::s_transition_rendertarget;
	HRESULT hr = brt->EndDraw();
	CheckError(hr);
	hr = brt->GetBitmap(&bitmap);
	CheckError(hr);
	if (this->m_progress < 1.0) {
		hr = rt->CreateLayer(&layer);
		CheckError(hr);
		layer_params.opacity = m_progress;
		rt->PushLayer(layer_params, layer);
	}
	rt->DrawBitmap(bitmap);
	if (this->m_progress < 1.0) {
		rt->PopLayer();
	}
cleanup:
	SafeRelease(&layer);
	SafeRelease(&bitmap);
}

void
d2_transition_blitclass_rect::update()
{
	gui_window *window = m_dst->get_gui_window();
	d2_window *cwindow = (d2_window *)window;
	d2_player* d2_player = cwindow->get_d2_player();

	lib::rect newrect_whole = m_newrect;
	newrect_whole.translate(m_dst->get_global_topleft());
	newrect_whole &= m_dst->get_clipped_screen_rect();
	lib::point LT = newrect_whole.left_top();
	lib::point RB = newrect_whole.right_bottom();
	if (newrect_whole.empty())
		return;
	ID2D1BitmapRenderTarget* brt = d2_transition_renderer::s_transition_rendertarget;
	HRESULT hr = brt->EndDraw();
	if (SUCCEEDED(hr)) {
		ID2D1RenderTarget* rt = (ID2D1RenderTarget*) d2_player->get_rendertarget();
		D2D1_RECT_F d2_new_rect_f = d2_rectf(newrect_whole);
		D2D1_SIZE_F d2_full_size_f = brt->GetSize();
		D2D1_RECT_F d2_full_rect_f = D2D1::RectF(0,0,d2_full_size_f.width,d2_full_size_f.height);
		ID2D1Bitmap* bitmap = NULL;
#ifdef	AM_DMP
		d2_player->dump(brt, "rect::update:bmt");
#endif//AM_DMP
		hr = brt->GetBitmap(&bitmap);
		if (SUCCEEDED(hr)) {
			rt->PushAxisAlignedClip(d2_new_rect_f,
							        D2D1_ANTIALIAS_MODE_ALIASED);
			rt->DrawBitmap(bitmap,
							d2_full_rect_f,
							1.0f,
							D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
							d2_full_rect_f);
			rt->PopAxisAlignedClip();
			hr = rt->Flush();
			if (FAILED(hr)) {
				lib::logger::get_logger()->trace("d2_transition_renderer::blitclass::rect::update: DrawBitmap returns 0x%x", hr);
			}
		} // other HRESULT failures ignored, may happen e.g. when bitmap is empty
	} 
	AM_DBG lib::logger::get_logger()->debug("d2_transition_blitclass_rect::update(%f) newrect_whole=(%d,%d),(%d,%d)",m_progress,LT.x,LT.y,RB.x,RB.y);
}

void
d2_transition_blitclass_r1r2r3r4::update()
{
	AM_DBG lib::logger::get_logger()->debug("d2_transition_blitclass_r1r2r3r4::update(%f)", m_progress);

	gui_window *window = m_dst->get_gui_window();
	d2_window *cwindow = (d2_window *)window;
	d2_player* d2_player = cwindow->get_d2_player();
	lib::rect newsrcrect = m_newsrcrect;
	lib::rect newdstrect = m_newdstrect;
	lib::rect oldsrcrect = m_oldsrcrect;
	lib::rect olddstrect = m_olddstrect;
	ID2D1Layer* layer = NULL;
	ID2D1Bitmap* bitmap_old = NULL; // bitmap for the "old" stuff
	ID2D1Bitmap* bitmap_new = NULL; // bitmap for the "new" stuff
	ID2D1RenderTarget* rt = (ID2D1RenderTarget*) d2_player->get_rendertarget();
	D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties();
	D2D1_PIXEL_FORMAT rt_format = rt->GetPixelFormat();
	D2D1_MATRIX_3X2_F d2_rt_transform, d2_brt_transform;
	ID2D1BitmapRenderTarget* brt = d2_transition_renderer::s_transition_rendertarget;
	rt->GetTransform(&d2_rt_transform);
	brt->GetTransform(&d2_brt_transform);
	HRESULT hr = brt->EndDraw();
	CheckError(hr);
	newsrcrect.translate(m_dst->get_global_topleft());
	newsrcrect &= m_dst->get_clipped_screen_rect();
	newdstrect.translate(m_dst->get_global_topleft());
	newdstrect &= m_dst->get_clipped_screen_rect();
	oldsrcrect.translate(m_dst->get_global_topleft());
	oldsrcrect &= m_dst->get_clipped_screen_rect();
	// compensate for any adjustments made by d2_player::_calc_fit(&xoff, &yoff)
	oldsrcrect.translate(lib::point((int) d2_rt_transform._31, (int) d2_rt_transform._32));
	olddstrect.translate(m_dst->get_global_topleft());
	olddstrect &= m_dst->get_clipped_screen_rect();
	// Get needed parts of the old and new stuff (as bitmaps) and draw them at their final destinations
	props.pixelFormat = rt_format;
	CheckError(hr);
	// copy the bits of the old stuff to the new destination
	hr = rt->CreateBitmap(d2_sizeu(oldsrcrect), props, &bitmap_old);
	CheckError(hr);
	hr = bitmap_old->CopyFromRenderTarget(NULL, rt, &d2_rectu(oldsrcrect));
	CheckError(hr);
	rt->DrawBitmap(bitmap_old, d2_rectf(olddstrect));
	// copy the bits of the new stuff (from the back bitmap 'brt') to the right spot on screen;
	// we need to use ID2D1Bitmap::CopyFromRenderTarget, therefore we must create the bitmap
	// where we put the data into ('bitmap_new') with equal properties as its data source ('brt')
	props.pixelFormat = brt->GetPixelFormat();
	FLOAT dpiX = 0.0, dpiY = 0.0;
	brt->GetDpi(&dpiX, &dpiY);
	props.dpiX = dpiX;
	props.dpiY = dpiY;
	hr = brt->CreateBitmap(d2_sizeu(newsrcrect), props, &bitmap_new);
	CheckError(hr);
	hr = bitmap_new->CopyFromRenderTarget(NULL, brt, &d2_rectu(newsrcrect));
	CheckError(hr);
	rt->DrawBitmap(bitmap_new, d2_rectf(newdstrect));

cleanup:
	SafeRelease(&bitmap_old);
	SafeRelease(&bitmap_new);
}

void
d2_transition_blitclass_rectlist::update()
{
	AM_DBG lib::logger::get_logger()->debug("cg_transition_blitclass_rectlist::update(%f)", m_progress);
	gui_window *window = m_dst->get_gui_window();
	d2_window *cwindow = (d2_window *)window;
	d2_player* d2_player = cwindow->get_d2_player();

	lib::rect newrect_whole = m_dst->get_rect();
	newrect_whole.translate(m_dst->get_global_topleft());
	newrect_whole &= m_dst->get_clipped_screen_rect();
	lib::point LT = newrect_whole.left_top();
	lib::point RB = newrect_whole.right_bottom();
	if (newrect_whole.empty())
		return;
	std::vector< lib::rect >::iterator newrect;

	ID2D1BitmapRenderTarget* brt = d2_transition_renderer::s_transition_rendertarget;	
	HRESULT hr = brt->EndDraw();
	if (FAILED(hr)) {
		return;
	}
	ID2D1RenderTarget* rt = (ID2D1RenderTarget*) d2_player->get_rendertarget();
	D2D1_SIZE_F d2_full_size_f = brt->GetSize();
	D2D1_RECT_F d2_full_rect_f = D2D1::RectF(0,0,d2_full_size_f.width,d2_full_size_f.height);
	ID2D1Bitmap* bitmap = NULL;
	hr = brt->GetBitmap(&bitmap);
	if (FAILED(hr)) {
		return;
	}
#ifdef	AM_DMP
	d2_player->dump(brt, "rect::update:bmt");
#endif//AM_DMP
	for (newrect=m_newrectlist.begin(); newrect != m_newrectlist.end(); newrect++) {
		lib::rect newrect_whole = *newrect;
		if (newrect_whole.empty()) {
			continue;
		}
//		is_clipped = true;
		newrect_whole.translate(m_dst->get_global_topleft());
		newrect_whole &= m_dst->get_clipped_screen_rect();
		D2D1_RECT_F d2_new_rect_f = d2_rectf(newrect_whole);
// Using axis aligened clip is more effecient than using a path, though DrawBitmap is called inside the loop
		rt->PushAxisAlignedClip(d2_new_rect_f, D2D1_ANTIALIAS_MODE_ALIASED);
		rt->DrawBitmap(bitmap, d2_full_rect_f, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,	d2_full_rect_f);
		rt->PopAxisAlignedClip();
		hr = rt->Flush();
		if (FAILED(hr)) {
			lib::logger::get_logger()->trace("d2_transition_renderer::blitclass::rectlist::update: DrawBitmap returns 0x%x", hr);
			break;
		}
	}
	AM_DBG lib::logger::get_logger()->debug("d2_transition_blitclass_rectlist::update(%f) newrect_whole=(%d,%d),(%d,%d)",m_progress,LT.x,LT.y,RB.x,RB.y);
}

// Helper function: add clipping path from the list of polygons
static ID2D1PathGeometry*
path_from_polygon_list(ID2D1Factory* factory, const lib::point& origin, std::vector< std::vector<lib::point> > polygon_list)
{
	ID2D1PathGeometry* path = NULL;
	ID2D1GeometrySink* sink = NULL;
	D2D1_POINT_2F new_d2_point_2f = D2D1::Point2F();
	lib:point old_point = lib::point();
	std::vector<lib::point>::iterator newpoint_p;

	HRESULT hr = factory->CreatePathGeometry(&path);
	CheckError(hr);
	hr = path->Open(&sink);
	CheckError(hr);
	sink->SetFillMode(D2D1_FILL_MODE_WINDING);

	for( std::vector< std::vector<lib::point> >::iterator polygon_p = polygon_list.begin(); polygon_p != polygon_list.end(); polygon_p++) {
		if ((*polygon_p).size() < 3) {
			lib::logger::get_logger()->debug("path_from_polygon_list: invalid polygon size=%d", (*polygon_p).size());
			continue;
		}
		newpoint_p = (*polygon_p).begin(); 
		old_point = *newpoint_p + origin;
		newpoint_p++;
		sink->BeginFigure(D2D1::Point2F((float) old_point.x, (float) old_point.y), D2D1_FIGURE_BEGIN_FILLED);
		for(;newpoint_p != (*polygon_p).end(); newpoint_p++) {
			lib::point p = *newpoint_p + origin;
			AM_DBG lib::logger::get_logger()->debug("path_from_polygon_list: point=%d, %d", p.x, p.y);
			new_d2_point_2f = D2D1::Point2F((float) p.x, (float) p.y);
			sink->AddLine(new_d2_point_2f);
			old_point  = p;
		}
		sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	}
	hr = sink->Close();

cleanup:
	SafeRelease(&sink);
	return path;
}
	

static void
_d2_polygon_list_update (common::surface* dst, std::vector< std::vector<lib::point> > polygon_list)
{
	gui_window *window = dst->get_gui_window();
	d2_window *cwindow = (d2_window *)window;
	d2_player* d2_player = cwindow->get_d2_player();
	const lib::point& dst_global_topleft = dst->get_global_topleft();
	
	ID2D1Layer* layer = NULL;
	ID2D1PathGeometry* path = NULL;
	D2D1_RECT_F d2_full_rect_f = D2D1::RectF();
	D2D1_SIZE_F d2_full_size_f = D2D1::SizeF();
	ID2D1BitmapRenderTarget* brt = d2_transition_renderer::s_transition_rendertarget;
	ID2D1RenderTarget* rt = (ID2D1RenderTarget*) d2_player->get_rendertarget();
	HRESULT hr = rt->CreateLayer(&layer);
	if (FAILED(hr)) {
		lib::logger::get_logger()->trace("d2_transition_renderer::blitclass::polygon[list]::update: CreateLayer returns 0x%x", hr);
	}
	CheckError(hr);
	hr = brt->EndDraw();
	CheckError(hr);
	d2_full_size_f = brt->GetSize();
	d2_full_rect_f = D2D1::RectF(0,0,d2_full_size_f.width,d2_full_size_f.height);
	ID2D1Bitmap* bitmap = NULL;
	hr = brt->GetBitmap(&bitmap);
	CheckError(hr);

	path = path_from_polygon_list(d2_player->get_D2D1Factory(), dst_global_topleft, polygon_list);

	rt->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(), path), layer);
	rt->DrawBitmap(bitmap, d2_full_rect_f, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,	d2_full_rect_f);
	rt->PopLayer();
	hr = rt->Flush();
	if (FAILED(hr)) {
		lib::logger::get_logger()->trace("d2_transition_renderer::blitclass::polygon[list]::update: DrawBitmap returns 0x%x", hr);
	}
cleanup:
	SafeRelease(&layer);
	SafeRelease(&path);
}

void
d2_transition_blitclass_poly::update()
{
	AM_DBG lib::logger::get_logger()->debug("cg_transition_blitclass_poly::update(%f)", m_progress);
	std::vector< std::vector<lib::point> > polygon_list;
	polygon_list.push_back(this->m_newpolygon);
	_d2_polygon_list_update(m_dst, polygon_list);
}

void
d2_transition_blitclass_polylist::update()
{
	AM_DBG lib::logger::get_logger()->debug("d2_transition_blitclass_polylist::update(%f)", m_progress);
	_d2_polygon_list_update(m_dst, this->m_newpolygonlist);
}

smil2::transition_engine *
d2_transition_engine(common::surface *dst, bool is_outtrans, const lib::transition_info *info)
{
	smil2::transition_engine *rv;

	switch(info->m_type) {
	// Series 1: edge wipes
	case lib::barWipe:
		rv = new d2_transition_engine_barwipe();
		break;
	case lib::boxWipe:
		rv = new d2_transition_engine_boxwipe();
		break;
	case lib::fourBoxWipe:
		rv = new d2_transition_engine_fourboxwipe();
		break;
	case lib::barnDoorWipe:
		rv = new d2_transition_engine_barndoorwipe();
		break;
	case lib::diagonalWipe:
		rv = new d2_transition_engine_diagonalwipe();
		break;
	case lib::miscDiagonalWipe:
		rv = new d2_transition_engine_miscdiagonalwipe();
		break;
	case lib::veeWipe:
		rv = new d2_transition_engine_veewipe();
		break;
	case lib::barnVeeWipe:
		rv = new d2_transition_engine_barnveewipe();
		break;
	case lib::zigZagWipe:
		rv = new d2_transition_engine_zigzagwipe();
		break;
	case lib::barnZigZagWipe:
		rv = new d2_transition_engine_barnzigzagwipe();
		break;
	case lib::bowTieWipe:
		rv = new d2_transition_engine_bowtiewipe();
		break;
	// series 2: iris wipes
	case lib::irisWipe:
		rv = new d2_transition_engine_iriswipe();
		break;
	case lib::pentagonWipe:
		rv = new d2_transition_engine_pentagonwipe();
		break;
	case lib::arrowHeadWipe:
		rv = new d2_transition_engine_arrowheadwipe();
		break;
	case lib::triangleWipe:
		rv = new d2_transition_engine_trianglewipe();
		break;
	case lib::hexagonWipe:
		rv = new d2_transition_engine_hexagonwipe();
		break;
	case lib::eyeWipe:
		rv = new d2_transition_engine_eyewipe();
		break;
	case lib::roundRectWipe:
		rv = new d2_transition_engine_roundrectwipe();
		break;
	case lib::ellipseWipe:
		rv = new d2_transition_engine_ellipsewipe();
		break;
	case lib::starWipe:
		rv = new d2_transition_engine_starwipe();
		break;
	case lib::miscShapeWipe:
		rv = new d2_transition_engine_miscshapewipe();
		break;
	// series 3: clock-type wipes
	case lib::clockWipe:
		rv = new d2_transition_engine_clockwipe();
		break;
	case lib::singleSweepWipe:
		rv = new d2_transition_engine_singlesweepwipe();
		break;
	case lib::doubleSweepWipe:
		rv = new d2_transition_engine_doublesweepwipe();
		break;
	case lib::saloonDoorWipe:
		rv = new d2_transition_engine_saloondoorwipe();
		break;
	case lib::windshieldWipe:
		rv = new d2_transition_engine_windshieldwipe();
		break;
	case lib::fanWipe:
		rv = new d2_transition_engine_fanwipe();
		break;
	case lib::doubleFanWipe:
		rv = new d2_transition_engine_doublefanwipe();
		break;
	case lib::pinWheelWipe:
		rv = new d2_transition_engine_pinwheelwipe();
		break;
	// series 4: matrix wipe types
	case lib::snakeWipe:
		rv = new d2_transition_engine_snakewipe();
		break;
	case lib::waterfallWipe:
		rv = new d2_transition_engine_waterfallwipe();
		break;
	case lib::spiralWipe:
		rv = new d2_transition_engine_spiralwipe();
		break;
	case lib::parallelSnakesWipe:
		rv = new d2_transition_engine_parallelsnakeswipe();
		break;
	case lib::boxSnakesWipe:
		rv = new d2_transition_engine_boxsnakeswipe();
		break;
	// series 5: SMIL-specific types
	case lib::pushWipe:
		rv = new d2_transition_engine_pushwipe();
		break;
	case lib::slideWipe:
		rv = new d2_transition_engine_slidewipe();
		break;
	case lib::fade:
	case lib::audioVisualFade:
		rv = new d2_transition_engine_fade();
		break;
	default:
		lib::logger::get_logger()->trace("d2_transition_engine: transition type %s not yet implemented",
			repr(info->m_type).c_str());
		rv = NULL;
	}
	if (rv)
		rv->init(dst, is_outtrans, info);
	return rv;
}
	
} // namespace d2

} // namespace gui

} //namespace ambulant

