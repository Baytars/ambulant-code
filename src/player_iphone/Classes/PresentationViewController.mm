//
//  PresentationViewController.m
//  PresentationView
//
//  Created by Kees Blom on 10/31/10.
//  Copyright Stg.CWI 2010. All rights reserved.
//

#import "PresentationViewController.h"

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

@implementation PresentationViewController
/*
// The designated initializer. Override to perform setup that is required before the view is loaded.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
}
*/

- (Presentation*) getPresentationFromPlaylistItem: (PlaylistItem*) item {
	Presentation* aPresentation = [ [ Presentation alloc ] init ];
	if (item != NULL) {
		aPresentation.title = [item ns_title];
		aPresentation.poster = [item cg_image];
		aPresentation.duration = [item ns_dur];
		aPresentation.description = [item ns_description];

	}
	return aPresentation;
}

- (NSArray*) get_playlist {
	NSArray* playlist;
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();

	if (isFavorites) {
		playlist = prefs->m_favorites->get_playlist();
		
	} else {
		playlist = prefs->m_history->get_playlist();
	}
	return playlist;
}

- (BOOL)
isFavorites {
	return isFavorites;
}

- (void) awakeFromNib
{
	AM_DBG NSLog(@"PresentationViewController awakeFromNib(0x%x)", self);
	if (presentationsArray == NULL) {
		presentationsArray = [ [ NSMutableArray alloc ] init ];
	}
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	prefs->load_preferences();
	BOOL favorites = [self.title isEqualToString:@"Favorites"];
	isFavorites = favorites;
	currentIndex = -1; //XXXX should ths be saved in properties ??
	if ( !isFavorites) {
		[delegate setHistoryViewController: self];
	}
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void) viewDidLoad
{
    [super viewDidLoad];
	AM_DBG NSLog(@"PresentationViewController viewDidLoad(0x%x)", self);
	self.tableView.rowHeight = 60;
	if (presentationsArray == NULL) {
		presentationsArray = [ [ NSMutableArray alloc ] init ];
	}
	[self updatePlaylist];
}

- (void) viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
}

- (IBAction) done:(id)sender
{
	AM_DBG NSLog(@"PresentationViewController done(0x%x)", self);
	[delegate auxViewControllerDidFinish:self];
}

// Overriden to allow orientations other than the default portrait orientation.
- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return [delegate canShowRotatedUIViews];
}


- (void) didReceiveMemoryWarning
{
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void) viewDidUnload
{
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}

// Customize the number of rows in the table view.
- (NSInteger) tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	AM_DBG NSLog(@"tableView:0x%x numberOfRowsInSection(0x%x) section=%d", self, section);
    return [ presentationsArray count ];
}

// Customize the appearance of table view cells.
- (UITableViewCell *) tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
//      cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
		[ [ NSBundle mainBundle ] loadNibNamed: @"PresentationTableCell" owner: self options: NULL ];
		cell = nibLoadedCell;
    }
    
	// Configure the cell.
	Presentation* aPresentation = [ presentationsArray objectAtIndex: indexPath.row ];
	UIImageView* posterView = (UIImageView*) [ cell viewWithTag:5]; // tags are assigned in the nib
	posterView.contentMode = UIViewContentModeScaleAspectFit;
	posterView.image = [UIImage imageWithCGImage:(CGImageRef) aPresentation.poster];
	[posterView setNeedsDisplay];
	UILabel* label = (UILabel*) [ cell viewWithTag: 1];
	label.text = aPresentation.title;
	label = (UILabel*) [ cell viewWithTag: 2];
	label.text = aPresentation.duration;
//	[ duration release ];
	label = (UILabel*) [ cell viewWithTag: 3];
	label.text = aPresentation.description;
#ifdef	FIRST_ITEM
	if (indexPath.row < FIRST_ITEM) {
		UIImageView* lineView = (UIImageView*) [ cell viewWithTag:4];
		lineView.image = NULL;
		[lineView setNeedsDisplay];
	}
#endif//FIRST_ITEM
	return cell;
}

// Support row selection in the table view.
- (void) tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if (editingStyle != UITableViewCellEditingStyleNone) {
		return;
	}
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	NSArray* playlist = isFavorites ? prefs->m_favorites->get_playlist() : prefs->m_history->get_playlist();
	NSUInteger playlistIndex = indexPath.row;
	currentIndex = playlistIndex;
