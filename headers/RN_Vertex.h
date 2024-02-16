/*
* Header file of class RN_Node.  RN (Road Network) node.
*/

#ifndef RN_VERTEX_H
#define RN_VERTEX_H

#include <QtOpenGL>
#include <GL/glu.h>

#include <vector>

#include "Vector.h"
#include "Collision.h"
#include "UI_Selectable.h"

#define RAYNODESELECTRADIUS 2.0

using namespace std;

class RN_Vertex : public UI_Selectable {

	public:

	RN_Vertex();
	RN_Vertex(Vector pos_xyz);

	void selectWithRay(Vector pos_xyz, Vector rayDir);
	void selectWithClosedCurve(vector <Vector> pointSet);
	void selectWithClosedCurve2D(vector <Vector> pointSet);

	void draw();
	Vector getPos();
	void setPos(Vector pos_xyz);

	private:

	Vector pos_xyz;

};

#endif
