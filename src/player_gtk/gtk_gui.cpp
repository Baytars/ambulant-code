// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

/* gtk_gui.cpp - GTK GUI for Ambulant
 *              
 */

#include <pthread.h>
#include <libgen.h>
#include <stdlib.h>
#include <fcntl.h>
#include "gtk_gui.h"
#include "gtk_mainloop.h"
#include "gtk_logger.h"
#include "gtk_renderer.h"
//#include <gtk-thread.h>
#if 1
#include "ambulant/config/config.h"
#include "ambulant/lib/logger.h"
#include "ambulant/version.h"
#endif

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define	WITH_GTK_LOGGER

const char *about_text = 
	"Ambulant SMIL 2.1 player.\n"
	"Copyright Stichting CWI, 2003-2005.\n\n"
	"License: LGPL.";


// Places where to look for the Welcome document
const char *welcome_locations[] = {
	"Welcome/Welcome.smil",
	"../Welcome/Welcome.smil",
	"Extras/Welcome/Welcome.smil",
	"../Extras/Welcome/Welcome.smil",
#ifdef AMBULANT_DATADIR
	AMBULANT_DATADIR "/Welcome/Welcome.smil",
#else
	"/usr/local/share/ambulant/Welcome/Welcome.smil",
#endif
#ifdef	GTK_NO_FILEDIALOG	/* Assume embedded Qt */
	"/home/zaurus/Documents/Ambulant/Extras/Welcome/Welcome.smil",
#endif/*QT_NO_FILEDIALOG*/
	NULL
};

// Places where to look for the helpfile
const char *helpfile_locations[] = {
	"Documentation/user/index.html",
	"../Documentation/user/index.html",
#ifdef AMBULANT_DATADIR
	AMBULANT_DATADIR "/AmbulantPlayerHelp/index.html",
#else
	"/usr/local/share/ambulant/AmbulantPlayerHelp/index.html",
#endif
	NULL
};

static const char * 
find_datafile(const char **locations)
{
	const char **p;
	for(p = locations; *p; p++) {
		if (access(*p, 0) >= 0) return *p;
	}
	return NULL;
}

gtk_gui::gtk_gui(const char* title,
	       const char* initfile)
 :
#ifdef	WITH_GTK_HTML_WIDGET
  KMainWindow(0L, title),
#else /*WITH_GTK_HTML_WIDGET*/
	GtkWidget(),  
#endif/*WITH_GTK_HTML_WIDGET*/
        m_busy(true),
#ifndef GTK_NO_FILEDIALOG	/* Assume plain GTK */
	//m_cursor_shape(Qt::ArrowCursor),
#else /*GTK_NO_FILEDIALOG*/	/* Assume embedded GTK */
	//m_cursor_shape(arrowCursor);
	m_fileselector(NULL),
	m_settings_selector(NULL),
#endif/*GTK_NO_FILEDIALOG*/
	m_mainloop(NULL),
	m_o_x(0),	 
	m_o_y(0),	 
	m_pause_id(),
	m_pausing(),
	m_play_id(),
	m_playing(),
//	m_playmenu(),
	m_programfilename(),
#ifdef	TRY_LOCKING
	m_gui_thread(0),
