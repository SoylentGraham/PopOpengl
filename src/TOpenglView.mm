#import "TOpenGLView.h"
#include <OpenGL/gl.h>


TOpenglView::TOpenglView(vec2f Position,vec2f Size,std::stringstream& Error) :
	mView	( nullptr )
{
	//	make "pixelformat" (context params)
	NSOpenGLPixelFormatAttribute attrs[] =
	{
		NSOpenGLPFADoubleBuffer,
		0
	};
	NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	if ( !pixelFormat )
	{
		Error << "Failed to create pixel format";
		return;
	}
 
	NSRect viewRect = NSMakeRect( Position.x, Position.y, Size.x, Size.y );
	mView = [[MacOpenglView alloc] initFrameWithParent:this viewRect:viewRect pixelFormat:pixelFormat];
	[mView retain];
	if ( !mView )
	{
		Error << "Failed to create view";
		return;
	}

}

TOpenglView::~TOpenglView()
{
	[mView release];
}

void RenderError(bool& Dummy)
{
	glClearColor(1,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT);
}


@implementation MacOpenglView


- (id)initFrameWithParent:(TOpenglView*)Parent viewRect:(NSRect)viewRect pixelFormat:(NSOpenGLPixelFormat*)pixelFormat;
{
	self = [super initWithFrame:viewRect pixelFormat: pixelFormat];
	if (self)
	{
		mParent = Parent;
	}
	return self;
}


-(void) drawRect: (NSRect) bounds
{
	//	render callback
	bool Dummy;
	if ( !mParent )
	{
		RenderError(Dummy);
	}
	else
	{
		mParent->mOnRender.OnTriggered( Dummy );
	}
	
	glFlush();
	
	//	swap OSX buffers - required with os double buffering (NSOpenGLPFADoubleBuffer)
	[[self openGLContext] flushBuffer];
}

@end

