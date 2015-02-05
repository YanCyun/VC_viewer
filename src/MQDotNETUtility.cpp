#include "MQDotNETUtility.h"


string MQString2string(String^ IString)
{
	string Ostring;
	using namespace System::Runtime::InteropServices;
	const char *chars = (const char*)(Marshal::StringToHGlobalAnsi(IString)).ToPointer();
	Ostring = chars;
	Marshal::FreeHGlobal(System::IntPtr((void*)chars));

	return Ostring;
}

wstring MQString2wstring(String^ IString)
{
	wstring Owstring;
	using namespace System::Runtime::InteropServices;
	const wchar_t *wchars = (const wchar_t*)(Marshal::StringToHGlobalUni(IString)).ToPointer();
	Owstring = wchars;
	Marshal::FreeHGlobal(System::IntPtr((void*)wchars));

	return Owstring;
}

void MQTexture::LoadFromFile(System::String ^FileName)
{
	using namespace System::Drawing;

	Bitmap ^_Bitmap = gcnew Bitmap(FileName);
	_Bitmap->RotateFlip(RotateFlipType::RotateNoneFlipY);

	this->Components = GL_RGBA;
	this->Width  = _Bitmap->Width;
	this->Height = _Bitmap->Height;
	this->Format = GL_RGBA;
	this->Type   = GL_UNSIGNED_BYTE;
	
	if(this->Pixels != NULL)
	{
		delete [] this->Pixels;
		this->Pixels = NULL;
	}
	this->Pixels = new GLubyte[_Bitmap->Width*_Bitmap->Height*4];

	for(int y = 0; y < _Bitmap->Height; y++)
	{
		for(int x = 0; x < _Bitmap->Width; x++)
		{
			System::Drawing::Color pixelColor = _Bitmap->GetPixel(x, y);

			int Onedarray = y*_Bitmap->Width*4 + x*4;
			this->Pixels[Onedarray+0] = pixelColor.R;
			this->Pixels[Onedarray+1] = pixelColor.G;
			this->Pixels[Onedarray+2] = pixelColor.B;
			this->Pixels[Onedarray+3] = pixelColor.A;
		}
	}
}
