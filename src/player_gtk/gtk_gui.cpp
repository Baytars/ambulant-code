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

#define UI_FILENAME "ui_manager.xml"

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

static GdkPixmap *pixmap = NULL;

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
	gtk_window_fullscreen(((gtk_gui*) userdata)->get_toplevel_container());
}
void gtk_C_callback_normal_screen(void *userdata)
{
	gtk_window_unfullscreen(((gtk_gui*) userdata)->get_toplevel_container());
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
/*
extern "C" {
void gtk_C_callback_file_selected(void *userdata)
{
	((gtk_gui*) userdata)->do_file_selected();
}
void gtk_C_callback_settings_selected(void *userdata)
{
	((gtk_gui*) userdata)->do_settings_selected();
}
void gtk_C_callback_url_selected(void *userdata)
{
	((gtk_gui*) userdata)->do_url_selected();
}
}
*/
extern "C" {
void gtk_C_callback_do_player_done(void *userdata)
{
	((gtk_gui*) userdata)->do_player_done();
}
void gtk_C_callback_do_need_redraw_simple(void *userdata)
{
	((gtk_gui*) userdata)->do_need_redraw();
}

void gtk_C_callback_do_need_redraw(void *userdata, void* r_call, void* w_call, void* pt_call)
{
	const void* r = r_call;
	void* w = w_call;
	const void* pt = pt_call;
//	((gtk_gui*) userdata)->do_need_redraw(r, w, pt);
}
void gtk_C_callback_do_internal_message(void *userdata, void* e)
{
	gtk_message_event* event = (gtk_message_event*) e;	
	((gtk_gui*) userdata)->do_internal_message(event);
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
        m_busy(true),
	m_file_chooser(NULL),
	m_settings_chooser(NULL),
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
//	m_settings(NULL),
	m_toplevelcontainer()
{

	GError *error = NULL;

	// Initialization of the Menu Bar Items
	// There is a problem in here because the callbacks in Actions go like g_signal_connect (but, we need g_sginal_connect_swapped)
	static GtkActionEntry entries[] = {
	{ "FileMenu", NULL, "_File"},
	{ "open", GTK_STOCK_OPEN, "_Open...", "<alt>F", "Open a file", NULL},
	{ "openurl", NULL, "Open _URL...", "<alt>U", "Open a url", NULL},
	{ "reload", GTK_STOCK_REFRESH, "_Reload...", "<alt>R", "Reload a document", NULL},
	{ "settings", GTK_STOCK_PREFERENCES , "_Settings", "<alt>S", "Launch settings window", NULL},
	{ "quit", GTK_STOCK_QUIT, "_Quit", "<alt>Q", "Quit ambulant", NULL},	
	// Play Menu
	{ "PlayMenu", "<alt>Y", "Pla_y"},
	{ "play", GTK_STOCK_MEDIA_PLAY, "Pla_y", "<alt>Y", "Play document", NULL},
	{ "pause", GTK_STOCK_MEDIA_PAUSE, "_Pause", "<alt>P", "Pause document", G_CALLBACK (gtk_C_callback_pause)},
	{ "stop", GTK_STOCK_MEDIA_STOP, "_Stop", "<alt>S", "Stop document", NULL},
	// View Menu
	{ "ViewMenu", "<alt>V", "_View"},
	{ "fullscreen", NULL, "_Full Screen", "<alt>F", "Full Screen", NULL},
	{ "window", NULL, "_Window", "<alt>W", "Normal Screen", NULL},
	{ "loadsettings", GTK_STOCK_PROPERTIES, "_Load Settings...", "<alt>R", "Launch the settings window", NULL},
	{ "logwindow", NULL, "_Log Window...", "<alt>L", "Launch log window", NULL},
	// Help Menu
	{ "HelpMenu", "<alt>H", "_Help"},
	{ "about", GTK_STOCK_ABOUT, "_About AmbulantPlayer", "<alt>A", "Information about Ambulant", NULL},
	{ "help", GTK_STOCK_HELP, "AmbulantPlayer _Help", "<alt>H", "Help in AmbulantPlayer Webpage", NULL},
	{ "website", NULL, "AmbulantPlayer _Website...", "<alt>W", "Open the Ambulant Webpage", NULL},
	{ "welcome", GTK_STOCK_HOME, "_Play Welcome Document", "<alt>P", "Plays a simple SMIL file", NULL}
	};

	int n_entries = G_N_ELEMENTS(entries);

	m_programfilename = title;
#ifdef	TRY_LOCKING
	pthread_cond_init(&m_cond_message, NULL);
	pthread_mutex_init(&m_lock_message, NULL);
	m_gui_thread = pthread_self();
#endif/*TRY_LOCKING*/
	if (initfile != NULL && initfile != "")
		m_smilfilename = initfile;

	m_playing = false;
	m_pausing = false;
		
	/*Initialization of the Main Window */
	m_toplevelcontainer = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_title(m_toplevelcontainer, initfile);
	gtk_window_set_resizable(m_toplevelcontainer, true); 	
	gtk_widget_set_size_request(GTK_WIDGET (m_toplevelcontainer), 320, 240);
	gtk_widget_set_uposition(GTK_WIDGET (m_toplevelcontainer), 240, 320);	
	g_signal_connect_swapped (GTK_OBJECT (m_toplevelcontainer), "delete-event", G_CALLBACK (gtk_C_callback_quit), (void *) this);
	
	/* Initialization of the signals */
	signal_player_done_id = g_signal_new ("signal-player-done", gtk_window_get_type(), G_SIGNAL_RUN_LAST, 0, 0, 0, g_cclosure_marshal_VOID__VOID,GTK_TYPE_NONE, 0, NULL);

	signal_need_redraw_id = g_signal_new ("signal-need-redraw", gtk_window_get_type(), G_SIGNAL_RUN_LAST, 0, 0, 0, gtk_marshal_NONE__POINTER_POINTER_POINTER,GTK_TYPE_NONE, 3, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_POINTER);
//(const void*, void*, const void*));
	
	signal_internal_message_id = g_signal_new ("signal-internal-message", gtk_window_get_type(), G_SIGNAL_RUN_LAST, 0, 0, 0, gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1, G_TYPE_POINTER);

	// Signal connections
	g_signal_connect_swapped (GTK_OBJECT (m_toplevelcontainer), "signal-player-done",  G_CALLBACK (gtk_C_callback_do_player_done), (void*)this);
	
	g_signal_connect_swapped (GTK_OBJECT (m_toplevelcontainer), "signal-need-redraw",  G_CALLBACK (gtk_C_callback_do_player_done), (void*)this);

	g_signal_connect_swapped (GTK_OBJECT (m_toplevelcontainer), "signal-internal-message",  G_CALLBACK (gtk_C_callback_do_internal_message), (void*)this);

	/* VBox (m_guicontainer) to place the Menu bar in the correct place */ 
	m_guicontainer = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(m_toplevelcontainer), GTK_WIDGET (m_guicontainer));
	
	/* The Action Group that includes the menu bar */
	m_actions = gtk_action_group_new("Actions");
	gtk_action_group_add_actions(m_actions, entries, n_entries, (void*)this);

	/* The Gtk UI Manager */
	GtkUIManager *ui = gtk_ui_manager_new();

	if (!gtk_ui_manager_add_ui_from_file(ui, UI_FILENAME, &error))
		g_error("Could not merge UI from %s, error was: %s\n", UI_FILENAME, error->message);
	gtk_ui_manager_insert_action_group(ui, m_actions, 0);
	
	/* Disable and make invisible menus and menu items */
#ifdef GTK_NO_FILEDIALOG
	gtk_action_set_sensitive(gtk_action_group_get_action(m_actions, "open"), true);
	gtk_action_set_sensitive(gtk_action_group_get_action(m_actions, "openurl"), false);
#endif
	gtk_action_set_sensitive(gtk_action_group_get_action(m_actions, "play"), false);
	gtk_action_set_sensitive(gtk_action_group_get_action(m_actions, "pause"), false);

	// The actual activation calls
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "open"), "activate",  G_CALLBACK (gtk_C_callback_open), (void *) this );	
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "openurl"), "activate",  G_CALLBACK (gtk_C_callback_open_url), (void*)this);		
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "reload"), "activate",  G_CALLBACK (gtk_C_callback_reload), (void*)this);		
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "settings"), "activate",  G_CALLBACK (gtk_C_callback_settings_select), (void *) this );	
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "quit"), "activate",  G_CALLBACK (gtk_C_callback_quit), (void*)this);		

	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "play"), "activate",  G_CALLBACK (gtk_C_callback_play), (void*)this);
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "pause"), "activate",  G_CALLBACK (gtk_C_callback_pause), (void *) this );	
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "stop"), "activate",  G_CALLBACK (gtk_C_callback_stop), (void*)this);

	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "fullscreen"), "activate",  G_CALLBACK (gtk_C_callback_full_screen), (void*)this);
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "window"), "activate",  G_CALLBACK (gtk_C_callback_normal_screen), (void*)this);		
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "loadsettings"), "activate",  G_CALLBACK (gtk_C_callback_load_settings), (void*)this);		
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "logwindow"), "activate",  G_CALLBACK (gtk_C_callback_logger_window), (void*)this);		

	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "about"), "activate",  G_CALLBACK (gtk_C_callback_about), (void*)this);
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "help"), "activate",  G_CALLBACK (gtk_C_callback_help), (void*)this);		
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "website"), "activate",  G_CALLBACK (gtk_C_callback_homepage), (void*)this);		
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "welcome"), "activate",  G_CALLBACK (gtk_C_callback_welcome), (void*)this);		

	
	/* Creation of the Menubar and Menu Items */
	GtkWidget *menubar = gtk_ui_manager_get_widget (ui, "/MenuBar");
	gtk_box_pack_start (GTK_BOX (m_guicontainer), menubar, FALSE, FALSE, 0);
	gtk_widget_show_all(GTK_WIDGET (m_toplevelcontainer));
	
	/* Old Menu bar */	
