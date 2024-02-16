#include "RN.h"

RN::RN(HeightMap *heightMap) {

	//make and delete an rn_edge (this preloads road network meshes)
	RN_Edge *tempEdge=new RN_Edge(); 
	delete tempEdge;	
	RN_Edge::setHeightMap(heightMap);

	objManager=new ObjectManager(this, heightMap);

}

//this will be called a lot for the preview network!
RN::~RN() {
        for (int i=0;i<edgeList.size();i++) {
		delete edgeList[i];
        }
	edgeList.clear();

        for (int i=0;i<intersectList.size();i++) {
		delete intersectList[i];
        }
	intersectList.clear();
}

//ui selectable interface
void RN::selectWithRay(Vector pos_xyz, Vector rayDir) {

	selectedEdgeList.clear();
	for (int i=0;i<edgeList.size();i++) {
		edgeList[i]->selectWithRay(pos_xyz,rayDir);
		if (edgeList[i]->isSelected()) {
			selectedEdgeList.push_back(i);
			
		}
	}

	selectedIntersectList.clear();
	for (int i=0;i<intersectList.size();i++) {
		intersectList[i]->selectWithRay(pos_xyz,rayDir);
		if (intersectList[i]->isSelected()) {
			selectedIntersectList.push_back(i);
		}
	}

}

void RN::draw() {

    for (int i=0;i<edgeList.size();i++) { //edges rendered as the roads they represent
        edgeList[i]->draw();

        /*
        glColor3f(.2,.2,.8);
        glPushMatrix();
        glTranslatef(edgeList[i]->getPoint(0).x,edgeList[i]->getPoint(0).y,edgeList[i]->getPoint(0).z);
        //glutSolidSphere(MIN_DIST_BETWEEN_POINTS/3.0f,20,20);
        gluSphere(gluNewQuadric(), MIN_DIST_BETWEEN_POINTS/3.0f, 20, 20);
        glPopMatrix();
        glColor3f(.8,.2,.8);
        glPushMatrix();
        glTranslatef(edgeList[i]->getPoint(edgeList[i]->getNumPoints()-1).x,
                     edgeList[i]->getPoint(edgeList[i]->getNumPoints()-1).y,
                     edgeList[i]->getPoint(edgeList[i]->getNumPoints()-1).z);
        //glutSolidSphere(MIN_DIST_BETWEEN_POINTS/3.0f,20,20);
        gluSphere(gluNewQuadric(), MIN_DIST_BETWEEN_POINTS/3.0f, 20, 20);
        glPopMatrix();
        */

    }


    /*
    for (int i=0;i<intersectList.size();i++) { //intersects rendered as red spheres
        intersectList[i]->draw();
    }
    */

    objManager->drawObjectsUnderwater();
}

void RN::drawObjects() {

	objManager->drawObjects();
}

void RN::selectWithClosedCurve(vector <Vector> & pointSet) {

	for (int i=0;i<edgeList.size();i++) {
		edgeList[i]->selectWithClosedCurve(pointSet);
	}
}

void RN::selectWithClosedCurve2D(vector <Vector> & pointSet) {
		
	selectedEdgeList.clear();
	for (int i=0;i<edgeList.size();i++) { 
		edgeList[i]->selectWithClosedCurve2D(pointSet);
		if (edgeList[i]->isSelected()) {
			selectedEdgeList.push_back(i);			
		}
	}

	selectedIntersectList.clear();
	for (int i=0;i<intersectList.size();i++) {
		intersectList[i]->selectWithClosedCurve2D(pointSet);
		if (intersectList[i]->isSelected()) {
			selectedIntersectList.push_back(i);
		}
	}
}

void RN::unSelectAll() {

	for (int i=0;i<edgeList.size();i++) {
		edgeList[i]->unselectAll();
	}
	selectedEdgeList.clear();
	selectedIntersectList.clear();
}

int RN::addEdge(RN_Edge *newEdge) {

	edgeList.push_back(newEdge);
	return edgeList.size()-1;

}

RN_Edge *RN::getEdge(int edgeIndex) {
	return edgeList[edgeIndex];
}

unsigned int RN::getNumEdges() {
	return edgeList.size();
}

vector <RN_Edge *> RN::getSelectedEdges() {

	vector <RN_Edge *> selectedEdges;

        for (int i=0;i<selectedEdgeList.size();i++) {
		selectedEdges.push_back(edgeList[selectedEdgeList[i]]);
        }

	return selectedEdges;
}

vector <RN_Intersection *> RN::getSelectedIntersects() {
	
	vector <RN_Intersection *> selectedIntersects;

	for (int i=0;i<selectedIntersectList.size();i++) 
		selectedIntersects.push_back(intersectList[selectedIntersectList[i]]);

	return selectedIntersects;

}

void RN::addRoad(vector <Vector> lassoPoints3D, HeightMap *heightMap) {
	
	if (lassoPoints3D.size()<2)
		return;	

	RN_Edge *newEdge=new RN_Edge();
	newEdge->setPointSet(lassoPoints3D);
	newEdge->useCatmullRomSpline(true);	

	if (newEdge->getNumPoints()<2)
		return;	
	
	newEdge->useClothoidSpline(heightMap);

		
	
	//for clothoid demo
	/*
	edgeList.clear();
	edgeList.push_back(newEdge);
	return;
	*/

	//modify edge to create bridges over water if necessary
	doWaterBridges(newEdge, heightMap); //(remove for GEOMETRY MODELLING)

	/*//ADDED FOR GEOMETRIC MODELLING EXAMPLE
	edgeList.clear();
	intersectList.clear();
	*/

	if (!doOversketch(newEdge)) {
		addEdge(newEdge);
	}		
	
	//do intersections and over/under processing
	///* REMOVED FOR GEOMETRY MODELLING EXAMPLE
	doIntersections();
	
	while (doOverUnders()) {};
	while (doJoins()) {};	

	doDepthRelations();

	//flatten terrain out where new edge being added
	doPrepareTerrain(newEdge, heightMap); //this is too buggy atm

	objManager->updateClearRange();
	doObjects();
	//*/

	/*
	printf("Numintersects: %i Numedges: %i\n",intersectList.size(),edgeList.size());
	for (int i=0;i<intersectList.size();i++) {
		printf("Intersect %i:\n",i);
		printf("	Segment1: %i ",intersectList[i]->getSegment(0));
		intersectList[i]->getEdge(0)->getPoint(intersectList[i]->getSegment(0)).print();
		printf("	Segment2: %i ",intersectList[i]->getSegment(1));
		intersectList[i]->getEdge(1)->getPoint(intersectList[i]->getSegment(1)).print();
	}
	for (int i=0;i<edgeList.size();i++) {
		printf("Edge %i:\n",i);
		printf("	Startpoint: ");
		edgeList[i]->getPoint(0).print();
		printf("	Endpoint: ");
		edgeList[i]->getPoint(edgeList[i]->getNumPoints()-1).print();
	}
	*/

	GLenum glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
                qDebug("AFTER.. hmmm youre wrong GL Error # %i (%s) \n", glErr, gluErrorString(glErr));
		glErr = glGetError();
	}	

}

void RN::doWaterBridges(RN_Edge *newEdge, HeightMap *heightMap) {

	for (int i=3;i<newEdge->getNumPoints()-3;i++) {
		if (newEdge->getPoint(i).y<=heightMap->getWaterHeight()) {

			int endIndex=-1;
			for (int j=i+1;j<newEdge->getNumPoints()-3;j++){
				if (newEdge->getPoint(j).y>heightMap->getWaterHeight()) {
					endIndex=j-1;
					break;
				}
			}

			if (endIndex==-1) //case - bridge doesnt even cross water back onto land
				continue;
			
			//what needs replacing is between i and endindex
			//create our new bridge edge
			RN_Edge *bridgeEdge=new RN_Edge();
			vector <Vector> bridgePointSet;			
			bridgePointSet.push_back(newEdge->getPoint(i-2));
			bridgePointSet.push_back(newEdge->getPoint(i-1)+Vector(0.0f,MIN_DIST_BETWEEN_POINTS/2.0f,0.0f));
			bridgePointSet.push_back(newEdge->getPoint(endIndex+1)+Vector(0.0f,MIN_DIST_BETWEEN_POINTS/2.0f,0.0f));		
			bridgePointSet.push_back(newEdge->getPoint(endIndex+2));			
			bridgeEdge->setPointSet(bridgePointSet);			
			
			for (int j=0;j<bridgeEdge->getNumPoints();j++) {
				bridgeEdge->setPointAttrib(j,POINT_ATTRIB_BRIDGE,true);
			}
			bridgeEdge->updateWhichRoadMesh();
						
			newEdge->replaceRangeWith(bridgeEdge, i-3, endIndex+3);

		}
	}

}

void RN::doPrepareTerrain(RN_Edge *newEdge, HeightMap *heightMap) {

	int flattenDist=MIN_DIST_BETWEEN_POINTS;

	for (int i=0;i<newEdge->getNumPoints();i++) {
		for (int j=newEdge->getPoint(i).x-flattenDist;j<=newEdge->getPoint(i).x+flattenDist;j++) {
			for (int k=newEdge->getPoint(i).z-flattenDist;k<=newEdge->getPoint(i).z+flattenDist;k++) {

				if (j<0||k<0||j>=heightMap->getWidth()||k>=heightMap->getHeight())
					continue;

				//get point from heightmap
				float eachHeight=heightMap->getMeshHeight(j,k);

				//set point from heightmap if greater than point sampled there
				if (eachHeight>newEdge->getPoint(i).y)
					heightMap->setMeshHeight(j,k,newEdge->getPoint(i).y);

			}
		}
	}

	//make terrain stick up against the road (pretty hacky atm)
	/*
	for (int i=0;i<newEdge->getNumPoints()-1;i++) {
		for (int j=0;j<MIN_DIST_BETWEEN_POINTS;j++) {
			float interp=(float)j/(float)MIN_DIST_BETWEEN_POINTS;
			heightMap->setMeshHeight(newEdge->getPoint(i).x*(1.0f-interp)+newEdge->getPoint(i+1).x*interp,
										newEdge->getPoint(i).z*(1.0f-interp)+newEdge->getPoint(i+1).z*interp,
										newEdge->getPoint(i).y*(1.0f-interp)+newEdge->getPoint(i+1).y*interp);
		}
	}
	*/

}

