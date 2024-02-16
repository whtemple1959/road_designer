#include "GUIcontrols.h"

GUIcontrols::GUIcontrols() {	

	selected=0;

	ecost_lowbound=0.001;
	ecost_highbound=0.08;
	slider_minx=10;
	slider_maxx=250;
	slider_miny=10;
	slider_maxy=50;
	slider_pos=0.5;

	curvthresh_lowbound=0.001f;
	curvthresh_highbound=0.2f;
	curvthresh_minx=450;
	curvthresh_maxx=600;
	curvthresh_miny=10;
	curvthresh_maxy=50;
	curvthresh_pos=0.5f;

	endpt_lowbound=1.0f;
	endpt_highbound=100.0f;
	endpt_minx=650.0f;
	endpt_maxx=800.0f;
	endpt_miny=10.0f;
	endpt_maxy=50.0f;
	endpt_pos=0.0f;

	button_size=48;

	texture_slidergroove=ilutGLLoadImage("menuicons/slidergroove.tga");
	texture_slider=ilutGLLoadImage("menuicons/slider.tga");	

	texture_buttong3_down=ilutGLLoadImage("menuicons/buttong3_down.tga");	
	texture_buttong3_up=ilutGLLoadImage("menuicons/buttong3_up.tga");	
	texture_buttong1_down=ilutGLLoadImage("menuicons/buttong1_down.tga");	
	texture_buttong1_up=ilutGLLoadImage("menuicons/buttong1_up.tga");
	texture_buttonclosed_down=ilutGLLoadImage("menuicons/buttonclosed_down.tga");
	texture_buttonclosed_up=ilutGLLoadImage("menuicons/buttonclosed_up.tga");

	g1ButtonPressed=false;
	g3ButtonPressed=false;
	closedButtonPressed=false;

}

