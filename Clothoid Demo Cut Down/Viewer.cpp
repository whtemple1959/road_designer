/*
 * Viewer.h - body of class Viewer,
 * a class which encompasses the interface's 3D view of the currently
 * generated road network
 */ 

#include "Viewer.h"

//static member declarations
int Viewer::screenWidth;
int Viewer::screenHeight;
HeightMap *Viewer::heightMap;
LassoMenu *Viewer::lassoMenu;
Camera *Viewer::lastCamera;
Camera *Viewer::mainCamera;
Camera *Viewer::interpCamera;
float Viewer::interpCameraVal;

int Viewer::mouse_x;
int Viewer::mouse_y;
bool Viewer::mouse_b1;
bool Viewer::mouse_b2;
bool Viewer::mouse_b3;

vector <Vector> Viewer::lassoPoints3D;

Vector Viewer::lastCollision;
Vector Viewer::lastpickray;

GUIcontrols *Viewer::guiControls;
ClothoidSpline *Viewer::clothoidSpline;

float Viewer::curClothoidPenalty;
double Viewer::tanImportance;
int Viewer::curIterationOfClosing;
double Viewer::closingError;

//timestamp Viewer::lastTime; 
//timestamp Viewer::curTime;
//float Viewer::timeDiffms;

int Viewer::specialModifiers;

void Viewer::init(int w, int h) {   	

	curClothoidPenalty=DEFAULT_CLOTHOID_PENALTY;

    screenWidth=w;
    screenHeight=h;  	

	//load heightmap
	heightMap=new HeightMap();

	//set default camera position based on dimensions of heightmap
	//mainCamera=new Camera(heightMap->getDefaultCameraTrackPos(),0,55.0f);	
	mainCamera=new Camera(heightMap->getDefaultCameraTrackPos(),0,90.0f);
	interpCamera=new Camera();
	interpCamera->setCamera(mainCamera);
	lastCamera=new Camera();
	lastCamera->setCamera(mainCamera);
	interpCameraVal=1.0f;

	lassoMenu=new LassoMenu();
	clothoidSpline=NULL;	

	guiControls=new GUIcontrols();

	//set perspective of viewer
	setPerspective();

	//set clear/active colours
    //glClearColor(0.6,0.6,0.7,1);
	glClearColor(1,1,1,1);

	glPolygonMode(GL_FRONT, GL_FILL);
	glEnable(GL_DEPTH_TEST);

	//We use glScale when drawing the scene
	glEnable(GL_NORMALIZE);
    
}