bool RN::doOversketch(RN_Edge *newEdge) {

	Vector point1=newEdge->getPoint(0);
	Vector point2=newEdge->getPoint(newEdge->getNumPoints()-1);
	Vector tangent1=newEdge->getPoint(1)-newEdge->getPoint(0);
	tangent1.Normal();
	Vector tangent2=newEdge->getPoint(newEdge->getNumPoints()-2)-newEdge->getPoint(newEdge->getNumPoints()-1);
	tangent2.Normal();

	float minDotProdTangent=cos(40.0f*M_PI/180.0f);
	int point1connectSegment;
	int point2connectSegment;	

	for (int i=0;i<edgeList.size();i++) {

		point1connectSegment=-1;
		point2connectSegment=-1;

		for (int j=0;j<edgeList[i]->getNumPoints()-1;j++) {

			if ((point1-edgeList[i]->getPoint(j)).GetLength()>MIN_DIST_BETWEEN_POINTS*0.5f&&
				(point1-edgeList[i]->getPoint(j)).GetLength()<MIN_DIST_BETWEEN_POINTS*2.0f) {
				Vector thisTangent=edgeList[i]->getPoint(j+1)-edgeList[i]->getPoint(j);
				thisTangent.Normal();

				if (fabs(tangent1.DotProduct3(thisTangent))>=minDotProdTangent) 		
					point1connectSegment=j;					

			}

			if ((point2-edgeList[i]->getPoint(j)).GetLength()>=MIN_DIST_BETWEEN_POINTS*0.5f&&
				(point2-edgeList[i]->getPoint(j)).GetLength()<MIN_DIST_BETWEEN_POINTS*2.0f) {
				Vector thisTangent=edgeList[i]->getPoint(j+1)-edgeList[i]->getPoint(j);
				thisTangent.Normal();

				if (fabs(tangent2.DotProduct3(thisTangent))>=minDotProdTangent) 				
					point2connectSegment=j;

			}
		}

		if (point1connectSegment!=-1&&point2connectSegment!=-1&&point1connectSegment!=point2connectSegment&&
			(int)fabs((float)point2connectSegment-(float)point1connectSegment)<edgeList[i]->getNumPoints()-4) {			

			//flip order of newedge if its out of order
			if (point2connectSegment<point1connectSegment) {
				newEdge->reverse();
				int temppoint=point1connectSegment;
				point1connectSegment=point2connectSegment;
				point2connectSegment=temppoint;
			}				

			//only if theres nothing special going on the interval do an oversketch
			bool specialSegment=false;
			for (int j=point1connectSegment;j<=point2connectSegment;j++) {
				//if (edgeList[i]->getPointAttribVal(j)!=0) {
				if (edgeList[i]->getPointAttrib(j,POINT_ATTRIB_CROSS)) {
					specialSegment=true;
					break;
				}
			}


			if (!specialSegment) {			
				//oversketch with this road
				edgeList[i]->replaceRangeWith(newEdge, point1connectSegment, point2connectSegment);
				//update intersect references
				for (int j=0;j<intersectList.size();j++) {					
					for (int k=0;k<2;k++) {
						if (intersectList[j]->getEdge(k)==edgeList[i]) {
							intersectList[j]->update();
						}
					}
				}
				
			} //note that the line doesnt get added if segment had special attributes!
			
			return true;

		}
	}	

	return false;

}

void RN::doIntersections() {

	for (int i=0;i<edgeList.size()-1;i++) {
		for (int j=i+1;j<edgeList.size();j++) {

			int segment1;
			int segment2;
			Vector intersectVec;

			do {
				edgeList[i]->edgesCross(edgeList[j], segment1, segment2, intersectVec);

				if (segment1>=0) {					

					edgeList[i]->setPointAttrib(segment1,POINT_ATTRIB_CROSS,true);
					edgeList[i]->setPointAttrib(segment1,POINT_ATTRIB_INTERSECT,true);	

					edgeList[j]->setPointAttrib(segment2,POINT_ATTRIB_CROSS,true);
					edgeList[j]->setPointAttrib(segment2,POINT_ATTRIB_INTERSECT,true);
					edgeList[j]->setPointAttrib(segment2,POINT_ATTRIB_DONOTRENDER,true);

					RN_Intersection *newIntersection=new RN_Intersection(edgeList[i], segment1, edgeList[j], segment2);
					newIntersection->setRelation(RELATION_INTERSECT);
					newIntersection->doUpdate(); //have to update points and tangents so they conform
					intersectList.push_back(newIntersection);
					
					//finally, update involved edges structure
					edgeList[i]->updateStructure();
					edgeList[j]->updateStructure();					
				}

			} while (segment1>=0);

		}
	}

}

bool RN::doJoins() {

	float maxDistForJoin=MIN_DIST_BETWEEN_POINTS*5.0f;
	float minDotProdForJoin=cos(45.0f*M_PI/180.0f);	
	
	for (int h=0;h<edgeList.size();h++) {		
		for (int i=h;i<edgeList.size();i++) {
			//between each 2 endpoints, determine if the line segment formed
			//intersects any other edge
			for (int eachpts=0;eachpts<4;eachpts++) {

				//0 - start-start
				//1 - start-end
				//2 - end-start
				//3 - end-end
				if (h!=i) {
					if (eachpts==1||eachpts==3) 
						doEdgeReversal(edgeList[i]);
					else if (eachpts==2)
						doEdgeReversal(edgeList[h]);
				}

				Vector point1=edgeList[h]->getPoint(edgeList[h]->getNumPoints()-1);
				Vector tangent1=edgeList[h]->getPoint(edgeList[h]->getNumPoints()-1)-
					edgeList[h]->getPoint(edgeList[h]->getNumPoints()-2);

				Vector point2=edgeList[i]->getPoint(0);
				Vector tangent2=edgeList[i]->getPoint(1)-edgeList[i]->getPoint(0); //inward tangents

				point1.y=0.0f; point2.y=0.0f;

				Vector p2p1diff=point2-point1;
				p2p1diff.Normal();

				tangent1.y=0.0f;
				tangent2.y=0.0f;
				tangent1.Normal();
				tangent2.Normal();
				
				//pass location/tangent tests				
				if ((point1-point2).GetLength()<maxDistForJoin&&
					tangent1.DotProduct3(tangent2)>minDotProdForJoin&&
					p2p1diff.DotProduct3(tangent1)>minDotProdForJoin&&
					p2p1diff.DotProduct3(tangent2)>minDotProdForJoin) {																			
					
					Vector intersectVec;
					vector <RN_Edge *> overEdges;
					vector <int> overEdgeSegments;

					//check for crossing of another edge
					for (int j=0;j<edgeList.size();j++) {
						for (int k=0;k<edgeList[j]->getNumPoints()-1;k++) {
							if (Collision::LineSegmentsCollide(point1, point2, edgeList[j]->getPoint(k), edgeList[j]->getPoint(k+1), intersectVec)&&
									!(j==h&&k>=edgeList[j]->getNumPoints()-2)&&
									!(j==i&&k==0)) {
																																
								overEdges.push_back(edgeList[j]);
								overEdgeSegments.push_back(k);								
							}
						}						
					}

					if ((h!=i)&&overEdges.empty()&&(point2-point1).GetLength()<maxDistForJoin/2.0f) { //join only if no over/unders to process			
						//this is a join, and not an over under
						
						edgeList[h]->concatenateEdge(edgeList[i]);	

						//have to "unreverse" (this must be done after concat and BEFORE structure update!)	
						if (eachpts>=2) {
							doEdgeReversal(edgeList[h]);
											
							/*
							for (int eachInt=0;eachInt<intersectList.size();eachInt++) {								
								if (intersectList[eachInt]->getEdge(0)==edgeList[h])
									intersectList[eachInt]->setSegment(0,intersectList[eachInt]->getSegment(0)+edgeList[i]->getNumPoints());
								if (intersectList[eachInt]->getEdge(1)==edgeList[h])
									intersectList[eachInt]->setSegment(1,intersectList[eachInt]->getSegment(1)+edgeList[i]->getNumPoints());
							}
							*/
						}

						edgeList[h]->updateStructure();

						//remove old entry
						RN_Edge *erasedEdge=edgeList[i];						
						edgeList.erase(edgeList.begin()+i);

						//ensure all intersections are good
						for (int eachInt=0;eachInt<intersectList.size();eachInt++) {	
							intersectList[eachInt]->doIntersectUpdate(erasedEdge,edgeList);
						}
						
						return true; //done
					}		
					else if (i==h&&overEdges.empty()&&(point2-point1).GetLength()<maxDistForJoin/2.0f) {
						//this is a join, and not an over under
						edgeList[h]->concatenatePoint(edgeList[h]->getPoint(0));
						edgeList[h]->setPointAttrib(edgeList[h]->getNumPoints()-1, POINT_ATTRIB_CLOSED_LOOP, true);
						edgeList[h]->updateStructure();							
					
						return true; //done
					}

				}				
				
			}

			//final unreversal (no joins found between h, i)
			if (h!=i)
				doEdgeReversal(edgeList[h]);
		}
	}

	return false;

}

