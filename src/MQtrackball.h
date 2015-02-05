#ifndef MQtrackball_h
#define MQtrackball_h


/*
 *  MODIFIED
 *
 *  Simple trackball-like motion adapted (ripped off) from projtex.c
 *  (written by David Yu and David Blythe).  See the SIGGRAPH '96
 *  Advanced OpenGL course notes.
 *
 * */


/*
		void init(void)
		{
			tbInit();
			tbReshape(width, height);
		   . . .
		}

		void reshape(int width, int height)
		{
			tbReshape(width, height);
		   . . .
		}

		void display(void)
		{
			glPushMatrix();

				tbMatrix();
				. . . draw the scene . . .

			glPopMatrix();
		}

		void MouseDown(int x, int y)
		{
			tbStartMotion(x, y);
			. . .
		}

		void MouseUp(void)
		{
			tbStopMotion();
			. . .
		}

		void motion(int x, int y)
		{
			tbMotion(x, y);
			. . .
		}
 */


class MQtrackball
{
   private:

      float tb_lastposition[3];
      float tb_angle;
      float tb_axis[3];
      float tb_transform[16];
      int   tb_width;
      int   tb_height;
      bool  tb_tracking;

   public:

      void tbInit(void);

      void tbMatrix(void);

      void tbReshape(int width, int height);

      void tbStartMotion(int x, int y);

      void tbStopMotion(void);

      void tbMotion(int x, int y);


      MQtrackball(void)
      {
         tb_angle = 0.0;
         tb_axis[0] = tb_axis[1] = tb_axis[2] = 0.0;
         tb_tracking = false;
      }
};


#endif	//MQtrackball_h
