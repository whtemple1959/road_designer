#include "LassoMenu.h"

#ifndef M_PI
#define M_PI 3.14159f
#endif

LassoMenu::LassoMenu() {

	closed=false;
	needComputeClosed=false;	

}

void LassoMenu::addPoint(Vector p2d, Vector pick) {

	points.push_back(p2d);
	pickrays.push_back(pick);

	if (!closed)
		needComputeClosed=true;

}

void LassoMenu::drawAsOpen() {

	bool tempClosed=closed;
	closed=false;

	draw();

	closed=tempClosed;

}

void LassoMenu::draw() {

	//display lasso selection (if being done) 
	if (points.size()<2) 
		return;
		
	GLdouble model_view[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	setOrthogonal();

	//glColor3f(1.0,1.0,1.0);
	glColor3f(0,0,0);
	glLineStipple(1,0xF0F0);
	glLineWidth(3.0);
	glEnable(GL_LINE_STIPPLE);
	glBegin(GL_LINE_STRIP);
	for (int i=0;i<points.size();i++) 	
		glVertex3f(points[i].x,points[i].y,-2.0f);
	glEnd();
	glDisable(GL_LINE_STIPPLE);			

	//restore old projection/modelview
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(projection);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(model_view);	

}

void LassoMenu::clear() {
	points.clear();
	pickrays.clear();	

	closed=false;

}

Vector LassoMenu::getPoint(int index) {
	return points[index];
}

Vector LassoMenu::getPickray(int index) {
	return pickrays[index];
}

vector <Vector> LassoMenu::getAllPoints() {
	return points;
}

vector <Vector> LassoMenu::getLassoPoints() {
	vector <Vector> lassoPoints;

	if (!closed)
		return lassoPoints;

	for (int i=closeStartIndex;i<closeEndIndex;i++) 
		lassoPoints.push_back(points[i]);

	return lassoPoints;
}

Vector LassoMenu::getLassoCentroid() {
	Vector centroid=Vector(0.0f,0.0f,0.0f);
	for (int i=0;i<points.size();i++) {
		centroid+=points[i];
	}
	centroid=centroid/points.size();

	return centroid;
}

vector <Vector> LassoMenu::getAllPickrays() {
	return pickrays;
}

vector <Vector> LassoMenu::getLassoPickrays() {
	vector <Vector> lassoPoints;

	if (!closed)
		return lassoPoints;

	for (int i=closeStartIndex;i<closeEndIndex;i++) 
		lassoPoints.push_back(pickrays[i]);

	return lassoPoints;
}

int LassoMenu::getNumPoints() {
	return points.size();
}

void LassoMenu::setOrthogonal() {

	glViewport(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, glutGet(GLUT_WINDOW_WIDTH), 
	    0.0, glutGet(GLUT_WINDOW_HEIGHT), -5.0, 5.0);	

	glMatrixMode(GL_MODELVIEW);

}

bool LassoMenu::isClosed() {

	return false; //REMEMBER TO REMOVE THIS LATER!

	if (!needComputeClosed)
		return closed;

	int sampleInterval=(int)ceil(points.size()/50.0f);
	int minLassoDist=8;

	for (int i=0;i+sampleInterval<points.size();i+=sampleInterval) {
		for (int j=i+sampleInterval*2;j+sampleInterval<points.size();j+=sampleInterval) {
			if (Collision::LineSegmentsCollide(Vector(points[i].x,0.0f,points[i].y),
												Vector(points[i+sampleInterval].x,0.0f,points[i+sampleInterval].y),
												Vector(points[j].x,0.0f,points[j].y),
												Vector(points[j+sampleInterval].x,0.0f,points[j+sampleInterval].y),
												closedPoint)&&j-i>minLassoDist) {
				closedPoint.y=closedPoint.z; closedPoint.z=0.0f;
				closeStartIndex=i+sampleInterval;
				closeEndIndex=j;

				needComputeClosed=false;
				closed=true;
				return closed;
			}

		}
	}

	needComputeClosed=false;
	closed=false;
	return closed;

}

/*
void LassoMenu::drawPieMenu() {

	float menuRadius=100.0f;	
	float menuTransparency=0.9f;

	GLdouble model_view[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	//render the circle
	//menu will be centred on intersection point
	glPushMatrix();
		
		int mx, my;
		viewer->getMouseXY(mx, my);
		my=glutGet(GLUT_WINDOW_HEIGHT)-my;
		float mouseAngle=atan2((float)my-closedPoint.y,(float)mx-closedPoint.x);
		if ((float)my-closedPoint.y<0.0f) {
			mouseAngle=2.0f*M_PI-fabs(mouseAngle);
		}		
		float mouseDistance=Vector((float)mx-closedPoint.x,(float)my-closedPoint.y,0.0f).GetLength();

		if (mouseDistance>menuRadius) {
			if (mouseButton!=-1)
				buttonChanged=true;

			mouseButton=-1;
		}
		else {
			for (int i=0;i<pieMenuButtons.size();i++) {
				if (mouseAngle>=i*2.0f*M_PI/pieMenuButtons.size()&&
					mouseAngle<(i+1)*2.0f*M_PI/pieMenuButtons.size()) {

					if (mouseButton!=i)
						buttonChanged=true;

					mouseButton=i;
					break;
				}
			}
		}

		//the base of the menu
		if (closedPoint.x<menuRadius)
			closedPoint.x=menuRadius+20.0f;
		if (closedPoint.x+menuRadius>glutGet(GLUT_WINDOW_WIDTH))
			closedPoint.x=glutGet(GLUT_WINDOW_WIDTH)-menuRadius-20.0f;
		if (closedPoint.y<menuRadius)
			closedPoint.y=menuRadius+20.0f;
		if (closedPoint.y+menuRadius>glutGet(GLUT_WINDOW_HEIGHT))
			closedPoint.y=glutGet(GLUT_WINDOW_HEIGHT)-menuRadius-20.0f;

		glTranslatef(closedPoint.x,closedPoint.y,0.0f);
		glColor4f(0.0f,0.2f,0.5f,menuTransparency);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(0.0f,0.0f);		
		for (float i=0.0f;i<=2.0f*M_PI;i+=(2.0f*M_PI)/64.0f) {				
			if (i>mouseButton*2.0f*M_PI/pieMenuButtons.size()&&
				i<=(mouseButton+1)*2.0f*M_PI/pieMenuButtons.size()&&
				mouseDistance<menuRadius)				
				glColor4f(0.3f,0.5f,1.0f,menuTransparency);
			else
				glColor4f(0.0f,0.2f,0.5f,menuTransparency);
			glVertex2f(cos(i)*menuRadius,sin(i)*menuRadius);			
		}
		glEnd();

		//division lines between buttons
		glTranslatef(0.0f,0.0f,1.0f);
		if (pieMenuButtons.size()>1) {
			glColor4f(0.3f,0.5f,1.0f,menuTransparency);
			glBegin(GL_LINES);
			for (int i=0;i<pieMenuButtons.size();i++) {
				float eachAngle=i*2.0f*M_PI/pieMenuButtons.size();			
				glVertex2f(0.0f,0.0f);
				glVertex2f(cos(eachAngle)*menuRadius,sin(eachAngle)*menuRadius);			
			}
			glEnd();
		}

		//the button labels
		glColor3f(1.0f,1.0f,1.0f);
		for (int i=0;i<pieMenuButtons.size();i++) {
			float eachAngle=((float)i+0.5f)*2.0f*M_PI/pieMenuButtons.size();
			drawText((GLint)(cos(eachAngle)*menuRadius)-15,(GLint)(sin(eachAngle)*menuRadius),pieMenuButtons[i]);
		}

		glDisable(GL_BLEND);
		
	glPopMatrix();

	//now reset the projection/modelview matrices to what was there before
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(projection);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(model_view);


}
*/

void LassoMenu::drawText(GLint x, GLint y, char* s)
{

	int lines;
	char* p;	
	
	glRasterPos2i(x, y);
	for(p = s, lines = 0; *p; p++) {
		if (*p == '\n') {
			lines++;
			glRasterPos2i(x, y-(lines*14));
		}
		else if (*p==' ') {
			lines++;
			glRasterPos2i(x,y-(lines*14));
		}
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *p);
	}

}
