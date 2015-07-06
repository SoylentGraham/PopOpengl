#pragma once


//	re-using unity opengl device interface
#include "UnityDevice.h"
#include "SoyOpengl.h"

class TOpenglWindow;
class TPopOpengl;

namespace Opengl
{
	class TContext;
};

class TTextureWindow
{
public:
	TTextureWindow(std::string Name,vec2f Position,vec2f Size,TPopOpengl& Parent);
	~TTextureWindow();
	
	bool				IsValid()			{	return mWindow!=nullptr;	}
	
	void				OnOpenglRender(Opengl::TRenderTarget& RenderTarget);
	Opengl::TContext*	GetContext();
	
private:
	TPopOpengl&			mParent;
	GlProgram			mTextureCopyProgram;
	std::shared_ptr<TUnityDevice_Opengl>	mDevice;	//	device for window's context
	std::shared_ptr<TOpenglWindow>	mWindow;
	Opengl::TTexture	mTestTexture;
};
