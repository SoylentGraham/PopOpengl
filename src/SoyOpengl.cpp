#include "SoyOpengl.h"
#include "UnityDevice.h"



namespace Opengl
{
	const char*		ErrorToString(GLenum Error);
}



// Returns false and logs the ShaderInfoLog on failure.
bool CompileShader( const GLuint shader, const char * src,const std::string& ErrorPrefix,std::ostream& Error)
{
	glShaderSource( shader, 1, &src, 0 );
	glCompileShader( shader );
	
	GLint r;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &r );
	if ( r == GL_FALSE )
	{
		Error << ErrorPrefix << " Compiling shader error: ";
		GLchar msg[4096] = {0};
		glGetShaderInfoLog( shader, sizeof( msg ), 0, msg );
		Error << msg;
		return false;
	}
	return true;
}

GlProgram BuildProgram( const char * vertexSrc,
					   const char * fragmentSrc,std::ostream& Error )
{
	GlProgram prog;

	bool Success = true;
	prog.vertexShader = glCreateShader( GL_VERTEX_SHADER );
	if ( !CompileShader( prog.vertexShader, vertexSrc, "Vertex Shader", Error ) )
	{
		Success = false;
	}
	
	prog.fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
	if ( !CompileShader( prog.fragmentShader, fragmentSrc, "Fragment Shader", Error ) )
	{
		Success = false;
	}
	
	if ( !Success )
		return GlProgram();
	
	prog.program = glCreateProgram();
	glAttachShader( prog.program, prog.vertexShader );
	glAttachShader( prog.program, prog.fragmentShader );
	
	// set attributes before linking
	glBindAttribLocation( prog.program, VERTEX_ATTRIBUTE_LOCATION_POSITION,			"Position" );
	glBindAttribLocation( prog.program, VERTEX_ATTRIBUTE_LOCATION_NORMAL,			"Normal" );
	glBindAttribLocation( prog.program, VERTEX_ATTRIBUTE_LOCATION_TANGENT,			"Tangent" );
	glBindAttribLocation( prog.program, VERTEX_ATTRIBUTE_LOCATION_BINORMAL,			"Binormal" );
	glBindAttribLocation( prog.program, VERTEX_ATTRIBUTE_LOCATION_COLOR,			"VertexColor" );
	glBindAttribLocation( prog.program, VERTEX_ATTRIBUTE_LOCATION_UV0,				"TexCoord" );
	glBindAttribLocation( prog.program, VERTEX_ATTRIBUTE_LOCATION_UV1,				"TexCoord1" );
	glBindAttribLocation( prog.program, VERTEX_ATTRIBUTE_LOCATION_JOINT_WEIGHTS,	"JointWeights" );
	glBindAttribLocation( prog.program, VERTEX_ATTRIBUTE_LOCATION_JOINT_INDICES,	"JointIndices" );
	glBindAttribLocation( prog.program, VERTEX_ATTRIBUTE_LOCATION_FONT_PARMS,		"FontParms" );
	
	// link and error check
	glLinkProgram( prog.program );
	GLint r;
	glGetProgramiv( prog.program, GL_LINK_STATUS, &r );
	if ( r == GL_FALSE )
	{
		Error << "Linking vertex & fragment shader failed: ";
		GLchar msg[1024] = {0};
		glGetProgramInfoLog( prog.program, sizeof( msg ), 0, msg );
		Error << msg;
		return GlProgram();
	}
	prog.uMvp = glGetUniformLocation( prog.program, "Mvpm" );
	prog.uModel = glGetUniformLocation( prog.program, "Modelm" );
	prog.uView = glGetUniformLocation( prog.program, "Viewm" );
	prog.uColor = glGetUniformLocation( prog.program, "UniformColor" );
	prog.uFadeDirection = glGetUniformLocation( prog.program, "UniformFadeDirection" );
	prog.uTexm = glGetUniformLocation( prog.program, "Texm" );
	prog.uTexm2 = glGetUniformLocation( prog.program, "Texm2" );
	prog.uJoints = glGetUniformLocation( prog.program, "Joints" );
	prog.uColorTableOffset = glGetUniformLocation( prog.program, "ColorTableOffset" );
	
	glUseProgram( prog.program );
	
	// texture and image_external bindings
	for ( int i = 0; i < 8; i++ )
	{
		char name[32];
		sprintf( name, "Texture%i", i );
		const GLint uTex = glGetUniformLocation( prog.program, name );
		if ( uTex != -1 )
		{
			glUniform1i( uTex, i );
		}
	}
	
	glUseProgram( 0 );
	
	return prog;
}

void DeleteProgram( GlProgram & prog )
{
	if ( prog.program != 0 )
	{
		glDeleteProgram( prog.program );
	}
	if ( prog.vertexShader != 0 )
	{
		glDeleteShader( prog.vertexShader );
	}
	if ( prog.fragmentShader != 0 )
	{
		glDeleteShader( prog.fragmentShader );
	}
	prog.program = 0;
	prog.vertexShader = 0;
	prog.fragmentShader = 0;
}



void Opengl::TJobQueue::Push(std::shared_ptr<TJob>& Job)
{
	mLock.lock();
	mJobs.push_back( Job );
	mLock.unlock();
}

void Opengl::TJobQueue::Flush(TContext& Context)
{
	while ( true )
	{
		//	pop task
		mLock.lock();
		std::shared_ptr<TJob> Job;
		auto NextJob = mJobs.begin();
		if ( NextJob != mJobs.end() )
		{
			Job = *NextJob;
			mJobs.erase( NextJob );
		}
		//bool MoreJobs = !mJobs.empty();
		mLock.unlock();
		
		if ( !Job )
			break;
		
		//	lock the context
		if ( !Context.Lock() )
		{
			mLock.lock();
			mJobs.insert( mJobs.begin(), Job );
			mLock.unlock();
			break;
		}
		auto ContextLock = SoyScope( []{}, [&Context]{Context.Unlock();} );
		
		//	execute task, if it returns false, we don't run any more this time and re-insert
		if ( !Job->Run( std::Debug ) )
		{
			mLock.lock();
			mJobs.insert( mJobs.begin(), Job );
			mLock.unlock();
			break;
		}
	}
}


void Opengl::TContext::PushJob(std::function<bool ()> Function)
{
	std::shared_ptr<TJob> Job( new TJob_Function( Function ) );
	PushJob( Job );
}


const char* Opengl::ErrorToString(GLenum Error)
{
	switch ( Error )
	{
		case GL_NO_ERROR:			return "GL_NO_ERROR";
		case GL_INVALID_ENUM:		return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:		return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:	return "GL_INVALID_OPERATION";
		case GL_INVALID_FRAMEBUFFER_OPERATION:	return "GL_INVALID_FRAMEBUFFER_OPERATION";
		case GL_OUT_OF_MEMORY:		return "GL_OUT_OF_MEMORY";
		case GL_STACK_UNDERFLOW:	return "GL_STACK_UNDERFLOW";
		case GL_STACK_OVERFLOW:		return "GL_STACK_OVERFLOW";
		default:
			return "Unknown opengl error value";
	};
}

bool Opengl::IsOkay(const std::string& Context)
{
//	if ( !IsInitialised(std::string("CheckIsOkay") + Context, false ) )
//		return false;
	
	auto Error = glGetError();
	if ( Error == GL_NONE )
		return true;
	
	std::stringstream ErrorStr;
	ErrorStr << "Opengl error in " << Context << ": " << Opengl::ErrorToString(Error) << std::endl;
	
	return Soy::Assert( Error == GL_NONE , ErrorStr.str() );
}

