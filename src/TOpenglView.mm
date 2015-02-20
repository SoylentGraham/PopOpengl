#import "TOpenGLView.h"
#include <OpenGL/gl.h>


static void drawAnObject ()
{
	glColor3f(1.0f, 0.85f, 0.35f);
	glBegin(GL_TRIANGLES);
	{
		glVertex3f(  0.0,  0.6, 0.0);
		glVertex3f( -0.2, -0.3, 0.0);
		glVertex3f(  0.2, -0.3 ,0.0);
	}
	glEnd();
}




@implementation MyOpenGLView


-(void) drawRect: (NSRect) bounds
{
	glClearColor(0.8, 0.4, 0.3, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	drawAnObject();
	glFlush();
}

@end