bool RN::doOverUnders() {

	float maxDistForJoin=MIN_DIST_BETWEEN_POINTS*5.0f;
	float minDotProdForJoin=cos(45.0f*M_PI/180.0f);	
	
	for (int h=0;h<edgeList.size();h++) {		
		for (int i=h;i<edgeList.size();i++) {
			
			//between each 2 endpoints, determine if the line segment formed
			//intersects any other edge
			for (int eachpts=0;eachpts<4;eachpts++) {

				//0 - start-start
				//1 - start-end
				//2 - end-start
				//3 - end-end
				if (h!=i) {
					if (eachpts==1||eachpts==3) 
						doEdgeReversal(edgeList[i]);
					else if (eachpts==2)
						doEdgeReversal(edgeList[h]);
				}

				Vector point1=edgeList[h]->getPoint(edgeList[h]->getNumPoints()-1);
				Vector tangent1=edgeList[h]->getPoint(edgeList[h]->getNumPoints()-1)-
					edgeList[h]->getPoint(edgeList[h]->getNumPoints()-2);

				Vector point2=edgeList[i]->getPoint(0);
				Vector tangent2=edgeList[i]->getPoint(1)-edgeList[i]->getPoint(0); //inward tangents

				Vector p2p1diff=point2-point1;
				p2p1diff.Normal();

				tangent1.y=0.0f;
				tangent2.y=0.0f;
				tangent1.Normal();
				tangent2.Normal();
				
				//pass location/tangent tests				
				if ((point1-point2).GetLength()<maxDistForJoin&&
					tangent1.DotProduct3(tangent2)>minDotProdForJoin&&
					p2p1diff.DotProduct3(tangent1)>minDotProdForJoin&&
					p2p1diff.DotProduct3(tangent2)>minDotProdForJoin) {																			
					
					Vector intersectVec;
					vector <RN_Edge *> overEdges;
					vector <int> overEdgeSegments;

					//check for crossing of another edge
					for (int j=0;j<edgeList.size();j++) {
						for (int k=0;k<edgeList[j]->getNumPoints()-1;k++) {
							if (Collision::LineSegmentsCollide(point1, point2, edgeList[j]->getPoint(k), edgeList[j]->getPoint(k+1), intersectVec)&&
									!(j==h&&k>=edgeList[j]->getNumPoints()-2)&&
									!(j==i&&k==0)) {
								
								//printf("h: %i i: %i j: %i k: %i hpoints: %i ipoints: %i jpoints: %i\n",h,i,j,k,
								//	edgeList[h]->getNumPoints(),edgeList[i]->getNumPoints(),edgeList[j]->getNumPoints());																
								overEdges.push_back(edgeList[j]);
								overEdgeSegments.push_back(k);
							}
						}						
					}

					if (!overEdges.empty()) {

						//set up the RN_Intersect for them, and do the height change stuff	
						int edgeSegmenth=edgeList[h]->getNumPoints()-1;
						edgeList[h]->setPointAttrib(edgeSegmenth,POINT_ATTRIB_CROSS, true);
						
						//update each "over edge"
						for (int eachInt=0;eachInt<overEdges.size();eachInt++) {

							overEdges[eachInt]->setPointAttrib(overEdgeSegments[eachInt],POINT_ATTRIB_CROSS, true);							

							//set up intersection, and do stuff
							RN_Intersection *newIntersect=new RN_Intersection(edgeList[h], edgeSegmenth,
																		overEdges[eachInt], overEdgeSegments[eachInt]);
							newIntersect->setRelation(RELATION_UNDEROVER);
							intersectList.push_back(newIntersect);

							if (edgeList[h]!=overEdges[eachInt])
								overEdges[eachInt]->updateStructure();
						}

						//join underedge(s)
						if (h!=i) {

							edgeList[h]->concatenateEdge(edgeList[i]);
							
							if (eachpts>=2) {//undo reversal
								doEdgeReversal(edgeList[h]);

								/*
								for (int eachInt=0;eachInt<intersectList.size();eachInt++) {								
									if (intersectList[eachInt]->getEdge(0)==edgeList[h])
										intersectList[eachInt]->setSegment(0,edgeList[h]->getNumPoints()-intersectList[eachInt]->getSegment(0)-1);
									if (intersectList[eachInt]->getEdge(1)==edgeList[h])
										intersectList[eachInt]->setSegment(1,edgeList[h]->getNumPoints()-intersectList[eachInt]->getSegment(1)-1);
								}
								*/
								
							}

							edgeList[h]->updateStructure();															

							//remove old entry
							RN_Edge *erasedEdge=edgeList[i];
							edgeList.erase(edgeList.begin()+i);														

							//ensure all intersections are good
							for (int eachInt=0;eachInt<intersectList.size();eachInt++) {	
								intersectList[eachInt]->doIntersectUpdate(erasedEdge,edgeList);
							}							
						}
						else {

							//concatenate edge and update structures						
							edgeList[h]->concatenatePoint(edgeList[h]->getPoint(0));
							edgeList[h]->setPointAttrib(edgeList[h]->getNumPoints()-1, POINT_ATTRIB_CLOSED_LOOP, true);
							edgeList[h]->updateStructure();		

						}

						//done						
						return true;
					}

				}				
				
			}

			//final unreversal (no joins found between h, i)
			if (h!=i)
				doEdgeReversal(edgeList[h]);
		
		}
	}

	return false;

}

void RN::doEdgeReversal(RN_Edge *edge) {

	edge->reverse();

	for (int i=0;i<intersectList.size();i++) {
		if (intersectList[i]->getEdge(0)==edge)
			intersectList[i]->setSegment(0,edge->getNumPoints()-1-intersectList[i]->getSegment(0));
		if (intersectList[i]->getEdge(1)==edge)
			intersectList[i]->setSegment(1,edge->getNumPoints()-1-intersectList[i]->getSegment(1));
	}

}

void RN::doIntersectionUpdates() {

	for (int i=0;i<intersectList.size();i++) {
		intersectList[i]->doUpdate();
	}

}

void RN::addIntersect(RN_Intersection *intersect) {
	intersectList.push_back(intersect);
}

RN *RN::getCloneNetwork(HeightMap *heightMap) {

	RN *cloneNetwork=new RN(heightMap);
	
	//new edges
	for (int i=0;i<edgeList.size();i++) {
		cloneNetwork->addEdge(edgeList[i]->getClone());
	}	

	//new intersects (pointing at new edges)
	for (int i=0;i<intersectList.size();i++) {
		RN_Intersection *newInt=intersectList[i]->getClone();
		for (int j=0;j<edgeList.size();j++) {
			if (newInt->getEdge(0)==edgeList[j])
				newInt->setEdgeSegment(0,cloneNetwork->edgeList[j],newInt->getSegment(0));
			if (newInt->getEdge(1)==edgeList[j])
				newInt->setEdgeSegment(1,cloneNetwork->edgeList[j],newInt->getSegment(1));
		}		
		cloneNetwork->addIntersect(newInt);
	}	

	//clone selection lists
	cloneNetwork->selectedEdgeList=vector <int> (selectedEdgeList.size());
	cloneNetwork->selectedIntersectList=vector <int> (selectedIntersectList.size());

	std::copy(selectedEdgeList.begin(), selectedEdgeList.end(), cloneNetwork->selectedEdgeList.begin());
	std::copy(selectedIntersectList.begin(), selectedIntersectList.end(), cloneNetwork->selectedIntersectList.begin());

	//clone obj manager (but make it point to new network)
	cloneNetwork->objManager=objManager->getClone(cloneNetwork);	

	return cloneNetwork;

}

void RN::doDeleteRoads() {

	//iterate through each selected road, and delete the selected portion
	//this may result in splitting or total deletion
	//for road portions that are removed that are involved in an intersection,
	//the list will have to be updated		

	for (int i=0;i<selectedEdgeList.size();i++) {

		vector <RN_Edge *> newEdges=edgeList[selectedEdgeList[i]]->getEdgesFromUnselected();								

		//add the new edges
		for (int j=0;j<newEdges.size();j++) 
			edgeList.push_back(newEdges[j]);

		//update intersectlist references:
		for (int j=0;j<intersectList.size();j++) {
			intersectList[j]->doIntersectUpdate(edgeList[selectedEdgeList[i]], newEdges);
		}		

		//remove the one that spawned them
		edgeList.erase(edgeList.begin()+selectedEdgeList[i]);					
		
		//update other selected references
		for (int j=i+1;j<selectedEdgeList.size();j++) 
			selectedEdgeList[j]--;

	}		

	for (int i=0;i<selectedIntersectList.size();i++) {

		intersectList.erase(intersectList.begin()+selectedIntersectList[i]);

		//update selected references
		for (int j=i+1;j<selectedIntersectList.size();j++) {
			selectedIntersectList[j]--;
		}

	}

	objManager->updateClearRange();
	doObjects();

}

