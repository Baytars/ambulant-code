//
//  MyDocument.m
//  cocoambulant
//
//  Created by Jack Jansen on Thu Sep 04 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//

#import "MyDocument.h"
#include "mainloop.h"

@implementation MyDocument

- (id)init
{
    self = [super init];
    if (self) {
        // Add your subclass-specific initialization here.
        // If an error occurs here, send a [self release] message and return nil.
    
    }
    return self;
}

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"MyDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
}

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
    // Insert code here to write your document from the given data.  You can also choose to override -fileWrapperRepresentationOfType: or -writeToFile:ofType: instead.
    return nil;
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
    // Insert code here to read your document from the given data.  You can also choose to override -loadFileWrapperRepresentation:ofType: or -readFromFile:ofType: instead.
    return YES;
}

- (IBAction)pause:(id)sender
{
    NSLog(@"Pause");
}

- (IBAction)play:(id)sender
{
    [NSThread detachNewThreadSelector: @selector(startPlay:) toTarget: self withObject: NULL];
}

- (void)startPlay: (id)dummy
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSString *filename = [self fileName];
    
    if (![NSThread isMultiThreaded]) {
        NSLog(@"startPlay: still not multi-threaded!");
    }
    NSLog(@"startPlay, self=0x%x, view=0x%x\n", self, view);
    my_cocoa_window_factory *wf = new my_cocoa_window_factory((void *)self);
    mainloop *ml = new mainloop();
    ml->run([filename UTF8String], (ambulant::lib::window_factory *)wf);
    [pool release];
}

- (IBAction)stop:(id)sender
{
    NSLog(@"Stop");
}

- (void *)view
{
    return view;
}
@end


void
my_cocoa_passive_window::need_redraw(const ambulant::lib::screen_rect<int> &r)
{
	ambulant::lib::logger::get_logger()->trace("my_cocoa_passive_window::need_redraw(0x%x)", (void *)this);
	if (!m_os_window) {
		ambulant::lib::logger::get_logger()->trace("my_cocoa_passive_window::need_redraw: no os_window");
		return;
	}
	NSView *my_view = (NSView *)[(MyDocument *)m_os_window view];
        // XXXX Should use setNeedsDisplayInRect:
	[my_view setNeedsDisplay: YES];
}


ambulant::lib::passive_window *
my_cocoa_window_factory::new_window(const std::string &name, ambulant::lib::size bounds)
{
    ambulant::lib::logger::get_logger()->trace("my_cocoa_window_factory: return window for 0x%x", m_os_window);
    ambulant::lib::passive_window *window = (ambulant::lib::passive_window *)new my_cocoa_passive_window(name, bounds, m_os_window);
    NSView *my_view = (NSView *)[(MyDocument *)m_os_window view];
    [my_view setAmbulantWindow: window];
    return window;
}