void GUIcontrols::draw() {

	glBindTexture(GL_TEXTURE_2D, texture_slidergroove);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(1,1,1,1);
	glBegin(GL_QUADS);

		glTexCoord2f(0,0);
		glVertex2f(slider_minx,slider_miny);
		glTexCoord2f(0,1);
		glVertex2f(slider_minx,slider_maxy);
		glTexCoord2f(1,1);
		glVertex2f(slider_maxx,slider_maxy);
		glTexCoord2f(1,0);
		glVertex2f(slider_maxx,slider_miny);

		if (g1ButtonPressed) 
			glColor4f(1,1,1,1);
		else
			glColor4f(1,1,1,.5);

		glTexCoord2f(0,0);
		glVertex2f(curvthresh_minx,curvthresh_miny);
		glTexCoord2f(0,1);
		glVertex2f(curvthresh_minx,curvthresh_maxy);
		glTexCoord2f(1,1);
		glVertex2f(curvthresh_maxx,curvthresh_maxy);
		glTexCoord2f(1,0);
		glVertex2f(curvthresh_maxx,curvthresh_miny);

		glColor4f(1,1,1,1);

		glTexCoord2f(0,0);
		glVertex2f(endpt_minx,endpt_miny);
		glTexCoord2f(0,1);
		glVertex2f(endpt_minx,endpt_maxy);
		glTexCoord2f(1,1);
		glVertex2f(endpt_maxx,endpt_maxy);
		glTexCoord2f(1,0);
		glVertex2f(endpt_maxx,endpt_miny);

	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture_slider);
	glBegin(GL_QUADS);

		glTexCoord2f(0,0);
		glVertex3f(slider_minx+(slider_maxx-slider_minx)*slider_pos-10.0f,slider_miny,1.0f);
		glTexCoord2f(0,1);
		glVertex3f(slider_minx+(slider_maxx-slider_minx)*slider_pos-10.0f,slider_maxy,1.0f);
		glTexCoord2f(1,1);
		glVertex3f(slider_minx+(slider_maxx-slider_minx)*slider_pos+10.0f,slider_maxy,1.0f);
		glTexCoord2f(1,0);
		glVertex3f(slider_minx+(slider_maxx-slider_minx)*slider_pos+10.0f,slider_miny,1.0f);

		if (g1ButtonPressed) 
			glColor4f(1,1,1,1);
		else
			glColor4f(1,1,1,.5);

		glTexCoord2f(0,0);
		glVertex3f(curvthresh_minx+(curvthresh_maxx-curvthresh_minx)*curvthresh_pos-10.0f,curvthresh_miny,1.0f);
		glTexCoord2f(0,1);
		glVertex3f(curvthresh_minx+(curvthresh_maxx-curvthresh_minx)*curvthresh_pos-10.0f,curvthresh_maxy,1.0f);
		glTexCoord2f(1,1);
		glVertex3f(curvthresh_minx+(curvthresh_maxx-curvthresh_minx)*curvthresh_pos+10.0f,curvthresh_maxy,1.0f);
		glTexCoord2f(1,0);
		glVertex3f(curvthresh_minx+(curvthresh_maxx-curvthresh_minx)*curvthresh_pos+10.0f,curvthresh_miny,1.0f);

		glColor4f(1,1,1,1);

		glTexCoord2f(0,0);
		glVertex3f(endpt_minx+(endpt_maxx-endpt_minx)*endpt_pos-10.0f,endpt_miny,1.0f);
		glTexCoord2f(0,1);
		glVertex3f(endpt_minx+(endpt_maxx-endpt_minx)*endpt_pos-10.0f,endpt_maxy,1.0f);
		glTexCoord2f(1,1);
		glVertex3f(endpt_minx+(endpt_maxx-endpt_minx)*endpt_pos+10.0f,endpt_maxy,1.0f);
		glTexCoord2f(1,0);
		glVertex3f(endpt_minx+(endpt_maxx-endpt_minx)*endpt_pos+10.0f,endpt_miny,1.0f);

	glEnd();

	glPushMatrix();

		if (g3ButtonPressed)
			glBindTexture(GL_TEXTURE_2D, texture_buttong3_down);
		else
			glBindTexture(GL_TEXTURE_2D, texture_buttong3_up);

		glTranslatef(slider_maxx,slider_miny,0.0f);
		glTranslatef(10.0f,0.0f,0.0f);
		glBegin(GL_QUADS);

			glTexCoord2f(0,0);
			glVertex3f(0,0,1.0f);
			glTexCoord2f(0,1);
			glVertex3f(0,button_size,1.0f);
			glTexCoord2f(1,1);
			glVertex3f(button_size,button_size,1.0f);
			glTexCoord2f(1,0);
			glVertex3f(button_size,0.0f,1.0f);

		glEnd();

		if (g1ButtonPressed)
			glBindTexture(GL_TEXTURE_2D, texture_buttong1_down);
		else
			glBindTexture(GL_TEXTURE_2D, texture_buttong1_up);

		glTranslatef(button_size+10.0f,0.0f,0.0f);
		glBegin(GL_QUADS);

			glTexCoord2f(0,0);
			glVertex3f(0,0,1.0f);
			glTexCoord2f(0,1);
			glVertex3f(0,button_size,1.0f);
			glTexCoord2f(1,1);
			glVertex3f(button_size,button_size,1.0f);
			glTexCoord2f(1,0);
			glVertex3f(button_size,0.0f,1.0f);

		glEnd();

		if (closedButtonPressed)
			glBindTexture(GL_TEXTURE_2D, texture_buttonclosed_down);
		else
			glBindTexture(GL_TEXTURE_2D, texture_buttonclosed_up);

		glTranslatef(button_size+10.0f,0.0f,0.0f);
		glBegin(GL_QUADS);

			glTexCoord2f(0,0);
			glVertex3f(0,0,1.0f);
			glTexCoord2f(0,1);
			glVertex3f(0,button_size,1.0f);
			glTexCoord2f(1,1);
			glVertex3f(button_size,button_size,1.0f);
			glTexCoord2f(1,0);
			glVertex3f(button_size,0.0f,1.0f);

		glEnd();

	glPopMatrix();

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);	

	//write ecost as str
	char *ecostasstr=new char[50];
	sprintf_s(ecostasstr,50,"E_cost: %f",getClothoidPenalty());
	DrawText(slider_minx,slider_maxy+10,ecostasstr, 0.5f, 0.5f, 0.5f);
	sprintf_s(ecostasstr,50,"G1 Curv.: %f",getG1DiscontCurvThresh());
	if (g1ButtonPressed)
		DrawText(curvthresh_minx,curvthresh_maxy+10,ecostasstr, 0.5f, 0.5f, 0.5f);
	else
		DrawText(curvthresh_minx,curvthresh_maxy+10,ecostasstr, 0.8f, 0.8f, 0.8f);
	sprintf_s(ecostasstr,50,"Endpt. Wt.: %f",getEndpointWeight());
	DrawText(endpt_minx,endpt_maxy+10,ecostasstr, 0.5f, 0.5f, 0.5f);

	delete [] ecostasstr;

}