#endif/*TRY_LOCKING*/
	m_smilfilename()
{

	printf("I am in heeree");

	m_programfilename = title;
#ifdef	TRY_LOCKING
	pthread_cond_init(&m_cond_message, NULL);
	pthread_mutex_init(&m_lock_message, NULL);
	m_gui_thread = pthread_self();
#endif/*TRY_LOCKING*/
	if (initfile != NULL && initfile != "")
		m_smilfilename = initfile;
#ifdef  GTK_NO_FILEDIALOG
	else
		m_smilfilename = GString("/home/zaurus/Documents/example.smil");
#endif/*GTK_NO_FILEDIALOG*/
	m_playing = false;
	m_pausing = false;
//	setCaption(initfile);

	/* Menu bar */
	// Not needed here!
	GtkWidget *window;
	window   = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (G_OBJECT (window), "delete-event",
                    G_CALLBACK (gtk_main_quit), NULL);
	m_menubar = (GtkMenu*)gtk_menu_new();
	{
		int id;
		/* File */

		m_filemenu = (GtkMenuItem*)gtk_menu_item_new_with_label("File");
		gtk_menu_append((GtkWidget*)m_menubar,(GtkWidget*)m_filemenu);
		gtk_container_add (GTK_CONTAINER (window), (GtkWidget*)m_menubar);
		gtk_widget_show_all (window);
/**		m_filemenu = new QPopupMenu (this);
		assert(m_filemenu);
		int open_id = m_filemenu->insertItem(gettext("&Open..."), this, SLOT(slot_open()));
		int url_id = m_filemenu->insertItem(gettext("Open &URL..."), this, SLOT(slot_open_url()));
		int reload_id = m_filemenu->insertItem(gettext("&Reload..."), this, SLOT(slot_reload()));
#ifdef QT_NO_FILEDIALOG	**//* Assume embedded Qt */
		// Disable unavailable menu entries
/**		m_filemenu->setItemEnabled(open_id, true);
		m_filemenu->setItemEnabled(url_id, false);
#endif**//*QT_NO_FILEDIALOG*/
/**		m_filemenu->insertSeparator();
		
		m_filemenu->insertItem(gettext("&Settings"), this, SLOT(slot_settings_select()));
		m_filemenu->insertSeparator();
		
		m_filemenu->insertItem(gettext("&Quit"), this, SLOT(slot_quit()));
		m_menubar->insertItem(gettext("&File"), m_filemenu);
**/		
		/* Play */
/**		m_playmenu = new QPopupMenu (this, "PlayA");
		assert(m_playmenu);
		m_play_id = m_playmenu->insertItem(gettext("Pla&y"), this, SLOT(slot_play()));
		m_playmenu->setItemEnabled(m_play_id, false);
		m_pause_id = m_playmenu->insertItem(gettext("&Pause"), this, SLOT(slot_pause()));
		m_playmenu->setItemEnabled(m_pause_id, false);
		m_playmenu->insertItem(gettext("&Stop"), this, SLOT(slot_stop()));
		m_menubar->insertItem(gettext("Pla&y"), m_playmenu);
**/		
		/* View */
/**		m_viewmenu = new QPopupMenu(this, "View");
		m_viewmenu->insertItem(gettext("&Full Screen"), this, SLOT(showFullScreen()));
		m_viewmenu->insertItem(gettext("&Window"), this, SLOT(showNormal()));
		m_viewmenu->insertSeparator();
		m_viewmenu->insertItem(gettext("&Load settings..."), this, SLOT(slot_load_settings()));
#ifdef	WITH_QT_LOGGER
		m_viewmenu->insertSeparator();
		m_viewmenu->insertItem(gettext("&Log Window..."), this, SLOT(slot_logger_window()));
#endif**//*WITH_QT_LOGGER*/
//		m_menubar->insertItem(gettext("&View"), m_viewmenu);
		
		/* Help */
/**		m_helpmenu = new QPopupMenu (this, "HelpA");
		assert(m_helpmenu);
		m_helpmenu->insertItem(gettext("&About AmbulantPlayer"), this, SLOT(slot_about()));
		m_helpmenu->insertItem(gettext("AmbulantPlayer &Help"), this, SLOT(slot_help()));
		m_helpmenu->insertSeparator();
		m_helpmenu->insertItem(gettext("AmbulantPlayer &Website..."), this, SLOT(slot_homepage()));
		m_helpmenu->insertItem(gettext("&Play Welcome Document"), this, SLOT(slot_welcome()));
		m_menubar->insertItem(gettext("&Help"), m_helpmenu);
		m_menubar->setGeometry(0,0,320,20);
		m_o_x = 0;
#ifndef QT_NO_FILEDIALOG**/	/* Assume plain Qt */
//		m_o_y = 27;
//#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
//		m_o_y = 20;
//#endif/*QT_NO_FILEDIALOG*/
	}
//	GTKObject::connect(this, SIGNAL(signal_player_done()),
//			    this, SLOT(slot_player_done()));
}

