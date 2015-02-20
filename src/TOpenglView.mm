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
	mView = [[MacOpenglView alloc] initWithFrame:viewRect pixelFormat: pixelFormat];
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

static void drawAnObject ()
{
	glColor3f(1.0f, 0.85f, 0.35f);
	glBegin(GL_TRIANGLES);
	{
		glVertex3f(  0.0,  0.6, 0.0);
		glVertex3f( -0.2, -0.3, 0.0);
		glVertex3f(  0.2, -0.3 ,0.0);
	}
	glEnd();
}




@implementation MacOpenglView

- (id)initWithParent:(TOpenglView*)Parent
{
	self = [super init];
	if (self)
	{
		mParent = Parent;
	}
	return self;
}


-(void) drawRect: (NSRect) bounds
{
	std::Debug << "draw rect" << std::endl;
	
	glClearColor(0.8, 0.4, 0.3, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	drawAnObject();
	glFlush();
	
	//	swap OSX buffers - required with os double buffering (NSOpenGLPFADoubleBuffer)
	[[self openGLContext] flushBuffer];
}

@end

