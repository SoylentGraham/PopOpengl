#include "PopOpengl.h"
#import <Cocoa/Cocoa.h>


class TOpenglView;

@interface MacOpenglView : NSOpenGLView
{
	TOpenglView*	mParent;
}

- (id)initWithParent:(TOpenglView*)Parent;

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
	MacOpenglView*	mView;
};
