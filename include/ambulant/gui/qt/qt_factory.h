/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

#ifndef QT_FACTORY_H
#define QT_FACTORY_H

#include "ambulant/common/playable.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/gui/none/none_gui.h"

#include "qt_includes.h"
#include "qt_fill.h"

namespace ambulant {

namespace gui {

namespace qt {

/// Debug function that dumps a pixmap to a file. An incrementing
/// count is appended to the filenname, and an extension added.
void dumpPixmap(QPixmap* qpm, std::string filename);

class qt_ambulant_widget;

class ambulant_qt_window : public common::gui_window {
  public:
	ambulant_qt_window(const std::string &name,
			   lib::rect* bounds,
			   common::gui_events *region);
	~ambulant_qt_window();
			   
	void set_ambulant_widget(qt_ambulant_widget* qaw);
	qt_ambulant_widget* get_ambulant_widget();

	void need_redraw(const lib::rect &r);
	void redraw(const lib::rect &r);
	void redraw_now();

	void mouse_region_changed();
	void user_event(const lib::point &where, int what=0);
	void need_events(bool want);

	QPixmap* get_ambulant_pixmap();
	QPixmap* new_ambulant_surface();
	QPixmap* get_ambulant_surface();
	QPixmap* get_ambulant_oldpixmap();
	QPixmap* get_pixmap_from_screen(const lib::rect &r);
	void reset_ambulant_surface(void);
	void set_ambulant_surface(QPixmap* surf);
	void delete_ambulant_surface();
#ifdef USE_SMIL21
	void startScreenTransition();
	void endScreenTransition();
	void screenTransitionStep(smil2::transition_engine* engine, lib::transition_info::time_type now);
		
	void _screenTransitionPreRedraw();
	void _screenTransitionPostRedraw(const lib::rect &r);
#endif
	
  private:
	qt_ambulant_widget* m_ambulant_widget;
	QPixmap* m_pixmap;
	QPixmap* m_oldpixmap;
	QPixmap* m_surface;
#ifdef USE_SMIL21
	int m_fullscreen_count;
	QPixmap* m_fullscreen_prev_pixmap;
	QPixmap* m_fullscreen_old_pixmap;
	smil2::transition_engine* m_fullscreen_engine;
	lib::transition_info::time_type m_fullscreen_now;
#endif

 public:
	QPixmap* m_tmppixmap;
};  // class ambulant_qt_window

class qt_ambulant_widget : public QWidget {
  public:
	qt_ambulant_widget(const std::string &name,
			   lib::rect* bounds,
			   QWidget* parent_widget);
	~qt_ambulant_widget();
	
	void set_qt_window( ambulant_qt_window* aqw);
	ambulant_qt_window* qt_window();
	
	void paintEvent(QPaintEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);

  private:
	ambulant_qt_window* m_qt_window;

#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
  protected:
	void mouseMoveEvent(QMouseEvent* e);
#endif/*QT_NO_FILEDIALOG*/

};  // class qt_ambulant_widget

class qt_window_factory : public common::window_factory {
  public:
	qt_window_factory( QWidget* parent_widget, int x, int y);
		
		common::gui_window* new_window(
			const std::string &name,
			lib::size bounds,
			common::gui_events *region);
		common::bgrenderer *new_background_renderer(
			const common::region_info *src);
  private:
	QWidget* m_parent_widget;
	lib::point m_p;
};  // class qt_window_factory

class qt_renderer_factory : public common::playable_factory {
  public:
	qt_renderer_factory(common::factories *factory);
	
	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp);
		
	common::playable *new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src);
  protected:
  	common::factories *m_factory;

};  // class qt_renderer_factory

//class qt_video_factory : qt_renderer_factory {
class qt_video_factory : public common::playable_factory {
  public:
  
	qt_video_factory(common::factories *factory)
	:   m_factory(factory) {}
	~qt_video_factory();

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
		net::audio_datasource *src);
 private:
        common::factories *m_factory;
			
}; // class qt_video_factory 

} // namespace qt

} // namespace gui

} // namespace ambulant

#endif // qt_FACTORY_H
