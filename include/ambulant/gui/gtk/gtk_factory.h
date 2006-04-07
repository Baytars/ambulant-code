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

#ifndef GTK_FACTORY_H
#define GTK_FACTORY_H

#include "ambulant/common/playable.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/common/gui_player.h"

#include "gtk_includes.h"
#include "gtk_fill.h"

#include <gtk/gtk.h>

namespace ambulant {

namespace gui {

namespace gtk {

/// Debug function that dumps a pixmap to a file. An incrementing
/// count is appended to the filenname, and an extension added.
void dumpPixmap(GdkPixmap *gtkpm, std::string filename);

class gtk_ambulant_widget;

/// GTK implementation of playable_factory
class gtk_renderer_factory : public common::playable_factory {
  public:
	gtk_renderer_factory(common::factories *factory);
	
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

};  // class gtk_renderer_factory

/// GTK implementation of window_factory
class gtk_window_factory : public common::window_factory {
  public:
//	gtk_window_factory( GtkWidget* parent_widget, int x, int y, GMainLoop* loop);
	gtk_window_factory(gtk_ambulant_widget* gtk_widget, GMainLoop* loop);
//	gtk_window_factory( GtkWidget* parent_widget, int x, int y);		
		common::gui_window* new_window(
			const std::string &name,
			lib::size bounds,
			common::gui_events *region);
		common::bgrenderer *new_background_renderer(
			const common::region_info *src);
  private:
//	GtkWidget* m_parent_widget;
	gtk_ambulant_widget* m_parent_widget;
	lib::point m_p;
	GMainLoop* m_main_loop;
};  // class gtk_window_factory


/// GTK implementation of another playable_factory that handles video.
class gtk_video_factory : public common::playable_factory {
  public:
  
	gtk_video_factory(common::factories *factory)
	:   m_factory(factory) {}
	~gtk_video_factory();

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
}; // class gtk_video_factory 

/// ambulant_gtk_window is the GTK implementation of gui_window, it is the
/// class that corresponds to a SMIL topLayout element.

class ambulant_gtk_window : public common::gui_window {
  public:
	ambulant_gtk_window(const std::string &name,
			   lib::rect* bounds,
			   common::gui_events *region);
	~ambulant_gtk_window();
			   
	// gui_window API:
	void need_redraw(const lib::rect &r);
	void redraw(const lib::rect &r);
	void redraw_now();
	
	// gui_events API:
	void mouse_region_changed();
	void user_event(const lib::point &where, int what=0);
	void need_events(bool want);

	// semi-private helpers:
    
	/// Set the corresponding widget.
	void set_ambulant_widget(gtk_ambulant_widget* gtkaw);
	
	/// Get the GTK widget corresponding to this ambulant window.
	gtk_ambulant_widget* get_ambulant_widget();

	// XXX These need to be documented...
	GdkPixmap* get_ambulant_pixmap();
	GdkPixmap* new_ambulant_surface();
	GdkPixmap* get_ambulant_surface();
	GdkPixmap* get_ambulant_oldpixmap();
	GdkPixmap* get_pixmap_from_screen(const lib::rect &r);
	void reset_ambulant_surface(void);
	void set_ambulant_surface(GdkPixmap* surf);
	void delete_ambulant_surface();

#ifdef USE_SMIL21
	void startScreenTransition();
	void endScreenTransition();
	void screenTransitionStep(smil2::transition_engine* engine, lib::transition_info::time_type now);
		
	void _screenTransitionPreRedraw();
	void _screenTransitionPostRedraw(const lib::rect &r);
#endif
	
  private:
	gtk_ambulant_widget* m_ambulant_widget;
	GdkPixmap* m_pixmap;
	GdkPixmap* m_oldpixmap;
	GdkPixmap* m_surface;
#ifdef USE_SMIL21
	int m_fullscreen_count;
	GdkPixmap* m_fullscreen_prev_pixmap;
	GdkPixmap* m_fullscreen_old_pixmap;
	smil2::transition_engine* m_fullscreen_engine;
	lib::transition_info::time_type m_fullscreen_now;
#endif

 public:
	GdkPixmap* m_tmppixmap;
	guint signal_redraw_id;
	GMainLoop* m_main_loop;
};  // class ambulant_gtk_window

/// gtk_ambulant_widget is the GTK-counterpart of ambulant_gtk_window: it is the
/// GTKWidget that corresponds to an Ambulant topLayout window.
class gtk_ambulant_widget : public GtkWidget, public ambulant::common::gui_screen
{
  public:
//	gtk_ambulant_widget(const std::string &name,
//			   lib::rect* bounds,
//			   GtkWidget* parent_widget);
	gtk_ambulant_widget(GtkWidget* widget);
	~gtk_ambulant_widget();
	
	/// Helper: set our counterpart gui_window.
	void set_gtk_window( ambulant_gtk_window* agtkw);

	/// Helper: get our counterpart gui_window.
	ambulant_gtk_window* gtk_window();

	/// Helper: get the actual GTK Widget
	GtkWidget* get_gtk_widget();	

	// GTKWidget API:
	void gtk_ambulant_widget::do_paint_event (GdkEventExpose * event);
	void gtk_ambulant_widget::do_motion_notify_event(GdkEventMotion *event);
	void gtk_ambulant_widget::do_button_release_event(GdkEventButton *event);
//	void mouseReleaseEvent(QMouseEvent* e);

	// gui_screen implementation
	void get_size(int *width, int *height);
	bool get_screenshot(const char *type, char **out_data, size_t *out_size);
	bool set_overlay(const char *type, const char *data, size_t size);
	bool clear_overlay();
	bool set_screenshot(char **screenshot_data, size_t *screenshot_size);

  public:
	// For the gui_screen implementation
	gchar * m_screenshot_data;
	gsize m_screenshot_size;

  private:
	ambulant_gtk_window* m_gtk_window;
	GtkWidget *m_widget;

#ifndef GTK_NO_FILEDIALOG	/* Assume plain GTK */
  protected:
//	void mouseMoveEvent(QMouseEvent* e);
#endif/*GTK_NO_FILEDIALOG*/

};  // class gtk_ambulant_widget

} // namespace gtk

} // namespace gui

} // namespace ambulant

#endif // GTK_FACTORY_H
