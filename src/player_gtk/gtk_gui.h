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

/* 
 * @$Id$ 
 */

#ifndef __GTK_GUI_H__
#define __GTK_GUI_H__

#include "unix_preferences.h"
#include <iostream>


//#include <qfeatures.h>
//#ifndef QT_NO_FILEDIALOG **/	 /* Assume plain Qt */
//# include <qapplication.h>
//#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
//#include <qpe/qpeapplication.h>
//#include <qpe/applnk.h>
//#include <fileselector.h>
//#endif/*QT_NO_FILEDIALOG*/
/**
#include <qcursor.h>
#include <qdial.h>
#include <qevent.h>
#include <qhbox.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qinputdialog.h>
#include <qiodevice.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qtextstream.h>
#include <qtooltip.h>
#include <qwidget.h>

#ifdef	WITH_QT_HTML_WIDGET
#include <kapp.h>
#include <kmainwindow.h>**/
//#endif/*WITH_QT_HTML_WIDGET*/

#include "gtk_logger.h"
//#include "gtk_settings.h"

class gtk_mainloop;

#ifdef	WITH_GTK_HTML_WIDGET
#define gtk_gui_BASE GtkWidget
#define gtk_gui_BASE GtkWidget
#else /*WITH_GTK_HTML_WIDGET*/
#define gtk_gui_BASE GtkWidget
#endif/*WITH_GTK_HTML_WIDGET*/

class gtk_gui: public GtkWidget{
//gtk_gui_BASE {
//   GTK_OBJECT

   public:
  	gtk_gui(const char* title, const char* initfile);
	~gtk_gui();
	bool is_busy() { return m_busy; }

	int  get_o_x() {
		return m_o_x;
	}

 	int  get_o_y() {
		return m_o_y;
	}

	const char* filename() { 
		return m_smilfilename;
	}

	bool openSMILfile(const char *smilfilename, int mode);

	// send an event to the gui thread
	void internal_message(int level, char* msg);

	// signal interfaces
	void need_redraw(const void*, void*, const void*);
	void player_done();
	void player_start(GString,bool,bool);
	
	// major containers
	GtkWidget* get_gui_container();
	GtkWidget* get_document_container();
	GtkWindow* get_toplevel_container();

/*TMP*/	gtk_mainloop* m_mainloop;
   private:
	bool	     m_busy;
//	QPushButton* m_cancel_pb; // for Settings window
	GtkMenuItem*  m_filemenu;
//	QHBox* 	     m_finish_hb; // for Settings window
	GtkMenuItem*  m_helpmenu;
	GtkMenuBar*    m_menubar;
//	QPushButton* m_ok_pb;	  // for Settings window
	int	     m_o_x;	  // x coord of origin play window
	int	     m_o_y;	  // y coord of origin play window
	bool         m_pausing;
	bool         m_playing;
	GtkMenuItem*  m_playmenu;
	GtkWidget*   m_playmenu_submenu_play;
	GtkWidget*   m_playmenu_submenu_pause; 
	const char*  m_programfilename;
	GtkSettings* m_settings; // the Settings window
	const char*  m_smilfilename;
	GtkMenuItem* m_viewmenu;
	GtkWindow*   m_toplevelcontainer;
	GtkWidget*   m_guicontainer;
	GtkWidget*   m_documentcontainer;

#define	TRY_LOCKING
#ifdef	TRY_LOCKING
	pthread_cond_t	  m_cond_message;
	pthread_mutex_t   m_lock_message;
	unsigned long int m_gui_thread;
#endif/*TRY_LOCKING*/
//#ifndef GTK_NO_FILEDIALOG	/* Assume plain Qt */
//	Qt::CursorShape m_cursor_shape;
//#else /*GTK_NO_FILEDIALOG*/	/* Assume embedded Qt */
	bool         m_pointinghand_cursor; //XXXX
	GtkFileSelection* m_fileselector;
	GtkFileSelection* m_settings_selector;
	//const DocLnk m_selectedDocLnk;
//#endif/*QT_NO_FILEDIALOG*/
	void	     fileError(GString smilfilename);

//public slots:
//	void setDocument(const GString&);
  /* following slots are needed for Qt Embedded, and are implemented
     as empty functions for normal Qt because Qt's moc doesn't recogzize
     #ifdef and #define
  */
#ifndef GTK_NO_FILEDIALOG	/* Assume plain Qt */
#define DocLnk void*
#endif/*QT_NO_FILEDIALOG*/
// Added by Pablo
    public:
	void do_file_selected();
	void do_settings_selected(const DocLnk&);
	void do_close_settings_selector();
	void do_play();
    public:
	void do_about();
	void do_homepage();
	void do_welcome();
	void do_help();
	void do_load_settings();
  	void do_logger_window();
	void do_open();
	void do_open_url();
	void do_pause();
	void do_player_done();
	void do_quit();
	void do_reload();
	void do_settings_cancel();
	void do_settings_ok();
	void do_settings_select();
	void do_stop();

	guint signal_player_done_id;

// Commented by Pablo
//  signals:
//	void signal_player_done();
//	void signal_need_redraw(const void*, void*, const void*);
  protected:
	//void customEvent(QCustomEvent*);
//#ifndef GTK_NO_FILEDIALOG	/* Assume plain Qt */
	void unsetCursor(); //XXXX
//#endif/*GTK_NO_FILEDIALOG*/
};
#endif/*__GTK_GUI_H__*/
