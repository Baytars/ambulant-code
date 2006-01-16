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

// callbacks for C++
/* File */
extern "C" {
void gtk_C_callback_open(void *userdata)
{
	((gtk_gui*) userdata)->do_open();
}
}
extern "C" {
void gtk_C_callback_open_url(void *userdata)
{
	((gtk_gui*) userdata)->do_open_url();
}
void gtk_C_callback_reload(void *userdata)
{
	((gtk_gui*) userdata)->do_reload();
}
void gtk_C_callback_settings_select(void *userdata)
{
	((gtk_gui*) userdata)->do_settings_select();
}
void gtk_C_callback_quit(void *userdata)
{
	((gtk_gui*) userdata)->do_quit();
}
/* Play */
void gtk_C_callback_play(void *userdata)
{
	((gtk_gui*) userdata)->do_play();
}
void gtk_C_callback_pause(void *userdata)
{
	((gtk_gui*) userdata)->do_pause();
}
void gtk_C_callback_stop(void *userdata)
{
	((gtk_gui*) userdata)->do_stop();
}
/* View */
void gtk_C_callback_full_screen(void *userdata)
{
	//((gtk_gui*) userdata)->showFullScreen();
}
void gtk_C_callback_normal_screen(void *userdata)
{
	//((gtk_gui*) userdata)->showNormal();
}
void gtk_C_callback_load_settings(void *userdata)
{
	((gtk_gui*) userdata)->do_load_settings();
}
void gtk_C_callback_logger_window(void *userdata)
{
	((gtk_gui*) userdata)->do_logger_window();
}
/* Help */
void gtk_C_callback_about(void *userdata)
{
	((gtk_gui*) userdata)->do_about();
}
void gtk_C_callback_help(void *userdata)
{
	((gtk_gui*) userdata)->do_help();
}
void gtk_C_callback_homepage(void *userdata)
{
	((gtk_gui*) userdata)->do_homepage();
}
void gtk_C_callback_welcome(void *userdata)
{
	((gtk_gui*) userdata)->do_welcome();
}
}

/* Internal */
extern "C" {
void gtk_C_callback_file_selected(void *userdata, gpointer data)
{
	((gtk_gui*) userdata)->do_file_selected((gpointer) data);
}
}
extern "C" {
void gtk_C_callback_do_player_done(void *userdata)
{
	((gtk_gui*) userdata)->do_player_done();
}
}

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

//  KMainWindow(0L, title),
#else /*WITH_GTK_HTML_WIDGET*/
//  QWidget(),
#endif/*WITH_GTK_HTML_WIDGET*/
        m_busy(true),
#ifndef GTK_NO_FILEDIALOG	/* Assume plain GTK */
	//m_cursor_shape(Qt::ArrowCursor),
#else /*GTK_NO_FILEDIALOG*/	/* Assume embedded GTK */
	//m_cursor_shape(arrowCursor);
	m_fileselector(NULL),
	m_settings_selector(NULL),
#endif/*GTK_NO_FILEDIALOG*/
	m_o_x(0),	 
	m_o_y(0),	 
	m_pausing(),
	m_playing(),
	m_programfilename(),
#ifdef	TRY_LOCKING
	m_gui_thread(0),
