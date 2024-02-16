#include "RN_Intersection.h"

RN_Intersection::RN_Intersection(RN_Edge *e1, int s1, RN_Edge *e2, int s2) {
	
	edge1=e1;
	segment1=s1;
	edge2=e2;
	segment2=s2;	

	point1=edge1->getPoint(segment1);
	point1.y=0.0f;
	point2=edge2->getPoint(segment2);
	point2.y=0.0f;

}

void RN_Intersection::setRelation(int relate) {
	relation=relate;
}

int RN_Intersection::getRelation() {
	return relation;
}

void RN_Intersection::draw() {	

	Vector selectPoint=(edge1->getPoint(segment1)+edge1->getPoint(segment1+1)+
							edge2->getPoint(segment2)+edge2->getPoint(segment2+1))/4.0f;
	float hoverHeight=20.0f;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f,0.0f,0.0f,0.25f);
	glPushMatrix();
		glTranslatef(selectPoint.x,selectPoint.y+hoverHeight,selectPoint.z);
                gluSphere(gluNewQuadric(), MIN_DIST_BETWEEN_POINTS/4.0, 15, 15);
	glPopMatrix();
	glBegin(GL_LINES);
		glVertex3f(selectPoint.x,selectPoint.y+hoverHeight,selectPoint.z);
		/*
		glVertex3f((edge1->getPoint(segment1).x+edge1->getPoint(segment1+1).x)/2.0f,
			(edge1->getPoint(segment1).y+edge1->getPoint(segment1+1).y)/2.0f,
			(edge1->getPoint(segment1).z+edge1->getPoint(segment1+1).z)/2.0f);
			*/
		glVertex3f(edge1->getPoint(segment1).x,
			edge1->getPoint(segment1).y,
			edge1->getPoint(segment1).z);

		glVertex3f(selectPoint.x,selectPoint.y+hoverHeight,selectPoint.z);
		/*
		glVertex3f((edge2->getPoint(segment2).x+edge2->getPoint(segment2+1).x)/2.0f,
			(edge2->getPoint(segment2).y+edge2->getPoint(segment2+1).y)/2.0f,
			(edge2->getPoint(segment2).z+edge2->getPoint(segment2+1).z)/2.0f);
			*/
		glVertex3f(edge2->getPoint(segment2).x,
			edge2->getPoint(segment2).y,
			edge2->getPoint(segment2).z);
	glEnd();

	glColor4f(0.0f,0.0f,1.0f,0.25f);
	glBegin(GL_LINES);
		glVertex3f(selectPoint.x,selectPoint.y+hoverHeight,selectPoint.z);
		glVertex3f(point1.x,point1.y,point1.z);

		glVertex3f(selectPoint.x,selectPoint.y+hoverHeight,selectPoint.z);
		glVertex3f(point2.x,point2.y,point2.z);
	glEnd();

	glDisable(GL_BLEND);

}

void RN_Intersection::selectWithRay(Vector pos_xyz, Vector rayDir) {	

	selected=false;

}

void RN_Intersection::selectWithClosedCurve(vector <Vector> & pointSet) {

	selected=false;

}

