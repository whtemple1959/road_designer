#include "TwistLens.h"

TwistLens::TwistLens() {

	lensCentre=Vector((GLfloat)glutGet(GLUT_WINDOW_WIDTH)/2.0f,
						(GLfloat)glutGet(GLUT_WINDOW_HEIGHT)/2.0f,
						0.0f);

	outerRadius=(float)glutGet(GLUT_WINDOW_WIDTH)/6.0f;
	innerRadius=(float)glutGet(GLUT_WINDOW_WIDTH)/8.0f;
	viewTwist=80.0f;

	texture_translate=ilutGLLoadImage("menuicons/twistlens_translate.tga");
	texture_radius=ilutGLLoadImage("menuicons/twistlens_radius.tga");	
	texture_twist=ilutGLLoadImage("menuicons/twistlens_twist.tga");

	iconSelected=0;

}

void TwistLens::draw() {

	//capture the current projection/modelview matrices,
	//as we must enter orthogonal projection mode
	GLdouble model_view[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	//enter orthogonal mode
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, (float)glutGet(GLUT_WINDOW_WIDTH), 
					0.0, (GLfloat)glutGet(GLUT_WINDOW_HEIGHT), -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//draw the concentric circles
	glColor3f(0.0f,0.0f,0.0f);
	glLineWidth(3.0f);
	glTranslatef(lensCentre.x,lensCentre.y,0.0f);

	glLineStipple(1,0xF0F0);
	glLineWidth(3.0);
	glEnable(GL_LINE_STIPPLE);
	glBegin(GL_LINE_LOOP);
	for (float i=0.0f;i<2.0f*M_PI;i+=(2.0f*M_PI)/50.0f) {
		glVertex2f(cos(i)*outerRadius,sin(i)*outerRadius);
	}
	glEnd();

	glDisable(GL_LINE_STIPPLE);
	glBegin(GL_LINE_LOOP);
	for (float i=0.0f;i<2.0f*M_PI;i+=(2.0f*M_PI)/50.0f) {
		glVertex2f(cos(i)*innerRadius,sin(i)*innerRadius);
	}
	glEnd();

	//relative to lens centre!
	drawIcon(translateIconCentre().x,translateIconCentre().y,texture_translate);
	drawIcon(outerRadiusIconCentre().x,outerRadiusIconCentre().y,texture_radius);
	drawIcon(innerRadiusIconCentre().x,innerRadiusIconCentre().y,texture_radius);
	drawIcon(twistIconCentre().x,twistIconCentre().y,texture_twist);

	//now reset the projection/modelview matrices to what was there before
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(projection);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(model_view);

}

Vector TwistLens::translateIconCentre() {
	return Vector(cos(M_PI/4.0f)*outerRadius+20.0f,sin(M_PI/4.0f)*outerRadius+20.0f,0.0f);
}

Vector TwistLens::outerRadiusIconCentre() {
	return Vector(outerRadius,0.0f,0.0f);
}

Vector TwistLens::innerRadiusIconCentre() {
	return Vector(innerRadius,0.0f,0.0f);
}

Vector TwistLens::twistIconCentre() {
	return Vector(cos(M_PI/2.0f+viewTwist*M_PI/180.0f)*(innerRadius+30.0f),sin(M_PI/2.0f+viewTwist*M_PI/180.0f)*(innerRadius+30.0f),0.0f);
}

void TwistLens::drawIcon(float xtrans, float ytrans, int textureIndex) {
	//display control icons
	glColor3f(1.0f,1.0f,1.0f);
	glPushMatrix();
		glTranslatef(xtrans-15.0f,ytrans-15.0f,0.0f);

		glBindTexture (GL_TEXTURE_2D, textureIndex);
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glVertex2f(0,0);
			glTexCoord2f(0,1);
			glVertex2f(0,30);
			glTexCoord2f(1,1);
			glVertex2f(30,30);
			glTexCoord2f(1,0);
			glVertex2f(30,0);			
		glEnd();
	glPopMatrix();
}

void TwistLens::mouseMotion(int x, int y) {

	float xf=(float)x;
	float yf=(float)glutGet(GLUT_WINDOW_HEIGHT)-(float)y;

	if (iconSelected==1) {
		lensCentre=Vector(xf-translateIconCentre().x,yf-translateIconCentre().y,0.0f);
	}
	else if (iconSelected==2) {
		float centreDist=(Vector(xf,yf,0.0f)-lensCentre).GetLength();
		if(centreDist>innerRadius+15.0f)
			outerRadius=centreDist;
	}
	else if (iconSelected==3) {
		float centreDist=(Vector(xf,yf,0.0f)-lensCentre).GetLength();
		if(centreDist>15.0f&&centreDist<outerRadius-15.0f)
			innerRadius=centreDist;
	}	
	else if (iconSelected==4) {
		float theAngle=atan2(yf-lensCentre.y,xf-lensCentre.x);
		theAngle=theAngle*180.0f/M_PI-90.0f;

		if (theAngle>-90.0f&&theAngle<90.0f)
			viewTwist=theAngle;
	}

}

void TwistLens::mouseClick(int x, int y, bool down) {
	//iconselected=1 - translate
	//iconselected=2 - outer radius
	//iconselected=3 - inner radius
	float xf=(float)x;
	float yf=(float)glutGet(GLUT_WINDOW_HEIGHT)-(float)y;	

	iconSelected=0;	

	if (down) {
		if (xf>lensCentre.x+translateIconCentre().x-15.0f&&
			xf<lensCentre.x+translateIconCentre().x+15.0f&&
			yf>lensCentre.y+translateIconCentre().y-15.0f&&
			yf<lensCentre.y+translateIconCentre().y+15.0f)
			iconSelected=1;
		else if (xf>lensCentre.x+outerRadiusIconCentre().x-15.0f&&
			xf<lensCentre.x+outerRadiusIconCentre().x+15.0f&&
			yf>lensCentre.y+outerRadiusIconCentre().y-15.0f&&
			yf<lensCentre.y+outerRadiusIconCentre().y+15.0f)
			iconSelected=2;
		else if (xf>lensCentre.x+innerRadiusIconCentre().x-15.0f&&
			xf<lensCentre.x+innerRadiusIconCentre().x+15.0f&&
			yf>lensCentre.y+innerRadiusIconCentre().y-15.0f&&
			yf<lensCentre.y+innerRadiusIconCentre().y+15.0f)
			iconSelected=3;		
		else if (xf>lensCentre.x+twistIconCentre().x-15.0f&&
			xf<lensCentre.x+twistIconCentre().x+15.0f&&
			yf>lensCentre.y+twistIconCentre().y-15.0f&&
			yf<lensCentre.y+twistIconCentre().y+15.0f)
			iconSelected=4;	
	}		

}

void TwistLens::setViewTwists(RN *roadNetwork) {
	//view twists have to be propagated to each road network edge
	//(this requires projecting the control vertex of each road
	//network edge into image space where the lens lies)

	GLdouble model_view[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLdouble eachxval, eachyval, eachzval;

	for (int i=0;i<roadNetwork->getNumEdges();i++) {

		RN_Edge *eachEdge=roadNetwork->getEdge(i);
		vector <Vector> edgePointSet=eachEdge->getPointSet();
		vector <float> viewTwistVector;

		//gluProject each point, assign viewTwist based on proximity to lensCentre
		for (int j=0;j<edgePointSet.size();j++) {
			gluProject(edgePointSet[j].x,edgePointSet[j].y,edgePointSet[j].z,
					model_view, projection, viewport, &eachxval, &eachyval, &eachzval);

			Vector projectedPoint=Vector(eachxval,eachyval,0.0f);
			float eachTwistVal;

			if ((projectedPoint-lensCentre).GetLength()>outerRadius)
				eachTwistVal=0.0f;
			else if ((projectedPoint-lensCentre).GetLength()<innerRadius)
				eachTwistVal=viewTwist;
			else
				eachTwistVal=viewTwist-(((projectedPoint-lensCentre).GetLength()-innerRadius)/(outerRadius-innerRadius))*viewTwist;
		
			viewTwistVector.push_back(eachTwistVal);			

		}		

		eachEdge->setViewTwists(viewTwistVector);
	
	}

}