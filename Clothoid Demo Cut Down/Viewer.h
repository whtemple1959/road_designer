/*
 * Viewer.h - header file for Viewer,
 * a class which encompasses the interface's 3D view of the currently
 * generated track
 */ 

#ifndef _VIEWER_H
#define _VIEWER_H

#include <windows.h>
#include <gl/glew.h>
#include <glut.h>
#include <IL/ilut.h>

#include <stdlib.h>
#include <vector>

#include "keys.h"

#include "Vector.h"
#include "HeightMap.h"
#include "Collision.h"
#include "Camera.h"
#include "LassoMenu.h"
#include "ClothoidSpline.h"

#include "GUIcontrols.h"

#define MIN_DIST_BETWEEN_POINTS 4.0f

using namespace std;

class LassoMenu;

class Viewer {
    
    public:
    
    static void init(int w, int h);
    static void draw(void);
    static void mouseMotion(int x, int y);
    static void passiveMouseMotion(int x, int y);
    static void mouseButton(int button, int state, int x, int y);    
	static void idle();    	
	static void changeSize(int width, int height);	

	static void DrawText(GLint x, GLint y, char* s, GLfloat r, GLfloat g, GLfloat b);
	static void makeSelection();
	static Vector *getCollision(Vector pickray);	
	static Camera *getCamera();
	static void getMouseXY(int &x, int &y);

    private:        	

	static void setPerspective();
	static void setOrthogonal();

	static void trackPosition(Vector position);
	static void updatePickray();
	static void updateCamera(Camera *whichCam);	

    static double rotationAngle, viewAngle;
    static long lastFrameUpdate;
        
    static int screenWidth;
    static int screenHeight;

	static HeightMap *heightMap;
	static LassoMenu *lassoMenu;

	//camera parameters
	static Camera *lastCamera;
	static Camera *mainCamera;	
	static Camera *interpCamera;
	static float interpCameraVal;

	static int mouse_x;
	static int mouse_y;
	static bool mouse_b1;
	static bool mouse_b2;
	static bool mouse_b3;
    		
	static vector <Vector> lassoPoints3D;

	static Vector lastCollision;	
	static Vector lastpickray;

	static GUIcontrols *guiControls;
	static ClothoidSpline *clothoidSpline;

	static float curClothoidPenalty;
	static double tanImportance;
	static int curIterationOfClosing;
	static double closingError;

	//timer
	//static timestamp curTime;
	//static timestamp lastTime;
	//static float timeDiffms;

	static int specialModifiers;	

};

#endif