void RN_Intersection::selectWithClosedCurve2D(vector <Vector> & pointSet) {

	Vector selectPoint=(edge1->getPoint(segment1)+edge1->getPoint(segment1+1)+
							edge2->getPoint(segment2)+edge2->getPoint(segment2+1))/4.0f;
	
	//first, gluProject this vertex
	GLdouble model_view[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLdouble eachxval, eachyval, eachzval;
	gluProject(selectPoint.x,selectPoint.y,selectPoint.z,
					model_view, projection, viewport, &eachxval, &eachyval, &eachzval);

	selected=false;
	for (int i=1;i<pointSet.size()-1;i++) {
		//do collision with triangles of 2d pointset
		bool collision=Collision::PointTriangleCollide(
			Vector(eachxval,eachyval,0),
			Vector(pointSet[0].x,pointSet[0].y,0),
			Vector(pointSet[i].x,pointSet[i].y,0),
			Vector(pointSet[i+1].x,pointSet[i+1].y,0));

		if (collision) {
			selected=true;
			break;
		}
	}	

}

Vector RN_Intersection::getIntersectPoint() {

	Collision::LineSegmentsCollide(edge1->getPoint(segment1), edge1->getPoint(segment1+1), 
							edge2->getPoint(segment2), edge2->getPoint(segment2+1), 
							intersectVec);
	return intersectVec;

}

void RN_Intersection::doUpdate() {

	//for a cross and intersection, this means ensuring the points line up nice

	if (edge1->getPointAttrib(segment1, POINT_ATTRIB_INTERSECT)) {
	
		getIntersectPoint();

		Vector edge1Vec=edge1->getPoint(segment1+1)-edge1->getPoint(segment1);
		edge1Vec.y=0.0f;
		Vector edge2Vec=Vector(edge1Vec.z, 0.0f, -edge1Vec.x);
		if (edge2Vec.DotProduct3(edge2->getPoint(segment2+1)-edge2->getPoint(segment2))<0.0f)
			edge2Vec=edge2Vec*(-1.0f);

		edge1Vec.SetLength(MIN_DIST_BETWEEN_POINTS*0.5f);
		edge2Vec.SetLength(MIN_DIST_BETWEEN_POINTS*0.5f);
		
		if (segment1-1>=0)
			edge1->setPoint(segment1-1, (intersectVec-edge1Vec*3.0f+edge1->getPoint(segment1-1))/2.0f);
		edge1->setPoint(segment1, intersectVec-edge1Vec);		
		edge1->setPoint(segment1+1, intersectVec+edge1Vec);
		if (segment1+2<edge1->getNumPoints())
			edge1->setPoint(segment1+2, (intersectVec+edge1Vec*3.0f+edge1->getPoint(segment1+2))/2.0f);

		if (segment2-1>=0)
			edge2->setPoint(segment2-1, (intersectVec-edge2Vec*3.0f+edge2->getPoint(segment2-1))/2.0f);
		edge2->setPoint(segment2, intersectVec-edge2Vec);
		edge2->setPoint(segment2+1, intersectVec+edge2Vec);
		if (segment2+2<edge2->getNumPoints())
			edge2->setPoint(segment2+2, (intersectVec+edge2Vec*3.0f+edge2->getPoint(segment2+2))/2.0f);		

	}
	
	//for a cross and under or over, this means changing heights appropriately
	/*
	if (edge1->getPointAttrib(segment1, POINT_ATTRIB_OVER)) {		
		raiseEdgeAbove(edge1,segment1,edge2,segment2);
	}
	else if (edge2->getPointAttrib(segment2, POINT_ATTRIB_OVER)) {		
		raiseEdgeAbove(edge2,segment2,edge1,segment1);
	}
	*/

	//keep new points	
	point1=edge1->getPoint(segment1);
	point1.y=0.0f;
	point2=edge2->getPoint(segment2);
	point2.y=0.0f;

}

RN_Edge *RN_Intersection::getEdge(int whichEdge) {
	if (whichEdge==0)
		return edge1;
	else 
		return edge2;	
}

int RN_Intersection::getSegment(int whichEdge) {
	if (whichEdge==0)
		return segment1;
	else 
		return segment2;	
}

void RN_Intersection::setSegment(int whichEdge, int newSegment) {
	if (whichEdge==0)
		segment1=newSegment;
	else
		segment2=newSegment;
}

void RN_Intersection::setEdgeSegment(int whichEdge, RN_Edge *newEdge, int newSegment) {
	if (whichEdge==0) {
		edge1=newEdge;
		segment1=newSegment;
	}
	else {
		edge2=newEdge;
		segment2=newSegment;
	}
}

void RN_Intersection::raiseEdgeAbove(RN_Edge *aboveEdge, int aboveSeg, RN_Edge *belowEdge, int belowSeg, float amountToRaise) {	

	float distance;
	int curIndex;

	const float maxdistance=MIN_DIST_BETWEEN_POINTS*5.0;	

	for (int i=0;i<2;i++) {

		if (i==0)
			curIndex=aboveSeg;
		else if (i==1)
			curIndex=aboveSeg+1;

		distance=0.0f;

		while (distance<maxdistance&&curIndex>=0&&curIndex<aboveEdge->getNumPoints()) {
					
			aboveEdge->setPoint(curIndex, aboveEdge->getPoint(curIndex)+
				Vector(0.0f,amountToRaise*cos(distance/maxdistance*(M_PI/2.0f)),0.0f));							
			aboveEdge->setPointAttrib(curIndex, POINT_ATTRIB_BRIDGE, true);	

			if (i==0) {

				if (curIndex>0)
					distance+=(aboveEdge->getPoint(curIndex)-aboveEdge->getPoint(curIndex-1)).GetLength();
				else
					distance=maxdistance;

				curIndex--;
			}
			else if (i==1) {

				if (curIndex<aboveEdge->getNumPoints()-1)
					distance+=(aboveEdge->getPoint(curIndex)-aboveEdge->getPoint(curIndex+1)).GetLength();
				else
					distance=maxdistance;

				curIndex++;
			}

		};

	}

	aboveEdge->updateStructure();

}

void RN_Intersection::doIntersectUpdate(RN_Edge *oldEdge, vector <RN_Edge *> newEdges) {
	
	for (int k=0;k<2;k++) {
		bool updateMade=false;
		if (oldEdge==getEdge(k)) {			
			for (int i=0;i<newEdges.size();i++) {
				for (int j=0;j<newEdges[i]->getNumPoints();j++) {
					if (getEdge(k)->getPoint(getSegment(k))==newEdges[i]->getPoint(j)) {
						setEdgeSegment(k,newEdges[i],j);
						printf("Made an update!\n");
						updateMade=true;
						break;
					}
				}
				if (updateMade) break;
			}			
		}
	}			

}

RN_Intersection *RN_Intersection::getClone() {

        RN_Intersection * newInt = new RN_Intersection(edge1,segment1,edge2,segment2);
	newInt->setRelation(relation);
	return newInt;

}

void RN_Intersection::update() {		

	int tempsegment1=segment1;
	int tempsegment2=segment2;

	segment1=-1;
	segment2=-1;

	for (int i=0;i<edge1->getNumPoints();i++) {
		if (edge1->getPoint(i).x==point1.x&&edge1->getPoint(i).z==point1.z) {
			segment1=i;
			break;
		}
	}

	if (segment1==-1&&edge1!=edge2) {
		for (int i=0;i<edge2->getNumPoints();i++) {
			if (edge2->getPoint(i).x==point1.x&&edge2->getPoint(i).z==point1.z) {
				segment1=i;
				edge1=edge2;
				break;
			}
		}
	}

	for (int i=0;i<edge2->getNumPoints();i++) {
		if (edge2->getPoint(i).x==point2.x&&edge2->getPoint(i).z==point2.z) {
			segment2=i;
			break;
		}
	}	

	if (segment2==-1&&edge1!=edge2) {
		for (int i=0;i<edge1->getNumPoints();i++) {
			if (edge1->getPoint(i).x==point2.x&&edge1->getPoint(i).z==point2.z) {
				segment2=i;
				edge2=edge1;
				break;
			}
		}	
	}

	if (segment1==-1||segment2==-1) {
		printf("Problem with intersection segment index!\n");		
		printf("Segment1: %i Segment2: %i\n",segment1,segment2);
	}

}

void RN_Intersection::changeIntersect(char *changeToStr) {

	if (strcmp(changeToStr,"Intersect")==0) {
				
		edge1->setPointAttrib(segment1, POINT_ATTRIB_INTERSECT, true);
		edge1->setPointAttrib(segment1, POINT_ATTRIB_DONOTRENDER, false);
		
		edge2->setPointAttrib(segment2, POINT_ATTRIB_INTERSECT, true);
		edge2->setPointAttrib(segment2, POINT_ATTRIB_DONOTRENDER, true);	

		relation=RELATION_INTERSECT;

	}
	else if (strcmp(changeToStr,"Over- Under")==0||
				strcmp(changeToStr,"Under-Over")==0) {

		if (strcmp(changeToStr,"Over- Under")==0) 	
			relation=RELATION_OVERUNDER;		
		else 			
			relation=RELATION_UNDEROVER;		

		edge1->setPointAttrib(segment1, POINT_ATTRIB_INTERSECT, false);
		edge1->setPointAttrib(segment1, POINT_ATTRIB_DONOTRENDER, false);		
		edge2->setPointAttrib(segment2, POINT_ATTRIB_INTERSECT, false);
		edge2->setPointAttrib(segment2, POINT_ATTRIB_DONOTRENDER, false);
		
	}

	edge1->attachToGround(segment1);
	edge2->attachToGround(segment2);

	doUpdate();

	edge1->updateStructure();
	edge2->updateStructure();	

}

float RN_Intersection::getHeight(int whichEdge) {	

	if (whichEdge==0) 
		return (edge1->getPoint(segment1).y+edge1->getPoint(segment1+1).y)/2.0f;
	else
		return (edge2->getPoint(segment2).y+edge2->getPoint(segment2+1).y)/2.0f;
	

}
