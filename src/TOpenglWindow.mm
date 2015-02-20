#include "PopOpengl.h"
#include <Cocoa/Cocoa.h>
#include "TOpenglWindow.h"
#include "TOpenglView.h"



class TWindowWrapper
{
public:
	TWindowWrapper() :
		mWindow	( nullptr ),
		mView	( nullptr )
	{
	}
	~TWindowWrapper()
	{
		[mWindow release];
		[mView release];
	}
	
	bool			IsValid()	{	return mWindow && mView;	}

public:
	NSWindow*		mWindow;
	MyOpenGLView*	mView;
};



TOpenglWindow::TOpenglWindow(const std::string& Name,vec2f Pos,vec2f Size,std::stringstream& Error) :
	mName	( Name )
{
	//	gr; check we have an NSApplication initalised here and fail if running as command line app

	mWrapper.reset( new TWindowWrapper );
	auto& Wrapper = *mWrapper;
	auto*& mWindow = Wrapper.mWindow;
	auto*& mView = Wrapper.mView;

/*
	NSRect frame = NSMakeRect(0, 0, 200, 200);
	NSWindow* window  = [[NSWindow alloc] initWithContentRect:frame
													 styleMask:NSBorderlessWindowMask
													   backing:NSBackingStoreBuffered
														 defer:NO];

	
	[window retain];
	[window setBackgroundColor:[NSColor blueColor]];
	[window makeKeyAndOrderFront:NSApp];

	
	*/
	
	//NSUInteger styleMask =    NSBorderlessWindowMask;
	NSUInteger Style = NSTitledWindowMask|NSClosableWindowMask|NSResizableWindowMask;
	//NSUInteger Style = NSBorderlessWindowMask;
	NSRect FrameRect = NSMakeRect( Pos.x, Pos.y, Size.x, Size.y );
	NSRect WindowRect = [NSWindow contentRectForFrameRect:FrameRect styleMask:Style];
//	NSRect WindowRect = NSMakeRect( Pos.x, Pos.y, Size.x, Size.y );
//	NSRect WindowRect = [[NSScreen mainScreen] frame];
	
	bool Defer = NO;
	mWindow = [[NSWindow alloc] initWithContentRect:WindowRect styleMask:Style backing:NSBackingStoreBuffered defer:Defer];
	[mWindow retain];

	if ( !mWindow )
	{
		Error << "failed to create window";
		return;
	}

	//	setup window
//	[Window setLevel:NSMainMenuWindowLevel+1];
//	[Window setOpaque:YES];
//	[Window setHidesOnDeactivate:YES];

	//	make "pixelformat" (context params)
	NSOpenGLPixelFormatAttribute attrs[] =
	{
		NSOpenGLPFADoubleBuffer,
		0
	};
	NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
 
	NSRect viewRect = NSMakeRect(0.0, 0.0, WindowRect.size.width, WindowRect.size.height);
	mView = [[MyOpenGLView alloc] initWithFrame:viewRect pixelFormat: pixelFormat];
	[mView retain];
	if ( !mView )
	{
		Error << "Failed to create view";
		return;
	}
	
	//	assign view to window
	[mWindow setContentView: mView];

	id Sender = NSApp;
	[mWindow setBackgroundColor:[NSColor blueColor]];
	[mWindow makeKeyAndOrderFront:Sender];
	
}

TOpenglWindow::~TOpenglWindow()
{
}
	
bool TOpenglWindow::IsValid()
{
	return mWrapper && mWrapper->IsValid();
}