#endif/*TRY_LOCKING*/
	m_smilfilename(NULL),
	m_mainloop(NULL),
	m_toplevelcontainer()
{

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
		m_smilfilename = "/home/zaurus/Documents/example.smil";
#endif/*GTK_NO_FILEDIALOG*/

	m_playing = false;
	m_pausing = false;

	/*Initialization of the GUI */
	m_toplevelcontainer = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_title(m_toplevelcontainer, initfile);
	gtk_signal_connect (GTK_OBJECT (m_toplevelcontainer), "delete-event",G_CALLBACK (gtk_main_quit), NULL);
	gtk_widget_set_size_request(GTK_WIDGET (m_toplevelcontainer), 320, 240);
	gtk_widget_set_uposition(GTK_WIDGET (m_toplevelcontainer), 240, 320);	

	/* Initialization of the signals */
//	signal_player_done_id = g_signal_new ("signal_player_done", G_TYPE_FROM_CLASS(this), G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, GTK_TYPE_NONE, 0, 0);

	/* VBox (m_guicontainer) to place the Menu bar in the correct place */ 
	m_guicontainer = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(m_guicontainer);
	gtk_container_add(GTK_CONTAINER(m_toplevelcontainer), GTK_WIDGET (m_guicontainer));	
	
	/* Menu bar */	
	m_menubar = (GtkMenuBar*)gtk_menu_bar_new();
	gtk_widget_show ((GtkWidget*)m_menubar);
	gtk_box_pack_start(GTK_BOX (m_guicontainer), GTK_WIDGET (m_menubar), FALSE, FALSE, 0);
	
	/* File */
	m_filemenu = (GtkMenuItem*)gtk_menu_item_new_with_mnemonic("_File");
	gtk_widget_show ((GtkWidget*)m_filemenu);
	gtk_container_add(GTK_CONTAINER (m_menubar), GTK_WIDGET (m_filemenu));
	GtkWidget* m_filemenu_submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM (m_filemenu), m_filemenu_submenu);
		
	GtkWidget* m_filemenu_submenu_open = gtk_menu_item_new_with_label("Open...");
	gtk_widget_show ((GtkWidget*)m_filemenu_submenu_open);
	gtk_container_add(GTK_CONTAINER (m_filemenu_submenu), GTK_WIDGET (m_filemenu_submenu_open));
	gtk_signal_connect (GTK_OBJECT (m_filemenu_submenu_open), "activate",  G_CALLBACK (gtk_C_callback_open), (void*) this );	

	GtkWidget* m_filemenu_submenu_openurl = gtk_menu_item_new_with_label("Open URL...");
	gtk_widget_show ((GtkWidget*)m_filemenu_submenu_openurl);
	gtk_container_add(GTK_CONTAINER (m_filemenu_submenu), GTK_WIDGET (m_filemenu_submenu_openurl));
	gtk_signal_connect (GTK_OBJECT (m_filemenu_submenu_openurl), "activate",  G_CALLBACK (gtk_C_callback_open_url), (void*)this);		

	GtkWidget* m_filemenu_submenu_reload = gtk_menu_item_new_with_label("Reload...");
	gtk_widget_show ((GtkWidget*)m_filemenu_submenu_reload);
	gtk_container_add(GTK_CONTAINER (m_filemenu_submenu), GTK_WIDGET (m_filemenu_submenu_reload));
	gtk_signal_connect (GTK_OBJECT (m_filemenu_submenu_reload), "activate",  G_CALLBACK (gtk_C_callback_reload), (void*)this);		

	GtkWidget*  m_filemenu_submenu_separator1 = gtk_separator_menu_item_new();
	gtk_widget_show ((GtkWidget*)m_filemenu_submenu_separator1);
	gtk_container_add(GTK_CONTAINER (m_filemenu_submenu), GTK_WIDGET (m_filemenu_submenu_separator1));

	GtkWidget* m_filemenu_submenu_settings = gtk_menu_item_new_with_label("Settings");
	gtk_widget_show ((GtkWidget*)m_filemenu_submenu_settings);
	gtk_container_add(GTK_CONTAINER (m_filemenu_submenu), GTK_WIDGET (m_filemenu_submenu_settings));
	gtk_signal_connect (GTK_OBJECT (m_filemenu_submenu_settings), "activate",  G_CALLBACK (gtk_C_callback_settings_select), (void*)this);		

	GtkWidget*  m_filemenu_submenu_separator2 = gtk_separator_menu_item_new();
	gtk_widget_show ((GtkWidget*)m_filemenu_submenu_separator2);
	gtk_container_add(GTK_CONTAINER (m_filemenu_submenu), GTK_WIDGET (m_filemenu_submenu_separator2));


	GtkWidget* m_filemenu_submenu_quit = gtk_menu_item_new_with_label("Quit");
	gtk_widget_show ((GtkWidget*)m_filemenu_submenu_quit);
	gtk_container_add(GTK_CONTAINER (m_filemenu_submenu), GTK_WIDGET (m_filemenu_submenu_quit));
	gtk_signal_connect (GTK_OBJECT (m_filemenu_submenu_quit), "activate",  G_CALLBACK (gtk_C_callback_quit), (void*)this);

	/* Play */
	m_playmenu = (GtkMenuItem*)gtk_menu_item_new_with_mnemonic("_Play");
	gtk_widget_show ((GtkWidget*)m_playmenu);
	gtk_container_add(GTK_CONTAINER (m_menubar), GTK_WIDGET (m_playmenu));
	GtkWidget* m_playmenu_submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM (m_playmenu), m_playmenu_submenu);
		
	m_playmenu_submenu_play = gtk_menu_item_new_with_label("Play");
	gtk_widget_show ((GtkWidget*)m_playmenu_submenu_play);
	gtk_container_add(GTK_CONTAINER (m_playmenu_submenu), GTK_WIDGET (m_playmenu_submenu_play));
	gtk_signal_connect (GTK_OBJECT (m_playmenu_submenu_play), "activate",  G_CALLBACK (gtk_C_callback_play), (void*)this);
	
	m_playmenu_submenu_pause = gtk_menu_item_new_with_label("Pause");
	gtk_widget_show ((GtkWidget*)m_playmenu_submenu_pause);
	gtk_container_add(GTK_CONTAINER (m_playmenu_submenu), GTK_WIDGET (m_playmenu_submenu_pause));
	gtk_signal_connect (GTK_OBJECT (m_playmenu_submenu_pause), "activate",  G_CALLBACK (gtk_C_callback_pause), (void*)this);
		
	GtkWidget* m_playmenu_submenu_stop = gtk_menu_item_new_with_label("Stop");
	gtk_widget_show ((GtkWidget*)m_playmenu_submenu_stop);
	gtk_container_add(GTK_CONTAINER (m_playmenu_submenu), GTK_WIDGET (m_playmenu_submenu_stop));
	gtk_signal_connect (GTK_OBJECT (m_playmenu_submenu_stop), "activate",  G_CALLBACK (gtk_C_callback_stop), (void*)this);

	/* View */
	m_viewmenu = (GtkMenuItem*)gtk_menu_item_new_with_mnemonic("_View");
	gtk_widget_show ((GtkWidget*)m_viewmenu);
	gtk_container_add(GTK_CONTAINER (m_menubar), GTK_WIDGET (m_viewmenu));
	GtkWidget* m_viewmenu_submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM (m_viewmenu), m_viewmenu_submenu);
		
	GtkWidget* m_viewmenu_submenu_fullscreen = gtk_menu_item_new_with_label("Full Screen");
	gtk_widget_show ((GtkWidget*)m_viewmenu_submenu_fullscreen);
	gtk_container_add(GTK_CONTAINER (m_viewmenu_submenu), GTK_WIDGET (m_viewmenu_submenu_fullscreen));
	gtk_signal_connect (GTK_OBJECT (m_viewmenu_submenu_fullscreen), "activate",  G_CALLBACK (gtk_C_callback_full_screen), (void*)this);

	GtkWidget* m_viewmenu_submenu_window = gtk_menu_item_new_with_label("Window");
	gtk_widget_show ((GtkWidget*)m_viewmenu_submenu_window);
	gtk_container_add(GTK_CONTAINER (m_viewmenu_submenu), GTK_WIDGET (m_viewmenu_submenu_window));
	gtk_signal_connect (GTK_OBJECT (m_viewmenu_submenu_window), "activate",  G_CALLBACK (gtk_C_callback_normal_screen), (void*)this);
	
	GtkWidget*  m_viewmenu_submenu_separator1 = gtk_separator_menu_item_new();
	gtk_widget_show ((GtkWidget*)m_viewmenu_submenu_separator1);
	gtk_container_add(GTK_CONTAINER (m_viewmenu_submenu), GTK_WIDGET (m_viewmenu_submenu_separator1));
		
	GtkWidget* m_viewmenu_submenu_settings = gtk_menu_item_new_with_label("Load Settings...");
	gtk_widget_show ((GtkWidget*)m_viewmenu_submenu_settings);
	gtk_container_add(GTK_CONTAINER (m_viewmenu_submenu), GTK_WIDGET (m_viewmenu_submenu_settings));
	gtk_signal_connect (GTK_OBJECT (m_viewmenu_submenu_settings), "activate",  G_CALLBACK (gtk_C_callback_load_settings), (void*)this);

