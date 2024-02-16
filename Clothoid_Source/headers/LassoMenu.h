#ifndef _LASSOMENU_H
#define _LASSOMENU_H

#include <vector>
#include <windows.h>
#include <gl/glew.h>
#include <glut.h>

#include "Vector.h"
#include "Collision.h"
#include "Viewer.h"

using namespace std;

class Viewer;

class LassoMenu {

	public:

	LassoMenu();
	void addPoint(Vector p2d, Vector pick);
	Vector getPoint(int index);
	Vector getPickray(int index);
	vector <Vector> getAllPoints();
	vector <Vector> getLassoPoints();
	Vector getLassoCentroid();
	vector <Vector> getAllPickrays();
	vector <Vector> getLassoPickrays();
	int getNumPoints();
	void clear();
	void draw();
	void drawAsOpen();

	bool isClosed();

	private:
	
	void drawText(GLint x, GLint y, char* s);
	void setOrthogonal();

	vector <Vector> points;
	vector <Vector> pickrays;	

	bool closed;
	bool needComputeClosed;
	Vector closedPoint;
	int closeStartIndex;
	int closeEndIndex;

	int mouseButton;
	bool buttonChanged;

};

#endif