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
//#define gtk_gui_BASE KMainWindow
#define gtk_gui_BASE GtkWidget
#else /*WITH_GTK_HTML_WIDGET*/
#define gtk_gui_BASE GtkWidget
#endif/*WITH_GTK_HTML_WIDGET*/

class gtk_gui : public gtk_gui_BASE {
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
//		return m_smilfilename.ascii();
		return m_smilfilename;
	}

	bool openSMILfile(const char *smilfilename, int mode);

	// send a QEvent to the gui thread
	void internal_message(int level, char* msg);

	// signal interfaces
	void need_redraw(const void*, void*, const void*);
	void player_done();
	void player_start(GString,bool,bool);

/*TMP*/	gtk_mainloop* m_mainloop;
   private:
	bool	     m_busy;
//	QPushButton* m_cancel_pb; // for Settings window
	GtkMenuItem*  m_filemenu;
//	QHBox* 	     m_finish_hb; // for Settings window
	GtkMenuItem*  m_helpmenu;
	GtkMenu*    m_menubar;
//	QPushButton* m_ok_pb;	  // for Settings window
	int	     m_o_x;	  // x coord of origin play window
	int	     m_o_y;	  // y coord of origin play window
	int          m_pause_id;
	bool         m_pausing;
	int          m_play_id;
	bool         m_playing;
	GtkMenuItem*  m_playmenu;
	const char*  m_programfilename;
//	qt_settings* m_settings; // the Settings window
	const char*  m_smilfilename;
	GtkMenuItem*  m_viewmenu;

#define	TRY_LOCKING
#ifdef	TRY_LOCKING
	pthread_cond_t	  m_cond_message;
	pthread_mutex_t   m_lock_message;
	unsigned long int m_gui_thread;
#endif/*TRY_LOCKING*/
#ifndef GTK_NO_FILEDIALOG	/* Assume plain Qt */
//	Qt::CursorShape m_cursor_shape;
#else /*GTK_NO_FILEDIALOG*/	/* Assume embedded Qt */
	bool         m_pointinghand_cursor; //XXXX
	FileSelector*m_fileselector;
	FileSelector*m_settings_selector;
	const DocLnk m_selectedDocLnk;
#endif/*QT_NO_FILEDIALOG*/
	void	     fileError(GString smilfilename);

//  public slots:
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
	void slot_file_selected(const DocLnk&);
	void slot_close_fileselector();
	void slot_settings_selected(const DocLnk&);
	void slot_close_settings_selector();
	void slot_play();

//Commented by PAblo
//  private slots:
// Included by Pablo
    private:
	void slot_about();
	void slot_homepage();
	void slot_welcome();
	void slot_help();
	void slot_load_settings();
  	void slot_logger_window();
	void slot_open();
	void slot_open_url();
	void slot_pause();
	void slot_player_done();
	void slot_quit();
	void slot_reload();
	void slot_settings_cancel();
	void slot_settings_ok();
	void slot_settings_select();
	void slot_stop();
// Commented by Pablo
//  signals:
	void signal_player_done();
	void signal_need_redraw(const void*, void*, const void*);
/**
  protected:
	void customEvent(QCustomEvent*);**/
#ifndef GTK_NO_FILEDIALOG	/* Assume plain Qt */
	void unsetCursor(); //XXXX
#endif/*GTK_NO_FILEDIALOG*/
};
#endif/*__GTK_GUI_H__*/