#ifdef	WITH_GTK_LOGGER
	GtkWidget*  m_viewmenu_submenu_separator2 = gtk_separator_menu_item_new();
	gtk_widget_show ((GtkWidget*)m_viewmenu_submenu_separator2);
	gtk_container_add(GTK_CONTAINER (m_viewmenu_submenu), GTK_WIDGET (m_viewmenu_submenu_separator2));

	GtkWidget* m_viewmenu_submenu_log = gtk_menu_item_new_with_label("Log Window...");
	gtk_widget_show ((GtkWidget*)m_viewmenu_submenu_log);
	gtk_container_add(GTK_CONTAINER (m_viewmenu_submenu), GTK_WIDGET (m_viewmenu_submenu_log));
	gtk_signal_connect (GTK_OBJECT (m_viewmenu_submenu_log), "activate",  G_CALLBACK (gtk_C_callback_logger_window), (void*)this);
#endif /*WITH_GTK_LOGGER*/

	/* Help */
	m_helpmenu = (GtkMenuItem*)gtk_menu_item_new_with_mnemonic("_Help");
		gtk_widget_show ((GtkWidget*)m_helpmenu);
		gtk_container_add(GTK_CONTAINER (m_menubar), GTK_WIDGET (m_helpmenu));
	GtkWidget* m_helpmenu_submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM (m_helpmenu), m_helpmenu_submenu);

	GtkWidget* m_helpmenu_submenu_about = gtk_menu_item_new_with_label("About AmbulantPlayer");
	gtk_widget_show ((GtkWidget*)m_helpmenu_submenu_about);
	gtk_container_add(GTK_CONTAINER (m_helpmenu_submenu), GTK_WIDGET (m_helpmenu_submenu_about));
	gtk_signal_connect (GTK_OBJECT (m_helpmenu_submenu_about), "activate",  G_CALLBACK (gtk_C_callback_about), (void*)this);
	
	GtkWidget* m_helpmenu_submenu_help = gtk_menu_item_new_with_label("AmbulantPlayer Help");
	gtk_widget_show ((GtkWidget*)m_helpmenu_submenu_help);
	gtk_container_add(GTK_CONTAINER (m_helpmenu_submenu), GTK_WIDGET (m_helpmenu_submenu_help));
	gtk_signal_connect (GTK_OBJECT (m_helpmenu_submenu_help), "activate",  G_CALLBACK (gtk_C_callback_help), (void*)this);	

	GtkWidget*  m_helpmenu_submenu_separator1 = gtk_separator_menu_item_new();
	gtk_widget_show ((GtkWidget*)m_helpmenu_submenu_separator1);
	gtk_container_add(GTK_CONTAINER (m_helpmenu_submenu), GTK_WIDGET (m_helpmenu_submenu_separator1));
		
	GtkWidget* m_helpmenu_submenu_website = gtk_menu_item_new_with_label("AmbulantPlayer Website...");
	gtk_widget_show ((GtkWidget*)m_helpmenu_submenu_website);
	gtk_container_add(GTK_CONTAINER (m_helpmenu_submenu), GTK_WIDGET (m_helpmenu_submenu_website));
	gtk_signal_connect (GTK_OBJECT (m_helpmenu_submenu_website), "activate",  G_CALLBACK (gtk_C_callback_homepage), (void*)this);

	GtkWidget* m_helpmenu_submenu_welcome = gtk_menu_item_new_with_label("Play Welcome Document");
	gtk_widget_show ((GtkWidget*)m_helpmenu_submenu_welcome);
	gtk_container_add(GTK_CONTAINER (m_helpmenu_submenu), GTK_WIDGET (m_helpmenu_submenu_welcome));
	gtk_signal_connect (GTK_OBJECT (m_helpmenu_submenu_welcome), "activate",  G_CALLBACK (gtk_C_callback_welcome), (void*)this);
	