/*
	m_menubar = (GtkMenuBar*)gtk_menu_bar_new();
	gtk_widget_show ((GtkWidget*)m_menubar);
	//gtk_box_pack_start(GTK_BOX (m_guicontainer), GTK_WIDGET (m_menubar), FALSE, FALSE, 0);	
*/
	/* Old File  Menu*/
/*
	m_filemenu = (GtkMenuItem*)gtk_menu_item_new_with_mnemonic("_File");
	gtk_widget_show ((GtkWidget*)m_filemenu);
	gtk_container_add(GTK_CONTAINER (m_menubar), GTK_WIDGET (m_filemenu));
	GtkWidget* m_filemenu_submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM (m_filemenu), m_filemenu_submenu);
		
	GtkWidget* m_filemenu_submenu_open = gtk_menu_item_new_with_label("Open...");
	gtk_widget_show ((GtkWidget*)m_filemenu_submenu_open);
	gtk_container_add(GTK_CONTAINER (m_filemenu_submenu), GTK_WIDGET (m_filemenu_submenu_open));
	g_signal_connect_swapped (GTK_OBJECT (m_filemenu_submenu_open), "activate",  G_CALLBACK (gtk_C_callback_open), (void *) this );	
*/
	m_o_x = 0;
#ifndef GTK_NO_FILEDIALOG	/* Assume plain GTK */
	m_o_y = 27;
