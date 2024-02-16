/*
* Header file of class RN_Intersection. 
*/

#ifndef _RN_INTERSECTION_H
#define _RN_INTERSECTION_H

#include <QtOpenGL>
#include <GL/glu.h>

#include <vector>

#include "RN.h"
#include "RN_Edge.h"
#include "Vector.h"
#include "Collision.h"
#include "UI_Selectable.h"

#define RELATION_INTERSECT 0
#define RELATION_OVERUNDER 1
#define RELATION_UNDEROVER 2

using namespace std;

class RN;
class RN_Edge;

class RN_Intersection : public UI_Selectable {

	public:

	RN_Intersection(RN_Edge *e1, int s1, RN_Edge *e2, int s2);
	
	void selectWithRay(Vector pos_xyz, Vector rayDir);
        void selectWithClosedCurve(vector <Vector>  & pointSet);
        void selectWithClosedCurve2D(vector <Vector> & pointSet);

	void draw();

	void doUpdate();
	RN_Edge *getEdge(int whichEdge);
	int getSegment(int whichEdge);	
	void setSegment(int whichEdge, int newSegment);
	void setEdgeSegment(int whichEdge, RN_Edge *newEdge, int newSegment);

	void setRelation(int relate);
	int getRelation();
	float getHeight(int whichEdge);
	void raiseEdgeAbove(RN_Edge *aboveEdge, int aboveSeg, RN_Edge *belowEdge, int belowSeg, float amountToRaise);

	void doIntersectUpdate(RN_Edge *oldEdge, vector <RN_Edge *> newEdges);

	RN_Intersection *getClone();
	void update();

	void changeIntersect(char *changeToStr);
	Vector getIntersectPoint();	

	private:	

	RN_Edge *edge1;
	int segment1;
	RN_Edge *edge2;
	int segment2;
	int relation;

	Vector point1;
	Vector point2;
	Vector intersectVec;	

};

#endif
