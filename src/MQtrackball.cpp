#include "MQtrackball.h"
#include <math.h>
#include "MQOpenGL.h"

/*
 *  MODIFIED
 *
 *  Simple trackball-like motion adapted (ripped off) from projtex.c
 *  (written by David Yu and David Blythe).  See the SIGGRAPH '96
 *  Advanced OpenGL course notes.
 */


void _tbPointToVector(int x, int y, int width, int height, float v[3])
{
  float d, a;

  /* project x, y onto a hemi-sphere centered within width, height. */
  v[0] = (2.0 * x - width) / width;
  v[1] = (height - 2.0 * y) / height;
  d = sqrt(v[0] * v[0] + v[1] * v[1]);
  v[2] = cos((3.14159265 / 2.0) * ((d < 1.0) ? d : 1.0));
  a = 1.0 / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  v[0] *= a;
  v[1] *= a;
  v[2] *= a;
}

void MQtrackball::tbStartMotion(int x, int y)
{
  tb_tracking = true;
  _tbPointToVector(x, y, tb_width, tb_height, tb_lastposition);
}

void MQtrackball::tbStopMotion(void)
{
  tb_tracking = false;
  tb_angle = 0.0;
}

void MQtrackball::tbInit(void)
{
  tb_angle = 0.0;
  tb_lastposition[0] = tb_lastposition[1] =tb_lastposition[2] = 0.0;
  tb_axis[0] = tb_axis[1] = tb_axis[2] = 0.0;

  /* put the identity in the trackball transform */
  glPushMatrix();
    glLoadIdentity();
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)tb_transform);
  glPopMatrix();

  tb_tracking = false;
}

void MQtrackball::tbMatrix()
{
  glPushMatrix();
    glLoadIdentity();
    glRotatef(tb_angle, tb_axis[0], tb_axis[1], tb_axis[2]);
    glMultMatrixf((GLfloat*)tb_transform);
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)tb_transform);
  glPopMatrix();

  glMultMatrixf((GLfloat*)tb_transform);
}

void MQtrackball::tbReshape(int width, int height)
{
  tb_width  = width;
  tb_height = height;
}

void MQtrackball::tbMotion(int x, int y)
{
  GLfloat current_position[3], dx, dy, dz;

  if (tb_tracking == false)
    return;

  _tbPointToVector(x, y, tb_width, tb_height, current_position);

  /* calculate the angle to rotate by (directly proportional to the
     length of the mouse movement */
  dx = current_position[0] - tb_lastposition[0];
  dy = current_position[1] - tb_lastposition[1];
  dz = current_position[2] - tb_lastposition[2];
  tb_angle = 90.0 * sqrt(dx * dx + dy * dy + dz * dz);
  /* calculate the axis of rotation (cross product) */
  tb_axis[0] = tb_lastposition[1] * current_position[2] -
               tb_lastposition[2] * current_position[1];
  tb_axis[1] = tb_lastposition[2] * current_position[0] -
               tb_lastposition[0] * current_position[2];
  tb_axis[2] = tb_lastposition[0] * current_position[1] -
               tb_lastposition[1] * current_position[0];

  /* reset for next time */
  tb_lastposition[0] = current_position[0];
  tb_lastposition[1] = current_position[1];
  tb_lastposition[2] = current_position[2];
}

void MQtrackball::reset(){

	this->tbInit();

	glPushMatrix();
		glLoadIdentity();
		glRotatef(tb_angle, tb_axis[0], tb_axis[1], tb_axis[2]);
		glMultMatrixf((GLfloat*)tb_transform);
		glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)tb_transform);
	glPopMatrix();

	glMultMatrixf((GLfloat*)tb_transform);
}