void RN::doObjects() {	

	int segmentsBackToWarn=8;
	int segmentsBackToWarnMid=5;
	int segmentsBackToWarnShort=3;
	float straightnessThreshold=0.02f;
	float sharpturnThreshold=0.06f;

	objManager->removeType(OBJ_TYPE_PILLAR);
	objManager->removeType(OBJ_TYPE_STOPSIGN);
	objManager->removeType(OBJ_TYPE_WARNING_STOPAHEAD);
	objManager->removeType(OBJ_TYPE_WARNING_SOLIDOBJECT);
	objManager->removeType(OBJ_TYPE_WARNING_BUMP);
	objManager->removeType(OBJ_TYPE_WARNING_DIP);
	objManager->removeType(OBJ_TYPE_WARNING_RIGHT);
	objManager->removeType(OBJ_TYPE_WARNING_LEFT);
	objManager->removeType(OBJ_TYPE_FLARE);
	objManager->removeType(OBJ_TYPE_WARNING_SPEED_SLOW);
	objManager->removeType(OBJ_TYPE_WARNING_SPEED_AVG);
	objManager->removeType(OBJ_TYPE_WARNING_SPEED_FAST);

	//bridge pillars
	for (int i=0;i<edgeList.size();i++) {
		for (int j=1;j<edgeList[i]->getNumPoints()-1;j++) {
			if (edgeList[i]->getPointAttrib(j,POINT_ATTRIB_BRIDGE)&&				
				!edgeList[i]->getPointAttrib(j,POINT_ATTRIB_CROSS)&&
				!edgeList[i]->getPointAttrib(j-1,POINT_ATTRIB_CROSS)&&
				!edgeList[i]->getPointAttrib(j-1,POINT_ATTRIB_TUNNEL)) {
				edgeList[i]->addAlongRoad(j,0.0f,0.25f*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_PILLAR,objManager);
				edgeList[i]->addAlongRoad(j,0.0f,-0.25f*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_PILLAR,objManager);
			}
		}
	}

	//add some stop signs!	
	for (int i=0;i<edgeList.size();i++) {
		for (int j=0;j<edgeList[i]->getNumPoints();j++) {
			if (edgeList[i]->getPointAttrib(j,POINT_ATTRIB_INTERSECT)) {
				if (j>0)
					edgeList[i]->addAlongRoad(j-1,0.9f,MIN_DIST_BETWEEN_POINTS*0.6, OBJ_TYPE_STOPSIGN, objManager);
				if (j<edgeList[i]->getNumPoints()-1)
					edgeList[i]->addAlongRoad(j+1,0.1f,-MIN_DIST_BETWEEN_POINTS*0.6, OBJ_TYPE_STOPSIGN, objManager);
			}
		}
	}	

	//stop sign warnings
	for (int i=0;i<edgeList.size();i++) {
		for (int j=segmentsBackToWarn;j<edgeList[i]->getNumPoints()-segmentsBackToWarn;j++) {
			if (edgeList[i]->getPointAttrib(j,POINT_ATTRIB_INTERSECT)) {				
				if (edgeList[i]->getPointAttribVal(j-segmentsBackToWarn)==0) 
					edgeList[i]->addAlongRoad(j-segmentsBackToWarn,0.5f,0.6f*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_STOPAHEAD,objManager);					
				if (edgeList[i]->getPointAttribVal(j+segmentsBackToWarn)==0) 
					edgeList[i]->addAlongRoad(j+segmentsBackToWarn,0.5f,-0.6f*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_STOPAHEAD,objManager);					
			}			
		}
	}	

	//speed warnings
	for (int i=0;i<edgeList.size();i++) {
		for (int j=segmentsBackToWarnShort;j<edgeList[i]->getNumPoints()-segmentsBackToWarnShort;j++) {

			if (edgeList[i]->getPointAttrib(j,POINT_ATTRIB_BRIDGE)||
					edgeList[i]->getPointAttrib(j,POINT_ATTRIB_TUNNEL)||
					edgeList[i]->getPointAttrib(j,POINT_ATTRIB_CROSS))
				continue;

			if (edgeList[i]->getPointAttrib(j,POINT_ATTRIB_SPEED_FAST)!=edgeList[i]->getPointAttrib(j+1,POINT_ATTRIB_SPEED_FAST)||
				edgeList[i]->getPointAttrib(j,POINT_ATTRIB_SPEED_SLOW)!=edgeList[i]->getPointAttrib(j+1,POINT_ATTRIB_SPEED_SLOW)) {
				if (edgeList[i]->getPointAttrib(j+1,POINT_ATTRIB_SPEED_FAST))
					edgeList[i]->addAlongRoad(j,0.1f,0.6*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_SPEED_FAST,objManager);
				else if (edgeList[i]->getPointAttrib(j+1,POINT_ATTRIB_SPEED_SLOW))
					edgeList[i]->addAlongRoad(j,0.1f,0.6*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_SPEED_SLOW,objManager);
				else
					edgeList[i]->addAlongRoad(j,0.1f,0.6*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_SPEED_AVG,objManager);

				if (edgeList[i]->getPointAttrib(j,POINT_ATTRIB_SPEED_FAST))
					edgeList[i]->addAlongRoad(j,0.9f,-0.6*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_SPEED_FAST,objManager);
				else if (edgeList[i]->getPointAttrib(j,POINT_ATTRIB_SPEED_SLOW))
					edgeList[i]->addAlongRoad(j,0.9f,-0.6*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_SPEED_SLOW,objManager);
				else
					edgeList[i]->addAlongRoad(j,0.9f,-0.6*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_SPEED_AVG,objManager);
			}
		}
	}

	//solid object warnings
	for (int i=0;i<edgeList.size();i++) {
		for (int j=segmentsBackToWarnShort;j<edgeList[i]->getNumPoints()-segmentsBackToWarnShort;j++) {
			if (edgeList[i]->getPointAttrib(j,POINT_ATTRIB_BRIDGE)&&
				edgeList[i]->getPointAttribVal(j-1)==0) {
				edgeList[i]->addAlongRoad(j-2,0.9f,0.001f, OBJ_TYPE_WARNING_SOLIDOBJECT, objManager);
			}

			if (edgeList[i]->getPointAttrib(j,POINT_ATTRIB_BRIDGE)&&
				edgeList[i]->getPointAttribVal(j+1)==0) {
				edgeList[i]->addAlongRoad(j+2,0.1f,-0.001f, OBJ_TYPE_WARNING_SOLIDOBJECT, objManager);
			}

		}
	}

	//bump/dip warnings
	for (int i=0;i<edgeList.size();i++) {
		for (int j=segmentsBackToWarnShort;j<edgeList[i]->getNumPoints()-segmentsBackToWarnShort;j++) {

			if (edgeList[i]->getPoint(j).y>edgeList[i]->getPoint(j+segmentsBackToWarnShort).y+MIN_DIST_BETWEEN_POINTS/3.0f&&
				edgeList[i]->getPoint(j).y>edgeList[i]->getPoint(j-segmentsBackToWarnShort).y+MIN_DIST_BETWEEN_POINTS/3.0f) {

				if (edgeList[i]->getPointAttribVal(j-segmentsBackToWarnShort)==0) 
					edgeList[i]->addAlongRoad(j-segmentsBackToWarnShort,0.5f,0.7f*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_BUMP,objManager);					
				if (edgeList[i]->getPointAttribVal(j+segmentsBackToWarnShort)==0) 
					edgeList[i]->addAlongRoad(j+segmentsBackToWarnShort,0.5f,-0.7f*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_BUMP,objManager);

				j+=segmentsBackToWarnShort;
			}
			else if (edgeList[i]->getPoint(j).y<edgeList[i]->getPoint(j+segmentsBackToWarnShort).y-MIN_DIST_BETWEEN_POINTS/3.0f&&
				edgeList[i]->getPoint(j).y<edgeList[i]->getPoint(j-segmentsBackToWarnShort).y-MIN_DIST_BETWEEN_POINTS/3.0f) {

				if (edgeList[i]->getPointAttribVal(j-segmentsBackToWarnShort)==0) 
					edgeList[i]->addAlongRoad(j-segmentsBackToWarnShort,0.5f,0.7f*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_DIP,objManager);					
				if (edgeList[i]->getPointAttribVal(j+segmentsBackToWarnShort)==0) 
					edgeList[i]->addAlongRoad(j+segmentsBackToWarnShort,0.5f,-0.7f*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_DIP,objManager);

				j+=segmentsBackToWarnShort;
			}
		}
	}

	//left/right turn warnings
	for (int i=0;i<edgeList.size();i++) {
		for (int j=segmentsBackToWarnMid;j<edgeList[i]->getNumPoints()-segmentsBackToWarnMid;j++) {			

			if (fabs(edgeList[i]->getCurvature(j))>sharpturnThreshold) { //test sharpness
								
				if (fabs(edgeList[i]->getCurvature(j+segmentsBackToWarnMid))>straightnessThreshold|| //test straightness before and after
					fabs(edgeList[i]->getCurvature(j-segmentsBackToWarnMid))>straightnessThreshold) 
					continue;

				bool shouldContinue=false;
				for (int k=j-segmentsBackToWarnMid;k<=j+segmentsBackToWarnMid;k++) {
					if (edgeList[i]->getPointAttribVal(k)>0) {
						shouldContinue=true;
						break;
					}
				}

				if (shouldContinue) 
					continue;
				
				if (edgeList[i]->getCurvature(j)>0.0f) {					
					edgeList[i]->addAlongRoad(j-segmentsBackToWarn,0.5f,0.7f*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_RIGHT,objManager);
					edgeList[i]->addAlongRoad(j+segmentsBackToWarn,0.5f,-0.7f*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_LEFT,objManager);
				}
				else {						
					edgeList[i]->addAlongRoad(j-segmentsBackToWarnMid,0.5f,0.7f*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_LEFT,objManager);											
					edgeList[i]->addAlongRoad(j+segmentsBackToWarnMid,0.5f,-0.7f*MIN_DIST_BETWEEN_POINTS,OBJ_TYPE_WARNING_RIGHT,objManager);
				}
				j+=segmentsBackToWarn;				
			}
		}
	}

	//light flares in tunnels
	for (int i=0;i<edgeList.size();i++) {
		for (int j=edgeList[i]->getNumPoints()-1;j>=0;j--) {
			if (edgeList[i]->getPointAttrib(j,POINT_ATTRIB_TUNNEL)) 
				edgeList[i]->addAlongRoad(j,0.4f,0.0f,OBJ_TYPE_FLARE,objManager);			
		}
	}

}

void RN::addFoliage(vector <Vector> lassoPoints3D, float foliageDensity) {

	objManager->addFoliage(lassoPoints3D, foliageDensity);

}

unsigned int RN::getNumIntersects() {
	return intersectList.size();
}

RN_Intersection *RN::getIntersect(int index) {
	return intersectList[index];
}

RN_Edge *RN::getMostSelectedEdge() {

	int maxVerts=0;
	int maxVertIndex=-1;

	for (int i=0;i<edgeList.size();i++) {
		if (edgeList[i]->isSelected()&&edgeList[i]->getNumPointsSelected()>maxVerts) {
			maxVertIndex=i;
			maxVerts=edgeList[i]->getNumPointsSelected();			
		}
	}

	if (maxVertIndex>=0)
		return edgeList[maxVertIndex];
	else
		return NULL;

}

void RN::updateClearRange() {
	objManager->updateClearRange();
}

void RN::changeIntersect(char *buttonText, HeightMap *heightMap) {
	
	//change whats necessary about the rn_edge
	intersectList[selectedIntersectList[0]]->changeIntersect(buttonText);

	doDepthRelations();

	updateClearRange();
	doObjects();	

	doPrepareTerrain(intersectList[selectedIntersectList[0]]->getEdge(0), heightMap);
	doPrepareTerrain(intersectList[selectedIntersectList[0]]->getEdge(1), heightMap);

}

void RN::setHeightMap(HeightMap *heightMap) {
	objManager=new ObjectManager(this, heightMap);
}

ObjectManager *RN::getObjectManager() {
	return objManager;
}

void RN::drawTunnelHoles() {

	//glDisable(GL_DEPTH_TEST);
	//glDepthMask(GL_FALSE);	

	//draw the tunnel pieces to the stencil	
	for (int i=0;i<edgeList.size();i++) {
		edgeList[i]->drawTunnelHoles();
	}

	//glEnable(GL_DEPTH_TEST);
	//glDepthMask(GL_TRUE);

}

