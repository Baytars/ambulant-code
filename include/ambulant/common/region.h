
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_REGION_H
#define AMBULANT_LIB_REGION_H

#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/event_processor.h"

namespace ambulant {

namespace lib {

// Forward
class active_region;
class active_renderer;

// NOTE: the "bounds" rectangles are currently all with respect
// to the parent, and in a coordinate system where (0,0) is the
// topleft point in the rectangle.
class passive_region {
  public:
	friend class active_region;
	
	passive_region() 
	:	m_name("unnamed"),
		m_inner_bounds(screen_rect<int>(0, 0, 0, 0)),
		m_outer_bounds(screen_rect<int>(0, 0, 0, 0)),
		m_window_topleft(lpoint(0, 0)),
		m_parent(NULL),
		m_cur_active_region(NULL) {}
	passive_region(const std::string &name)
	:	m_name(name),
		m_inner_bounds(screen_rect<int>(0, 0, 0, 0)),
		m_outer_bounds(screen_rect<int>(0, 0, 0, 0)),
		m_window_topleft(lpoint(0, 0)),
		m_parent(NULL),
		m_cur_active_region(NULL) {}
	virtual ~passive_region() {}
	
	virtual void show(active_region *cur);
	virtual void redraw(const screen_rect<int> &r);

	virtual passive_region *subregion(const std::string &name, screen_rect<int> bounds);
	active_region *activate(event_processor *const evp, const node *node);
  protected:
	passive_region(const std::string &name, passive_region *parent, screen_rect<int> bounds,
		lpoint window_topleft)
	:	m_name(name),
		m_inner_bounds(bounds.inset(bounds)),
		m_outer_bounds(bounds),
		m_window_topleft(window_topleft),
		m_parent(parent),
		m_cur_active_region(NULL) {}
	virtual void need_redraw(const screen_rect<int> &r);

  	std::string m_name; // for debugging
  	screen_rect<int> m_inner_bounds;
  	screen_rect<int> m_outer_bounds;
	lpoint m_window_topleft;
  	passive_region *m_parent;
  	active_region *m_cur_active_region;
  	std::vector<passive_region *>m_children;
};

#ifdef __OBJC__
// This is a workaround for a problem when using gcc 3.3 to compile
// ObjC++
;
#endif

class passive_window : public passive_region {
  public:
  	passive_window(const std::string &name, size bounds)
  	:	passive_region(name, NULL, screen_rect<int>(0, 0, bounds.w, bounds.h),
		lpoint(0, 0)) {}
  	virtual ~passive_window() {}
  	
	virtual void need_redraw(const screen_rect<int> &r);
};

#ifdef __OBJC__
// This is a workaround for a problem when using gcc 3.3 to compile
// ObjC++
;
#endif

class window_factory {
  public:
	virtual passive_window *new_window(const std::string &name, size bounds) = 0;
};

class active_region {
  public:
	active_region(event_processor *const evp,
		passive_region *const source,
		const node *node)
	:	m_event_processor(evp),
		m_source(source),
		m_node(node),
		m_renderer(NULL) {}
	virtual ~active_region() {}
	
	virtual void show(active_renderer *renderer);
	virtual void redraw(const screen_rect<int> &r);
	virtual void need_redraw(const screen_rect<int> &r);
	virtual void need_redraw();
	virtual void done();	

  private:
  	event_processor *const m_event_processor;
	passive_region *const m_source;
	const node *m_node;
	active_renderer *m_renderer;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_REGION_H
