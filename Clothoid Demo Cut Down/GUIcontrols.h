#include <windows.h>
#include <gl/glew.h>
#include <glut.h>
#include <IL/ilut.h>

#include "UI_Selectable.h"

class GUIcontrols {

	public:

	GUIcontrols();

	void mouseMotion(int x, int y);
	void mouseClick(int x, int y, bool down);
	void draw();
	int getSelected();

	float getClothoidPenalty();
	float getG1DiscontCurvThresh();
	float getEndpointWeight();

	bool getG1ButtonPressed();
	bool getG3ButtonPressed();
	bool getClosedButtonPressed();

	private:

	void DrawText(GLint x, GLint y, char* s, GLfloat r, GLfloat g, GLfloat b);

	int selected;

	//textures for widget
	GLuint texture_slidergroove;
	GLuint texture_slider;	

	GLuint texture_buttong3_down, texture_buttong3_up;
	GLuint texture_buttong1_down, texture_buttong1_up;
	GLuint texture_buttonclosed_down, texture_buttonclosed_up;

	float ecost_lowbound;
	float ecost_highbound;
	float slider_minx;
	float slider_maxx;
	float slider_miny;
	float slider_maxy;
	float slider_pos;

	float curvthresh_lowbound;
	float curvthresh_highbound;
	float curvthresh_minx;
	float curvthresh_maxx;
	float curvthresh_miny;
	float curvthresh_maxy;
	float curvthresh_pos;

	float endpt_lowbound;
	float endpt_highbound;
	float endpt_minx;
	float endpt_maxx;
	float endpt_miny;
	float endpt_maxy;
	float endpt_pos;

	float button_size;

	bool g1ButtonPressed;
	bool g3ButtonPressed;
	bool closedButtonPressed;

};