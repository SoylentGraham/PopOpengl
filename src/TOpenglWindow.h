#pragma once

#include <SoyTypes.h>


#if __has_feature(objc_arc)
#error expected ARC off, if we NEED arc, then the NSWindow & view need to go in a pure obj-c wrapper to auto retain the refcounted object
#endif


class MacWindow;
class TOpenglView;

class TOpenglWindow
{
public:
	TOpenglWindow(const std::string& Name,vec2f Pos,vec2f Size,std::stringstream& Error);
	~TOpenglWindow();
	
	bool			IsValid();

private:
	void			OnViewRender(bool& Dummy)
	{
		mOnRender.OnTriggered(Dummy);
	}
	
public:
	SoyEvent<bool>	mOnRender;			//	for now we have a 1:1 view window connection
	SoyListenerId	mOnRenderListener;
	
	std::string		mName;
	std::shared_ptr<TOpenglView>	mView;

private:
	std::shared_ptr<MacWindow>		mMacWindow;
};