void RN::export_COLLADA(char *exportFilename, HeightMap *heightMap) {

	/*
	printf("Exporting to %s...",exportFilename);

	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "utf-8", "" );

	TiXmlElement * element = new TiXmlElement( "COLLADA" );
	element->SetAttribute("xmlns", "http://www.collada.org/2005/11/COLLADASchema");
	element->SetAttribute("version", "1.4.1");

	//ASSET INFO
	TiXmlElement * assetElement = new TiXmlElement("asset");
	TiXmlElement * assetContribElement=new TiXmlElement("contributor");
	TiXmlElement * assetAuthoringTool=new TiXmlElement("authoring_tool");
	TiXmlText * authoringToolText = new TiXmlText( "Drive" );
	assetAuthoringTool->LinkEndChild(authoringToolText);
	assetContribElement->LinkEndChild(assetAuthoringTool);
	assetElement->LinkEndChild(assetContribElement);
	TiXmlElement * assetCreated=new TiXmlElement("created");
	TiXmlText * assetCreatedText = new TiXmlText( "2008-03-21T08:13:42Z" );
	assetCreated->LinkEndChild(assetCreatedText);
	assetElement->LinkEndChild(assetCreated);
	TiXmlElement * assetModified=new TiXmlElement("modified");
	TiXmlText * assetModifiedText = new TiXmlText( "2008-03-21T08:13:42Z" );
	assetModified->LinkEndChild(assetModifiedText);
	assetElement->LinkEndChild(assetModified);
	TiXmlElement * assetUnit=new TiXmlElement("unit");
	assetUnit->SetAttribute("name","meters");
	assetUnit->SetAttribute("meter","1.0");
	assetElement->LinkEndChild(assetUnit);
	TiXmlElement * assetUpAxis=new TiXmlElement("up_axis");
	TiXmlText * assetUpAxisText = new TiXmlText( "Y_UP" ); //maybe this is why it wont display in google earth yet?
	assetUpAxis->LinkEndChild(assetUpAxisText);
	assetElement->LinkEndChild(assetUpAxis);    

	element->LinkEndChild(assetElement);

	//lib IMAGES (this will be needed for diffuse textures)
	TiXmlElement *libraryImages=new TiXmlElement("library_images");

	for (int i=1;i<=6;i++) {
		
		if (i==6&&heightMap==NULL) continue;

		TiXmlElement *libraryImages_image1=new TiXmlElement("image");
		ostringstream roadpieceNtexture;
		if (i<6)			
			roadpieceNtexture << "Image" << i;
		else
			roadpieceNtexture << "ImageTer";
		libraryImages_image1->SetAttribute("id",roadpieceNtexture.str());
		libraryImages_image1->SetAttribute("name",roadpieceNtexture.str());
		TiXmlElement *libraryImages_image1_initFrom=new TiXmlElement("init_from");
		TiXmlText *libraryImages_image1_initFrom_Tex;

		if (i==1)
			libraryImages_image1_initFrom_Tex=new TiXmlText(ROAD_PIECE_1_TEXTURE);
		else if (i==2)
			libraryImages_image1_initFrom_Tex=new TiXmlText(ROAD_PIECE_2_TEXTURE);
		else if (i==3)
			libraryImages_image1_initFrom_Tex=new TiXmlText(ROAD_PIECE_3_TEXTURE);
		else if (i==4)
			libraryImages_image1_initFrom_Tex=new TiXmlText(ROAD_PIECE_4_TEXTURE);
		else if (i==5)
			libraryImages_image1_initFrom_Tex=new TiXmlText(ROAD_PIECE_5_TEXTURE);
		else
			libraryImages_image1_initFrom_Tex=new TiXmlText(heightMap->getTextureFilename());

		libraryImages_image1_initFrom->LinkEndChild(libraryImages_image1_initFrom_Tex);
		libraryImages_image1->LinkEndChild(libraryImages_image1_initFrom);
		libraryImages->LinkEndChild(libraryImages_image1);
	}	

	element->LinkEndChild(libraryImages);

	//lib MATERIALS
	TiXmlElement *libraryMaterials=new TiXmlElement("library_materials");

	for (int i=1;i<=6;i++) {

		if (i==6&&heightMap==NULL) continue;

		TiXmlElement *libraryMaterials_Material1=new TiXmlElement("material");
		ostringstream materialNid;
		if (i<6)
			materialNid << "Material" << i << "ID";
		else
			materialNid << "MaterialTerID";
		libraryMaterials_Material1->SetAttribute("id",materialNid.str());
		ostringstream materialN;
		if (i<6)
			materialN << "Material" << i;
		else
			materialN << "MaterialID";
		libraryMaterials_Material1->SetAttribute("name",materialN.str());
		TiXmlElement *libraryMaterials_Material1_InstanceEffect=new TiXmlElement("instance_effect");
		ostringstream hash_materialN_effect;
		if (i<6)
			hash_materialN_effect << "#Material" << i << "-effect";
		else
			hash_materialN_effect << "#MaterialTer-effect";
		libraryMaterials_Material1_InstanceEffect->SetAttribute("url",hash_materialN_effect.str());
		libraryMaterials_Material1->LinkEndChild(libraryMaterials_Material1_InstanceEffect);
		libraryMaterials->LinkEndChild(libraryMaterials_Material1);		
	}	

	element->LinkEndChild(libraryMaterials);

	//lib EFFECTS
	TiXmlElement *libraryEffects=new TiXmlElement("library_effects");

	for (int i=1;i<=6;i++) {

		if (i==6&&heightMap==NULL) continue; //only do this if there's a terrain to export

		TiXmlElement *libraryEffects_Effect=new TiXmlElement("effect");
		ostringstream materialN_effect;
		if (i<6)
			materialN_effect << "Material" << i << "-effect";
		else
			materialN_effect << "MaterialTer-effect";
		libraryEffects_Effect->SetAttribute("id",materialN_effect.str());
		libraryEffects_Effect->SetAttribute("name",materialN_effect.str());
		
		TiXmlElement *libraryEffects_Effect_Profile=new TiXmlElement("profile_COMMON");

		TiXmlElement *libraryEffects_Effect_Profile_newParam1=new TiXmlElement("newparam");
		ostringstream ImageN_surface;
		if (i<6)
			ImageN_surface << "Image" << i << "-surface";
		else
			ImageN_surface << "ImageTer-surface";
		libraryEffects_Effect_Profile_newParam1->SetAttribute("sid",ImageN_surface.str());
		TiXmlElement *libraryEffects_Effect_Profile_newParam1_surface=new TiXmlElement("surface");
		libraryEffects_Effect_Profile_newParam1_surface->SetAttribute("type","2D");
		TiXmlElement *libraryEffects_Effect_Profile_newParam1_surface_initfrom=new TiXmlElement("init_from");
		ostringstream ImageN;
		if (i<6)
			ImageN << "Image" << i;
		else
			ImageN << "ImageTer";
		TiXmlText *libraryEffects_Effect_Profile_newParam1_surface_initfrom_text=new TiXmlText(ImageN.str());
		libraryEffects_Effect_Profile_newParam1_surface_initfrom->LinkEndChild(libraryEffects_Effect_Profile_newParam1_surface_initfrom_text);
		libraryEffects_Effect_Profile_newParam1_surface->LinkEndChild(libraryEffects_Effect_Profile_newParam1_surface_initfrom);
		libraryEffects_Effect_Profile_newParam1->LinkEndChild(libraryEffects_Effect_Profile_newParam1_surface);
		libraryEffects_Effect_Profile->LinkEndChild(libraryEffects_Effect_Profile_newParam1);

		TiXmlElement *libraryEffects_Effect_Profile_newParam2=new TiXmlElement("newparam");
		ostringstream ImageN_sampler;
		if (i<6)
			ImageN_sampler << "Image" << i << "-sampler";
		else
			ImageN_sampler << "ImageTer-sampler";
		libraryEffects_Effect_Profile_newParam2->SetAttribute("sid",ImageN_sampler.str());
		TiXmlElement *libraryEffects_Effect_Profile_newParam2_sampler2d=new TiXmlElement("sampler2D");
		TiXmlElement *libraryEffects_Effect_Profile_newParam2_sampler2d_source=new TiXmlElement("source");
		TiXmlText *libraryEffects_Effect_Profile_newParam2_sampler2d_sourceTex=new TiXmlText(ImageN_surface.str());
		libraryEffects_Effect_Profile_newParam2_sampler2d_source->LinkEndChild(libraryEffects_Effect_Profile_newParam2_sampler2d_sourceTex);
		libraryEffects_Effect_Profile_newParam2_sampler2d->LinkEndChild(libraryEffects_Effect_Profile_newParam2_sampler2d_source);
		libraryEffects_Effect_Profile_newParam2->LinkEndChild(libraryEffects_Effect_Profile_newParam2_sampler2d);
		libraryEffects_Effect_Profile->LinkEndChild(libraryEffects_Effect_Profile_newParam2);
		
		TiXmlElement *libraryEffects_Effect_Profile_Tech=new TiXmlElement("technique");
		libraryEffects_Effect_Profile_Tech->SetAttribute("sid","COMMON");
		TiXmlElement *libraryEffects_Effect_Profile_Tech_Phong=new TiXmlElement("phong");	
		TiXmlElement *libraryEffects_Effect_Profile_Tech_Phong_Emission=new TiXmlElement("emission");	
		TiXmlElement *libraryEffects_Effect_Profile_Tech_Phong_Ambient=new TiXmlElement("ambient");	
		TiXmlElement *libraryEffects_Effect_Profile_Tech_Phong_Diffuse=new TiXmlElement("diffuse");	
		TiXmlElement *libraryEffects_Effect_Profile_Tech_Phong_Specular=new TiXmlElement("specular");	
		TiXmlElement *libraryEffects_Effect_Profile_Tech_Phong_Shininess=new TiXmlElement("shininess");	
		TiXmlElement *libraryEffects_Effect_Profile_Tech_Phong_Reflectivity=new TiXmlElement("reflectivity");	
		TiXmlElement *libraryEffects_Effect_Profile_Tech_Phong_Transparent=new TiXmlElement("transparent");	
		TiXmlElement *libraryEffects_Effect_Profile_Tech_Phong_Transparency=new TiXmlElement("transparency");	

		TiXmlText *zzz1Text1=new TiXmlText("0.000000 0.000000 0.000000 1");
		TiXmlElement *zzz1Element1=new TiXmlElement("color");	
		zzz1Element1->LinkEndChild(zzz1Text1);

		TiXmlText *zzz1Text2=new TiXmlText("0.000000 0.000000 0.000000 1");
		TiXmlElement *zzz1Element2=new TiXmlElement("color");	
		zzz1Element2->LinkEndChild(zzz1Text2);

		//TiXmlText *zzz1Text3=new TiXmlText("1.000000 1.000000 1.000000 1");
		//TiXmlElement *zzz1Element3=new TiXmlElement("color");	
		//zzz1Element3->LinkEndChild(zzz1Text3);
	
		TiXmlElement *zzz1Element3=new TiXmlElement("texture");	
		zzz1Element3->SetAttribute("texture",ImageN_sampler.str());

		TiXmlElement *p3Element=new TiXmlElement("color");
		TiXmlText *p3Text=new TiXmlText("0.330000 0.330000 0.330000 1");
		p3Element->LinkEndChild(p3Text);

		TiXmlElement *float20=new TiXmlElement("float");
		TiXmlText *float20Text=new TiXmlText("20.000000");
		float20->LinkEndChild(float20Text);

		TiXmlElement *floatp1=new TiXmlElement("float");
		TiXmlText *floatp1Text=new TiXmlText("0.100000");
		floatp1->LinkEndChild(floatp1Text);

		TiXmlElement *all1Element=new TiXmlElement("color");
		TiXmlText *all1Text=new TiXmlText("1 1 1 1");
		all1Element->LinkEndChild(all1Text);

		TiXmlElement *float0=new TiXmlElement("float");
		TiXmlText *float0Text=new TiXmlText("0.000000");
		float0->LinkEndChild(float0Text);

		libraryEffects_Effect_Profile_Tech_Phong_Emission->LinkEndChild(zzz1Element1);
		libraryEffects_Effect_Profile_Tech_Phong_Ambient->LinkEndChild(zzz1Element2);
		libraryEffects_Effect_Profile_Tech_Phong_Diffuse->LinkEndChild(zzz1Element3);
		libraryEffects_Effect_Profile_Tech_Phong_Specular->LinkEndChild(p3Element);
		libraryEffects_Effect_Profile_Tech_Phong_Shininess->LinkEndChild(float20);
		libraryEffects_Effect_Profile_Tech_Phong_Reflectivity->LinkEndChild(floatp1);
		libraryEffects_Effect_Profile_Tech_Phong_Transparent->LinkEndChild(all1Element);
		libraryEffects_Effect_Profile_Tech_Phong_Transparency->LinkEndChild(float0);

		libraryEffects_Effect_Profile_Tech_Phong->LinkEndChild(libraryEffects_Effect_Profile_Tech_Phong_Emission);
		libraryEffects_Effect_Profile_Tech_Phong->LinkEndChild(libraryEffects_Effect_Profile_Tech_Phong_Ambient);
		libraryEffects_Effect_Profile_Tech_Phong->LinkEndChild(libraryEffects_Effect_Profile_Tech_Phong_Diffuse);
		libraryEffects_Effect_Profile_Tech_Phong->LinkEndChild(libraryEffects_Effect_Profile_Tech_Phong_Specular);
		libraryEffects_Effect_Profile_Tech_Phong->LinkEndChild(libraryEffects_Effect_Profile_Tech_Phong_Shininess);
		libraryEffects_Effect_Profile_Tech_Phong->LinkEndChild(libraryEffects_Effect_Profile_Tech_Phong_Reflectivity);
		libraryEffects_Effect_Profile_Tech_Phong->LinkEndChild(libraryEffects_Effect_Profile_Tech_Phong_Transparent);
		libraryEffects_Effect_Profile_Tech_Phong->LinkEndChild(libraryEffects_Effect_Profile_Tech_Phong_Transparency);

		libraryEffects_Effect_Profile_Tech->LinkEndChild(libraryEffects_Effect_Profile_Tech_Phong);

		libraryEffects_Effect_Profile->LinkEndChild(libraryEffects_Effect_Profile_Tech);

		TiXmlElement *libraryEffects_Effect_Profile_Extra=new TiXmlElement("extra");
		TiXmlElement *libraryEffects_Effect_Profile_Extra_Tech=new TiXmlElement("technique");
		libraryEffects_Effect_Profile_Extra_Tech->SetAttribute("profile","GOOGLEEARTH");
		TiXmlElement *libraryEffects_Effect_Profile_Extra_Tech_Double=new TiXmlElement("double_sided");
		TiXmlText *libraryEffects_Effect_Profile_Extra_Tech_DoubleText=new TiXmlText("1");

		libraryEffects_Effect_Profile_Extra_Tech_Double->LinkEndChild(libraryEffects_Effect_Profile_Extra_Tech_DoubleText);
		libraryEffects_Effect_Profile_Extra_Tech->LinkEndChild(libraryEffects_Effect_Profile_Extra_Tech_Double);
		libraryEffects_Effect_Profile_Extra->LinkEndChild(libraryEffects_Effect_Profile_Extra_Tech);

		libraryEffects_Effect_Profile->LinkEndChild(libraryEffects_Effect_Profile_Extra);

		libraryEffects_Effect->LinkEndChild(libraryEffects_Effect_Profile);

		libraryEffects->LinkEndChild(libraryEffects_Effect);

	}

	element->LinkEndChild(libraryEffects);


	//GEOMETRY
	TiXmlElement * geometryElement = new TiXmlElement("library_geometries");
	
	vector <string> bindMaterialsVec;
	vector <string> bindMaterialsVecHashID;

	unsigned int curMeshNumber=0;

	for (int i=0;i<edgeList.size();i++) {

		for (int j=0;j<edgeList[i]->getNumPoints()-1;j++) {

			if (edgeList[i]->getWhichRoadMesh(j)<0)
				continue;

			curMeshNumber++;

			ostringstream meshN_geometry;
			meshN_geometry << "mesh" << curMeshNumber << "-geometry";

			ostringstream meshN_geometry_position;
			meshN_geometry_position << "mesh" << curMeshNumber << "-geometry-position";
			ostringstream meshN_geometry_position_array;
			meshN_geometry_position_array << "mesh" << curMeshNumber << "-geometry-position-array";
			ostringstream hash_meshN_geometry_position_array;
			hash_meshN_geometry_position_array << "#mesh" << curMeshNumber << "-geometry-position-array";

			ostringstream meshN_geometry_normal;
			meshN_geometry_normal << "mesh" << curMeshNumber << "-geometry-normal";
			ostringstream meshN_geometry_normal_array;
			meshN_geometry_normal_array << "mesh" << curMeshNumber << "-geometry-normal-array";
			ostringstream hash_meshN_geometry_normal_array;
			hash_meshN_geometry_normal_array << "#mesh" << curMeshNumber << "-geometry-normal-array";

			ostringstream meshN_geometry_uv;
			meshN_geometry_uv << "mesh" << curMeshNumber << "-geometry-uv";
			ostringstream meshN_geometry_uv_array;
			meshN_geometry_uv_array << "mesh" << curMeshNumber << "-geometry-uv-array";
			ostringstream hash_meshN_geometry_uv_array;
			hash_meshN_geometry_uv_array << "#mesh" << curMeshNumber << "-geometry-uv-array";			

			vector <Vector> meshVerts;
			vector <Vector> meshNormals;
			vector <Vector> meshUVs;
			vector <int> meshFaceIndexes;

			vector <Vector> eachEdgeMeshVerts=edgeList[i]->getMeshVertices(j);
			//offset by min altitude
			if (heightMap!=NULL) {
				for (int eachVert=0;eachVert<eachEdgeMeshVerts.size();eachVert++) {
					eachEdgeMeshVerts[eachVert].y-=255.0f;
				}
			}
			meshVerts.insert(meshVerts.end(), eachEdgeMeshVerts.begin(), eachEdgeMeshVerts.end()); 			

			vector <Vector> eachEdgeMeshNormals=edgeList[i]->getMeshNormals(j);
			meshNormals.insert(meshNormals.end(), eachEdgeMeshNormals.begin(), eachEdgeMeshNormals.end()); 

			vector <Vector> eachEdgeMeshUVs=edgeList[i]->getMeshUVs(j);
			meshUVs.insert(meshUVs.end(), eachEdgeMeshUVs.begin(), eachEdgeMeshUVs.end()); 

			vector <int> eachEdgeMeshFaceIndexes=edgeList[i]->getMeshFaceIndexes(j);
			meshFaceIndexes.insert(meshFaceIndexes.end(), eachEdgeMeshFaceIndexes.begin(), eachEdgeMeshFaceIndexes.end()); 	

			//start new mesh
			TiXmlElement * meshElement=new TiXmlElement("geometry");
			meshElement->SetAttribute("id",meshN_geometry.str());
			meshElement->SetAttribute("name",meshN_geometry.str());
			
			TiXmlElement * meshElement_Mesh=new TiXmlElement("mesh");	

			//geometry - VERTICES
			TiXmlElement * meshElement_Mesh_SourcePosition=new TiXmlElement("source");
			meshElement_Mesh_SourcePosition->SetAttribute("id",meshN_geometry_position.str());

			TiXmlElement * meshElement_Mesh_SourcePosition_FloatArray=new TiXmlElement("float_array");
			meshElement_Mesh_SourcePosition_FloatArray->SetAttribute("id",meshN_geometry_position_array.str());
			
			if (meshVerts.size()>0) {

				//get the count
				ostringstream meshCountStr;
				meshCountStr << meshVerts.size()*3;
				meshElement_Mesh_SourcePosition_FloatArray->SetAttribute("count",meshCountStr.str());

				//save the verts
				ostringstream meshVertTexStr;

				for (int i=0;i<meshVerts.size();i++) 			
					meshVertTexStr << fixed << meshVerts[i].x << " " << meshVerts[i].y << " " << meshVerts[i].z << " ";		

				TiXmlText *meshVertTex=new TiXmlText(meshVertTexStr.str());		

				meshElement_Mesh_SourcePosition_FloatArray->LinkEndChild(meshVertTex);
			}
			else {
				meshElement_Mesh_SourcePosition_FloatArray->SetAttribute("count","0");
			}	

			meshElement_Mesh_SourcePosition->LinkEndChild(meshElement_Mesh_SourcePosition_FloatArray);

			TiXmlElement * meshElement_Mesh_SourcePosition_Technique=new TiXmlElement("technique_common");

			TiXmlElement * meshElement_Mesh_SourcePosition_Technique_Accessor=new TiXmlElement("accessor");
			meshElement_Mesh_SourcePosition_Technique_Accessor->SetAttribute("source",hash_meshN_geometry_position_array.str());
			ostringstream meshCountStr;
			meshCountStr << meshVerts.size();
			meshElement_Mesh_SourcePosition_Technique_Accessor->SetAttribute("count",meshCountStr.str());
			meshElement_Mesh_SourcePosition_Technique_Accessor->SetAttribute("stride","3");

			TiXmlElement * paramX=new TiXmlElement("param");
			paramX->SetAttribute("name","X");
			paramX->SetAttribute("type","float");
			TiXmlElement * paramY=new TiXmlElement("param");
			paramY->SetAttribute("name","Y");
			paramY->SetAttribute("type","float");
			TiXmlElement * paramZ=new TiXmlElement("param");
			paramZ->SetAttribute("name","Z");
			paramZ->SetAttribute("type","float");

			meshElement_Mesh_SourcePosition_Technique_Accessor->LinkEndChild(paramX);
			meshElement_Mesh_SourcePosition_Technique_Accessor->LinkEndChild(paramY);
			meshElement_Mesh_SourcePosition_Technique_Accessor->LinkEndChild(paramZ);

			meshElement_Mesh_SourcePosition_Technique->LinkEndChild(meshElement_Mesh_SourcePosition_Technique_Accessor);

			meshElement_Mesh_SourcePosition->LinkEndChild(meshElement_Mesh_SourcePosition_Technique);

			meshElement_Mesh->LinkEndChild(meshElement_Mesh_SourcePosition);

			//geometry - NORMALS
			TiXmlElement * meshElement_Mesh_SourceNormal=new TiXmlElement("source");
			meshElement_Mesh_SourceNormal->SetAttribute("id",meshN_geometry_normal.str());

			TiXmlElement * meshElement_Mesh_SourceNormal_FloatArray=new TiXmlElement("float_array");
			meshElement_Mesh_SourceNormal_FloatArray->SetAttribute("id",meshN_geometry_normal_array.str());

			if (meshNormals.size()>0) {

				//get the count
				ostringstream meshNCountStr;
				meshNCountStr << meshNormals.size()*3;
				meshElement_Mesh_SourceNormal_FloatArray->SetAttribute("count",meshNCountStr.str());

				//save the verts
				ostringstream meshNormTexStr;
				
				for (int i=0;i<meshNormals.size();i++) 			
					meshNormTexStr << fixed << meshNormals[i].x << " " << meshNormals[i].y << " " << meshNormals[i].z << " ";		

				TiXmlText *meshNormTex=new TiXmlText(meshNormTexStr.str());		

				meshElement_Mesh_SourceNormal_FloatArray->LinkEndChild(meshNormTex);
			}
			else {
				meshElement_Mesh_SourceNormal_FloatArray->SetAttribute("count","0");
			}	

			meshElement_Mesh_SourceNormal->LinkEndChild(meshElement_Mesh_SourceNormal_FloatArray);

			TiXmlElement * meshElement_Mesh_SourceNormal_Technique=new TiXmlElement("technique_common");

			TiXmlElement * meshElement_Mesh_SourceNormal_Technique_Accessor=new TiXmlElement("accessor");
			meshElement_Mesh_SourceNormal_Technique_Accessor->SetAttribute("source",hash_meshN_geometry_normal_array.str());
			ostringstream meshNCountStr;
			meshNCountStr << meshNormals.size();
			meshElement_Mesh_SourceNormal_Technique_Accessor->SetAttribute("count",meshNCountStr.str());
			meshElement_Mesh_SourceNormal_Technique_Accessor->SetAttribute("stride","3");

			TiXmlElement * paramNX=new TiXmlElement("param");
			paramNX->SetAttribute("name","X");
			paramNX->SetAttribute("type","float");
			TiXmlElement * paramNY=new TiXmlElement("param");
			paramNY->SetAttribute("name","Y");
			paramNY->SetAttribute("type","float");
			TiXmlElement * paramNZ=new TiXmlElement("param");
			paramNZ->SetAttribute("name","Z");
			paramNZ->SetAttribute("type","float");

			meshElement_Mesh_SourceNormal_Technique_Accessor->LinkEndChild(paramNX);
			meshElement_Mesh_SourceNormal_Technique_Accessor->LinkEndChild(paramNY);
			meshElement_Mesh_SourceNormal_Technique_Accessor->LinkEndChild(paramNZ);

			meshElement_Mesh_SourceNormal_Technique->LinkEndChild(meshElement_Mesh_SourceNormal_Technique_Accessor);

			meshElement_Mesh_SourceNormal->LinkEndChild(meshElement_Mesh_SourceNormal_Technique);

			meshElement_Mesh->LinkEndChild(meshElement_Mesh_SourceNormal);
				
			//geometry - TEXTURE COORDINATES

			TiXmlElement * meshElement_Mesh_SourceTexUV=new TiXmlElement("source");
			meshElement_Mesh_SourceTexUV->SetAttribute("id",meshN_geometry_uv.str());

			TiXmlElement * meshElement_Mesh_SourceTexUV_FloatArray=new TiXmlElement("float_array");
			meshElement_Mesh_SourceTexUV_FloatArray->SetAttribute("id",meshN_geometry_uv_array.str());

			if (meshUVs.size()>0) {

				//get the count
				ostringstream meshUVCountStr;
				meshUVCountStr << meshUVs.size()*2;
				meshElement_Mesh_SourceTexUV_FloatArray->SetAttribute("count",meshUVCountStr.str());

				//save the verts
				ostringstream meshUVTexStr;
				
				for (int i=0;i<meshUVs.size();i++) 			
					meshUVTexStr << fixed << meshUVs[i].x << " " << meshUVs[i].y << " ";		

				TiXmlText *meshUVTex=new TiXmlText(meshUVTexStr.str());		

				meshElement_Mesh_SourceTexUV_FloatArray->LinkEndChild(meshUVTex);
			}
			else {
				meshElement_Mesh_SourceTexUV_FloatArray->SetAttribute("count","0");
			}	

			meshElement_Mesh_SourceTexUV->LinkEndChild(meshElement_Mesh_SourceTexUV_FloatArray);

			TiXmlElement * meshElement_Mesh_SourceTexUV_Technique=new TiXmlElement("technique_common");

			TiXmlElement * meshElement_Mesh_SourceTexUV_Technique_Accessor=new TiXmlElement("accessor");
			meshElement_Mesh_SourceTexUV_Technique_Accessor->SetAttribute("source",hash_meshN_geometry_uv_array.str());
			ostringstream meshUVCountStr;
			meshUVCountStr << meshUVs.size();
			meshElement_Mesh_SourceTexUV_Technique_Accessor->SetAttribute("count",meshUVCountStr.str());
			meshElement_Mesh_SourceTexUV_Technique_Accessor->SetAttribute("stride","2");

			TiXmlElement * paramS=new TiXmlElement("param");
			paramS->SetAttribute("name","S");
			paramS->SetAttribute("type","float");
			TiXmlElement * paramT=new TiXmlElement("param");
			paramT->SetAttribute("name","T");
			paramT->SetAttribute("type","float");

			meshElement_Mesh_SourceTexUV_Technique_Accessor->LinkEndChild(paramS);
			meshElement_Mesh_SourceTexUV_Technique_Accessor->LinkEndChild(paramT);	

			meshElement_Mesh_SourceTexUV_Technique->LinkEndChild(meshElement_Mesh_SourceTexUV_Technique_Accessor);

			meshElement_Mesh_SourceTexUV->LinkEndChild(meshElement_Mesh_SourceTexUV_Technique);

			meshElement_Mesh->LinkEndChild(meshElement_Mesh_SourceTexUV);

			ostringstream meshN_geometry_vertex;
			meshN_geometry_vertex << "mesh" << curMeshNumber << "-geometry-vertex";
			ostringstream hash_meshN_geometry_position;
			hash_meshN_geometry_position << "#mesh" << curMeshNumber << "-geometry-position";
			ostringstream hash_meshN_geometry_vertex;
			hash_meshN_geometry_vertex << "#mesh" << curMeshNumber << "-geometry-vertex";
			ostringstream hash_meshN_geometry_normal;
			hash_meshN_geometry_normal << "#mesh" << curMeshNumber << "-geometry-normal";
			ostringstream hash_meshN_geometry_uv;
			hash_meshN_geometry_uv << "#mesh" << curMeshNumber << "-geometry-uv";

			//vertices of geometry
			TiXmlElement *meshElement_Mesh_Vertices=new TiXmlElement("vertices");
			meshElement_Mesh_Vertices->SetAttribute("id",meshN_geometry_vertex.str());

			TiXmlElement *meshElement_Mesh_Vertices_Input=new TiXmlElement("input");
			meshElement_Mesh_Vertices_Input->SetAttribute("semantic","POSITION");
			meshElement_Mesh_Vertices_Input->SetAttribute("source",hash_meshN_geometry_position.str());	

			meshElement_Mesh_Vertices->LinkEndChild(meshElement_Mesh_Vertices_Input);

			meshElement_Mesh->LinkEndChild(meshElement_Mesh_Vertices);	

			TiXmlElement *meshElement_Mesh_Triangles=new TiXmlElement("triangles");

			ostringstream whichMaterialStr;			
			whichMaterialStr << "Material" << edgeList[i]->getWhichRoadMesh(j);
			meshElement_Mesh_Triangles->SetAttribute("material",whichMaterialStr.str());
			
			ostringstream hash_whichMaterialIDStr;
			hash_whichMaterialIDStr << "#Material" << edgeList[i]->getWhichRoadMesh(j) << "ID";

			bindMaterialsVec.push_back(whichMaterialStr.str());
			bindMaterialsVecHashID.push_back(hash_whichMaterialIDStr.str());			

			TiXmlElement *meshElement_Mesh_Triangles_InputVertex=new TiXmlElement("input");
			meshElement_Mesh_Triangles_InputVertex->SetAttribute("semantic","VERTEX");
			meshElement_Mesh_Triangles_InputVertex->SetAttribute("source",hash_meshN_geometry_vertex.str());
			meshElement_Mesh_Triangles_InputVertex->SetAttribute("offset","0");
			TiXmlElement *meshElement_Mesh_Triangles_InputNormal=new TiXmlElement("input");
			meshElement_Mesh_Triangles_InputNormal->SetAttribute("semantic","NORMAL");
			meshElement_Mesh_Triangles_InputNormal->SetAttribute("source",hash_meshN_geometry_normal.str());
			meshElement_Mesh_Triangles_InputNormal->SetAttribute("offset","1");
			TiXmlElement *meshElement_Mesh_Triangles_InputUV=new TiXmlElement("input");
			meshElement_Mesh_Triangles_InputUV->SetAttribute("semantic","TEXCOORD");
			meshElement_Mesh_Triangles_InputUV->SetAttribute("source",hash_meshN_geometry_uv.str());
			meshElement_Mesh_Triangles_InputUV->SetAttribute("offset","2");
			meshElement_Mesh_Triangles_InputUV->SetAttribute("set","0");	

			meshElement_Mesh_Triangles->LinkEndChild(meshElement_Mesh_Triangles_InputVertex);
			meshElement_Mesh_Triangles->LinkEndChild(meshElement_Mesh_Triangles_InputNormal);
			meshElement_Mesh_Triangles->LinkEndChild(meshElement_Mesh_Triangles_InputUV);

			if (meshFaceIndexes.size()>0) {

				ostringstream numTrysStr;
				numTrysStr << meshFaceIndexes.size()/3;

				meshElement_Mesh_Triangles->SetAttribute("count",numTrysStr.str());

				ostringstream meshTrysStr;

				TiXmlElement *paragraphElement=new TiXmlElement("p");
				for (int i=0;i<meshFaceIndexes.size();i++) 			
					meshTrysStr << meshFaceIndexes[i] << " " << i << " " << i << " "; 

				TiXmlText *meshFaceIndexTex=new TiXmlText(meshTrysStr.str());		

				paragraphElement->LinkEndChild(meshFaceIndexTex);

				meshElement_Mesh_Triangles->LinkEndChild(paragraphElement);

			}

			meshElement_Mesh->LinkEndChild(meshElement_Mesh_Triangles);

			//conclude mesh
			meshElement->LinkEndChild(meshElement_Mesh);
			//conclude this geometry
			geometryElement->LinkEndChild(meshElement);			

		}
		
	}
	//geometry of heightmap
	vector <TiXmlElement *> terrainElementsVec;
	if (heightMap!=NULL) {
		terrainElementsVec=heightMap->export_COLLADA_geometry();

		for (int i=0;i<terrainElementsVec.size();i++) {
			geometryElement->LinkEndChild(terrainElementsVec[i]);
			bindMaterialsVec.push_back("MaterialTer");
			bindMaterialsVecHashID.push_back("#MaterialTerID");
		}
	}

	//conclude all geometries of model
	element->LinkEndChild(geometryElement);

	//lib VISUAL SCENES
	TiXmlElement *libraryVisScenes=new TiXmlElement("library_visual_scenes");
	TiXmlElement *libraryVisScenes_VisScene=new TiXmlElement("visual_scene");
	libraryVisScenes_VisScene->SetAttribute("id","SketchUpScene");
	libraryVisScenes_VisScene->SetAttribute("name","SketchUpScene");
	TiXmlElement *libraryVisScenes_VisScene_NodeModel=new TiXmlElement("node");
	libraryVisScenes_VisScene_NodeModel->SetAttribute("id","Model");
	libraryVisScenes_VisScene_NodeModel->SetAttribute("name","Model");

	if (curMeshNumber>0) {

		for (int i=1;i<=curMeshNumber;i++) {

			ostringstream meshN;
			meshN << "mesh" << i;
			ostringstream hash_meshN_geometry;
			hash_meshN_geometry << "#mesh" << i << "-geometry";

			TiXmlElement *libraryVisScenes_VisScene_NodeModel_NodeMesh=new TiXmlElement("node");
			libraryVisScenes_VisScene_NodeModel_NodeMesh->SetAttribute("id",meshN.str());
			libraryVisScenes_VisScene_NodeModel_NodeMesh->SetAttribute("name",meshN.str());
			TiXmlElement *libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo=new TiXmlElement("instance_geometry");
			libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo->SetAttribute("url",hash_meshN_geometry.str());
			TiXmlElement *libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat=new TiXmlElement("bind_material");
			TiXmlElement *libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat_Tech=new TiXmlElement("technique_common");
			TiXmlElement *libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat_Tech_Instmat=new TiXmlElement("instance_material");
			
			libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat_Tech_Instmat->SetAttribute("symbol",bindMaterialsVec[i-1]);
			libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat_Tech_Instmat->SetAttribute("target",bindMaterialsVecHashID[i-1]);
			libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat_Tech->LinkEndChild(libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat_Tech_Instmat);
			libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat->LinkEndChild(libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat_Tech);
			libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo->LinkEndChild(libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat);
			libraryVisScenes_VisScene_NodeModel_NodeMesh->LinkEndChild(libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo);
			libraryVisScenes_VisScene_NodeModel->LinkEndChild(libraryVisScenes_VisScene_NodeModel_NodeMesh);

		}

	}

	if (heightMap!=NULL) {
		for (int i=0;i<terrainElementsVec.size();i++) {

			ostringstream meshN;
			meshN << "meshTer" << i;
			ostringstream hash_meshN_geometry;
			hash_meshN_geometry << "#meshTer" << i << "-geometry";

			TiXmlElement *libraryVisScenes_VisScene_NodeModel_NodeMesh=new TiXmlElement("node");
			libraryVisScenes_VisScene_NodeModel_NodeMesh->SetAttribute("id",meshN.str());
			libraryVisScenes_VisScene_NodeModel_NodeMesh->SetAttribute("name",meshN.str());
			TiXmlElement *libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo=new TiXmlElement("instance_geometry");
			libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo->SetAttribute("url",hash_meshN_geometry.str());
			TiXmlElement *libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat=new TiXmlElement("bind_material");
			TiXmlElement *libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat_Tech=new TiXmlElement("technique_common");
			TiXmlElement *libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat_Tech_Instmat=new TiXmlElement("instance_material");
			
			libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat_Tech_Instmat->SetAttribute("symbol",bindMaterialsVec[bindMaterialsVec.size()-1]);
			libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat_Tech_Instmat->SetAttribute("target",bindMaterialsVecHashID[bindMaterialsVec.size()-1]);
			libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat_Tech->LinkEndChild(libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat_Tech_Instmat);
			libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat->LinkEndChild(libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat_Tech);
			libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo->LinkEndChild(libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo_BindMat);
			libraryVisScenes_VisScene_NodeModel_NodeMesh->LinkEndChild(libraryVisScenes_VisScene_NodeModel_NodeMesh_InstanceGeo);
			libraryVisScenes_VisScene_NodeModel->LinkEndChild(libraryVisScenes_VisScene_NodeModel_NodeMesh);

		}
	}

	libraryVisScenes_VisScene->LinkEndChild(libraryVisScenes_VisScene_NodeModel);
	libraryVisScenes->LinkEndChild(libraryVisScenes_VisScene);
	element->LinkEndChild(libraryVisScenes);

	//scene
	TiXmlElement *sceneElement=new TiXmlElement("scene");
	TiXmlElement *sceneElement_Instance=new TiXmlElement("instance_visual_scene");
	sceneElement_Instance->SetAttribute("url","#SketchUpScene");
	sceneElement->LinkEndChild(sceneElement_Instance);
	element->LinkEndChild(sceneElement);
   

	doc.LinkEndChild( decl );
	doc.LinkEndChild( element );

	doc.SaveFile(exportFilename);

	printf("Finished!  ");
	if (heightMap==NULL)
		printf("Wrote %i meshes.\n",curMeshNumber);
	else
		printf("Wrote %i road meshes, %i terrain meshes.\n",curMeshNumber,terrainElementsVec.size());

	*/

}