void Viewer::setPerspective() {

	//sets perspective to current screen width/height
	glViewport(0, 0, (GLfloat)glutGet(GLUT_WINDOW_WIDTH), (GLfloat)glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(80.0,(GLfloat)glutGet(GLUT_WINDOW_WIDTH)/(GLfloat)glutGet(GLUT_WINDOW_HEIGHT),0.001,MAXSELECTDISTANCE);        
    
	glMatrixMode(GL_MODELVIEW);

}

void Viewer::setOrthogonal() {

	glViewport(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, glutGet(GLUT_WINDOW_WIDTH), 
	    0.0, glutGet(GLUT_WINDOW_HEIGHT), -10.0, 10.0);	

	glMatrixMode(GL_MODELVIEW);

}

void Viewer::DrawText(GLint x, GLint y, char* s, GLfloat r, GLfloat g, GLfloat b)
{

	int lines;
	char* p;

	glDisable(GL_DEPTH_TEST);
	glPushMatrix();

		glLoadIdentity();
		setOrthogonal();	

		glColor4f(r,g,b,1.0);
		glRasterPos2i(x, y);
		for(p = s, lines = 0; *p; p++) {
			if (*p == '\n') {
				lines++;
				glRasterPos2i(x, y-(lines*18));
			}
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
		}

		setPerspective();

	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
     
}

void Viewer::updatePickray() {

	//update pickray
	GLdouble proj[16]; 
	GLdouble modelview[16];
	GLint viewport[4];

	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetIntegerv(GL_VIEWPORT,viewport); 

	GLdouble objx1;
	GLdouble objy1;
	GLdouble objz1;

	GLdouble objx2;
	GLdouble objy2;
	GLdouble objz2;
	
	gluUnProject(mouse_x,viewport[3]-mouse_y-1,0.0,modelview,proj,viewport,&objx1,&objy1,&objz1);
	gluUnProject(mouse_x,viewport[3]-mouse_y-1,1.0,modelview,proj,viewport,&objx2,&objy2,&objz2);

	lastpickray=Vector(objx2-objx1, objy2-objy1, objz2-objz1);
	lastpickray.Normal();	

}

void Viewer::updateCamera(Camera *whichCam) {

	//translate/rotate to camera view		
	whichCam->updateTrackPosition();
	whichCam->rotateLookAtTrackPosition();

}

void Viewer::draw(void) { 	

	glPushMatrix();					
		
		updateCamera(mainCamera);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);        		
		
		glDisable(GL_TEXTURE_2D);	
		
		heightMap->draw(mainCamera); //display heightmap	
		
		//draw clothoid here (if there is one)
		if (clothoidSpline!=NULL) {

			if (guiControls->getClosedButtonPressed()) {
				/*
				if (tanImportance<5.0)
					tanImportance*=1.01;
								
				clothoidSpline->DoOneConvergeIter(tanImportance);
				clothoidSpline->setupArcLengthSamples();
				clothoidSpline->setupFitTransform(1.0);
				clothoidSpline->setupFineSamples(0.1);				

				curIterationOfClosing++;
				*/
			}

			glDisable(GL_DEPTH_TEST);
			clothoidSpline->draw();
			setOrthogonal();
			clothoidSpline->drawCurvaturePlot2D();
			setPerspective();
			glEnable(GL_DEPTH_TEST);

		}

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHTING);	

	glPopMatrix();    	
	
	//display GUI elements
	setOrthogonal();
	guiControls->draw();
	setPerspective();

	/*
	if (clothoidSpline!=NULL) {

		char *curvIntAsStr=new char[100];
		sprintf(curvIntAsStr,"curvInt: %f tanImportance: %f curIteration: %i closingError: %f",clothoidSpline->GetCurvatureIntegral(0,clothoidSpline->GetNumCurvaturePoints()-1),tanImportance,curIterationOfClosing,closingError);
		DrawText(10,10,curvIntAsStr,0,0,0);
		delete [] curvIntAsStr;
	}
	*/

	//
	if (IsKeyPressed('a'))
		clothoidSpline->outputToIllustratorAI("out.ai");
	if (IsKeyPressed('m'))
		clothoidSpline->outputToMayaMEL("out.mel");
	if (IsKeyPressed('s'))
		clothoidSpline->outputToSVG("out.svg");

	//display lasso
	if (lassoMenu->getNumPoints()>=2)
		lassoMenu->draw();

    glutSwapBuffers(); 
    
}

void Viewer::idle() {	

	//key inputs to translate	
	if (IsKeyPressed('f')) {	

		if (interpCameraVal>=1.0f) {
			interpCameraVal=0.0f;
			lastCamera->setCamera(mainCamera);
			interpCamera->setCamera(mainCamera);
			
			mainCamera->setTrackPosition(heightMap->getDefaultCameraTrackPos());
			mainCamera->setTrackDist(heightMap->getDefaultCameraTrackDist());
			mainCamera->setSpin(0.0);
			mainCamera->setTilt(55.0f);			

		}

	}

	glutPostRedisplay();	

}
 
void Viewer::passiveMouseMotion(int x, int y) {
        
}
    