gtk_gui::~gtk_gui() {
/**
#define DELETE(X) if (X) { delete X; X = NULL; }
	AM_DBG printf("%s0x%X\n", "gtk_gui::~gtk_gui(), m_mainloop=",m_mainloop);
	setCaption(QString::null);
#ifdef  QT_NO_FILEDIALOG**/	/* Assume embedded Qt */
/**	DELETE(m_fileselector)
	DELETE(m_settings_selector)
#endif**//*QT_NO_FILEDIALOG*/
/**	DELETE(m_mainloop) 
	DELETE(m_filemenu)
	DELETE(m_helpmenu)
	DELETE(m_playmenu)
	DELETE(m_viewmenu)
	DELETE(m_menubar)
	m_smilfilename = (char*) NULL;**/
}

void 
gtk_gui::slot_about() {
	//int but = QMessageBox::information(this, gettext("About AmbulantPlayer"),about_text,gettext("OK"));
}

void 
gtk_gui::slot_homepage() {
	open_web_browser("http://www.ambulantplayer.org");
}

void 
gtk_gui::slot_welcome() {
	const char *welcome_doc = "/ufs/garcia/native/share/ambulant/Welcome/Welcome.smil";
	//find_datafile(welcome_locations);
	
	if (welcome_doc) {
		if( openSMILfile(welcome_doc, 1)) {
			slot_play();
		}
	} else {
		//QMessageBox::information(this, m_programfilename, 
		//	gettext("Cannot find Welcome.smil document"));
	}
}

void 
gtk_gui::slot_help() {
	const char *help_doc = find_datafile(helpfile_locations);
	
	if (help_doc) {
		open_web_browser(help_doc);
	} else {
//		QMessageBox::information(this, m_programfilename, 
//			gettext("Cannot find Ambulant Player Help"));
	}
}

void
gtk_gui::slot_logger_window() {
	AM_DBG printf("slot_logger_window()\n");
#ifndef GTK_NO_FILEDIALOG	 /* Assume plain Qt */
//	QTextEdit* logger_window =
//		gtk_logger::get_gtk_logger()->get_logger_window();
/**	if (logger_window->isHidden())
		logger_window->show();
	else
		logger_window->hide();
**/
#endif/*QT_NO_FILEDIALOG*/
}

bool 
checkFilename(GString filename, int mode) {
	//QFile* file = new QFile(filename);
	//return file->open(mode);
}

void
gtk_gui::fileError(GString smilfilename) {
 	char buf[1024];
	sprintf(buf, gettext("%s: Cannot open file: %s"),
		(const char*) smilfilename.str, strerror(errno));
//	QMessageBox::information(this, m_programfilename, buf);
}

bool 
gtk_gui::openSMILfile(const char *smilfilename, int mode) {

	if (smilfilename==NULL)
		return false;
#if 0
	if (! checkFilename(smilfilename, mode)) {
		fileError(smilfilename);
		return false;
	}
#endif
	char* filename = strdup(smilfilename);
//	setCaption(basename(filename));
	free(filename);
//	m_playmenu->setItemEnabled(m_pause_id, false);
//	m_playmenu->setItemEnabled(m_play_id, true);
	m_smilfilename = smilfilename;
	if (m_mainloop != NULL)
		m_mainloop->release();
 	m_mainloop = new gtk_mainloop(this);
	m_playing = false;
	m_pausing = false;
	return m_mainloop->is_open();
}

void 
gtk_gui::slot_open() {
#ifndef GTK_NO_FILEDIALOG
/**
	GString smilfilename =
		QFileDialog::getOpenFileName(
				 ".", // Initial dir
				 gettext("SMIL files (*.smil *.smi);; All files (*.smil *.smi *.mms *.grins);; Any file (*)"), // file types
				 this,
				 gettext("open file dialog"),
				 gettext("Double Click a file to open")
				 );
	if (openSMILfile(smilfilename, 1))
		slot_play();**/
#else	/*QT_NO_FILEDIALOG*/	
	if (m_fileselector == NULL) {
		GString mimeTypes("application/smil;");
		m_fileselector = new FileSelector(mimeTypes, NULL,
						  "slot_open", false);
		m_fileselector->resize(240, 280);
	} else {
		m_fileselector->reread();
	}
/**
	QObject::connect(m_fileselector, SIGNAL(fileSelected(const DocLnk&)),
			 this, SLOT(slot_file_selected(const DocLnk&)));
	QObject::connect(m_fileselector, SIGNAL(closeMe()), 
			 this, SLOT(slot_close_fileselector()));
	m_fileselector->show();
**/
#endif	/*QT_NO_FILEDIALOG*/
}


