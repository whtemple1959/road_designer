#ifndef COLLISION_H
#define COLLISION_H

#include "Vector.h"

class Collision {

	public:

	Collision();

	static Vector *LineTriangleCollide(Vector pos_xyz, Vector pickray, Vector v1, Vector v2, Vector v3);
	static bool LineSegmentsCollide(Vector line1A, Vector line1B, Vector line2A, Vector line2B, Vector &intersection);	

};

#endif