#else /*GTK_NO_FILEDIALOG*/	/* Assume embedded GTK */
	m_o_y = 20;
#endif/*QT_NO_FILEDIALOG*/
	
// This is the old manner of creating the menu bar 
	/* A canvas with fixed layout should be the document_container */ 
	//m_documentcontainer = gtk_drawing_area_new();
	//g_signal_connect_swapped (G_OBJECT (m_documentcontainer), "expose_event", G_CALLBACK (gtk_C_callback_do_need_redraw_simple), (void*) this);
	//gtk_widget_show(m_documentcontainer);
 	//gtk_box_pack_start (GTK_BOX (m_guicontainer), m_documentcontainer, TRUE, TRUE, 0);
	
	// emits the signal that the player is done - it is not needed
	//g_signal_emit(GTK_OBJECT (m_toplevelcontainer), signal_player_done_id, 0);
}

void 
gtk_gui::do_need_redraw () {

//	AM_DBG printf("gtk_gui::need_redraw(0x%x)-r=(0x%x)\n", (void *)this,r?r:0);
/*	GdkRectangle update_rect;
	GdkColor color;
	GdkColormap *cmap = gdk_colormap_get_system(  );	

	if (gdk_color_parse("PeachPuff", &color) && gdk_colormap_alloc_color(cmap, &color, FALSE, TRUE))
	{
 */ 		/* Use our MediumSpringGreen for a nice pastoral scene */
/*
		update_rect.x = 50;
		update_rect.y = 50;
		update_rect.width = 100;
		update_rect.height = 100;	
		
		gtk_widget_modify_bg (m_documentcontainer, GTK_STATE_NORMAL, &color );

		if (pixmap)
			gdk_pixmap_unref(pixmap);	

  		pixmap = gdk_pixmap_new(m_documentcontainer->window,
                          	m_documentcontainer->allocation.width,
                          	m_documentcontainer->allocation.height,
                          	-1);
  		gdk_draw_rectangle (pixmap,
			m_documentcontainer->style->bg_gc[GTK_STATE_NORMAL],
                      	TRUE,
			update_rect.x,
			update_rect.y,
			update_rect.width,
                      	update_rect.height);
  		gdk_draw_pixmap (m_documentcontainer->window,
			m_documentcontainer->style->bg_gc[GTK_STATE_NORMAL],
			pixmap,
			update_rect.x, update_rect.y,
			0, 0,
			update_rect.width,
                      	update_rect.height);
	}
	else
	{
		printf("we haven't found the color\n");
	} */
}