void RN::export_DAT(char *outfilename) {

	/*
	ostringstream outStr;

	outStr << fixed << "EDGES " << edgeList.size() << "\n";

	for (int i=0;i<edgeList.size();i++) 
		edgeList[i]->export_DAT(outStr);	

	outStr << "INTERSECTS " << intersectList.size() << "\n";
	for (int i=0;i<intersectList.size();i++) {
		RN_Edge *edge1=intersectList[i]->getEdge(0);
		int segment1=intersectList[i]->getSegment(0);
		RN_Edge *edge2=intersectList[i]->getEdge(1);
		int segment2=intersectList[i]->getSegment(1);
		int edge1int=-1, edge2int=-1;
		int relation=intersectList[i]->getRelation();

		for (int j=0;j<edgeList.size();j++) {
			if (edge1==edgeList[j])
				edge1int=j;
			if (edge2==edgeList[j])
				edge2int=j;
		}

		if (edge1int==-1||edge2int==-1) 
			printf("Warning saving ambiguous intersection.\n");

		outStr << edge1int << " " << segment1 << " " << edge2int << " " << segment2 << " " << relation << "\n";
	}

	//do objects
	objManager->export_DAT(outStr);

	//done making string, lets move it out!
	ofstream out(outfilename, ios::out);
	if (!out) {
		printf("Could not write %s.\n",outfilename);
		return;
	}
	
	out << outStr.str().c_str();

	out.close();
	*/

}

