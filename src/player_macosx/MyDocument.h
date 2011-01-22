/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2010 Stichting CWI,
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

// MyDocument also implements part of thhe NSWindowDelegate protocol, but how do I state that if
// I also need to inherit NSDocument??
@interface MyDocument : NSDocument
{
    IBOutlet NSView* scaler_view;
	IBOutlet NSView* view;
	IBOutlet NSButton* play_button;
	IBOutlet NSButton* stop_button;
	IBOutlet NSButton* pause_button;
    IBOutlet NSView* hud_controls;
	IBOutlet NSButton* play_button_2;
	IBOutlet NSButton* stop_button_2;
	IBOutlet NSButton* pause_button_2;
	IBOutlet NSPanel* ask_url_panel;
	IBOutlet NSTextField* url_field;
	IBOutlet NSTextField* status_line;
//	void *window_factory;
	mainloop *myMainloop;
#ifdef WITH_OVERLAY_WINDOW
	mainloop *myAuxMainloop;
	MyAmbulantView *myAuxView;
	NSWindow *myAuxWindow;
#endif
	NSTimer *uitimer;
	document_embedder *embedder;
	NSWindow *saved_window;
	NSRect saved_view_rect;
}
- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError;
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
- (void)showWindows;
#ifdef WITH_OVERLAY_WINDOW
- (BOOL)openAuxDocument: (NSURL *)auxUrl;
- (void)closeAuxDocument;
#endif

// Window delegate method:
- (void)windowDidChangeScreen: (NSNotification *)notification;
@end

@interface NSDocumentController(MyDocumentControllerCategory)
- (NSString *)typeForContentsOfURL:(NSURL *)inAbsoluteURL error:(NSError **)outError;
@end