//	gtk_widget_show(GTK_WIDGET (m_toplevelcontainer));

	m_o_x = 0;
#ifndef GTK_NO_FILEDIALOG	/* Assume plain GTK */
	m_o_y = 27;
#else /*GTK_NO_FILEDIALOG*/	/* Assume embedded GTK */
	m_o_y = 20;
#endif/*QT_NO_FILEDIALOG*/

	//g_signal_connect (G_OBJECT (this), "signal_player_done",  G_CALLBACK (gtk_C_callback_do_player_done), (void*)this);	

	//	g_signal_emit(GTK_OBJECT (this), signal_player_done_id, 0);
}

gtk_gui::~gtk_gui() {

#define DELETE(X) if (X) { delete X; X = NULL; }
	AM_DBG printf("%s0x%X\n", "gtk_gui::~gtk_gui(), m_mainloop=",m_mainloop);
	//setCaption(QString::null);
	gtk_widget_destroy(GTK_WIDGET (m_fileselector));
		gtk_widget_destroy(GTK_WIDGET (m_settings_selector));
	gtk_widget_destroy(GTK_WIDGET (m_filemenu));
	gtk_widget_destroy(GTK_WIDGET (m_playmenu));
	gtk_widget_destroy(GTK_WIDGET (m_viewmenu));
	gtk_widget_destroy(GTK_WIDGET (m_menubar));
	//DELETE(m_mainloop)
	m_mainloop->release();
	m_smilfilename = (char*) NULL;
}

