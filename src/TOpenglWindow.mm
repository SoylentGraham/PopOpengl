#include "PopOpengl.h"
#include <Cocoa/Cocoa.h>
#include "TOpenglWindow.h"
#include "TOpenglView.h"
#include "PopMain.h"


class MacWindow
{
public:
	MacWindow() :
		mWindow	( nullptr )
	{
	}
	~MacWindow()
	{
		[mWindow release];
	}
	
	bool			IsValid()	{	return mWindow;	}

public:
	NSWindow*		mWindow;
};



TOpenglWindow::TOpenglWindow(const std::string& Name,vec2f Pos,vec2f Size,std::stringstream& Error) :
	mName	( Name )
{
	//	gr; check we have an NSApplication initalised here and fail if running as command line app
#if !defined(TARGET_OSX_BUNDLE)
	{
		Error << "Cannot create windows in non-bundle apps, I don't think." << std::endl;
		return;
	}
#endif

	if ( !Soy::Platform::BundleInitialised )
	{
		Error << "NSApplication hasn't been started. Cannot create window" << std::endl;
		return;
	}
	
	mMacWindow.reset( new MacWindow );
	auto& Wrapper = *mMacWindow;
	auto*& mWindow = Wrapper.mWindow;

	
	
	
	//NSUInteger styleMask =    NSBorderlessWindowMask;
	NSUInteger Style = NSTitledWindowMask|NSClosableWindowMask|NSResizableWindowMask;
	//NSUInteger Style = NSBorderlessWindowMask;
	NSRect FrameRect = NSMakeRect( Pos.x, Pos.y, Size.x, Size.y );
	NSRect WindowRect = [NSWindow contentRectForFrameRect:FrameRect styleMask:Style];
//	NSRect WindowRect = NSMakeRect( Pos.x, Pos.y, Size.x, Size.y );
//	NSRect WindowRect = [[NSScreen mainScreen] frame];

	//	create a view
	std::stringstream ViewError;
	mView.reset( new TOpenglView( vec2f(FrameRect.origin.x,FrameRect.origin.y), vec2f(FrameRect.size.width,FrameRect.size.height), ViewError ) );
	if ( !mView->IsValid() )
	{
		mView.reset();
		Error << "Failed to create view: " << ViewError.str();
		return;
	}
	mOnRenderListener = mView->mOnRender.AddListener( *this, &TOpenglWindow::OnViewRender );

	
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

	
	//	assign view to window
	[mWindow setContentView: mView->mView];

	id Sender = NSApp;
	[mWindow setBackgroundColor:[NSColor blueColor]];
	[mWindow makeKeyAndOrderFront:Sender];
	
}

TOpenglWindow::~TOpenglWindow()
{
}
	
bool TOpenglWindow::IsValid()
{
	return mMacWindow && mMacWindow->IsValid() && mView && mView->IsValid();
}
