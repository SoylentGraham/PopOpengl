#include "TTextureWindow.h"
#include "TOpenglWindow.h"


TTextureWindow::TTextureWindow(std::string Name,vec2f Position,vec2f Size,std::stringstream& Error)
{
	mWindow.reset( new TOpenglWindow( Name, Position, Size, Error ) );
	if ( !mWindow->IsValid() )
	{
		mWindow.reset();
		return;
	}
	
	//	create device
	//	gr: assume this will need a context at some point
	mDevice.reset( new TUnityDevice_Opengl );

	mWindow->mOnRender.AddListener(*this,&TTextureWindow::OnOpenglRender);
}
	
void TTextureWindow::SetTexture(const SoyPixelsImpl& Pixels)
{
	//	buffer for next render to copy
	mPendingTexture.lock();
	mPendingTexture.Copy( Pixels );
	mPendingTexture.unlock();
}

void TTextureWindow::OnOpenglRender(bool& Dummy)
{
	if ( !mDevice )
		return;
	mDevice->SetRenderThread();
	mDevice->OnRenderThreadUpdate();
	
	//	new texture!
	if ( mPendingTexture.Get().IsValid() )
	{
		mPendingTexture.lock();
		
		//	alloc texture
		if ( !mTexture.IsValid() )
		{
			SoyPixelsMetaFull Meta( mPendingTexture.GetWidth(), mPendingTexture.GetHeight(), mPendingTexture.GetFormat() );
			mTexture = mDevice->AllocTexture( Meta );
		}
		mDevice->CopyTexture( mTexture, mPendingTexture, true, true );

		mPendingTexture.Clear();
		mPendingTexture.unlock();
	}
	
	//	render texture to fbo
	
	glClearColor(0.8, 0.4, 0.3, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0f, 0.85f, 0.35f);
	glBegin(GL_TRIANGLES);
	{
		glVertex3f(  0.0,  0.6, 0.0);
		glVertex3f( -0.2, -0.3, 0.0);
		glVertex3f(  0.2, -0.3 ,0.0);
	}
	glEnd();
}