gtk_gui::~gtk_gui() {

#define DELETE(X) if (X) { delete X; X = NULL; }
	AM_DBG printf("%s0x%X\n", "gtk_gui::~gtk_gui(), m_mainloop=",m_mainloop);
	//setCaption(QString::null);
	gtk_widget_destroy(GTK_WIDGET (m_file_chooser));
	gtk_widget_destroy(GTK_WIDGET (m_settings_chooser));
	DELETE(m_actions);
//	gtk_widget_destroy(GTK_WIDGET (m_filemenu));
//	gtk_widget_destroy(GTK_WIDGET (m_playmenu));
//	gtk_widget_destroy(GTK_WIDGET (m_viewmenu));
//	gtk_widget_destroy(GTK_WIDGET (m_menubar));
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
		if( openSMILfile(welcome_doc, 0)) {
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
checkFilename(const gchar* filename, int mode) {
	FILE * file;
	if (mode==0)
  	return fopen (filename,"r");
}

void
gtk_gui::fileError(const gchar* smilfilename) {
 	char buf[1024];
	sprintf(buf, gettext("%s: Cannot open file: %s"),
		(const char*) smilfilename, strerror(errno));
	GtkMessageDialog* dialog = 
	(GtkMessageDialog*) gtk_message_dialog_new (NULL,
         GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_ERROR,
         GTK_BUTTONS_OK,
	 m_programfilename);
	gtk_message_dialog_format_secondary_text(dialog, buf);
 	gtk_dialog_run (GTK_DIALOG (dialog));
 	gtk_widget_destroy (GTK_WIDGET (dialog));
}

bool 
gtk_gui::openSMILfile(const char *smilfilename, int mode) {

	if (smilfilename==NULL)
		return false;
	
	if (mode == 0)
		if (! checkFilename(smilfilename, mode)) {
			fileError(smilfilename);
			return false;
		}

	gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "pause"), false);
	gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "play"), true);
	gtk_window_set_title(GTK_WINDOW (m_toplevelcontainer), smilfilename);

	char* filename = strdup(smilfilename);
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
	m_file_chooser = GTK_FILE_CHOOSER (gtk_file_chooser_dialog_new("Please, select a file", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL));

	GtkFileFilter *filter_smil = gtk_file_filter_new();
	gtk_file_filter_set_name(filter_smil, "SMIL files");
	gtk_file_filter_add_pattern(filter_smil, "*.smil");
	gtk_file_filter_add_pattern(filter_smil, "*.smi");
	gtk_file_chooser_add_filter(m_file_chooser, filter_smil);
	GtkFileFilter *filter_all = gtk_file_filter_new();
	gtk_file_filter_set_name(filter_all, "All files");
	gtk_file_filter_add_pattern(filter_all, "*.smil");
	gtk_file_filter_add_pattern(filter_all, "*.smi");	
	gtk_file_filter_add_pattern(filter_all, "*.mms");	
	gtk_file_filter_add_pattern(filter_all, "*.grins");	
	gtk_file_chooser_add_filter(m_file_chooser, filter_all);
	GtkFileFilter *filter_any = gtk_file_filter_new();
	gtk_file_filter_set_name(filter_any, "Any file");
	gtk_file_filter_add_pattern(filter_any, "*");
	gtk_file_chooser_add_filter(m_file_chooser, filter_any);

	gint result = gtk_dialog_run (GTK_DIALOG (m_file_chooser));
  	switch (result){
      	case GTK_RESPONSE_ACCEPT:
		do_file_selected();
         	break;
      	default:
         	break;
    	}
	gtk_widget_hide(GTK_WIDGET (m_file_chooser));	
}