//void
//gtk_gui::setDocument(const char* smilfilename) {
//#ifdef	GTK_NO_FILEDIALOG	/* Assume embedded Qt */
//	if (openSMILfile(smilfilename, 1))
//		slot_play();
//#endif/*GTK_NO_FILEDIALOG*/
//}

void
gtk_gui::slot_file_selected(const DocLnk& selected_file) {
#ifdef	QT_NO_FILEDIALOG	/* Assume embedded Qt */
	GString* smilfilepointer = new GString(selected_file.file());
	GString smilfilename = *smilfilepointer;
	delete smilfilepointer;
	m_fileselector->hide();
	if (openSMILfile(smilfilename, 1))
		slot_play();
#endif/*QT_NO_FILEDIALOG*/
}

void
gtk_gui::slot_close_fileselector()
{
#ifdef	QT_NO_FILEDIALOG	/* Assume embedded Qt */
	m_fileselector->hide();
#endif/*QT_NO_FILEDIALOG*/
}

void
gtk_gui::slot_settings_selected(const DocLnk& selected_file) {
#ifdef	QT_NO_FILEDIALOG	/* Assume embedded Qt */
	GTKString settings_filename(selected_file.file());
	smil2::test_attrs::load_test_attrs(settings_filename.ascii());
	if (openSMILfile(m_smilfilename, 1))
		slot_play();
#endif/*QT_NO_FILEDIALOG*/
}
void
gtk_gui::slot_close_settings_selector()
{
#ifdef	QT_NO_FILEDIALOG	/* Assume embedded Qt */
	m_settings_selector->hide();
#endif/*QT_NO_FILEDIALOG*/
}

void 
gtk_gui::slot_load_settings() {
	if (m_playing)
		slot_stop();
#ifndef GTK_NO_FILEDIALOG	/* Assume plain Qt */
/**	GString settings_filename =
		QFileDialog::getOpenFileName(
				 ".", // Initial dir
				 gettext("Settings files (*.xml)"), // file types
				 this,
				 gettext("open settings file dialog"),
				 gettext("Double Click a settings file to open")
				 );**/
	/**
	if ( ! settings_filename.isNull()) {
		smil2::test_attrs::load_test_attrs(settings_filename.ascii());
		if (openSMILfile(m_smilfilename, 1))
			slot_play();
	}**/
#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
	/* TBD embedded Qt settings file dialog XXXX */
	printf("1.m_settings_selector =0x%x\n",m_settings_selector );
	if (m_settings_selector == NULL) {
		QString mimeTypes("text/xml;");
		m_settings_selector = new FileSelector(mimeTypes, NULL,
						       "slot_open", false);
		printf("2.m_settings_selector =0x%x\n",m_settings_selector );
		m_settings_selector->resize(240, 280);
	} else {
		m_settings_selector->reread();
	}
/**
	QObject::connect(m_settings_selector, SIGNAL(fileSelected(const DocLnk&)),
			 this, SLOT(slot_settings_selected(const DocLnk&)));
	QObject::connect(m_settings_selector, SIGNAL(closeMe()), 
			 this, SLOT(slot_close_settings_selector()));
	m_settings_selector->show();**/
#endif/*QT_NO_FILEDIALOG*/
}

void 
gtk_gui::slot_open_url() {
#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
  	bool ok;
/**	QString smilfilename =
		QInputDialog::getText(
				      "AmbulantPlayer",
				      gettext("URL to open:"),
				      QLineEdit::Normal,
				      QString::null,
				      &ok,
				      this
				 );**/
/**	if (ok && !smilfilename.isEmpty()
	    && openSMILfile(smilfilename, 1)) {
		slot_play();
	}
**/
#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
//	QMessageBox::information (this, m_programfilename,
//		gettext("Open URL not implemented for Embedded Qt"));
#endif/*QT_NO_FILEDIALOG*/
}

void 
gtk_gui::slot_player_done() {
	AM_DBG printf("%s-%s\n", m_programfilename, "slot_player_done");
	/*
	if (m_mainloop->player_done()) {
		m_playmenu->setItemEnabled(m_pause_id, false);
		m_playmenu->setItemEnabled(m_play_id, true);
		m_playing = false;
	}
	*/
}