void RN::import_DAT(char *infilename, HeightMap *heightMap) {

	FILE *inf;
	inf=fopen(infilename,"r");

	if (inf==NULL) {
		printf("Could not read %s.\n",infilename);
		return;
	}

	edgeList.clear();
	intersectList.clear();
	selectedEdgeList.clear();
	selectedIntersectList.clear();


	char buf[255];

	fgets(buf,255,inf);

	int edgeListSize;
	sscanf(buf,"EDGES %i",&edgeListSize);

	edgeList.reserve(edgeListSize);
	for (int i=0;i<edgeListSize;i++) {
		RN_Edge *eachEdge=new RN_Edge();
		eachEdge->import_DAT(inf);

		eachEdge->useCatmullRomSpline(true);
		
		doPrepareTerrain(eachEdge, heightMap);

		edgeList.push_back(eachEdge);
	}

	fgets(buf,255,inf);
	int intersectListSize;
	sscanf(buf,"INTERSECTS %i",&intersectListSize);

	intersectList.reserve(intersectListSize);
	for (int i=0;i<intersectListSize;i++) {
		fgets(buf,255,inf);

		int e1, s1, e2, s2, r;
		sscanf(buf,"%i %i %i %i %i",&e1,&s1,&e2,&s2,&r);
		
		RN_Intersection *eachInt=new RN_Intersection(edgeList[e1],s1,edgeList[e2],s2);
		eachInt->setRelation(r);

		intersectList.push_back(eachInt);
	}

	//object manager stuff
	objManager->import_DAT(inf);

	//done!
	fclose(inf);

}

