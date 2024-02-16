#include "RN_Vertex.h"

RN_Vertex::RN_Vertex() {
	selected=false;
}

RN_Vertex::RN_Vertex(Vector pos_xyz) {
	this->pos_xyz=pos_xyz;
}

void RN_Vertex::selectWithRay(Vector pos_xyz, Vector rayDir) {	

	//intersection between line/sphere calculated by finding shortest distance of point to line
	//if this distance falls below sphere radius, then intersection occurs
	selected=Collision::LineSphereCollide(this->pos_xyz, RAYNODESELECTRADIUS, pos_xyz, rayDir);

}

void RN_Vertex::selectWithClosedCurve(vector <Vector> pointSet) {	

	selected=false;

	for (int i=1;i<pointSet.size()-1;i++) {
		selected=Collision::PointTriangleCollide(
			Vector(pos_xyz.x,0,pos_xyz.z),
			Vector(pointSet[0].x,0,pointSet[0].z),
			Vector(pointSet[i].x,0,pointSet[i].z),
			Vector(pointSet[i+1].x,0,pointSet[i+1].z));
		if (selected) break;
	}
}

void RN_Vertex::selectWithClosedCurve2D(vector <Vector> pointSet) {	

	selected=false;

	//first, gluProject this vertex
	GLdouble model_view[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLdouble eachxval, eachyval, eachzval;

	gluProject(pos_xyz.x,pos_xyz.y,pos_xyz.z,
					model_view, projection, viewport, &eachxval, &eachyval, &eachzval);

	for (int i=1;i<pointSet.size()-1;i++) {
		//then do collision with triangles of 2d pointset

		selected=Collision::PointTriangleCollide(
			Vector(eachxval,eachyval,0),
			Vector(pointSet[0].x,pointSet[0].y,0),
			Vector(pointSet[i].x,pointSet[i].y,0),
			Vector(pointSet[i+1].x,pointSet[i+1].y,0));
		if (selected) break;
	}
}

Vector RN_Vertex::getPos() {
	return pos_xyz;
}

void RN_Vertex::setPos(Vector pos_xyz) {
	this->pos_xyz=pos_xyz;
}

void RN_Vertex::draw() {		

	if (!selected) {
		glColor3f(0.4,0.4,0.4);				
	}
	else {
		glColor3f(1.0,1.0,1.0);		
	}	

	glPushMatrix();
		glTranslatef(pos_xyz.x,pos_xyz.y,pos_xyz.z);
                gluSphere(gluNewQuadric(), 5.0, 20, 20);
	glPopMatrix();
	
}
