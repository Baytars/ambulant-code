
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_PLAYER_H
#define AMBULANT_LIB_PLAYER_H

#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/timelines.h"
#include "ambulant/lib/event_processor.h"

namespace ambulant {

namespace lib {

// Forward
class active_player;

namespace detail {

class timeline_done_arg {
};

} // namespace detail

// I'm not clear on the split between active and passive player yet. The
// passive_player could do loading of the document and parsing it, and
// possibly even generating the timelines.
// Splitting the player in active/passive may be a case of over-generality,
// in which case we get rid of the passive_player later.
class passive_player {
  public:
	friend class active_player;
	
	passive_player() 
	:	m_url("") {}
	passive_player(char *url)
	:	m_url(url) {}
	~passive_player() {}
	
	active_player *activate();
  private:
  	char *m_url;
};

class active_player : public ref_counted {
  public:
	active_player(passive_player *const source, node *tree);
	~active_player();
	
	void start(event_processor *evp, event *playdone);
	void stop();
	
	void timeline_done_callback(detail::timeline_done_arg *p) {
		std::cout << "active_player.timeline_done_callback()" << std::endl;
		m_done = true;
	}
	
	////////////////////////
	// lib::ref_counted interface implementation
	
	long add_ref() {return ++m_refcount;}

	long release() {
		std::cout << "active_player.release, count=" << m_refcount << std::endl;
		if(--m_refcount == 0){
			delete this;
			return 0;
		}
		return m_refcount;
	}

	long get_ref_count() const {return m_refcount;}

  private:
  	passive_timeline *build_timeline();
  	event_processor *const m_event_processor;
	passive_player *const m_source;
	node *m_tree;
	bool m_playing;
	std::vector<active_timeline *> m_active_timelines;
	bool m_done;

	basic_atomic_count<unix::critical_section> m_refcount;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_PLAYER_H