void Viewer::mouseMotion(int x, int y) {	        				

	int mouse_x_diff=mouse_x-x;
	int mouse_y_diff=mouse_y-y;
	mouse_x=x;
	mouse_y=y;	

	if (guiControls->getSelected()>0)
		guiControls->mouseMotion(x,y);

    //modify spin/tilt accordingly    	
	if (mouse_b1&&specialModifiers==GLUT_ACTIVE_ALT) {
		mainCamera->setSpin(mainCamera->getSpin()-mouse_x_diff);
		mainCamera->setTilt(mainCamera->getTilt()-mouse_y_diff);			

	}	
	else if (mouse_b1) {		

		glPushMatrix();
			
			updateCamera(mainCamera);
			updatePickray();		
		
			if (guiControls->getSelected()>0) {

				//do GUI stuff
				
				//RN_Edge::piecePenalty=guiControls->getClothoidPenalty();
				//RN_Edge::g1discontCurvThresh=guiControls->getG1DiscontCurvThresh();
				//RN_Edge::endpointWeight=guiControls->getEndpointWeight();		

				if (clothoidSpline!=NULL) {
					vector <Vector> curPointSet=clothoidSpline->getPointSet();				

					delete clothoidSpline;

					clothoidSpline=new ClothoidSpline(curPointSet, guiControls->getClothoidPenalty(),
						guiControls->getG3ButtonPressed(), guiControls->getG1ButtonPressed(),
						guiControls->getG1DiscontCurvThresh(), guiControls->getEndpointWeight(),
						guiControls->getClosedButtonPressed());

				}

			}			
			else {
				
				lassoMenu->addPoint(Vector(x,screenHeight-y,0),lastpickray);

			}
		glPopMatrix();		
		
	}	
	else if (mouse_b2&&specialModifiers==GLUT_ACTIVE_ALT) {
		mainCamera->setTrackDist(mainCamera->getTrackDist()*pow(1.02f,mouse_y_diff));		

	}
	else if (mouse_b3&&specialModifiers==GLUT_ACTIVE_ALT) {
		Vector xPlane;
		mainCamera->updateTrackPosition();
		xPlane.CrossProduct(Vector(0.0f,1.0f,0.0f),mainCamera->getTrackPosition()-mainCamera->getPosition());
		Vector yPlane=Vector(0.0f,1.0f,0.0f);
		yPlane=yPlane.GetRotatedAxis(mainCamera->getTilt(),xPlane);
		xPlane.Normal();
		yPlane.Normal();
		
		mainCamera->setTrackPosition(mainCamera->getTrackPosition()-xPlane*mouse_x_diff-yPlane*mouse_y_diff);
		mainCamera->setPosition(mainCamera->getPosition()-xPlane*mouse_x_diff-yPlane*mouse_y_diff);

	}

	glutPostRedisplay();
    
}

