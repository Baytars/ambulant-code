/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* qt_gui.cpp - Qt GUI for Ambulant
 *              
 *              Initial version renders images & text
 *
 * Kees Blom, Oct.29 2003
 */

#include <pthread.h>
#include <libgen.h>
#include <stdlib.h>
#include "qt_gui.h"
#include "qt_mainloop.h"
#include "qt_renderer.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

const QString about_text = "This is the Qt GUI for Ambulant.\n"
			   "Work in  progress by Kees Blom (C) 2004";

qt_gui::qt_gui(const char* title,
	       const char* initfile) :
        m_busy(true),
	m_o_x(0),	 
	m_o_y(0),	 
	m_mainloop(NULL),
	m_pause_id(),
	m_pausing(),
	m_play_id(),
	m_playing(),
	m_playmenu(),
	m_programfilename(),
	m_smilfilename()
{

	m_programfilename = title;
	if (initfile != NULL && initfile != "")
		m_smilfilename = initfile;
#ifdef  QT_NO_FILEDIALOG
	else
		m_smilfilename = QString(
			"/home/zaurus/Documents/example.smil");
#endif/*QT_NO_FILEDIALOG*/
	m_playing = false;
	m_pausing = false;
	setCaption(initfile);

	/* Menu bar */
	QMenuBar* menubar = new QMenuBar(this,"MainMenu");
	{
		int id;
		/* File */
		QPopupMenu* filemenu = new QPopupMenu (this);
		assert(filemenu);
		filemenu->insertItem("&Open", this, SLOT(slot_open()));
		filemenu->insertItem("&Full Screen", this,
				     SLOT(showFullScreen()));
		filemenu->insertItem("&Normal", this,SLOT(showNormal()));
//		filemenu->insertItem("&Quit", qApp, SLOT(quit()));
		filemenu->insertItem("&Quit", this, SLOT(slot_quit()));
		menubar->insertItem("&File", filemenu);
		
		/* Play */
		m_playmenu = new QPopupMenu (this, "PlayA");
		assert(m_playmenu);
		m_play_id = m_playmenu->insertItem("Pla&y", this,
						   SLOT(slot_play()));
		m_playmenu->setItemEnabled(m_play_id, false);
		m_pause_id = m_playmenu->insertItem("&Pause", this,
						    SLOT(slot_pause()));
		m_playmenu->setItemEnabled(m_pause_id, false);
		m_playmenu->insertItem("&Stop",	this, SLOT(slot_stop()));
		menubar->insertItem("Pla&y", m_playmenu);
		
		/* Help */
		QPopupMenu* helpmenu = new QPopupMenu (this, "HelpA");
		assert(helpmenu);
		helpmenu->insertItem("&About", this, SLOT(slot_about()));
		menubar->insertItem("&Help", helpmenu);
		menubar->setGeometry(0,0,320,20);
		m_o_x = 0;
		m_o_y = 27;
	}
}

qt_gui::~qt_gui() {
	setCaption(QString::null);
}

void qt_gui::slot_about() {
	QMessageBox::information(this, m_programfilename, about_text,
				 QMessageBox::Ok | QMessageBox::Default
				 );
}

bool checkFilename(QString filename, int mode) {
	QFile* file = new QFile(filename);
	return file->open(mode);
}

bool qt_gui::openSMILfile(QString smilfilename, int mode) {
	if (smilfilename.isNull())
		return false;
	if (! checkFilename(smilfilename, mode)) {
		char buf[1024];
		sprintf(buf, "Cannot open file \"%s\":\n%s\n",
			(const char*) smilfilename, strerror(errno));
		QMessageBox::information(this, m_programfilename, buf);
		return false;
	}
	char* filename = strdup(smilfilename);
	setCaption(basename(filename));
	free(filename);
	m_playmenu->setItemEnabled(m_pause_id, false);
	m_playmenu->setItemEnabled(m_play_id, true);
	if ( ! (*smilfilename == '/')) {
 	         char* workdir = getcwd(NULL, 0);
		 int workdirlen = strlen(workdir);
		 int pathnamelen = workdirlen + strlen(smilfilename) + 2;
		 char* pathname = (char*) malloc(pathnamelen);
		 strcpy(pathname, workdir);
		 strcpy(&pathname[workdirlen], "/");
		 strcpy(&pathname[workdirlen+1], smilfilename);
		 strcpy(&pathname[pathnamelen], "\0");
		 smilfilename = pathname;
	} else   smilfilename = strdup(smilfilename);
	        
//      if (m_smilfilename != NULL)
//	         free(m_smilfilename);
	m_smilfilename = smilfilename;
	return true;
}

