#import "TOpenGLView.h"
#include <OpenGL/gl.h>
#include <SoyMath.h>

namespace Opengl
{
	void Clear(Soy::TRgb Rgb)
	{
		glClearColor( Rgb.r(), Rgb.g(), Rgb.b(), 1 );
		glClear(GL_COLOR_BUFFER_BIT);
	}
}


TOpenglView::TOpenglView(vec2f Position,vec2f Size) :
	mView			( nullptr ),
	mRenderTarget	( "osx gl view" ),
	mContext		( *this )
{
	//	gr: for device-specific render choices..
	//	https://developer.apple.com/library/mac/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_pixelformats/opengl_pixelformats.html#//apple_ref/doc/uid/TP40001987-CH214-SW9
	//	make "pixelformat" (context params)
	NSOpenGLPixelFormatAttribute attrs[] =
	{
		NSOpenGLPFADoubleBuffer,
		0
	};
	NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	Soy::Assert( pixelFormat, "Failed to create pixel format" );
 
	NSRect viewRect = NSMakeRect( Position.x, Position.y, Size.x, Size.y );
	mView = [[MacOpenglView alloc] initFrameWithParent:this viewRect:viewRect pixelFormat:pixelFormat];
	[mView retain];
	Soy::Assert( mView, "Failed to create view" );
}

TOpenglView::~TOpenglView()
{
	[mView release];
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
	//	render callback, probbaly on main thread
	
	GLint RenderBufferName = -1;
	glGetIntegerv( GL_RENDERBUFFER_BINDING, &RenderBufferName );
	Opengl::IsOkay("glget GL_RENDERBUFFER_BINDING");
	std::Debug <<"Render buffer name is " << RenderBufferName << std::endl;
	
	//	do parent's minimal render
	auto ParentRender = [self]()
	{
		if ( !mParent )
		{
			Opengl::Clear( Soy::TRgb(1,0,0) );
		}
		else
		{
			//	gr: don't really wanna send the context here I don't think.... probably wanna send render target
			Opengl::Clear( Soy::TRgb(0,1,0) );
			mParent->mOnRender.OnTriggered( mParent->mRenderTarget );
		}
		return true;
	};
	
	auto BufferFlip = [self]()
	{
		//	swap OSX buffers - required with os double buffering (NSOpenGLPFADoubleBuffer)
		[[self openGLContext] flushBuffer];
		return true;
	};

	
	//	Run through our contexts jobs, then do a render and a flip, by queueing correctly.
	
	mParent->mContext.PushJob( ParentRender );
	//	flush all commands before another thread uses this context.... maybe put this in context unlock()
	mParent->mContext.PushJob( []{glFlush();return true;} );
	mParent->mContext.PushJob( BufferFlip );
	mParent->mContext.Iteration();
}

@end



vec2x<GLint> GlViewRenderTarget::GetSize()
{
	return vec2x<GLint>(100,100);
	//	gr: don't know what renderer is :/
	vec2x<GLint> wh(0,0);
	glGetRenderbufferParameteriv( GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &wh.x );
	Opengl_IsOkay();
	glGetRenderbufferParameteriv( GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &wh.y );
	Opengl_IsOkay();
	return wh;
}


bool GlViewContext::Lock()
{
	if ( !mParent.mView )
		return false;
	
	[mParent.mView.openGLContext makeCurrentContext];
	return true;
}

void GlViewContext::Unlock()
{
	//	leaves artifacts everywhere
	//[mParent.mView.openGLContext flushBuffer];
}

