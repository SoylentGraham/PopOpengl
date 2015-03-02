#pragma once

#include <ostream>


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