void 
gtk_gui::need_redraw (const void* r, void* w, const void* pt) {
/**
	AM_DBG printf("gtk_gui::need_redraw(0x%x)-r=(0x%x)\n",
	(void *)this,r?r:0);
	emit signal_need_redraw(r,w,pt);
**/
}

void 
gtk_gui::player_done() {
/**	AM_DBG printf("%s-%s\n", m_programfilename, "player_done");
	emit signal_player_done();
**/
}

void
no_fileopen_infodisplay(GtkWidget* w, const char* caption) {
/**	QMessageBox::information(w,caption,gettext("No file open: Please first select File->Open"));**/
}

void 
gtk_gui::slot_play() {
	//AM_DBG 
	printf("%s-%s\n", m_programfilename, "slot_play");
	if (m_smilfilename == NULL || m_mainloop == NULL || ! m_mainloop->is_open()) {
		no_fileopen_infodisplay(this, m_programfilename);
		return;
	}
	if (!m_playing) {
		m_playing = true;
//		m_playmenu->setItemEnabled(m_play_id, false);
//		m_playmenu->setItemEnabled(m_pause_id, true);
		m_mainloop->play();
	}
	if (m_pausing) {
		m_pausing = false;
//		m_playmenu->setItemEnabled(m_pause_id, true);
//		m_playmenu->setItemEnabled(m_play_id, false);
		m_mainloop->set_speed(1);
	}
}

void 
gtk_gui::slot_pause() {
	AM_DBG printf("%s-%s\n", m_programfilename, "slot_pause");
	if (! m_pausing) {
		m_pausing = true;
//		m_playmenu->setItemEnabled(m_pause_id, false);
//		m_playmenu->setItemEnabled(m_play_id, true);
		m_mainloop->set_speed(0);
	}
}

void 
gtk_gui::slot_reload() {
	AM_DBG printf("%s-%s\n", m_programfilename, "slot_reload");
	if (openSMILfile(m_smilfilename, 1)) {
		slot_play();
	} else {
		no_fileopen_infodisplay(this, m_programfilename);
	}
}

void 
gtk_gui::slot_stop() {
	AM_DBG printf("%s-%s\n", m_programfilename, "slot_stop");
	if(m_mainloop)
		m_mainloop->stop();
//	m_playmenu->setItemEnabled(m_pause_id, false);
//	m_playmenu->setItemEnabled(m_play_id, true);
	m_playing = false;
}

void 
gtk_gui::slot_settings_select() {
/**
	m_settings = new qt_settings();
	QWidget* settings_widget = m_settings->settings_select();
	m_finish_hb = new QHBox(settings_widget);
	m_ok_pb	= new QPushButton(gettext("OK"), m_finish_hb);
	m_finish_hb->setSpacing(50);
	QPushButton* m_cancel_pb= new QPushButton(gettext("Cancel"), m_finish_hb);
	QObject::connect(m_ok_pb, SIGNAL(released()),
			 this, SLOT(slot_settings_ok()));
	QObject::connect(m_cancel_pb, SIGNAL(released()),
			 this, SLOT(slot_settings_cancel()));
	settings_widget->show();
**/
}

void
gtk_gui::slot_settings_ok() {
//	m_settings->settings_ok();
//	slot_settings_cancel();
}

void
gtk_gui::slot_settings_cancel() {
//	m_settings->settings_finish();
//	delete m_settings;
//	m_settings = NULL;
}

void
gtk_gui::slot_quit() {
/**
	AM_DBG printf("%s-%s\n", m_programfilename, "slot_quit");
	if (m_mainloop)	{
		m_mainloop->stop();
//		m_mainloop->release();
		delete m_mainloop;
		m_mainloop = NULL;
	}
	m_busy = false;
	qApp->quit();
**/
}