int GUIcontrols::getSelected() {

	return selected;

}

float GUIcontrols::getClothoidPenalty() {
	
	return ecost_lowbound*(1.0f-slider_pos)+ecost_highbound*slider_pos;

}

float GUIcontrols::getG1DiscontCurvThresh() {
	return curvthresh_lowbound*(1.0f-curvthresh_pos)+curvthresh_highbound*curvthresh_pos;
}

float GUIcontrols::getEndpointWeight() {
	return endpt_lowbound*(1.0f-endpt_pos)+endpt_highbound*endpt_pos;
}
	
void GUIcontrols::mouseMotion(int x, int y) {

	y=glutGet(GLUT_WINDOW_HEIGHT)-y;

	if (selected==1) {
		if (x>=slider_minx&&x<=slider_maxx) 
			slider_pos=((float)x-slider_minx)/(slider_maxx-slider_minx);
	}
	else if (selected==4) {
		if (x>=curvthresh_minx&&x<=curvthresh_maxx) 
			curvthresh_pos=((float)x-curvthresh_minx)/(curvthresh_maxx-curvthresh_minx);
	}
	else if (selected==5) {
		if (x>=endpt_minx&&x<=endpt_maxx) 
			endpt_pos=((float)x-endpt_minx)/(endpt_maxx-endpt_minx);
	}

}

void GUIcontrols::mouseClick(int x, int y, bool down) {

	y=glutGet(GLUT_WINDOW_HEIGHT)-y;

	selected=0;

	if (down) {

		if (x>slider_minx+(slider_maxx-slider_minx)*slider_pos-10.0f&&
			x<slider_minx+(slider_maxx-slider_minx)*slider_pos+10.0f&&
			y>slider_miny&&y<slider_maxy) {
			selected=1;
		}
		else if (x>slider_maxx+10.0f&&x<slider_maxx+10.0f+button_size&&
			y>slider_miny&&y<slider_miny+button_size) {
			selected=2;
			g3ButtonPressed=!g3ButtonPressed;
		}
		else if (x>slider_maxx+20.0f+button_size&&x<slider_maxx+20.0f+2.0f*button_size&&
			y>slider_miny&&y<slider_miny+button_size) {
			selected=3;
			g1ButtonPressed=!g1ButtonPressed;
		}
		else if (x>slider_maxx+30.0f+2.0f*button_size&&x<slider_maxx+30.0f+3.0f*button_size&&
			y>slider_miny&&y<slider_miny+button_size) {
			selected=6;
			closedButtonPressed=!closedButtonPressed;
		}
		else if (x>curvthresh_minx+(curvthresh_maxx-curvthresh_minx)*curvthresh_pos-10.0f&&
			x<curvthresh_minx+(curvthresh_maxx-curvthresh_minx)*curvthresh_pos+10.0f&&
			y>curvthresh_miny&&y<curvthresh_maxy) {
			if (g1ButtonPressed)
				selected=4;
			
		}
		else if (x>endpt_minx+(endpt_maxx-endpt_minx)*endpt_pos-10.0f&&
			x<endpt_minx+(endpt_maxx-endpt_minx)*endpt_pos+10.0f&&
			y>endpt_miny&&y<endpt_maxy) {
			selected=5;
		}		

	}

}

void GUIcontrols::DrawText(GLint x, GLint y, char* s, GLfloat r, GLfloat g, GLfloat b)
{

	int lines;
	char* p;

	glDisable(GL_DEPTH_TEST);
	glPushMatrix();	

	glColor4f(r,g,b,1.0);
	glRasterPos2i(x, y);
	for(p = s, lines = 0; *p; p++) {
		if (*p == '\n') {
			lines++;
			glRasterPos2i(x, y-(lines*18));
		}
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
	}
	
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
     
}

bool GUIcontrols::getG1ButtonPressed() {
	return g1ButtonPressed;
}

bool GUIcontrols::getG3ButtonPressed() {
	return g3ButtonPressed;
}

bool GUIcontrols::getClosedButtonPressed() {
	return closedButtonPressed;
}