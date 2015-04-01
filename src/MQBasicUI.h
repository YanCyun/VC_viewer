#ifndef MQBasicUI_h
#define MQBasicUI_h


#include "MQtrackball.h"


class MQBasicUI
{
   private:

	MQtrackball TrackBall;

	
	double TranslateLstX, TranslateLstY;
	double TranslateDifX, TranslateDifY, TranslateDepth;
	double TranslateStrX, TranslateStrY;
	double ScaleX,ScaleY;

	double ZoomStr, ZoomLstY, ZoomDifY;

	int    Viewport[4];
	double ModelViewMatrix[16], ProjectionMatrix[16];
	

   public:
	
	enum {TRACKBALL, TRANSLATE, ZOOM, NONE} Mode;
	void InitialSetup(int Width, int Height);
	void ViewportResize(int Width, int Height);
	void MouseDown(int X, int Y);
	void MouseMove(int X, int Y);
	void MouseUp();
	void Paint();
	void set_position(double x,double y);
	void set_zoom(double z);
	void reset();

	//X, Y are input OpenGL window coordinates. OX, OY, OZ are output 3D coordinates
	//The OpenGL window should be "MakeCurrent"
	//The model-view matrix should not be changed after the Paint() function call
	void ProjectTo3D(int X, int Y, double &OX, double &OY, double &OZ);

   public:

	MQBasicUI()
	{
		//private
		this->TranslateStrX = 0.0;
		this->TranslateStrY = 0.0;
		this->ZoomStr = 0.0;
		this->ScaleX = this->ScaleY = 1.0;

		//public
		this->Mode = this->NONE;
	}
		
	~MQBasicUI()
	{}
};


#endif	//MQBasicUI_h