//
//  AmbulantWebView.mm
//  AmbulantWebKitPlugin
//
//  Created by Jack Jansen on 20-12-05.
//  Copyright 2005 __MyCompanyName__. All rights reserved.
//

#import "AmbulantWebView.h"
#import <WebKit/WebKit.h>

@implementation AmbulantWebView

- (void)drawRect:(NSRect)rect {
    // Drawing code here.
}

+ (NSView *)plugInViewWithArguments:(NSDictionary *)arguments
{
    AmbulantWebView *view = [[[self alloc] init] autorelease];
    [view setArguments:arguments];
    return view;
}

- (void)dealloc
{   
    [m_arguments release];
    [super dealloc];
}

- (void)setArguments:(NSDictionary *)arguments
{
    [arguments copy];
    [m_arguments release];
    m_arguments = arguments;
}

- (void)webPlugInInitialize
{
}

- (void)webPlugInStart
{
    if (!m_mainloop) {
        NSDictionary *webPluginAttributesObj = [m_arguments objectForKey:WebPlugInAttributesKey];
        NSString *URLString = [webPluginAttributesObj objectForKey:@"src"];
        if (URLString != nil && [URLString length] != 0) {
            NSURL *baseURL = [m_arguments objectForKey:WebPlugInBaseURLKey];
            NSURL *URL = [NSURL URLWithString:URLString relativeToURL:baseURL];
            m_mainloop = new mainloop([[URL absoluteString] UTF8String], NULL /*myWindowFactory*/, false, NULL /*embedder*/);			
        }
    }
	[self startPlayer];
}

- (void)webPlugInStop
{
	[self stopPlayer];
}

- (void)webPlugInDestroy
{
}

- (void)webPlugInSetIsSelected:(BOOL)isSelected
{
}

// Scripting support

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)selector
{
    if (selector == @selector(startPlayer) 
			|| selector == @selector(stopPlayer) 
			|| selector == @selector(restartPlayer)
			|| selector == @selector(pausePlayer) 
			|| selector == @selector(resumePlayer) 
		) {
        return NO;
    }
    return YES;
}

+ (BOOL)isKeyExcludedFromWebScript:(const char *)property
{
    return YES;
}

- (id)objectForWebScript
{
    return self;
}

- (void)startPlayer
{
}

- (void)stopPlayer
{
}

- (void)restartPlayer
{
}

- (void)pausePlayer
{
}

- (void)resumePlayer
{
}


@end
