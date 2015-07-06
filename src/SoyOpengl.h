#pragma once

#include <ostream>
#include <SoyEvent.h>
#include <SoyMath.h>


//	opengl stuff
#if defined(TARGET_ANDROID)

#define OPENGL_ES_3	//	need 3 for FBO's
//#define GL_NONE				GL_NO_ERROR	//	declared in GLES3
#define GL_ASSET_INVALID	0

#if defined(OPENGL_ES_3)
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES/glext.h>	//	need for EOS
static const int GL_ES_VERSION = 3;	// This will be passed to EglSetup() by App.cpp
#endif


#if defined(OPENGL_ES_2)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES/glext.h>	//	need for EOS
static const int GL_ES_VERSION = 2;	// This will be passed to EglSetup() by App.cpp
#endif

#endif

#if defined(TARGET_OSX)
#include <Opengl/gl.h>
#include <OpenGL/OpenGL.h>

#endif





namespace Opengl
{
	class TContext;			//	opengl context abstraction - contexts are not thread safe, but can share objects
	class TRenderTarget;	//	FBO, but sometimes with additional stuff (buffer flip etc)
	
	//	gr: renamed this to job to replace with SoyJob's later
	class TJobQueue;
	class TJob;
	class TJob_Function;
	
	
#define Opengl_IsInitialised()	Opengl::IsInitialised(__func__,true)
#define Opengl_IsOkay()			Opengl::IsOkay(__func__)

	bool	IsOkay(const std::string& Context);	//	throws exception
};





enum VertexAttributeLocation
{
	VERTEX_ATTRIBUTE_LOCATION_POSITION		= 0,
	VERTEX_ATTRIBUTE_LOCATION_NORMAL		= 1,
	VERTEX_ATTRIBUTE_LOCATION_TANGENT		= 2,
	VERTEX_ATTRIBUTE_LOCATION_BINORMAL		= 3,
	VERTEX_ATTRIBUTE_LOCATION_COLOR			= 4,
	VERTEX_ATTRIBUTE_LOCATION_UV0			= 5,
	VERTEX_ATTRIBUTE_LOCATION_UV1			= 6,
	VERTEX_ATTRIBUTE_LOCATION_JOINT_INDICES	= 7,
	VERTEX_ATTRIBUTE_LOCATION_JOINT_WEIGHTS	= 8,
	VERTEX_ATTRIBUTE_LOCATION_FONT_PARMS	= 9
};

struct GlProgram
{
	GlProgram() :
	program( 0 ),
	vertexShader( 0 ),
	fragmentShader( 0 ),
	uMvp( 0 ),
	uModel( 0 ),
	uView( 0 ),
	uColor( 0 ),
	uFadeDirection( 0 ),
	uTexm( 0 ),
	uTexm2( 0 ),
	uJoints( 0 ),
	uColorTableOffset( 0 ) {};
	
	
	bool		IsValid()
	{
		return (program != 0);
	}
	
	
	// These will always be > 0 after a build, any errors will abort()
	unsigned	program;
	unsigned	vertexShader;
	unsigned	fragmentShader;
	
	// Uniforms that aren't found will have a -1 value
	int		uMvp;				// uniform Mvpm
	int		uModel;				// uniform Modelm
	int		uView;				// uniform Viewm
	int		uColor;				// uniform UniformColor
	int		uFadeDirection;		// uniform FadeDirection
	int		uTexm;				// uniform Texm
	int		uTexm2;				// uniform Texm2
	int		uJoints;			// uniform Joints
	int		uColorTableOffset;	// uniform offset to apply to color table index
};


GlProgram	BuildProgram( const char * vertexSrc, const char * fragmentSrc,std::ostream& Error  );

void		DeleteProgram( GlProgram & prog );


class Opengl::TJob
{
public:
	//	returns "im finished"
	//	return false to stop any more tasks being run and re-insert this in the queue
	virtual bool		Run(std::ostream& Error)=0;
	
	//	todo:
	SoyEvent<bool>		mOnFinished;
};



class Opengl::TJob_Function : public TJob
{
public:
	TJob_Function(std::function<bool ()> Function) :
		mFunction	( Function )
	{
	}

	virtual bool		Run(std::ostream& Error)
	{
		return mFunction();
	}
	
	std::function<bool ()>	mFunction;
};



class Opengl::TJobQueue
{
public:
	void		Push(std::shared_ptr<TJob>& Job);
	void		Flush(TContext& Context);

private:
	//	gr: change this to a nice soy ringbuffer
	std::vector<std::shared_ptr<TJob>>	mJobs;
	std::recursive_mutex				mLock;
};

class Opengl::TContext
{
public:
	virtual ~TContext()				{}

	void			Iteration()		{	mJobQueue.Flush( *this );	}
	virtual bool	Lock()			{	return true;	}
	virtual void	Unlock()		{	}
	void			PushJob(std::function<bool(void)> Lambda);
	void			PushJob(std::shared_ptr<TJob>& Job)			{	mJobQueue.Push( Job );	}
	
	TJobQueue		mJobQueue;
};

//	FBO (or render buffer)
class Opengl::TRenderTarget
{
public:
	TRenderTarget(const std::string& Name) :
		mName	( Name )
	{
	}
	virtual ~TRenderTarget()	{}
	
	virtual vec2x<GLint>	GetSize()=0;
	
	std::string		mName;
};


