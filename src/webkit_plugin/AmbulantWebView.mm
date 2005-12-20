//
//  AmbulantWebView.mm
//  AmbulantWebKitPlugin
//
//  Created by Jack Jansen on 20-12-05.
//  Copyright 2005 __MyCompanyName__. All rights reserved.
//

#import "AmbulantWebView.h"


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
#if 0
    if (!_loadedMovie) {
        _loadedMovie = YES;
        NSDictionary *webPluginAttributesObj = [_arguments objectForKey:WebPlugInAttributesKey];
        NSString *URLString = [webPluginAttributesObj objectForKey:@"src"];
        if (URLString != nil && [URLString length] != 0) {
            NSURL *baseURL = [_arguments objectForKey:WebPlugInBaseURLKey];
            NSURL *URL = [NSURL URLWithString:URLString relativeToURL:baseURL];
            NSMovie *movie = [[NSMovie alloc] initWithURL:URL byReference:NO];
            [self setMovie:movie];
            [movie release];
        }
    }
    
#endif
    [self startPlayer];
}

- (void)webPlugInStop
{
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