void RN::doDepthRelations() {

	//given the list of depth relations, here we do the optimization to figure out how everything should fit
	bool finishedProcessing;

	do {

		finishedProcessing=true;

		//printf("\nChecking intersection heights.\n");
		for (int i=0;i<intersectList.size();i++) {
			//printf("%i - %f, %f ",i,intersectList[i]->getHeight(0),intersectList[i]->getHeight(1));
			if (intersectList[i]->getRelation()==RELATION_UNDEROVER) {
				if (intersectList[i]->getHeight(1)<intersectList[i]->getHeight(0)+MIN_DIST_BETWEEN_POINTS) {					
					intersectList[i]->raiseEdgeAbove(intersectList[i]->getEdge(1),intersectList[i]->getSegment(1),
													intersectList[i]->getEdge(0),intersectList[i]->getSegment(0),
													intersectList[i]->getHeight(0)+MIN_DIST_BETWEEN_POINTS-intersectList[i]->getHeight(1));
					finishedProcessing=false;					
				}				
			}
			else if (intersectList[i]->getRelation()==RELATION_OVERUNDER) {
				if (intersectList[i]->getHeight(0)<intersectList[i]->getHeight(1)+MIN_DIST_BETWEEN_POINTS) {					
					intersectList[i]->raiseEdgeAbove(intersectList[i]->getEdge(0),intersectList[i]->getSegment(0),
													intersectList[i]->getEdge(1),intersectList[i]->getSegment(1),
													intersectList[i]->getHeight(1)+MIN_DIST_BETWEEN_POINTS-intersectList[i]->getHeight(0));
					finishedProcessing=false;
				}
			}
		}

	} while (!finishedProcessing);

}

void RN::changeTiming(char *buttonText) {

	for (int i=0;i<selectedEdgeList.size();i++) {
		for (int j=edgeList[selectedEdgeList[i]]->getFirstSegmentSelected();
				j<=edgeList[selectedEdgeList[i]]->getLastSegmentSelected();j++) {
			if (strcmp(buttonText,"Slow")==0) {
				edgeList[selectedEdgeList[i]]->setPointAttrib(j,POINT_ATTRIB_SPEED_SLOW,true);
				edgeList[selectedEdgeList[i]]->setPointAttrib(j,POINT_ATTRIB_SPEED_FAST,false);
			}
			else if (strcmp(buttonText,"Fast")==0) {
				edgeList[selectedEdgeList[i]]->setPointAttrib(j,POINT_ATTRIB_SPEED_SLOW,false);
				edgeList[selectedEdgeList[i]]->setPointAttrib(j,POINT_ATTRIB_SPEED_FAST,true);
			}
			else {
				edgeList[selectedEdgeList[i]]->setPointAttrib(j,POINT_ATTRIB_SPEED_SLOW,false);
				edgeList[selectedEdgeList[i]]->setPointAttrib(j,POINT_ATTRIB_SPEED_FAST,false);
			}			

		}
	}

	doObjects();

}

RN_Edge *RN::getPlayModeLongEdge() {	

	vector <Vector> pointSet;
	
	int curEdge=0;
	int curSegment=0;

	while (true) {

		pointSet.push_back(edgeList[curEdge]->getPoint(curSegment));

		if (edgeList[curEdge]->getPointAttrib(curSegment,POINT_ATTRIB_CROSS)) {
			//determine if there is another edge to continue onto

			for (int i=0;i<intersectList.size();i++) {
				
				if (intersectList[i]->getRelation()!=RELATION_INTERSECT)
					continue;

				if (intersectList[i]->getEdge(0)==edgeList[curEdge]&&
					intersectList[i]->getSegment(0)==curSegment) {

					RN_Edge *curNewEdge=intersectList[i]->getEdge(1);
					curSegment=intersectList[i]->getSegment(1);

					for (int j=0;j<edgeList.size();j++) {
						if (curNewEdge==edgeList[j]) {
							curEdge=j;
							break;
						}
					}

					break;
				}
				else if (intersectList[i]->getEdge(1)==edgeList[curEdge]&&
					intersectList[i]->getSegment(1)==curSegment) {

					RN_Edge *curNewEdge=intersectList[i]->getEdge(0);
					curSegment=intersectList[i]->getSegment(0);

					for (int j=0;j<edgeList.size();j++) {
						if (curNewEdge==edgeList[j]) {
							curEdge=j;
							break;
						}
					}

					break;
				}
			}

		}

		curSegment++;

		if (curSegment>=edgeList[curEdge]->getNumPoints())
			break;

	}

	RN_Edge *returnEdge=new RN_Edge();
	returnEdge->setPointSet(pointSet);
	returnEdge->useCatmullRomSpline(true);
	returnEdge->setSelected(true);	
	for (int i=0;i<returnEdge->getNumPoints();i++) {
		returnEdge->setPointSelected(i,true);
		returnEdge->setPointAttrib(i,POINT_ATTRIB_SPEED_FAST,true);
	}

	return returnEdge;

}
