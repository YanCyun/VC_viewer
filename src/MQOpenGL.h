#ifndef MQOpenGL_h
#define MQOpenGL_h


#if defined(_WIN32)

#define  WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma comment (lib, "opengl32.lib")  /* link with Microsoft OpenGL lib */
#pragma comment (lib, "glu32.lib")     /* link with OpenGL Utility lib */

#endif

#include <gl\gl.h>
#include <gl\glu.h>


#endif	//MQOpenGL_h