GtkWindow*
gtk_gui::get_toplevel_container()
{
	return this->m_toplevelcontainer;
}

GtkWidget*
gtk_gui::get_gui_container()
{
	return this->m_guicontainer;
}

GtkWidget*
gtk_gui::get_document_container()
{
	return this->m_documentcontainer;
}


void 
gtk_gui::do_about() {
	GtkMessageDialog* dialog = 
	(GtkMessageDialog*) gtk_message_dialog_new (NULL,
         GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_INFO,
         GTK_BUTTONS_OK,
	 "About AmbulantPlayer");
	gtk_message_dialog_format_secondary_text(dialog,about_text);
 	gtk_dialog_run (GTK_DIALOG (dialog));
 	gtk_widget_destroy (GTK_WIDGET (dialog));
}

void 
gtk_gui::do_homepage() {
	open_web_browser("http://www.ambulantplayer.org");
}

void 
gtk_gui::do_welcome() {
	const char *welcome_doc = find_datafile(welcome_locations);
	
	if (welcome_doc) {
		if( openSMILfile(welcome_doc, 1)) {
			do_play();
		}
	} else {
		GtkMessageDialog* dialog = 
		(GtkMessageDialog*) gtk_message_dialog_new (NULL,
         	GTK_DIALOG_DESTROY_WITH_PARENT,
         	GTK_MESSAGE_ERROR,
         	GTK_BUTTONS_OK,
	 	"Cannot find Welcome.smil document");
 		gtk_dialog_run (GTK_DIALOG (dialog));
 		gtk_widget_destroy (GTK_WIDGET (dialog));
	}
}

void 
gtk_gui::do_help() {
	const char *help_doc = find_datafile(helpfile_locations);
	
	if (help_doc) {
		open_web_browser(help_doc);
	} else {
		GtkMessageDialog* dialog = 
		(GtkMessageDialog*) gtk_message_dialog_new (NULL,
         	GTK_DIALOG_DESTROY_WITH_PARENT,
         	GTK_MESSAGE_ERROR,
         	GTK_BUTTONS_OK,
	 	"Cannot find Ambulant Player Help");
 		gtk_dialog_run (GTK_DIALOG (dialog));
 		gtk_widget_destroy (GTK_WIDGET (dialog));
	}
}

void
gtk_gui::do_logger_window() {
	AM_DBG printf("do_logger_window()\n");
	GtkWindow* logger_window =  gtk_logger::get_gtk_logger()->get_logger_window();
	if (GTK_WIDGET_VISIBLE (GTK_WIDGET (logger_window)))
		gtk_widget_hide(GTK_WIDGET (logger_window));
	else
		gtk_widget_show(GTK_WIDGET (logger_window));
}