#ifdef	FIRST_ITEM
	if (playlistIndex >= FIRST_ITEM) {
		playlistIndex -= FIRST_ITEM;
	} else {
		playlistIndex = FIRST_ITEM;
	}
#endif//FIRST_ITEM
	PlaylistItem* selectedItem = [playlist objectAtIndex: playlistIndex];
	[delegate playPresentation:[[selectedItem ns_url] absoluteString] fromPresentationViewController: self];
}

// Support conditional editing of the table view.
- (BOOL) tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
#ifdef	FIRST_ITEM
	if (indexPath.row < FIRST_ITEM) {
		return NO;
	}
#endif//FIRST_ITEM
    return YES;
}

// Show editing style button
- (UITableViewCellEditingStyle) tableView:(UITableView *)tableView editingStyleForRowAtIndexPath:(NSIndexPath *)indexPath
{
	return editingStyle;
}

// Support editing the table view (deletion only, adding is automatic).
- (void) tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyleArg 
											 forRowAtIndexPath:(NSIndexPath *)indexPath
{    
	NSUInteger playlistIndex = indexPath.row;
#ifdef	FIRST_ITEM
	playlistIndex -= FIRST_ITEM;
#endif//FIRST_ITEM
    if (editingStyleArg == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source.
		ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
		ambulant::Playlist* playlist = isFavorites ? prefs->m_favorites : prefs->m_history;
		
		playlist->remove_playlist_item_at_index(playlistIndex);
		[presentationsArray removeObjectAtIndex: indexPath.row];
		[tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath]
						 withRowAnimation:UITableViewRowAnimationMiddle];
		prefs->save_preferences();
// correct currentIndex for this deletion s.t. selectNextPresentation will select the same presentation as before
		if (playlistIndex <= currentIndex) {
			currentIndex--;
		}
		if (currentIndex < -1) {
			currentIndex = -1;
		}
 	}   
    else if (editingStyleArg == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view.
		[self insertCurrentItemAtIndexPath: indexPath];
    } 
}

- (IBAction) toggleEditMode
{
	switch (editingStyle) {
		case UITableViewCellEditingStyleNone:
			editingStyle = UITableViewCellEditingStyleDelete;
			break;
		default:
			editingStyle = UITableViewCellEditingStyleNone;
			break;
	}
	[[self tableView] setEditing: editingStyle != UITableViewCellEditingStyleNone animated: YES];
}

// Support re-arranging table items (Favorites only)
- (BOOL) tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be moveable.
#ifdef	FIRST_ITEM
	if (indexPath.row < FIRST_ITEM) {
		return NO;
	}
#endif//FIRST_ITEM
	return isFavorites;
}

// Implement re-arranging table items (Favorites only)
- (void) tableView:(UITableView *)tableView moveRowAtIndexPath: (NSIndexPath*) fromIndexPath toIndexPath: (NSIndexPath*) toIndexPath
{
	NSUInteger fromPlaylistIndex = fromIndexPath.row, toPlaylistIndex = toIndexPath.row;
	NSLog(@"moveRowAtIndexPath: %d toIndexPath: %d", fromPlaylistIndex, toPlaylistIndex);
	if (fromPlaylistIndex == toPlaylistIndex) {
		return;
	}
#ifdef	FIRST_ITEM
	fromPlaylistIndex -= FIRST_ITEM;
	toPlaylistIndex -= FIRST_ITEM;
#endif//FIRST_ITEM
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	ambulant::Playlist* playlist = isFavorites ? prefs->m_favorites : prefs->m_history;
	PlaylistItem* selectedItem = [playlist->get_playlist() objectAtIndex: fromPlaylistIndex];
	playlist->insert_item_at_index(selectedItem, toPlaylistIndex);
	if (toPlaylistIndex > fromPlaylistIndex) {
		playlist->remove_playlist_item_at_index(fromPlaylistIndex);
	} else {
		playlist->remove_playlist_item_at_index(fromPlaylistIndex + 1);
	}
}