void gtk_gui::do_file_selected() {
	const gchar *smilfilename  = gtk_file_chooser_get_filename (m_file_chooser);
	if (openSMILfile(smilfilename, 0)){
		do_play();
	}
}



void
gtk_gui::setDocument(const char* smilfilename) {
#ifdef	GTK_NO_FILEDIALOG	/* Assume embedded GTK */
	if (openSMILfile(smilfilename, 0))
		do_play();
#endif/*GTK_NO_FILEDIALOG*/
}


void
gtk_gui::do_settings_selected() {
	const gchar *settings_filename  = gtk_file_chooser_get_filename (m_settings_chooser);
	if ( settings_filename != NULL) {
		smil2::test_attrs::load_test_attrs(settings_filename);
		if (openSMILfile(m_smilfilename, 0))
			do_play();
	}
}

void 
gtk_gui::do_load_settings() {
	if (m_playing)
		do_stop();
	
	m_settings_chooser = GTK_FILE_CHOOSER (gtk_file_chooser_dialog_new("Please, select a settings file", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL));

	GtkFileFilter *filter_xml = gtk_file_filter_new();
	gtk_file_filter_set_name(filter_xml, "XML files");
	gtk_file_filter_add_pattern(filter_xml, "*.xml");
	gtk_file_chooser_add_filter(m_settings_chooser, filter_xml);

	gint result = gtk_dialog_run (GTK_DIALOG (m_settings_chooser));
  	switch (result){
      	case GTK_RESPONSE_ACCEPT:
		do_settings_selected();
         	break;
      	default:
         	break;
    	}
	gtk_widget_hide(GTK_WIDGET (m_settings_chooser));	

	/* Ensure that the dialog box is hidden when the user clicks a button.
	We don't need anymore the callbacks, because they can be embedded in this part of the code */
/*
	g_signal_connect_swapped (GTK_FILE_SELECTION (m_settings_selector)->ok_button,
                             "clicked",
                             G_CALLBACK (gtk_widget_hide), 
                             m_settings_selector);

   	g_signal_connect_swapped (GTK_FILE_SELECTION (m_settings_selector)->cancel_button,
                             "clicked",
                             G_CALLBACK (gtk_widget_hide),
                             m_settings_selector);
	g_signal_connect_swapped (GTK_OBJECT ((m_settings_selector)->ok_button),"clicked", G_CALLBACK (gtk_C_callback_settings_selected),(void*) this);
*/
}


void 
gtk_gui::do_open_url() {
	GtkDialog* url_dialog =  GTK_DIALOG (gtk_dialog_new_with_buttons
	("AmbulantPlayer", NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	
	GtkLabel* label = GTK_LABEL (gtk_label_new("URL to open:"));
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
	gtk_widget_show(GTK_WIDGET (label));
	
	m_url_text_entry = GTK_ENTRY (gtk_entry_new());
	gtk_entry_set_editable(m_url_text_entry, true);
	gtk_entry_set_text(m_url_text_entry,"http://www");
	gtk_widget_show(GTK_WIDGET (m_url_text_entry));
	
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(url_dialog)->vbox), GTK_WIDGET (label));
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(url_dialog)->vbox), GTK_WIDGET (m_url_text_entry));

	gint result = gtk_dialog_run (GTK_DIALOG (url_dialog));
  	switch (result){
      	case GTK_RESPONSE_ACCEPT:
		do_url_selected();
         	break;
      	default:
         	break;
    	}
	gtk_widget_destroy (GTK_WIDGET (url_dialog));