bool 
checkFilename(GString filename, int mode) {
	//FILE* file = new GFile(filename);
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
	/*
	if (mode == 0)
		if (! checkFilename(smilfilename, mode)) {
			fileError(smilfilename);
			return false;
		}
*/
	char* filename = strdup(smilfilename);
	//GtkWindow *tmp = m_toplevelcontainer;
	m_smilfilename = smilfilename;

	if (m_mainloop != NULL){
		m_mainloop->release();
 	}

	m_mainloop = new gtk_mainloop(this);
	m_playing = false;
	m_pausing = false;

	return m_mainloop->is_open();
}

void 
gtk_gui::do_open(){
	m_fileselector =  (GtkFileSelection*) gtk_file_selection_new("Please, select a file");
	gtk_file_selection_set_filename(m_fileselector, "."); // Initial dir
	gtk_widget_show(GTK_WIDGET (m_fileselector));
	gtk_file_selection_complete(m_fileselector,"SMIL files (*.smil *.smi);; All files (*.smil *.smi *.mms *.grins);; Any file (*)");

	/* Ensure that the dialog box is hidden when the user clicks a button. */
	g_signal_connect_swapped (GTK_FILE_SELECTION (m_fileselector)->ok_button,
                             "clicked",
                             G_CALLBACK (gtk_widget_hide), 
                             m_fileselector);

   	g_signal_connect_swapped (GTK_FILE_SELECTION (m_fileselector)->cancel_button,
                             "clicked",
                             G_CALLBACK (gtk_widget_hide),
                             m_fileselector);
	g_signal_connect(GTK_FILE_SELECTION (m_fileselector)->ok_button,
                     "clicked",
                     G_CALLBACK (gtk_C_callback_file_selected),
                     (gpointer) m_fileselector);
}

//void
//gtk_gui::setDocument(const char* smilfilename) {
//#ifdef	GTK_NO_FILEDIALOG	/* Assume embedded Qt */
//	if (openSMILfile(smilfilename, 1))
//		do_play();
//#endif/*GTK_NO_FILEDIALOG*/
//}

void
gtk_gui::do_file_selected(gpointer fileselector) {
	//GtkWidget *file_selector = GTK_WIDGET (fileselector);
   	//const gchar *smilfilename  = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
	const gchar *smilfilename  = gtk_file_selection_get_filename (GTK_FILE_SELECTION (m_fileselector));
	printf(smilfilename);
	printf("ESte es el fichero/n");
	//gtk_window_set_title(GTK_WINDOW (m_toplevelcontainer), smilfilename);
	//gtk_widget_set_sensitive (GTK_WIDGET (m_playmenu_submenu_pause), false);
	//gtk_widget_set_sensitive (GTK_WIDGET (m_playmenu_submenu_play), true);

	if (openSMILfile(smilfilename, 1)){		
//		do_play();
	}
}

void
gtk_gui::do_settings_selected(const DocLnk& selected_file) {
#ifdef	QT_NO_FILEDIALOG	/* Assume embedded Qt */
	GTKString settings_filename(selected_file.file());
	smil2::test_attrs::load_test_attrs(settings_filename.ascii());
	if (openSMILfile(m_smilfilename, 1))
		do_play();
#endif/*QT_NO_FILEDIALOG*/
}
void
gtk_gui::do_close_settings_selector()
{
#ifdef	QT_NO_FILEDIALOG	/* Assume embedded Qt */
	m_settings_selector->hide();
#endif/*QT_NO_FILEDIALOG*/
}

void 
gtk_gui::do_load_settings() {
	if (m_playing)
		do_stop();
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
			do_play();
	}**/
#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
	/* TBD embedded Qt settings file dialog XXXX */
	printf("1.m_settings_selector =0x%x\n",m_settings_selector );
	if (m_settings_selector == NULL) {
		QString mimeTypes("text/xml;");
		m_settings_selector = new FileSelector(mimeTypes, NULL,
						       "do_open", false);
		printf("2.m_settings_selector =0x%x\n",m_settings_selector );
		m_settings_selector->resize(240, 280);
	} else {
		m_settings_selector->reread();
	}
/**
	QObject::connect(m_settings_selector, SIGNAL(fileSelected(const DocLnk&)),
			 this, SLOT(do_settings_selected(const DocLnk&)));
	QObject::connect(m_settings_selector, SIGNAL(closeMe()), 
			 this, SLOT(do_close_settings_selector()));
	m_settings_selector->show();**/