- (void) insertCurrentItemAtIndexPath: (NSIndexPath*) indexPath
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (isFavorites) {
		NSInteger playlistIndex = indexPath != NULL ? indexPath.row : -1;
#ifdef	FIRST_ITEM
		playlistIndex -= FIRST_ITEM;
#endif//FIRST_ITEM
		ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
		ambulant::Playlist* playlist = prefs->m_favorites;
		PlaylistItem* new_item = prefs->m_history->get_last_item();
		// Check if we have 'new_item' already in the playlist; if so ignore
//		NSLog(@"new_item.ns_url=0x%x: %@", new_item.ns_url, new_item.ns_url != NULL ? [new_item.ns_url absoluteString]:@"<nil>");
		BOOL found = NO;
		BOOL* found_ref = &found;
		NSArray* items = playlist->get_playlist();
		[items enumerateObjectsWithOptions: nil usingBlock:
		 ^(id obj, NSUInteger idx, BOOL *stop)
		 {
			 PlaylistItem* item = (PlaylistItem*) obj;
//			 NSLog(@"item.ns_url=0x%x: %@", item.ns_url, item.ns_url != NULL ? [item.ns_url absoluteString]:@"<nil>");
			 if ([new_item.ns_url isEqual: (id) item.ns_url]) {
				 *found_ref = YES;
			 }
		 }];
		if (found) {
			return;
		}
		playlist->insert_item_at_index(new_item, playlistIndex);
		newPresentation = [self getPresentationFromPlaylistItem: new_item];
		if (playlistIndex < 0 || [presentationsArray count] == 0) {
			[presentationsArray addObject: newPresentation] ;
			[self.tableView reloadData];
		} else {
			[presentationsArray insertObject: newPresentation atIndex: indexPath.row ];
			NSIndexPath* updatedPath = [ NSIndexPath indexPathForRow:indexPath.row inSection: 0 ];
			NSLog(@"updatedPath.row=%d",updatedPath.row);
			NSMutableArray* updatedPaths = [ [NSMutableArray alloc] init ];
			[updatedPaths addObject: updatedPath];
			[self.tableView insertRowsAtIndexPaths: updatedPaths withRowAnimation: UITableViewRowAnimationMiddle]; //UITableViewRowAnimationMiddle ];
			[updatedPaths release];
		}
	}
	[pool release];
}

- (void) updatePlaylist {

	NSArray* playlist = [self get_playlist];
	
#ifdef	FIRST_ITEM
	if ([playlist count] + FIRST_ITEM > [presentationsArray count]) {
#else //FIRST_ITEM
	if ([playlist count] > [presentationsArray count]) {
#endif//FIRST_ITEM
		currentIndex++; //assume insert at 0 occurred
	}
	[presentationsArray removeAllObjects];
#ifdef	FIRST_ITEM
	for (int i = 0; i < FIRST_ITEM; i++) {
		[presentationsArray addObject:[self getPresentationFromPlaylistItem: NULL]];
	}
#endif//FIRST_ITEM	
	// populate the table view with objects in 'playlist'
	[playlist enumerateObjectsWithOptions: nil usingBlock:
	 ^(id obj, NSUInteger idx, BOOL *stop)
	 {
		 PlaylistItem* item = (PlaylistItem*) obj;
		 Presentation* presentation = [self getPresentationFromPlaylistItem: item];
		 [presentationsArray addObject: presentation];
		 [presentation release]; // the array now has ownership
	}];
	[[self tableView] reloadData];
/*JNK
	self.tableView.frame.origin.y = 66;
	UITableViewCell* cell = [[self tableView] cellForRowAtIndexPath: [NSIndexPath indexPathForRow: 1 inSection:0]];
	cell.opaque = false;
	cell.hidden = true;
	cell.alpha = 0.0;*/
}

- (void) selectNextPresentation
{
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	NSArray* playlist = isFavorites ? prefs->m_favorites->get_playlist() : prefs->m_history->get_playlist();
	NSUInteger playlistIndex = ++currentIndex;
	if (currentIndex >= [playlist count]) {
		playlistIndex = currentIndex = [playlist count] - 1;
	}
	PlaylistItem* selectedItem = [playlist objectAtIndex: playlistIndex];
	[delegate playPresentation:[[selectedItem ns_url] absoluteString] fromPresentationViewController: self];
}
	
- (void) viewWillDisappear:(BOOL)animated
{
	if (editingStyle != UITableViewCellEditingStyleNone) {
		[self toggleEditMode];
	}
}

- (void) dealloc
{
    [super dealloc];
	[presentationsArray dealloc];
}

@end