/*	
	g_signal_connect_swapped (url_dialog,"response", G_CALLBACK (gtk_C_callback_url_selected), (void*) this);
	
	g_signal_connect_swapped (url_dialog, "response",
                             G_CALLBACK (gtk_widget_hide), 
                            GTK_WIDGET (url_dialog));
	gtk_widget_show_all(GTK_WIDGET (url_dialog));
*/
}

void gtk_gui::do_url_selected() {
	const gchar *smilfilename  = gtk_entry_get_text(m_url_text_entry);
	gtk_window_set_title(GTK_WINDOW (m_toplevelcontainer), smilfilename);
	if (openSMILfile(smilfilename, 0)){		
		do_play();
	}
}

void 
gtk_gui::do_player_done() {
	AM_DBG printf("%s-%s\n", m_programfilename, "do_player_done");
	if (m_mainloop == NULL){
		gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "pause"), false);
		gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "play"), true);
		m_playing = false;
	}else if ((m_mainloop->player_done())) {

	}
}

void 
gtk_gui::need_redraw (const void* r, void* w, const void* pt) {

	AM_DBG printf("gtk_gui::need_redraw(0x%x)-r=(0x%x)\n",
	(void *)this,r?r:0);
	g_signal_emit(GTK_OBJECT (m_toplevelcontainer), signal_need_redraw_id, 0, r, w, pt);
}

void 
gtk_gui::player_done() {
	AM_DBG printf("%s-%s\n", m_programfilename, "player_done");
	g_signal_emit(GTK_OBJECT (m_toplevelcontainer), signal_player_done_id, 0);
}

void
no_fileopen_infodisplay(gtk_gui* w, const char* caption) {
	GtkMessageDialog* dialog = 
	(GtkMessageDialog*) gtk_message_dialog_new (NULL,
         GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_INFO,
         GTK_BUTTONS_OK,
	 caption);
	gtk_message_dialog_format_secondary_markup (dialog, "No file open: Please first select File->Open");
 	gtk_dialog_run (GTK_DIALOG (dialog));
 	gtk_widget_destroy (GTK_WIDGET (dialog));
}

void 
gtk_gui::do_play() {
	AM_DBG printf("%s-%s\n", m_programfilename, "do_play");
	if (m_smilfilename == NULL || m_mainloop == NULL || ! m_mainloop->is_open()) {
		no_fileopen_infodisplay(this, m_programfilename);
		return;
	}
	if (!m_playing) {
		m_playing = true;
		gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "play"), false);
		gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "pause"), true);
		m_mainloop->play();
	}
	if (m_pausing) {
		m_pausing = false;
		gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "pause"), true);
		gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "play"), false);
		m_mainloop->set_speed(1);
	}
}

void 
gtk_gui::do_pause() {
	AM_DBG printf("%s-%s\n", m_programfilename, "do_pause");
	if (! m_pausing) {
		m_pausing = true;
		gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "pause"), false);
		gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "play"), true);
		m_mainloop->set_speed(0);
	}
}

void 
gtk_gui::do_reload() {
	AM_DBG printf("%s-%s\n", m_programfilename, "do_reload");
	if (openSMILfile(m_smilfilename, 0)) {
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
	gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "pause"), false);
	gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "play"), true);
	m_playing = false;
}

void 
gtk_gui::do_settings_select() {

	m_settings = new gtk_settings();
	gint result = gtk_dialog_run (GTK_DIALOG (m_settings->getWidget()));
  	switch (result){
      	case GTK_RESPONSE_ACCEPT:
		m_settings->settings_ok();
	       	break;
      	default:
         	break;
    	}
	gtk_widget_destroy (GTK_WIDGET (m_settings->getWidget()));
}

