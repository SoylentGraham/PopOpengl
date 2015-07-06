#pragma once


//	re-using unity opengl device interface
#include "UnityDevice.h"
#include "SoyOpengl.h"

class TOpenglWindow;

class TTextureWindow
{
public:
	TTextureWindow(std::string Name,vec2f Position,vec2f Size,std::stringstream& Error);
	
	bool		IsValid()			{	return mWindow!=nullptr;	}
	
	void		SetTexture(const SoyPixelsImpl& Pixels);
	void		OnOpenglRender(bool& Dummy);	//	render callback

private:
	GlProgram			mTextureCopyProgram;
	Unity::TTexture_Opengl		mTexture;
	std::shared_ptr<TUnityDevice_Opengl>	mDevice;	//	device for window's context
	std::shared_ptr<TOpenglWindow>	mWindow;
	ofMutexT<SoyPixels>	mPendingTexture;
};
