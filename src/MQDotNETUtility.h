#ifndef MQDotNETUtility_h
#define MQDotNETUtility_h


#include <string>
#include "MQOpenGL.h"


using System::String;
using std::string;
using std::wstring;

string MQString2string(String^ IString);

wstring MQString2wstring(String^ IString);

class MQTexture
{
public:

	GLint    Components;
	GLsizei  Width;
	GLsizei  Height;
	GLenum   Format;
	GLenum   Type;
	GLubyte *Pixels;

public:

	void LoadFromFile(String^ FileName);

public:

	MQTexture(): Pixels(NULL)	{	}
	~MQTexture()	{ delete [] Pixels; }
};


#endif	//MQDotNETUtility_h