#endif/*QT_NO_FILEDIALOG*/
}

void 
gtk_gui::do_open_url() {
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
		do_play();
	}
**/
#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
//	QMessageBox::information (this, m_programfilename,
//		gettext("Open URL not implemented for Embedded Qt"));
#endif/*QT_NO_FILEDIALOG*/
}

void 
gtk_gui::do_player_done() {
	AM_DBG printf("%s-%s\n", m_programfilename, "do_player_done");
	printf("PLAYER DONE\n");
	if ((m_mainloop != NULL) && (m_mainloop->player_done())) {
		gtk_widget_set_sensitive (m_playmenu_submenu_pause, false);
		gtk_widget_set_sensitive (m_playmenu_submenu_play, true);
		m_playing = false;
	}
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
	AM_DBG printf("%s-%s\n", m_programfilename, "player_done");
	gtk_signal_emit(GTK_OBJECT (this), signal_player_done_id, 0);
/*
	emit signal_player_done();
*/
}

void
no_fileopen_infodisplay(GtkWidget* w, const char* caption) {
/**	QMessageBox::information(w,caption,gettext("No file open: Please first select File->Open"));**/
}

void 
gtk_gui::do_play() {
	//AM_DBG 
	printf("%s-%s\n", m_programfilename, "do_play");
	if (m_smilfilename == NULL || m_mainloop == NULL || ! m_mainloop->is_open()) {
		no_fileopen_infodisplay(this, m_programfilename);
		return;
	}
	if (!m_playing) {
		m_playing = true;
		gtk_widget_set_sensitive (m_playmenu_submenu_play, false);
		gtk_widget_set_sensitive (m_playmenu_submenu_pause, true);
		m_mainloop->play();
	}
	if (m_pausing) {
		m_pausing = false;
		gtk_widget_set_sensitive (m_playmenu_submenu_pause, true);
		gtk_widget_set_sensitive (m_playmenu_submenu_play, false);
		m_mainloop->set_speed(1);
	}
}

void 
gtk_gui::do_pause() {
	AM_DBG printf("%s-%s\n", m_programfilename, "do_pause");
	if (! m_pausing) {
		m_pausing = true;
		gtk_widget_set_sensitive (m_playmenu_submenu_pause, false);
		gtk_widget_set_sensitive (m_playmenu_submenu_play, true);
		m_mainloop->set_speed(0);
	}
}

void 
gtk_gui::do_reload() {
	AM_DBG printf("%s-%s\n", m_programfilename, "do_reload");
	if (openSMILfile(m_smilfilename, 1)) {
		do_play();
	} else {
		no_fileopen_infodisplay(this, m_programfilename);
	}
}

void 
gtk_gui::do_stop() {
	AM_DBG printf("%s-%s\n", m_programfilename, "do_stop");
	if(m_mainloop)
		m_mainloop->stop();
	gtk_widget_set_sensitive (m_playmenu_submenu_pause, false);
	gtk_widget_set_sensitive (m_playmenu_submenu_play, true);
	m_playing = false;
}

void 
gtk_gui::do_settings_select() {
/**
	m_settings = new qt_settings();
	QWidget* settings_widget = m_settings->settings_select();
	m_finish_hb = new QHBox(settings_widget);
	m_ok_pb	= new QPushButton(gettext("OK"), m_finish_hb);
	m_finish_hb->setSpacing(50);
	QPushButton* m_cancel_pb= new QPushButton(gettext("Cancel"), m_finish_hb);
	QObject::connect(m_ok_pb, SIGNAL(released()),
			 this, SLOT(do_settings_ok()));
	QObject::connect(m_cancel_pb, SIGNAL(released()),
			 this, SLOT(do_settings_cancel()));
	settings_widget->show();
**/
}

void
gtk_gui::do_settings_ok() {
//	m_settings->settings_ok();
//	do_settings_cancel();
}

void
gtk_gui::do_settings_cancel() {
//	m_settings->settings_finish();
//	delete m_settings;
//	m_settings = NULL;
}