void
gtk_gui::do_quit() {
	AM_DBG printf("%s-%s\n", m_programfilename, "do_quit");
	if (m_mainloop)	{
		m_mainloop->stop();
		m_mainloop->release();
		m_mainloop = NULL;
	}
	m_busy = false;
	gtk_main_quit();
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

void
gtk_gui::do_internal_message(gtk_message_event* e) {
	char* msg = (char*)e->get_message();
	//std::string id("gtk_gui::do_internal_message");
	//std::cerr<<id<<std::endl;
	//std::cerr<<id+" type: "<<e->get_type()<<" msg:"<<msg<<std::endl;
	int level = e->get_type() - gtk_logger::CUSTOM_OFFSET;
	GtkMessageDialog* dialog; // just in case is needed
	switch (level) {
	case gtk_logger::CUSTOM_NEW_DOCUMENT:
		if (m_mainloop) {
			bool start = msg[0] == 'S' ? true : false;
			bool old = msg[2] == 'O' ? true : false;
			m_mainloop->player_start(&msg[4], start, old);
		}
		break;
	case gtk_logger::CUSTOM_LOGMESSAGE:
		gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER (gtk_logger::get_gtk_logger()->get_logger_buffer()), msg, strlen(msg));
		break;
	case ambulant::lib::logger::LEVEL_FATAL:
		dialog = (GtkMessageDialog*) gtk_message_dialog_new (NULL,
         	GTK_DIALOG_DESTROY_WITH_PARENT,
         	GTK_MESSAGE_ERROR,
         	GTK_BUTTONS_OK,
	 	msg);
 		gtk_dialog_run (GTK_DIALOG (dialog));
 		gtk_widget_destroy (GTK_WIDGET (dialog));
		break;
	case ambulant::lib::logger::LEVEL_ERROR:
		dialog = (GtkMessageDialog*) gtk_message_dialog_new (NULL,
         	GTK_DIALOG_DESTROY_WITH_PARENT,
         	GTK_MESSAGE_WARNING,
         	GTK_BUTTONS_OK,
	 	msg);
 		gtk_dialog_run (GTK_DIALOG (dialog));
 		gtk_widget_destroy (GTK_WIDGET (dialog));
		break;
	case ambulant::lib::logger::LEVEL_WARN:
	default:
		dialog = (GtkMessageDialog*) gtk_message_dialog_new (NULL,
         	GTK_DIALOG_DESTROY_WITH_PARENT,
         	GTK_MESSAGE_INFO,
         	GTK_BUTTONS_OK,
	 	msg);
 		gtk_dialog_run (GTK_DIALOG (dialog));
 		gtk_widget_destroy (GTK_WIDGET (dialog));
		break;
	}
#ifdef	TRY_LOCKING
	if (level >= ambulant::lib::logger::LEVEL_WARN) {
		pthread_mutex_lock(&m_lock_message);
		pthread_cond_signal(&m_cond_message);
		pthread_mutex_unlock(&m_lock_message);
	}
#endif/*TRY_LOCKING*/
	free(msg);
}

void
gtk_gui::internal_message(int level, char* msg) {
	
	int msg_id = level+gtk_logger::CUSTOM_OFFSET;
	gtk_message_event* event = new gtk_message_event(msg_id, msg);
	g_signal_emit(GTK_OBJECT (m_toplevelcontainer), signal_internal_message_id, 0, event);

#ifdef	TRY_LOCKING
	if (level >= ambulant::lib::logger::LEVEL_WARN
	    && pthread_self() != m_gui_thread) {
	  // wait until the message as been OK'd by the user
		pthread_mutex_lock(&m_lock_message);
		pthread_cond_wait(&m_cond_message, &m_lock_message);
		pthread_mutex_unlock(&m_lock_message);
	}
#endif /*TRY_LOCKING*/
}

int
main (int argc, char*argv[]) {

	/** Start gthread **/
//	if( !g_thread_supported() ){
//      		g_thread_init(NULL);
 //      		gdk_threads_init();
//	}
	gtk_init(&argc,&argv);

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
	AM_DBG for (int i=1;i<argc;i++){fprintf(DBG,"%s\n", argv[i]);}
	
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
						   0)
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
						  0)) {
				mywidget->do_play();
				prefs->m_welcome_seen = true;
			}
		}
		exec_flag = true;
	}
	
	if (exec_flag){
//		gdk_threads_enter();
		gtk_main();			
//		gdk_threads_leave();
	}else if (argc > 1) {
		std::string error_message = gettext("Cannot open: ");
		error_message = error_message + "\"" + argv[1] + "\"";
		std::cerr << error_message << std::endl;
//		gdk_threads_enter();
		gtk_main();
//		gdk_threads_leave();
	}	
	unix_prefs.save_preferences();
	delete gtk_logger::get_gtk_logger();
	mywidget->do_quit();
	std::cout << "Exiting program" << std::endl;
	return exec_flag ? 0 : -1;
}