#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
void
gtk_gui::unsetCursor() { //XXXX Hack
//	AM_DBG printf("%s-%s\n", m_programfilename, ":unsetCursor");
/**
	Qt::CursorShape cursor_shape = m_mainloop->get_cursor() ?
		Qt::PointingHandCursor : Qt::ArrowCursor;
	if (cursor_shape != m_cursor_shape) {
		m_cursor_shape = cursor_shape;
		setCursor(cursor_shape);
	}
#ifdef	QCURSOR_ON_ZAURUS
	bool pointinghand_cursor = m_mainloop->get_cursor();
	QCursor cursor_shape = pointinghand_cursor ?
		pointingHandCursor : arrowCursor;
	if (m_pointinghand_cursor != pointinghand_cursor) {
		m_pointinghand_cursor = pointinghand_cursor;
		setCursor(cursor_shape);
	}
#endif**//*QCURSOR_ON_ZAURUS*/
//	m_mainloop->set_cursor(0);
}
#endif/*QT_NO_FILEDIALOG*/

//void
//gtk_gui::customEvent(QCustomEvent* e) {
//	char* msg = (char*)e->data();
//	std::string id("gtk_gui::customEvent");
//	std::cerr<<id<<std::endl;
//	std::cerr<<id+" type: "<<e->type()<<" msg:"<<msg<<std::endl;
//	int level = e->type() - gtk_logger::CUSTOM_OFFSET;
//	switch (level) {
//	case gtk_logger::CUSTOM_NEW_DOCUMENT:
//		if (m_mainloop) {
//			bool start = msg[0] == 'S' ? true : false;
//			bool old = msg[2] == 'O' ? true : false;
//			m_mainloop->player_start(&msg[4], start, old);
//		}
//		break;
//	casegtkt_logger::CUSTOM_LOGMESSAGE:
//#ifndef QT_NO_FILEDIALOG	 /* Assume plain Qt */
//		gtk_logger::get_gtk_logger()->get_logger_window()->append(msg);
//#else /*QT_NO_FILEDIALOG*/
/* No logger window on an embedded system, logging there on file */
//#endif/*QT_NO_FILEDIALOG*/
//		break;
//	case ambulant::lib::logger::LEVEL_FATAL:
//		QMessageBox::critical(NULL, "AmbulantPlayer", msg);
//		break;
//	case ambulant::lib::logger::LEVEL_ERROR:
//		QMessageBox::warning(NULL, "AmbulantPlayer", msg);
//		break;
//	case ambulant::lib::logger::LEVEL_WARN:
//	default:
//		QMessageBox::information(NULL, "AmbulantPlayer", msg);
//		break;
//	}
//#ifdef	TRY_LOCKING
//	if (level >= ambulant::lib::logger::LEVEL_WARN) {
//		pthread_mutex_lock(&m_lock_message);
//		pthread_cond_signal(&m_cond_message);
//		pthread_mutex_unlock(&m_lock_message);
//	}
//#endif/*TRY_LOCKING*/
//	free(msg);
//}

void
gtk_gui::internal_message(int level, char* msg) {
/**
	int msg_id = level+gtk_logger::CUSTOM_OFFSET;
  	gtk_message_event* qme = new gtk_message_event(msg_id, msg);
//#ifdef	QT_THREAD_SUPPORT
//	QThread::postEvent(this, qme);
//#else**/ /*QT_THREAD_SUPPORT*/
//	QApplication::postEvent(this, qme);
//#endif/*QT_THREAD_SUPPORT*/
//#ifdef	TRY_LOCKING
	if (level >= ambulant::lib::logger::LEVEL_WARN
	    && pthread_self() != m_gui_thread) {
	  // wait until the message as been OK'd by the user
		pthread_mutex_lock(&m_lock_message);
		pthread_cond_wait(&m_cond_message, &m_lock_message);
		pthread_mutex_unlock(&m_lock_message);

	}
//#endif/*TRY_LOCKING*/
}

int
main (int argc, char*argv[]) {

//#undef	ENABLE_NLS
#ifdef	ENABLE_NLS
	// Load localisation database
	printf("Started");
	bool private_locale = false;
	char *home = getenv("HOME");
	if (home) {
		std::string localedir = std::string(home) + "/.ambulant/locale";
		if (access(localedir.c_str(), 0) >= 0) {
			private_locale = true;
			bindtextdomain(PACKAGE, localedir.c_str());
		}
	}
	printf("Started");
	if (!private_locale)
		bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);
	printf("Started 2");
#endif /*ENABLE_NLS*/

	// Load preferences, initialize app and logger
	unix_preferences unix_prefs;
	unix_prefs.load_preferences();
	FILE* DBG = stdout;