void qt_gui::slot_open() {
#ifndef QT_NO_FILEDIALOG
	QString smilfilename =
		QFileDialog::getOpenFileName(
				 ".", // Initial dir
				 "SMIL files (*.smil *.smi);; All files (*.smil *.smi *.mms *.grins);; Any file (*)", // file types
				 this,
				 "open file dialog",
				 "Double Click a file to open"
				 );
#endif/*QT_NO_FILEDIALOG*/
	openSMILfile(smilfilename, IO_ReadOnly);
}

void qt_gui::slot_player_done() {
	printf("%s-%s\n", m_programfilename, "slot_player_done");
	m_playmenu->setItemEnabled(m_pause_id, false);
	m_playmenu->setItemEnabled(m_play_id, true);
	m_playing = false;
	QObject::disconnect(this, SIGNAL(signal_player_done()),
			    this, SLOT(slot_player_done()));
}

void qt_gui::need_redraw (const void* r, void* w, const void* pt) {
	printf("qt_gui::need_redraw(0x%x)-r=(0x%x)\n",
	(void *)this,r);
}

void qt_gui::player_done() {
	printf("%s-%s\n", m_programfilename, "player_done");
	emit signal_player_done();
}

void qt_gui::slot_play() {
	AM_DBG printf("%s-%s\n", m_programfilename, "slot_play");
	if (m_smilfilename == NULL) {
		QMessageBox::information(
			this, m_programfilename,
			"Please first select File->Open");
		return;
	}
	if (!m_playing) {
		m_playing = true;
		QObject::connect(this, SIGNAL(signal_player_done()),
				 this, SLOT(slot_player_done()));
		m_playmenu->setItemEnabled(m_play_id, false);
		m_playmenu->setItemEnabled(m_pause_id, true);
		pthread_t playthread;
		if (m_mainloop != NULL)
			delete m_mainloop;
		m_mainloop = new qt_mainloop(this);
		int rv = pthread_create(&playthread, NULL,
					&qt_mainloop::run,
					m_mainloop);
	}
	if (m_pausing) {
		m_pausing = false;
		m_playmenu->setItemEnabled(m_pause_id, true);
		m_playmenu->setItemEnabled(m_play_id, false);
		m_mainloop->set_speed(1);
	}
}

void qt_gui::slot_pause() {
	printf("%s-%s\n", m_programfilename, "slot_pause");
	if (! m_pausing) {
		m_pausing = true;
		m_playmenu->setItemEnabled(m_pause_id, false);
		m_playmenu->setItemEnabled(m_play_id, true);
		m_mainloop->set_speed(0);
	}
}

void qt_gui::slot_stop() {
	printf("%s-%s\n", m_programfilename, "slot_stop");
	m_mainloop->stop();
}

void qt_gui::slot_quit() {
	printf("%s-%s\n", m_programfilename, "slot_quit");
	delete m_mainloop;
	m_mainloop = NULL;
	m_busy = false;
//TBD setting m_busy = false gives core dump, there is also an event thread
	exit(0);
}

int main (int argc, char*argv[]) {
#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
	QApplication myapp(argc, argv);
#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
	QPEApplication myapp(argc, argv);
#endif/*QT_NO_FILEDIALOG*/

	/* Setup widget */
	qt_gui* mywidget = new qt_gui(argv[0], argc > 1 ? argv[1] : "");
#ifndef QT_NO_FILEDIALOG     /* Assume plain Qt */
	mywidget->setGeometry(750, 50, 320, 240);
	/* Fire */
	myapp.setMainWidget(mywidget);
#else /*QT_NO_FILEDIALOG*/   /* Assume embedded Qt */
	myapp.showMainWidget(mywidget);
#endif/*QT_NO_FILEDIALOG*/
	mywidget->show();
	myapp.processEvents();
	if (argc > 1) {
	    mywidget->openSMILfile(argv[1], IO_ReadOnly);
	    mywidget->slot_play();
	}
//	myapp.processEvents();
//	myapp.exec();
	while(mywidget->is_busy() || myapp.hasPendingEvents())
		myapp.processEvents();
	std::cout << "Exiting program" << std::endl;
	return 0;
}
