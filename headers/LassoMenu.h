#ifndef _LASSOMENU_H
#define _LASSOMENU_H

#include <QtOpenGL>

#include <vector>

#include "Vector.h"
#include "Collision.h"
#include "Viewer.h"

using namespace std;

class Viewer;

class LassoMenu {

	public:

        LassoMenu(QGLWidget * v);
	void addPoint(Vector p2d, Vector pick);
	Vector getPoint(int index);
	Vector getPickray(int index);
	vector <Vector> getAllPoints();
        void getLassoPoints(vector <Vector> & lassoPoints);
	Vector getLassoCentroid();
	vector <Vector> getAllPickrays();
	vector <Vector> getLassoPickrays();
	int getNumPoints();
	void clear();
        void draw(int mousex, int mousey);
        void drawAsOpen(int mousex, int mousey);

	bool isClosed();
        void setPieMenuButtons(vector <vector <char *> > buttons, vector <vector <bool> > enabled);
	void setPieMenuVisible(bool v);
	void setPieMenuCentre(Vector v);
	bool getPieMenuVisible();
	void setPieMenuColour(Vector dark, Vector light);
	void setUseSpecialLabels(int labels);
	
	char *getSelectedButtonText();
	bool getButtonChanged();

	private:

        void drawPieMenu(int mousex, int mousey);
	void drawPieMenuLabel(float startAngle, float endAngle, int texIndex, float startRad, float endRad);

	void setOrthogonal();

	vector <Vector> points;
	vector <Vector> pickrays;	

	bool closed;
	bool needComputeClosed;
	Vector closedPoint;
	int closeStartIndex;
	int closeEndIndex;

        QGLWidget * m_pViewer;

	//pie menu stuff
	Vector pieMenuCentre;
	bool pieMenuVisible;
        vector <vector <char *> > pieMenuButtons;
        vector <vector <bool> > pieMenuButtonEnabled;

	int mouseButton, mouseSubButton;
	bool buttonChanged;
	Vector curColourDark;
	Vector curColourLight;

	//texture indexes for pie menu labels
	int pieMenuUseSpecialLabels; //0 - don't, 1 - usual, 2 - camera stuff
	int texView, texFoliage, texTiming, texDeletePath, texPlay, texBreakout, texCrossing, texPath;
	int texClose, texMidway, texBirdseye, texSparse, texDense, texSlow, texAverage, texFast, texTwisty, texStraighten, texOverunder, texIntersect, texUnderover, texLens;
	int texFree, texHelicopter, texHood, texInside, texStop, texBehind;

};

#endif
