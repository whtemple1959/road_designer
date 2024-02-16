/*
 * sketchInteract
 *
 * sketchInteract is a program which faciliates interaction within a general
 * framework for sketch-based environment creation.
 *
 * Author: James McCrae
 * Last update: Mar. 2/08
 */

#include <windows.h>
#include <gl/glew.h>
#include <glut.h>
#include <IL/ilut.h>

#include <stdlib.h>

#include "keys.h"
#include "Viewer.h"

static int winWidth=1024;
static int winHeight=768;

//Global interface classes
Viewer *viewer;

/* Program entry point */

int 
main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitWindowSize(winWidth,winHeight);
    glutInitWindowPosition(0,0);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);

    glutCreateWindow("Sketching Piecewise Clothoid Curves");

	//callbacks
	glutDisplayFunc(Viewer::draw);
	glutMotionFunc(Viewer::mouseMotion);
	glutPassiveMotionFunc(Viewer::passiveMouseMotion);
	glutMouseFunc(Viewer::mouseButton);    
	glutIdleFunc(Viewer::idle);
	glutReshapeFunc(Viewer::changeSize);

	 //setup keyboard
    InitKeyboard();

    ilInit();  
    iluInit();      
    ilutRenderer(ILUT_OPENGL);                  		

	Viewer::init(winWidth,winHeight);

    glutMainLoop();

    return EXIT_SUCCESS;
}