void Viewer::mouseButton(int button, int state, int x, int y) {       
             
	mouse_x=x;
    mouse_y=y;				

	specialModifiers=glutGetModifiers();

	if (button==GLUT_LEFT_BUTTON) {		
		if (state==GLUT_DOWN) {
			mouse_b1=true;		

			guiControls->mouseClick(mouse_x,mouse_y,true);

			if (guiControls->getSelected()>0) {

				if (clothoidSpline!=NULL) {
					vector <Vector> curPointSet=clothoidSpline->getPointSet();				

					delete clothoidSpline;

					clothoidSpline=new ClothoidSpline(curPointSet, guiControls->getClothoidPenalty(),
						guiControls->getG3ButtonPressed(), guiControls->getG1ButtonPressed(),
						guiControls->getG1DiscontCurvThresh(), guiControls->getEndpointWeight(),
						guiControls->getClosedButtonPressed());					

					if (guiControls->getClosedButtonPressed()) {

						curIterationOfClosing=0;
						tanImportance=0.1;
						while (curIterationOfClosing<1000&&clothoidSpline->DoOneConvergeIter(tanImportance)>0.002) { 
						
							if (tanImportance<5.0)
								tanImportance*=1.01;
			
							/*
							clothoidSpline->DoOneConvergeIter(tanImportance);
							clothoidSpline->setupArcLengthSamples();
							clothoidSpline->setupFitTransform(1.0);
							clothoidSpline->setupFineSamples(0.1);
							*/

							curIterationOfClosing++; 								

						};

						closingError=clothoidSpline->DoOneConvergeIter(tanImportance);
						clothoidSpline->setupArcLengthSamples();
						clothoidSpline->setupFitTransform(1.0);
						clothoidSpline->setupFineSamples(0.1);
					}

				}				

			}
			
		}
		else if (state==GLUT_UP) {
			mouse_b1=false;			

			guiControls->mouseClick(mouse_x,mouse_y,false);			
			tanImportance=0.1;

			//draw a road in this case
			if (guiControls->getSelected()==0) {
				if (lassoMenu->getNumPoints()>=2) {

					lassoPoints3D=heightMap->selectWithRays(mainCamera->getPosition(), lassoMenu->getAllPickrays());
					
					//filter the input points (enforce a min dist between neighbours so theyre spaced reasonably)
					for (int i=0;i<lassoPoints3D.size();i++) {
						for (int j=i+1;j<lassoPoints3D.size();j++) {
							if ((lassoPoints3D[i]-lassoPoints3D[j]).GetLength()<MIN_DIST_BETWEEN_POINTS) {
								lassoPoints3D.erase(lassoPoints3D.begin()+j);
								j=i;
							}
						}
					}

					if (lassoPoints3D.size()>=3) {

						if (clothoidSpline!=NULL)
							delete clothoidSpline;

						//create the clothoid!
						clothoidSpline=new ClothoidSpline(lassoPoints3D, guiControls->getClothoidPenalty(),
							guiControls->getG3ButtonPressed(), guiControls->getG1ButtonPressed(),
							guiControls->getG1DiscontCurvThresh(), guiControls->getEndpointWeight(),
							guiControls->getClosedButtonPressed());

						

						if (guiControls->getClosedButtonPressed()) {

							curIterationOfClosing=0;
							tanImportance=0.1;
							while (curIterationOfClosing<1000&&clothoidSpline->DoOneConvergeIter(tanImportance)>0.002) { 
							
								if (tanImportance<5.0)
									tanImportance*=1.01;
				
								/*
								clothoidSpline->DoOneConvergeIter(tanImportance);
								clothoidSpline->setupArcLengthSamples();
								clothoidSpline->setupFitTransform(1.0);
								clothoidSpline->setupFineSamples(0.1);
								*/

								curIterationOfClosing++; 								

							};

							closingError=clothoidSpline->DoOneConvergeIter(tanImportance);
							clothoidSpline->setupArcLengthSamples();
							clothoidSpline->setupFitTransform(1.0);
							clothoidSpline->setupFineSamples(0.1);
						}

					}
				}
			}
			
			//delete lasso list
			lassoMenu->clear();
			lassoPoints3D.clear();

		}		
		
    }
    else if (button==GLUT_RIGHT_BUTTON) {
        if (state==GLUT_DOWN) 
            mouse_b2=true;
        else if (state==GLUT_UP)
            mouse_b2=false;
    } 
	else if (button==GLUT_MIDDLE_BUTTON) {
		if (state==GLUT_DOWN) 
            mouse_b3=true;
        else if (state==GLUT_UP)
            mouse_b3=false;
	}

	glutPostRedisplay();

}

void Viewer::changeSize(int width, int height) {

	screenWidth=width;
    screenHeight=height;    		

	//reset the perspective
	setPerspective();		

	glutPostRedisplay();

}

Camera *Viewer::getCamera() {
	return mainCamera;
}

void Viewer::getMouseXY(int &x, int &y) {
	x=mouse_x;
	y=mouse_y;
}