void
gtk_gui::do_quit() {
	AM_DBG printf("%s-%s\n", m_programfilename, "do_quit");
	if (m_mainloop)	{
		m_mainloop->stop();
		m_mainloop->release();
//		delete m_mainloop;
		m_mainloop = NULL;
	}
	m_busy = false;
	gtk_main_quit();
}

void 
gtk_gui::start_mainloop() {
	gtk_main();	
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
	
	int msg_id = level+gtk_logger::CUSTOM_OFFSET;
//  	gtk_message_event* me = new gtk_message_event(msg_id, msg);
//	g_signal_emit (G_OBJECT (me),0, NULL);

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

	gtk_init(&argc,&argv);
	/** Start gthread **/
/*	if( !g_thread_supported() ){
       		g_thread_init(NULL);
       		gdk_threads_init();
	}
*/
//#undef	ENABLE_NLS
#ifdef	ENABLE_NLS
	// Load localisation database
	bool private_locale = false;
	char *home = getenv("HOME");
	if (home) {
		std::string localedir = std::string(home) + "/.ambulant/locale";
		if (access(localedir.c_str(), 0) >= 0) {
			private_locale = true;
			bindtextdomain(PACKAGE, localedir.c_str());
		}
	}
	if (!private_locale)
		bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);
#endif /*ENABLE_NLS*/

	// Load preferences, initialize app and logger
	unix_preferences unix_prefs;
	unix_prefs.load_preferences();
	FILE* DBG = stdout;
	
	/* Setup widget */
	gtk_gui *mywidget = new gtk_gui(argv[0], argc > 1 ? argv[1] : "AmbulantPlayer");
	gtk_widget_show(GTK_WIDGET (mywidget->get_toplevel_container()));

	// take log level from preferences	
	gtk_logger::set_gtk_logger_gui(mywidget);
	gtk_logger* gtk_logger = gtk_logger::get_gtk_logger();
	lib::logger::get_logger()->debug("Ambulant Player: now logging to a window");
	// Print welcome banner
	lib::logger::get_logger()->debug(gettext("Ambulant Player: compile time version %s, runtime version %s"), AMBULANT_VERSION, ambulant::get_version());
	lib::logger::get_logger()->debug(gettext("Ambulant Player: built on %s for Unix/GTK"), __DATE__);
#if ENABLE_NLS
	lib::logger::get_logger()->debug(gettext("Ambulant Player: localization enabled (english)"));
#endif
	AM_DBG fprintf(DBG, "argc=%d argv[0]=%s\n", argc, argv[0]);
	AM_DBG for (int i=1;i<argc;i++){fprintf(DBG,"%s\n", argv[i]);
	}
	
	bool exec_flag = false;

	if (argc > 1) {
		char last[6];
		char* str = argv[argc-1];
		int len = strlen(str);
		strcpy(last, &str[len-5]);
		AM_DBG fprintf(DBG, "%s %s %x\n", str, last);
		if (strcmp(last, ".smil") == 0
		|| strcmp(&last[1], ".smi") == 0
	  	|| strcmp(&last[1], ".sml") == 0) {
 			if (mywidget->openSMILfile(argv[argc-1],
						   1)
			    && (exec_flag = true)){
				mywidget->do_play();
			}
		}
	} else {
		preferences* prefs = preferences::get_preferences();
		if ( ! prefs->m_welcome_seen) {
			const char *welcome_doc = find_datafile(welcome_locations);
			if (welcome_doc
			&& mywidget->openSMILfile(welcome_doc,
						  1)) {
				mywidget->do_play();
				prefs->m_welcome_seen = true;
			}
		}
		exec_flag = true;
	}
	
	if (exec_flag){
		//gdk_threads_enter();
		mywidget->start_mainloop();	
		//gdk_threads_leave();
	}else if (argc > 1) {
		std::string error_message = gettext("Cannot open: ");
		error_message = error_message + "\"" + argv[1] + "\"";
		std::cerr << error_message << std::endl;
		gdk_threads_enter();
		mywidget->start_mainloop();
		gdk_threads_leave();
	}	
	unix_prefs.save_preferences();
	delete gtk_logger::get_gtk_logger();
	mywidget->do_quit();
	std::cout << "Exiting program" << std::endl;
	return exec_flag ? 0 : -1;
}

