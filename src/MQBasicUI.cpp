#include "MQBasicUI.h"
#include "MQOpenGL.h"


void MQBasicUI::InitialSetup(int Width, int Height)
{
	glClearColor(0.5, 0.5, 0.5, 1.0);

	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//light values and coordinates
	GLfloat positionLight[] = {1.5f, 1.5f, 10.0f, 1.0f};
	GLfloat ambientLight[] = {0.32f, 0.32f, 0.32f, 1.0f};
	GLfloat diffuseLight[] = {0.5f, 0.5f, 0.5f, 1.0f};
	GLfloat specularLight[] = {0.32f, 0.32f, 0.32f, 1.0f};
	GLfloat specref[] = {0.32f, 0.32f, 0.32f, 1.0f};

	//enable lighting
	glEnable(GL_LIGHTING);

	//setup and enable light 0
	glLightfv(GL_LIGHT0, GL_POSITION, positionLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
	glEnable(GL_LIGHT0);

	//material properties follow glColor values
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	//enable color tracking
	glEnable(GL_COLOR_MATERIAL);

	//all materials hereafter have full specular reflectivity with a high shine
	glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
	glMateriali(GL_FRONT, GL_SHININESS, 128);

	//texture environment
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	//setup viewport
	glViewport(0, 0, Width, Height);
	glGetIntegerv(GL_VIEWPORT, this->Viewport);

	//setup viewing frustum
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, GLdouble(Width)/GLdouble(Height), 1.0, 1024.0);
	glGetDoublev(GL_PROJECTION_MATRIX, this->ProjectionMatrix);

	//trackball's tbInit() requires MODELVIEW matrix mode is set
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//trackball
	this->TrackBall.tbInit();
	this->TrackBall.tbReshape(Width, Height);
}

void MQBasicUI::ViewportResize(int Width, int Height)
{
	//update viewport
	glViewport(0, 0, Width, Height);
	glGetIntegerv(GL_VIEWPORT, this->Viewport);

	//update viewing frustum
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLdouble)(Width)/(GLdouble)(Height), 1.0, 1024.0);
	glGetDoublev(GL_PROJECTION_MATRIX, this->ProjectionMatrix);

	//MODELVIEW matrix mode
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//trackball
	this->TrackBall.tbReshape(Width, Height);
}

void MQBasicUI::MouseDown(int X, int Y)
{
	if(this->Mode == this->TRACKBALL)
		this->TrackBall.tbStartMotion(X, Y);
	else if(this->Mode == this->TRANSLATE)
	{
		GLint    _GLWY;
		GLfloat  _depths;
		GLdouble _x, _y, _z;

		_GLWY = Viewport[3] - (Y + 1);

		glReadPixels(X, _GLWY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &_depths);
		if(_depths > 0.998f)	_depths = 0.8f;

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glGetDoublev(GL_MODELVIEW_MATRIX, this->ModelViewMatrix);

		gluUnProject((GLdouble)X, (GLdouble)_GLWY, (GLdouble)_depths,
				   this->ModelViewMatrix, this->ProjectionMatrix, this->Viewport, &_x, &_y, &_z);

		this->TranslateLstX = _x;
		this->TranslateLstY = _y;
		this->TranslateDepth = (double)_depths;
	}
	else if(this->Mode == this->ZOOM)
		this->ZoomLstY = Y;
}

void MQBasicUI::MouseMove(int X, int Y)
{
	if(this->Mode == this->TRACKBALL)
		this->TrackBall.tbMotion(X, Y);
	else if(this->Mode == this->TRANSLATE)
	{
		GLint    _GLWY;
		GLdouble _x, _y, _z;

		_GLWY = Viewport[3] - (Y + 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glGetDoublev(GL_MODELVIEW_MATRIX, this->ModelViewMatrix);

		gluUnProject((GLdouble)X, (GLdouble)_GLWY, (GLdouble)this->TranslateDepth,
				   this->ModelViewMatrix, this->ProjectionMatrix, this->Viewport, &_x, &_y, &_z);

		this->TranslateDifX = _x - this->TranslateLstX;
		this->TranslateDifY = _y - this->TranslateLstY;
		this->TranslateLstX = _x;
		this->TranslateLstY = _y;
		this->TranslateStrX += this->TranslateDifX;
		this->TranslateStrY += this->TranslateDifY;
	}
	else if(this->Mode == this->ZOOM)
	{
		this->ZoomDifY = Y - this->ZoomLstY;
		this->ZoomLstY = Y;
		this->ZoomStr += this->ZoomDifY;
	}
}

void MQBasicUI::MouseUp()
{
	if(this->Mode == this->TRACKBALL)
	{
		this->TrackBall.tbStopMotion();
		this->Mode = this->NONE;
	}
	else if(this->Mode == this->TRANSLATE)
		this->Mode = this->NONE;
	else if(this->Mode == this->ZOOM)
		this->Mode = this->NONE;
}

void MQBasicUI::SetScale(double x,double y){
	this->ScaleX = x;
	this->ScaleY = y;
}

void MQBasicUI::Paint()
{
	gluLookAt(0.0, 0.0, 256.0 + 0.02*(this->ZoomStr), 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	glTranslatef((float)this->TranslateStrX, (float)this->TranslateStrY, 0.0f);
	glScalef((float)this->ScaleX,(float)this->ScaleY,1.0f);
	this->TrackBall.tbMatrix();
}

void MQBasicUI::ProjectTo3D(int X, int Y, double &OX, double &OY, double &OZ)
{
	GLint    _GLWY;
	GLfloat  _depths;
	GLdouble _x, _y, _z;

	_GLWY = this->Viewport[3] - (Y + 1);

	glReadPixels(X, _GLWY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &_depths);

	//MODELVIEW matrix mode
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();
		gluLookAt(0.0, 0.0, 5.0 + 0.02*(this->ZoomStr), 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		glTranslatef((float)this->TranslateStrX, (float)this->TranslateStrY, 0.0f);
		this->TrackBall.tbMatrix();
		glGetDoublev(GL_MODELVIEW_MATRIX, this->ModelViewMatrix);
	glPopMatrix();

	gluUnProject((GLdouble)X, (GLdouble)_GLWY, (GLdouble)_depths,
			   this->ModelViewMatrix, this->ProjectionMatrix, this->Viewport, &_x, &_y, &_z);

	OX = _x;
	OY = _y;
	OZ = _z;
}
