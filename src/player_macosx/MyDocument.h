/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
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

/* MyDocument */

#import <Cocoa/Cocoa.h>
#include "mainloop.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"

#include "ambulant/common/embedder.h"
#include "ambulant/net/url.h"

// Defining WITH_OVERLAY_WINDOW will allow a second document to live
// in the same window, on top of the normal presentation.
#ifdef WITH_OVERLAY_WINDOW
#import "MyAmbulantView.h"
#endif

class document_embedder : public ambulant::common::embedder {
  public:
	document_embedder(id mydocument)
	:   m_mydocument(mydocument) {}

	// common:: embedder interface
	void show_file(const ambulant::net::url& href);
	void close(ambulant::common::player *p);
	void open(ambulant::net::url newdoc, bool start, ambulant::common::player *old=NULL);
#ifdef WITH_OVERLAY_WINDOW
	bool aux_open(const ambulant::net::url& href);
#endif
  private:
	id m_mydocument;
};

@interface MyDocument : NSDocument
{
//	IBOutlet id main_view;
    IBOutlet id view;
	IBOutlet id play_button;
	IBOutlet id stop_button;
	IBOutlet id pause_button;
	IBOutlet id ask_url_panel;
	IBOutlet id url_field;
	IBOutlet id status_line;
//    void *window_factory;
	mainloop *myMainloop;
#ifdef WITH_OVERLAY_WINDOW
	mainloop *myAuxMainloop;
	MyAmbulantView *myAuxView;
#ifndef WITH_OVERLAY_WINDOW_IN_VIEW
	NSWindow *myAuxWindow;
#endif
#endif
	NSTimer *uitimer;
	document_embedder *embedder;
	NSWindow *saved_window;
	NSRect saved_view_rect;
}
- (void)askForURL: (id)sender;
- (IBAction)closeURLPanel:(id)sender;
- (void)askForURLDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo;
- (void)openTheDocument;
- (BOOL) validateUIItem:(id)UIItem;
- (BOOL) validateMenuItem:(id)menuItem;
- (void) validateButtons:(id)dummy;
- (IBAction)pause:(id)sender;
- (IBAction)play:(id)sender;
- (IBAction)stop:(id)sender;
- (void *)view;
- (void)startPlay: (id)dummy;
- (void)close;
- (void)close: (id)dummy;
- (void)fixMouse: (id)dummy;
- (void)resetMouse: (id)dummy;
- (void)keyDown: (NSEvent *)ev;
- (void) setStatusLine: (NSString *)message;
- (IBAction)goWindowMode:(id)sender;
- (void)_goWindowMode;
- (IBAction)goFullScreen:(id)sender;
- (IBAction)toggleFullScreen:(id)sender;
#ifdef WITH_OVERLAY_WINDOW
- (BOOL)openAuxDocument: (NSURL *)auxUrl;
- (void)closeAuxDocument;
#endif

@end
