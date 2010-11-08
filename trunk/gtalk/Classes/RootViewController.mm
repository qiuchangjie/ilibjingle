/*
 * Copyright 2010, CIWAV Inc. www.ciwav.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
//
//  RootViewController.m
//  gtalk
//
//  Copyright CIWAV Inc. 2010. All rights reserved.
//

#import "RootViewController.h"

#include <time.h>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <vector>
#include "talk/base/logging.h"
#include "talk/base/flags.h"
#include "talk/base/pathutils.h"
#include "talk/base/stream.h"
#include "talk/base/ssladapter.h"
#include "talk/base/win32socketserver.h"
#include "talk/xmpp/xmppclientsettings.h"
#include "talk/examples/login/xmppthread.h"
#include "talk/examples/login/xmppauth.h"
#include "talk/examples/login/xmpppump.h"
#include "talk/examples/iphone/gtalkclient.h"

static const int DEFAULT_PORT = 5222;

@implementation RootViewController

@synthesize roster_;
@synthesize tableView_;

#pragma mark -
#pragma mark View lifecycle

- (void)loadView
{
	NSLog(@"loadView");
	
	self.title = @"GTalk";
	
	roster_ = [[NSMutableArray alloc] initWithObjects:nil ];
	
	tableView_ = [[UITableView alloc] initWithFrame:[[UIScreen mainScreen] applicationFrame] style:UITableViewStylePlain];
	tableView_.autoresizingMask = UIViewAutoresizingFlexibleHeight|UIViewAutoresizingFlexibleWidth;
	tableView_.delegate = self; 
	tableView_.dataSource = self; 
	[tableView_ reloadData];
	self.view = tableView_; 
}

- (void)viewDidLoad {
	NSLog(@"viewDidLoad");
	
    [super viewDidLoad];
	
	[NSThread detachNewThreadSelector:@selector(_mainGtalkThread:)
							 toTarget:self withObject:nil];
	
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}

- (void)_mainGtalkThread:(id)sender
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	//you need to setup name and passwd manuelly here
	//[self gtalk_main:(char*)name userpassword:(char*)password];
    [pool release];
}

- (void)viewWillAppear:(BOOL)animated {
	NSLog(@"viewWillAppear");
    [super viewWillAppear:animated];
}

- (void)viewDidAppear:(BOOL)animated {
	NSLog(@"viewDidAppear");
    [super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated {
	NSLog(@"viewWillDisappear");
	[super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated {
	NSLog(@"viewDidDisappear");
	[super viewDidDisappear:animated];
}

/*
 // Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	// Return YES for supported orientations.
	return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
 */

- (void)do_reloadTableViewData {
	[self.tableView_ reloadData];
}

- (void)reloadTableViewData {
	[self performSelectorOnMainThread:@selector(do_reloadTableViewData) 
						withObject:nil 
						waitUntilDone:false];
}

#pragma mark -
#pragma mark Table view data source

// Customize the number of sections in the table view.
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    // only one section now
	return 1;
}


// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [roster_ count];
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
	static NSString *CellIdentifier = @"Cell";
    //static NSString *PlaceholderCellIdentifier = @"PlaceholderCell";
	//buzz::Jid callto_jid(name);
	
	//int nodeCount = [roster_ count];
	
	/*if (nodeCount == 0 && indexPath.row == 0)
	{
        UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:PlaceholderCellIdentifier];
        if (cell == nil)
		{
            cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle
										   reuseIdentifier:PlaceholderCellIdentifier] autorelease];   
            //cell.detailTextLabel.textAlignment = UITextAlignmentCenter;
			//cell.selectionStyle = UITableViewCellSelectionStyleNone;
        }
		
		cell.detailTextLabel.text = @"xxx";
		return cell;
    }*/
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        //cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
		cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:CellIdentifier] autorelease];
    }
	// Configure the cell.
	cell.textLabel.text = [roster_ objectAtIndex:indexPath.row];
	cell.detailTextLabel.text = @"detailed";
    return cell;
}


/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/


/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source.
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationFade];
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view.
    }   
}
*/


/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/


/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/


#pragma mark -
#pragma mark Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    
	/*
	 <#DetailViewController#> *detailViewController = [[<#DetailViewController#> alloc] initWithNibName:@"<#Nib name#>" bundle:nil];
     // ...
     // Pass the selected object to the new view controller.
	 [self.navigationController pushViewController:detailViewController animated:YES];
	 [detailViewController release];
	 */
	//NSString* message = [roster_ objectAtIndex:indexPath.row];
	//UIAlertView* alert = [[[UIAlertView alloc] init] autorelease];
	//alert.message = message;
	//[alert addButtonWithTitle:@"OK"];
	//[alert show];
	
	
}


#pragma mark -
#pragma mark Memory management

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Relinquish ownership any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    // Relinquish ownership of anything that can be recreated in viewDidLoad or on demand.
    // For example: self.myOutlet = nil;
}


- (void)dealloc {
	[roster_ release];
	[tableView_ release];
    [super dealloc];
}
/*
- (id)init {
	if ( (self = [super initWithStyle:UITableViewStylePlain]) ) {
		NSLog(@"init");
		
		self.title = @"GTalk";
		roster_ = [[NSMutableArray alloc] initWithObjects:
                  @"user1",
                  @"user2",
                  nil ];
	}
	return self;
}
*/
#pragma mark -
#pragma mark gtalk implementation

- (int) gtalk_main:(char*)un userpassword:(char*)password 
{
	// This app has three threads. The main thread will run the XMPP client,
	// which will print to the screen in its own thread. A second thread
	// will get input from the console, parse it, and pass the appropriate
	// message back to the XMPP client's thread. A third thread is used
	// by MediaSessionClient as its worker thread.
	
	buzz::Jid jid;
	talk_base::InsecureCryptStringImpl pass;
	
	std::string username = un;
	if (username.find('@') == std::string::npos) {
		username.append("@localhost");
	}
	jid = buzz::Jid(username);
	if (!jid.IsValid() || jid.node() == "") {
		printf("Invalid JID. JIDs should be in the form user@domain\n");
		return 1;
	}
	pass.password() = password;
	
	buzz::XmppClientSettings xcs;
	xcs.set_user(jid.node());
	xcs.set_resource("call");
	xcs.set_host(jid.domain());
	xcs.set_pass(talk_base::CryptString(pass));
	xcs.set_server(talk_base::SocketAddress("talk.google.com", DEFAULT_PORT/*host, port*/));
	//printf("Logging in to %s as %s\n", server.c_str(), jid.Str().c_str());
	
	talk_base::InitializeSSL();
	
#if WIN32
	// Need to pump messages on our main thread on Windows.
	talk_base::Win32Thread w32_thread;
	talk_base::ThreadManager::SetCurrent(&w32_thread);
#endif
	talk_base::Thread athread;
	talk_base::ThreadManager::SetCurrent(&athread);
	
	talk_base::Thread* main_thread = talk_base::Thread::Current();
	assert(main_thread!=NULL);
	
	XmppPump pump;
	//CallClient *client = new CallClient(pump.client());
	gtalkClient_ = new gtalkClient(pump.client(), self);
	
	pump.DoLogin(xcs, new XmppSocket(true), NULL);
	main_thread->Run();
	
	// stop it
	//pump.DoDisconnect();
	
	//console->Stop();
	//delete console;
	//delete client;
	
	return 0;
}
@end