#ifdef	WITH_GTK_HTML_WIDGET
//	KApplication myapp( argc, argv, "AmbulantPlayer" );
#else /*WITH_QT_HTML_WIDGET*/
#ifndef GTK_NO_FILEDIALOG	/* Assume plain Qt */
//	KApplication myapp(argc, argv);
#else /*GTK_NO_FILEDIALOG*/	/* Assume embedded Qt */
//	KApplication myapp(argc, argv);
#endif/*GTK_NO_FILEDIALOG*/
#endif/*WITH_GTK_HTML_WIDGET*/

	/* Setup widget */
	gtk_init(&argc,&argv);
	gtk_gui* mywidget = new gtk_gui(argv[0], argc > 1 ? argv[1] 
				      : "AmbulantPlayer");
	printf("Started 3");
#ifndef QT_NO_FILEDIALOG     /* Assume plain GTK */
//	mywidget->setGeometry(240, 320, 320, 240);
	//QCursor qcursor(Qt::ArrowCursor);
	//mywidget->setCursor(qcursor);
	//myapp.setMainWidget(mywidget);
#else /*QT_NO_FILEDIALOG*/   /* Assume embedded Qt */
	//if (argc > 1 && strcmp(argv[1], "-qcop") != 0)
	 // myapp.showMainWidget(mywidget);
	//else
	  //myapp.showMainDocumentWidget(mywidget);
#endif/*QT_NO_FILEDIALOG*/
	//mywidget->show();
/*TMP initialize logger after gui*/	
	// take log level from preferences
/**
	gtk_logger::set_gtk_logger_gui(mywidget);
	gtk_logger* gtk_logger = gtk_logger::get_gtk_logger();
	lib::logger::get_logger()->debug("Ambulant Player: now logging to a window");
	// Print welcome banner
	lib::logger::get_logger()->debug(gettext("Ambulant Player: compile time version %s, runtime version %s"), AMBULANT_VERSION, ambulant::get_version());
	lib::logger::get_logger()->debug(gettext("Ambulant Player: built on %s for Unix/Qt"), __DATE__);
#if ENABLE_NLS
	lib::logger::get_logger()->debug(gettext("Ambulant Player: localization enabled (english)"));
#endif
**/
	//AM_DBG 
	fprintf(DBG, "argc=%d argv[0]=%s\n", argc, argv[0]);
	//AM_DBG 
	for (int i=1;i<argc;i++){fprintf(DBG,"%s\n", argv[i]);
	}

	bool exec_flag = false;

	if (argc > 1) {
		char last[6];
		char* str = argv[argc-1];
		int len = strlen(str);
		strcpy(last, &str[len-5]);
		//AM_DBG 
		fprintf(DBG, "%s %s %x\n", str, last);
		if (strcmp(last, ".smil") == 0
		|| strcmp(&last[1], ".smi") == 0
	  	|| strcmp(&last[1], ".sml") == 0) {
 			if (mywidget->openSMILfile(argv[argc-1],
						   1)
			    && (exec_flag = true)){
				printf("!!!!OKKKK!!!\n");
			// So we don't start yet!!	mywidget->slot_play();
				printf("!!!!OKKKK!!! 90 90\n");

			}
		}
	} else {
		preferences* prefs = preferences::get_preferences();
		if ( ! prefs->m_welcome_seen) {
			const char *welcome_doc = find_datafile(welcome_locations);
			if (welcome_doc
			&& mywidget->openSMILfile(welcome_doc,
						  1)) {
				mywidget->slot_play();
				prefs->m_welcome_seen = true;
			}
		}
		exec_flag = true;
	}
	
/**	
	if (exec_flag)
		myapp.exec();
	else if (argc > 1) {
		std::string error_message = gettext("Cannot open: ");
		error_message = error_message + "\"" + argv[1] + "\"";
		std::cerr << error_message << std::endl;
		//myapp.exec();
	}
**/	
#ifndef	WITH_GTK_HTML_WIDGET
	delete mywidget;
#endif/*WITH_GTK_HTML_WIDGET*/
	unix_prefs.save_preferences();
//	delete gtk_logger::get_gtk_logger();
	std::cout << "Exiting program" << std::endl;
	return exec_flag ? 0 : -1;
}
