#include "PopOpengl.h"
#import <Cocoa/Cocoa.h>


class TOpenglView;

@interface MacOpenglView : NSOpenGLView
{
	TOpenglView*	mParent;
}

- (id)initFrameWithParent:(TOpenglView*)Parent viewRect:(NSRect)viewRect pixelFormat:(NSOpenGLPixelFormat*)pixelFormat;


//	overloaded
- (void) drawRect: (NSRect) bounds;
@end


class TOpenglView
{
public:
	TOpenglView(vec2f Position,vec2f Size,std::stringstream& Error);
	~TOpenglView();
	
	bool			IsValid()	{	return mView != nullptr;	}
	
public:
	SoyEvent<bool>	mOnRender;
	MacOpenglView*	mView;
};
