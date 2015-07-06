#include "TTextureWindow.h"
#include "TOpenglWindow.h"
#include "SoyOpengl.h"
#include "PopOpengl.h"


TTextureWindow::TTextureWindow(std::string Name,vec2f Position,vec2f Size,TPopOpengl& Parent) :
	mParent		( Parent )
{
	mWindow.reset( new TOpenglWindow( Name, Position, Size ) );
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

TTextureWindow::~TTextureWindow()
{
	if ( mWindow )
	{
		mWindow->WaitToFinish();
		mWindow.reset();
	}
	
	mDevice.reset();
}


vec2f GetFrameBufferSize(GLint FrameBufferId)
{
	vec2f Error(200,200);

	
	auto BindScope = SoyScope( [FrameBufferId]{ glBindRenderbuffer( GL_RENDERBUFFER, FrameBufferId ); }, []{ glBindRenderbuffer( GL_RENDERBUFFER, 0 ); } );
	//	auto BindScope = SoyScope( [FrameBufferId]{ glBindRenderbuffer( GL_FRAMEBUFFER, FrameBufferId ); }, []{ glBindRenderbuffer( GL_FRAMEBUFFER, 0 ); } );
	if ( TUnityDevice_Opengl::HasError() )
		return Error;

	
	//	0 = screen
	//	glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,

	GLint FrameBufferObjectType;
	glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &FrameBufferObjectType );
	if ( TUnityDevice_Opengl::HasError() )
		return Error;
	
	
	if ( FrameBufferObjectType == GL_NONE )
		return Error;
	
	if ( FrameBufferObjectType == GL_FRAMEBUFFER_DEFAULT )
	{
		return Error;
	}
	
	if ( FrameBufferObjectType == GL_TEXTURE )
	{
		return Error;
	}

	if ( FrameBufferObjectType == GL_RENDERBUFFER )
	{
		GLint Width,Height;
		glGetRenderbufferParameteriv( GL_FRAMEBUFFER, GL_RENDERBUFFER_WIDTH, &Width );
		glGetRenderbufferParameteriv( GL_FRAMEBUFFER, GL_RENDERBUFFER_WIDTH, &Height );
		return vec2f( Width, Height );
	}

	Soy::Assert( false, std::stringstream() << "Unknown frame buffer object type " << FrameBufferObjectType );
	return Error;
}


void TTextureWindow::OnOpenglRender(Opengl::TRenderTarget& RenderTarget)
{
	std::Debug << __func__ << " at " << SoyTime(true) << std::endl;
	if ( !mDevice )
		return;

	mDevice->SetRenderThread();
	mDevice->OnRenderThreadUpdate();
	
	
	
	//	gr: move this to a simple job queuing
	/*
	//	new texture!
	if ( mPendingTexture.Get().IsValid() )
	{
		mPendingTexture.lock();
		
		//	alloc texture
		if ( !mTexture.IsValid() )
		{
			SoyPixelsMetaFull Meta( mPendingTexture.GetWidth(), mPendingTexture.GetHeight(), mPendingTexture.GetFormat() );
			mTexture = Unity::TTexture_Opengl( mDevice->AllocTexture( Meta ) );
		}
		mDevice->CopyTexture( mTexture, mPendingTexture, true, true );

		mPendingTexture.Clear();
		mPendingTexture.unlock();
	}
	 */
	
	/*
	//	load copy movie program
	if ( !mTextureCopyProgram.IsValid() )
	{
		auto& ErrorStream = std::Debug;
		mTextureCopyProgram = BuildProgram(
										"uniform highp mat4 Mvpm;\n"
										"attribute vec4 Position;\n"
										"attribute vec2 TexCoord;\n"
										"varying  highp vec2 oTexCoord;\n"
										"void main()\n"
										"{\n"
										"   gl_Position = Position;\n"
										"   oTexCoord = TexCoord;\n"
										"}\n"
										,
										"#extension GL_OES_EGL_image_external : require\n"
										"uniform samplerExternalOES Texture0;\n"
										"varying highp vec2 oTexCoord;\n"
										"void main()\n"
										"{\n"
										"	gl_FragColor = texture2D( Texture0, oTexCoord );\n"
										"}\n",
										   ErrorStream
										);

	}
	*/

	auto FrameBufferSize = RenderTarget.GetSize();
	
	std::Debug << "Frame buffer size: " << FrameBufferSize.x << "x" << FrameBufferSize.y << std::endl;
	
	//	set viewport (scissor so we see the real area)
	glViewport( 0, 0, FrameBufferSize.x, FrameBufferSize.y );
	//glScissor( 0, 0, FrameBufferSize.x, FrameBufferSize.y );
	glClearColor( 0, 0, 1, 1 );
	glClear(GL_COLOR_BUFFER_BIT);

	
	
	if ( mTextureCopyProgram.IsValid() )
	{
		glUseProgram( mTextureCopyProgram.program );
//	UnitSquare.Draw();
		glUseProgram( 0 );
	}

	
	//	render all render target textures
	Soy::Rectf Rect( 0,0,1,1 );
	float z = 0;
	for ( int rt=0;	rt<mParent.mRenderTargets.GetSize();	rt++ )
	{
		auto& RenderTarget = *mParent.mRenderTargets[rt];
		auto Texture = RenderTarget.GetTexture();
		if ( !Texture.IsValid() )
			continue;

		glColor3f(1,1,1);
		if ( Texture.Bind() )
		{
			glBegin(GL_QUADS);
			{
				glVertex3f(  Rect.Left(), Rect.Top(), z );
				glTexCoord2f(  0.0,  0.0	);
				
				glVertex3f( Rect.Right(), Rect.Top(), z	);
				glTexCoord2f(  1.0,  0.0	);
				
				glVertex3f( Rect.Right(), Rect.Bottom(), z	);
				glTexCoord2f(  1.0,  1.0	);

				glVertex3f( Rect.Left(), Rect.Bottom(), z	);
				glTexCoord2f(  0.0,  1.0	);
			}
			Texture.Unbind();
			glEnd();
		}
		
		Rect.y += Rect.h;
	}
	
	/*
	{
		glColor3f(1.0f, 1.f, 0.f);
		glBegin(GL_TRIANGLES);
		{
			glVertex3f(  0.0,  0.6, 0.0);
			glVertex3f( -0.2, -0.3, 0.0);
			glVertex3f(  0.2, -0.3 ,0.0);
		}
		glEnd();
	}
	 */
}

Opengl::TContext* TTextureWindow::GetContext()
{
	if ( !mWindow )
		return nullptr;
	
	return mWindow->GetContext();
}
