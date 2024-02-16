#include "RN_Edge.h"

vector <TriMeshTextured *> RN_Edge::roadMesh;

HeightMap *RN_Edge::heightMap;

RN_Edge::RN_Edge() {

    catmullRomSpline=NULL;

    const char *roadObjs[] = {
        "objs/assets/road.obj", //#define ROAD_PIECE_1_TEXTURE "objs/assets/road.png"
        "objs/assets/road_intersect.obj", //"objs/assets/road_intersect.png"
        "objs/assets/road_at_intersect.obj", //"objs/assets/road_at_intersect.png"
        "objs/assets/road_pass_intersect.obj", //"objs/assets/road_pass_intersect.png"
        "objs/assets/bridgeroad.obj", // "objs/assets/bridgeroad.png"
        "objs/assets/tunnelroad.obj", //ROAD_PIECE_6_INDEX  PIECE_5_INDEX ??
        "objs/assets/tunnelhole.obj"}; //ROAD_PIECE_7_INDEX  0 ??

    if (roadMesh.empty()) {

        for (int i=0;i<7;i++) {
            TriMeshTextured *eachRoadPiece=new TriMeshTextured();
            eachRoadPiece->Read(roadObjs[i]);

            roadMesh.push_back(eachRoadPiece);
        }

    }

    pointViewTwist.clear();

    numPointsSelected=0;
    firstPointSelected=-1;
    lastPointSelected=-1;

}

RN_Edge::~RN_Edge() {

	suggestedPointSet.clear();
	pointSelected.clear();	
	pointAttrib.clear();
	pointViewTwist.clear();
	whichRoadMesh.clear();

	//clothoidSpline.clear();	

}

void RN_Edge::setSelected(bool sel) {
	selected=sel;
}

void RN_Edge::selectWithRay(Vector pos_xyz, Vector rayDir) {	
	selected=false;
}

void RN_Edge::selectWithClosedCurve(vector <Vector> & pointSet) {
	selected=false;
}

/*
 * This selects the largest CONTIGUOUS set of selected points.
 */
void RN_Edge::selectWithClosedCurve2D(vector <Vector> & pointSet) {

	//at this point, at least one vertex was selected, therefore some
	//points along the edge which connects the two vertexes must also
	//be selected	
	
	//first, gluProject this vertex
	GLdouble model_view[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLdouble eachxval, eachyval, eachzval;
	
	selected=false;

	vector <int> startStreak;
	vector <int> endStreak;
	bool doingStreak=false;
	
	for (int each=0;each<suggestedPointSet.size();each++) {

		gluProject(suggestedPointSet[each].x,suggestedPointSet[each].y,suggestedPointSet[each].z,
					model_view, projection, viewport, &eachxval, &eachyval, &eachzval);

		pointSelected[each]=false;
		for (int i=1;i<pointSet.size()-1;i++) {

			//then do collision with triangles of 2d pointset
			pointSelected[each]=Collision::PointTriangleCollide(
				Vector(eachxval,eachyval,0),
				Vector(pointSet[0].x,pointSet[0].y,0),
				Vector(pointSet[i].x,pointSet[i].y,0),
				Vector(pointSet[i+1].x,pointSet[i+1].y,0));
			if (pointSelected[each])
				break;			
		}	

		if (pointSelected[each]) {				
			selected=true;
			if (!doingStreak) { //accumulate a streak
				startStreak.push_back(each);
				doingStreak=true;
			}			
		}
		else {
			if (doingStreak) {
				endStreak.push_back(each-1);
				doingStreak=false;
			}

		}

	}	

	if (doingStreak) {
		endStreak.push_back(suggestedPointSet.size()-1);
	}

	//maximal streak will be selection region
	int maxStreakIndex;
	numPointsSelected=0;

	if (startStreak.size()==0)
		return;

	for (int i=0;i<startStreak.size();i++) {
		if (endStreak[i]-startStreak[i]+1>numPointsSelected) {
			numPointsSelected=endStreak[i]-startStreak[i]+1;
			maxStreakIndex=i;
		}
	}

	unselectAll();

	selected=true;
	firstPointSelected=startStreak[maxStreakIndex];
	lastPointSelected=endStreak[maxStreakIndex];
	numPointsSelected=lastPointSelected-firstPointSelected+1;

	for (int i=firstPointSelected;i<=lastPointSelected;i++) 
		pointSelected[i]=true;

	startStreak.clear();
	endStreak.clear();

}

void RN_Edge::updateWhichRoadMesh() {

	whichRoadMesh.clear();
	for (int i=0;i<suggestedPointSet.size();i++) {
		if (getPointAttrib(i,POINT_ATTRIB_DONOTRENDER)) {						
			whichRoadMesh.push_back(-1);
		}
		else if (getPointAttrib(i,POINT_ATTRIB_INTERSECT)) {					
			whichRoadMesh.push_back(PIECE_2_INDEX);
		}
		else if (i>0&&getPointAttrib(i-1,POINT_ATTRIB_INTERSECT))
			whichRoadMesh.push_back(PIECE_4_INDEX);
		else if (i<suggestedPointSet.size()-2&&getPointAttrib(i+1,POINT_ATTRIB_INTERSECT))
			whichRoadMesh.push_back(PIECE_3_INDEX);		
		else if ((i>0&&getPointAttrib(i-1,POINT_ATTRIB_TUNNEL))||
				getPointAttrib(i,POINT_ATTRIB_TUNNEL)||
				i<suggestedPointSet.size()-2&&getPointAttrib(i+1,POINT_ATTRIB_TUNNEL))
			whichRoadMesh.push_back(PIECE_6_INDEX);		
		else if ((i>0&&getPointAttrib(i-1,POINT_ATTRIB_BRIDGE))||
				getPointAttrib(i,POINT_ATTRIB_BRIDGE)||
				i<suggestedPointSet.size()-2&&getPointAttrib(i+1,POINT_ATTRIB_BRIDGE))
			whichRoadMesh.push_back(PIECE_5_INDEX);					
		else if (getPointAttrib(i,POINT_ATTRIB_TUNNEL))
			whichRoadMesh.push_back(PIECE_6_INDEX);		
		else if (getPointAttrib(i,POINT_ATTRIB_BRIDGE))
			whichRoadMesh.push_back(PIECE_5_INDEX);		
		else {			
			//setPointAttrib(i,POINT_ATTRIB_TUNNEL,true);
			//whichRoadMesh.push_back(PIECE_6_INDEX);
			whichRoadMesh.push_back(PIECE_1_INDEX);
		}
	}

}

void RN_Edge::useCatmullRomSpline(bool useCatmullRom) {

	if (catmullRomSpline!=NULL) 
		delete catmullRomSpline;  

	//set up clothoid spline
	if (useCatmullRom) {
		if (suggestedPointSet.size()>=2) { //careful with this!

			if (getPointAttrib(suggestedPointSet.size()-1,POINT_ATTRIB_CLOSED_LOOP)) {
				suggestedPointSet[suggestedPointSet.size()-1]=suggestedPointSet[0];				
			}

			//construct the spline (using the meshes)
			catmullRomSpline=new CatmullRomSpline(this);

			//determine meshes to use in road's construction
			updateWhichRoadMesh();
			
			//set special tangents for intersection, and closed loop			
			for (int i=0;i<suggestedPointSet.size();i++) {					

				if (i<suggestedPointSet.size()-1&&getPointAttrib(i,POINT_ATTRIB_INTERSECT))
					catmullRomSpline->setTangent(i,suggestedPointSet[i+1]-suggestedPointSet[i]);
				if (i>0&&getPointAttrib(i-1,POINT_ATTRIB_INTERSECT))
					catmullRomSpline->setTangent(i,suggestedPointSet[i]-suggestedPointSet[i-1]);	
				if (i==suggestedPointSet.size()-1&&getPointAttrib(i,POINT_ATTRIB_CLOSED_LOOP)) {					
					catmullRomSpline->setTangent(i,catmullRomSpline->getTangent(0,0));
				}

			}

			//have to set tangents and positions for all start/endpoints of clothoids
			
			//finally, create the final mesh for this road
			catmullRomSpline->processMeshSegments(roadMesh, whichRoadMesh);
		}
	}
	else {
		catmullRomSpline=NULL;
	}

}

void RN_Edge::useClothoidSpline(HeightMap *heightMap) {	

	//ClothoidSpline *clothoidSpline=new ClothoidSpline(this,startPoint,endPoint);
	//clothoidSpline=new ClothoidSpline(this,startPoint,endPoint,piecePenalty);
	//clothoidSpline=new ClothoidSpline(this,startPoint,endPoint);
	clothoidSpline=new ClothoidSpline(suggestedPointSet);
	
	for (int i=0;i<suggestedPointSet.size();i++) {	
		
		//set points to follow clothoid spline
		Vector oldPoint=suggestedPointSet[i];
		suggestedPointSet[i]=clothoidSpline->getPoint(i);

		Vector heightMapSelectPoint;
		heightMap->selectWithRay(Vector(suggestedPointSet[i].x,256.0f,suggestedPointSet[i].z),
								Vector(0.0f,-1.0f,0.0f), heightMapSelectPoint);

		if (getPointAttrib(i,POINT_ATTRIB_BRIDGE))
			oldPoint.y;
		else
			suggestedPointSet[i].y=heightMapSelectPoint.y;
	}

	updateStructure();

	//export the spline and the polyline to maya
	
	//clothoidSpline->setupFineSamples();
	//clothoidSpline->outputToMayaMEL(exportClothoidFilename);
	//clothoidSpline->outputToIllustratorAI(exportClothoidFilename);

	//exportClothoidFilename++;

}

/*
void RN_Edge::useClothoidSpline(bool useClothoid, float penalty) {

	if (clothoidSpline!=NULL) 
		delete clothoidSpline;  

	//set up clothoid spline
	if (useClothoid) {
		if (suggestedPointSet.size()>=2) { //careful with this!
			clothoidSpline=new ClothoidSpline(this, penalty);		
		}
	}
	else {
		clothoidSpline=NULL;
	}

}
*/

void RN_Edge::draw() {		
	
	if (catmullRomSpline!=NULL) {

		glEnable(GL_TEXTURE_2D);		
		
                for (int i=0; i<suggestedPointSet.size()-1; ++i) 	{
			
			if (getPointAttrib(i,POINT_ATTRIB_DONOTRENDER)||whichRoadMesh[i]==-1)
				continue;			
 			
			catmullRomSpline->drawSegmentWithMesh(i);
		}

		glDisable(GL_TEXTURE_2D);
	}

	/*
	if (IsKeyPressed('z')||IsKeyPressed('x')) {
		if (IsKeyPressed('z'))
			piecePenalty*=1.01f;
		else
			piecePenalty=piecePenalty/(1.01f);

		delete clothoidSpline;
		clothoidSpline=new ClothoidSpline(this,0,suggestedPointSet.size()-1,piecePenalty);
		
		printf("penalty: %f\n",piecePenalty);
	}	
	else if (IsKeyPressed('v')||IsKeyPressed('c')) {
		if (IsKeyPressed('v'))
			endpointWeight*=1.05f;
		else
			endpointWeight/=1.05f;

		clothoidSpline->setupFitTransform(endpointWeight);

		printf("endpointWeight: %f\n",endpointWeight);
	}
	else if (IsKeyPressed('s')) {
		clothoidSpline->setupFineSamples();
		clothoidSpline->outputToMayaMEL(exportClothoidFilename);
		//clothoidSpline->outputToIllustratorAI(exportClothoidFilename);
		printf("Wrote %i.ai/mb\n",exportClothoidFilename);
		exportClothoidFilename++;
	}
	*/	

	
	if (clothoidSpline!=NULL) {			
		//clothoidSpline->draw();	
		//if (IsKeyPressed('c')) {
			//clothoidSpline->drawCurvaturePlot2D();
			//glutPostRedisplay();
		//}
	}	

}

void RN_Edge::drawTunnelHoles() {

	if (catmullRomSpline!=NULL) {
		
		glDisable(GL_TEXTURE_2D);

		for (int i=2;i<suggestedPointSet.size()-2;i++) 	{

			if (whichRoadMesh[i]!=PIECE_6_INDEX)
				continue;

			if (!getPointAttrib(i-2,POINT_ATTRIB_TUNNEL)||!getPointAttrib(i-1,POINT_ATTRIB_TUNNEL)||
				!getPointAttrib(i+2,POINT_ATTRIB_TUNNEL)||!getPointAttrib(i+1,POINT_ATTRIB_TUNNEL))
			catmullRomSpline->drawSegmentWithHole(i,PIECE_7_INDEX);
		}

	}
}

void RN_Edge::setPoint(int index, Vector newPoint) {

	if (index>=0&&index<suggestedPointSet.size())
		suggestedPointSet[index]=newPoint;

}

Vector RN_Edge::getPoint(int index) {
	
	if (index>=0&&index<suggestedPointSet.size())
		return suggestedPointSet[index];
	

	printf("Warning Index %i being called when size is %i.\n",index, suggestedPointSet.size());
	return Vector(0.0f,0.0f,0.0f);
	
}

void RN_Edge::removePoint(int index) {

	//printf("removing at index: %i\n",index);
	//printSizes();

	suggestedPointSet.erase(suggestedPointSet.begin()+index);
	pointSelected.erase(pointSelected.begin()+index);
	pointAttrib.erase(pointAttrib.begin()+index);	
	if (!pointViewTwist.empty())		
		pointViewTwist.erase(pointViewTwist.begin()+index);	
	whichRoadMesh.erase(whichRoadMesh.begin()+index);	

}

void RN_Edge::printSizes() {
	printf("sugsize: %i pointsel: %i ptattrib: %i ptviewtwist: %i whichrmesh: %i\n",
		suggestedPointSet.size(),pointSelected.size(), pointAttrib.size(), pointViewTwist.size(),
		whichRoadMesh.size());
}

void RN_Edge::insertPoint(int index, Vector pos, bool sel, int attrib, float twist, int whichMesh) {

	//printf("inserting into index: %i\n",index);
	//printSizes();

	suggestedPointSet.insert(suggestedPointSet.begin()+index,pos);
	pointSelected.insert(pointSelected.begin()+index,sel);
	pointAttrib.insert(pointAttrib.begin()+index,attrib);	
	if (!pointViewTwist.empty())		
		pointViewTwist.insert(pointViewTwist.begin()+index,twist);	
	whichRoadMesh.insert(whichRoadMesh.begin()+index,whichMesh);	

}

void RN_Edge::filterThisEdge() {

	for (int i=1;i<suggestedPointSet.size();i++) {
		//printf("i: %i cursize: %i\n",i,suggestedPointSet.size());
		Vector point1=suggestedPointSet[i];
		point1.y=0.0f;
		Vector point2=suggestedPointSet[i-1];
		point2.y=0.0f;
		float eachdist=(point2-point1).GetLength();		

		if (eachdist<MIN_DIST_BETWEEN_POINTS*0.75f&&pointAttrib[i-1]==0&&pointAttrib[i]==0&&
			((i<suggestedPointSet.size()-1&&pointAttrib[i+1]==0)||i==suggestedPointSet.size()-1)) {  //point too close
			
			removePoint(i);
			i--;
		}
		else if (eachdist>=MIN_DIST_BETWEEN_POINTS*1.5f) { //point too far

			Vector newVec=suggestedPointSet[i]-suggestedPointSet[i-1];
			newVec.SetLength(MIN_DIST_BETWEEN_POINTS);

			if (getPointAttrib(i,POINT_ATTRIB_BRIDGE)||getPointAttrib(i-1,POINT_ATTRIB_BRIDGE))
				insertPoint(i,suggestedPointSet[i-1]+newVec,false,POINT_ATTRIB_BRIDGE,0.0f,PIECE_1_INDEX);
			else
				insertPoint(i,suggestedPointSet[i-1]+newVec,false,0,0.0f,PIECE_1_INDEX);
			
		}
		else if (i>1&&!getPointAttrib(i-1,POINT_ATTRIB_CROSS)&&!getPointAttrib(i,POINT_ATTRIB_CROSS)&&
			((i<suggestedPointSet.size()-1&&!getPointAttrib(i+1,POINT_ATTRIB_CROSS))||i==suggestedPointSet.size()-1)) { //next point has to be within halfplane defined by tangent
			Vector tangentBefore=suggestedPointSet[i-1]-suggestedPointSet[i-2];
			tangentBefore.Normal();
			Vector myDisplace=suggestedPointSet[i]-suggestedPointSet[i-1];

			if (tangentBefore.DotProduct3(myDisplace.getNormal())<0.0f) {

				removePoint(i);
				i--;

			}
		}

	}

}

void RN_Edge::setPointSet(const vector <Vector> & set) {
	
	suggestedPointSet.clear();
	pointSelected.clear();
	pointAttrib.clear();
	pointViewTwist.clear();
	whichRoadMesh.clear();

	//clothoidSpline.clear();	

	if (set.empty()) {
		return;	
	}

	suggestedPointSet=set;

	for (int i=0;i<suggestedPointSet.size();i++) {
		pointSelected.push_back(false);
		pointAttrib.push_back(0);		
		whichRoadMesh.push_back(PIECE_1_INDEX);

		//clothoidSpline.push_back(NULL);		
	}	
	
	filterThisEdge();

	updateStructure();
	
}

int RN_Edge::getNumPoints() {
	return suggestedPointSet.size();
}

int RN_Edge::getNumPointsSelected() {
	return numPointsSelected;
}

void RN_Edge::edgesCross(RN_Edge *otherEdge, int &mySegment, int &otherSegment, Vector &intersectVec) {

	for (int i=0;i<suggestedPointSet.size()-1;i++) {
		for (int j=0;j<otherEdge->getNumPoints()-1;j++) {

			if (Collision::LineSegmentsCollide(suggestedPointSet[i], 
												suggestedPointSet[i+1], 
												otherEdge->getPoint(j),
												otherEdge->getPoint(j+1), intersectVec)&&
					!getPointAttrib(i,POINT_ATTRIB_CROSS)) {					

				mySegment=i;
				otherSegment=j;
				return;
			}

		}
	}

	mySegment=-1;
	otherSegment=-1;

}

float RN_Edge::getLength() {
	
	float length=0.0;
	
	for (int i=1;i<suggestedPointSet.size();i++) {		
		length+=(suggestedPointSet[i]-suggestedPointSet[i-1]).GetLength();
	}

	return length;

}

Vector RN_Edge::getFirstPointSelected() {

	if (firstPointSelected>=0)
		return suggestedPointSet[firstPointSelected];
	else 
		return Vector(0,0,0);
}

Vector RN_Edge::getLastPointSelected() {
	
	if (lastPointSelected>=0)
		return suggestedPointSet[lastPointSelected];
	else 
		return Vector(0,0,0);
}

int RN_Edge::getFirstSegmentSelected() {
	return firstPointSelected;
}

int RN_Edge::getLastSegmentSelected() {
	return lastPointSelected;
}

bool RN_Edge::getPointSelected(int i) {
	return pointSelected[i];
}

void RN_Edge::replaceRangeWith(RN_Edge *newEdge, int start, int end) {

	suggestedPointSet.erase(suggestedPointSet.begin()+start,suggestedPointSet.begin()+end);
	pointAttrib.erase(pointAttrib.begin()+start,pointAttrib.begin()+end);
	if (!pointViewTwist.empty())
		pointViewTwist.erase(pointViewTwist.begin()+start, pointViewTwist.begin()+end);
	whichRoadMesh.erase(whichRoadMesh.begin()+start,whichRoadMesh.begin()+end);
	pointSelected.erase(pointSelected.begin()+start,pointSelected.begin()+end);

	suggestedPointSet.insert(suggestedPointSet.begin()+start,newEdge->suggestedPointSet.begin(),newEdge->suggestedPointSet.end());
	pointAttrib.insert(pointAttrib.begin()+start,newEdge->pointAttrib.begin(),newEdge->pointAttrib.end());
	if (!pointViewTwist.empty()&&!newEdge->pointViewTwist.empty())
		pointViewTwist.insert(pointViewTwist.begin()+start,newEdge->pointViewTwist.begin(),newEdge->pointViewTwist.end());
	whichRoadMesh.insert(whichRoadMesh.begin()+start,newEdge->whichRoadMesh.begin(),newEdge->whichRoadMesh.end());
	pointSelected.insert(pointSelected.begin()+start,newEdge->pointSelected.begin(),newEdge->pointSelected.end());
	
	filterThisEdge();

	if (catmullRomSpline!=NULL) {
		delete catmullRomSpline;
		catmullRomSpline=new CatmullRomSpline(this);	
		catmullRomSpline->processMeshSegments(roadMesh,whichRoadMesh);
	}

}

void RN_Edge::replaceHeightOfSelected(vector <Vector> pointSet2D, vector <Vector> pointSet3D) {

	int selectStart=getFirstSegmentSelected();
	int selectEnd=getLastSegmentSelected();	

	int pointSet2DminX=1000000;
	int pointSet2DmaxX=0;

	for (int i=0;i<pointSet2D.size();i++) {
		if (pointSet2D[i].x>pointSet2DmaxX)
			pointSet2DmaxX=pointSet2D[i].x;
		if (pointSet2D[i].x<pointSet2DminX)
			pointSet2DminX=pointSet2D[i].x;
	}

	//Camera should be set to breakout view for these matrices to be correct!
	GLdouble model_view[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);	

	for (int i=selectStart;i<=selectEnd;i++) {		

		//eachvals are window coordinates of each point
		GLdouble eachxval;
		GLdouble eachyval;
		GLdouble eachzval;
		gluProject(suggestedPointSet[i].x,suggestedPointSet[i].y,suggestedPointSet[i].z,
					model_view, projection, viewport, &eachxval, &eachyval, &eachzval);		

		if (eachxval>pointSet2DminX&&eachxval<pointSet2DmaxX) {

			//interpolate y value to use linearly from pointset
			float interpYval;
			for (int j=0;j<pointSet2D.size()-1;j++) {
				if (eachxval>=pointSet2D[j].x&&eachxval<=pointSet2D[j+1].x) {
					float interp=(eachxval-pointSet2D[j].x)/(pointSet2D[j+1].x-pointSet2D[j].x);
					interpYval=pointSet3D[j].y*(1.0-interp)+pointSet3D[j+1].y*interp;
					break;
				}
				else if (eachxval>=pointSet2D[j+1].x&&eachxval<=pointSet2D[j].x) {
					float interp=(eachxval-pointSet2D[j+1].x)/(pointSet2D[j].x-pointSet2D[j+1].x);
					interpYval=pointSet3D[j+1].y*(1.0-interp)+pointSet3D[j].y*interp;
					break;
				}
			}
			
			suggestedPointSet[i].y=interpYval;
			setPointAttrib(i,POINT_ATTRIB_BRIDGE, true);
			
		}

	}	

	//then just reconstruct the road
	updateStructure();

}

Vector RN_Edge::getInterpPosition(int segment, float interp) {

	Vector withoutY=catmullRomSpline->interp(segment, interp);
	float yComponent=suggestedPointSet[segment].y*(1.0-interp)+suggestedPointSet[segment+1].y*(interp);

        Vector returnvec = withoutY+Vector(0,yComponent,0);

        qDebug() << "getInterpPosition" << returnvec.x << returnvec.y << returnvec.z;
        return returnvec;
}

Vector RN_Edge::getInterpTangent(int segment, float interp) {	

	Vector withoutY=catmullRomSpline->getTangent(segment,interp);
	Vector yComponent;
		
	if (catmullRomSpline->getYSpline()!=NULL) {
		yComponent=catmullRomSpline->getYSpline()->yInterp(segment, interp+0.01f)-
					catmullRomSpline->getYSpline()->yInterp(segment, interp);
		yComponent.Normal();
		yComponent.y=yComponent.z;
	}
	else {
		printf("ycomponent was null!\n");		
		if (segment<suggestedPointSet.size()-2)
			yComponent=(suggestedPointSet[segment+2]-suggestedPointSet[segment+1])*interp+(suggestedPointSet[segment+1]-suggestedPointSet[segment])*(1.0-interp);
		else
			yComponent=suggestedPointSet[segment+1]-suggestedPointSet[segment];

		yComponent.Normal();	
	}

	return withoutY+Vector(0,yComponent.y,0);
}

void RN_Edge::setViewTwists(vector <float> newViewTwists, Vector pointOfRotation, Vector axisOfRotation) {

	pointViewTwist=newViewTwists;
	this->pointOfRotation=pointOfRotation;
	this->axisOfRotation=axisOfRotation;

	//rebuild mesh based on these viewtwists/axis of rotation
	if (catmullRomSpline!=NULL)
		useCatmullRomSpline(this);	

}

vector <float> RN_Edge::getViewTwists() {

	return pointViewTwist;

}

Vector RN_Edge::getPointOfRotation() {
	return pointOfRotation;
}

Vector RN_Edge::getAxisOfRotation() {
	return axisOfRotation;
}

void RN_Edge::setPointSelected(int index, bool sel) {
	if (index<0||index>=pointSelected.size())
		return;

	pointSelected[index]=sel;

	if (firstPointSelected==-1||firstPointSelected>index)
		firstPointSelected=index;
	if (lastPointSelected==-1||lastPointSelected<index)
		lastPointSelected=index;

}

void RN_Edge::unselectAll() {
	firstPointSelected=-1;
	lastPointSelected=-1;
	numPointsSelected=0;
	selected=false;
	for (int i=0;i<pointSelected.size();i++)
		pointSelected[i]=false;

}

void RN_Edge::updateStructure() {

	//if clothoids or catmullromsplines are used, they need regeneration
	if (catmullRomSpline!=NULL)
		useCatmullRomSpline(true);

	//if (clothoidSpline!=NULL)
	//	useClothoidSpline(true, clothoidSpline->getPiecePenalty());

}

bool RN_Edge::getPointAttrib(int segment, int attrib) {	
	return ((pointAttrib[segment]&attrib)>0);
}

int RN_Edge::getPointAttribVal(int segment) {
	return pointAttrib[segment];
}


void RN_Edge::setPointAttrib(int segment, int attrib, bool val) {

	if (val) {
		pointAttrib[segment]=pointAttrib[segment]|attrib;
	}
	else {
		pointAttrib[segment]=pointAttrib[segment]&(0xFFFFFFFF^attrib);
	}
}

//assumption is that otherEdge begins where the endpoint of this
//edge ends!
void RN_Edge::concatenateEdge(RN_Edge *otherEdge) {	

	for (int i=0;i<otherEdge->suggestedPointSet.size();i++) {
		suggestedPointSet.push_back(otherEdge->suggestedPointSet[i]);
		pointSelected.push_back(otherEdge->pointSelected[i]);						
		if (!otherEdge->pointViewTwist.empty())
			pointViewTwist.push_back(otherEdge->pointViewTwist[i]);			
		pointAttrib.push_back(otherEdge->pointAttrib[i]);		
		whichRoadMesh.push_back(otherEdge->whichRoadMesh[i]);
	}

	filterThisEdge();

}

void RN_Edge::concatenatePoint(Vector newPoint) {

	suggestedPointSet.push_back(newPoint);
	pointSelected.push_back(false);
	if (!pointViewTwist.empty())
		pointViewTwist.push_back(0.0f);
	pointAttrib.push_back(0);
	whichRoadMesh.push_back(PIECE_1_INDEX);

}

void RN_Edge::reverse() {	        

	std::reverse(suggestedPointSet.begin(), suggestedPointSet.end());
	std::reverse(pointSelected.begin(), pointSelected.end());	
	std::reverse(pointViewTwist.begin(), pointViewTwist.end());	
	std::reverse(pointAttrib.begin(), pointAttrib.end());			
	std::reverse(whichRoadMesh.begin(), whichRoadMesh.end());		
	
}

RN_Edge *RN_Edge::getClone() {

	RN_Edge *newClone=new RN_Edge();

	newClone->suggestedPointSet=vector <Vector> (suggestedPointSet.size());
	newClone->pointSelected=vector <bool> (pointSelected.size());
	newClone->pointViewTwist=vector <float> (pointViewTwist.size());
	newClone->pointAttrib=vector <int> (pointAttrib.size());
	newClone->whichRoadMesh=vector <int> (whichRoadMesh.size());
	
	std::copy(suggestedPointSet.begin(),suggestedPointSet.end(),newClone->suggestedPointSet.begin());
	std::copy(pointSelected.begin(),pointSelected.end(),newClone->pointSelected.begin());
	std::copy(pointViewTwist.begin(),pointViewTwist.end(),newClone->pointViewTwist.begin());
	std::copy(pointAttrib.begin(),pointAttrib.end(),newClone->pointAttrib.begin());
	std::copy(whichRoadMesh.begin(),whichRoadMesh.end(),newClone->whichRoadMesh.begin());	

	//selection
	newClone->setSelected(selected);
	newClone->numPointsSelected=numPointsSelected;
	newClone->firstPointSelected=firstPointSelected;
	newClone->lastPointSelected=lastPointSelected;

	//spline
	if (catmullRomSpline!=NULL) {
		//newClone->catmullRomSpline=new CatmullRomSpline(newClone);	
		//newClone->catmullRomSpline->processMeshSegments(newClone->roadMesh,newClone->whichRoadMesh);
		newClone->useCatmullRomSpline(true);
	}	
	
	return newClone;

}

/*
 * This routine is used to create new edges, after a "delete edge" command.
 *		One extra complication is that if delete edge is used on a road that's a closed loop,
 *		then there should just be 1 road coming from that, not 2.
 */
vector <RN_Edge *> RN_Edge::getEdgesFromUnselected() {

	bool makingEdge=false;
	RN_Edge *eachNewEdge;	
	vector <RN_Edge *> newEdges;

	Vector firstPoint=suggestedPointSet[0];

	for (int i=0;i<suggestedPointSet.size();i++) {
		
		if (!makingEdge&&!pointSelected[i]) {			
			eachNewEdge=new RN_Edge();			
			makingEdge=true;

			eachNewEdge->suggestedPointSet.clear();
			eachNewEdge->pointAttrib.clear();
			eachNewEdge->pointViewTwist.clear();
			eachNewEdge->whichRoadMesh.clear();
			eachNewEdge->pointSelected.clear();
		}
		else if (makingEdge&&pointSelected[i]) {

			makingEdge=false;

			if (eachNewEdge->getNumPoints()>=2) {				
				if (catmullRomSpline!=NULL) {
					eachNewEdge->catmullRomSpline=new CatmullRomSpline(eachNewEdge);	
					eachNewEdge->catmullRomSpline->processMeshSegments(roadMesh,eachNewEdge->whichRoadMesh);
				}
				newEdges.push_back(eachNewEdge);
			}
		}

		if (makingEdge) {
			eachNewEdge->suggestedPointSet.push_back(suggestedPointSet[i]);
			eachNewEdge->pointAttrib.push_back(pointAttrib[i]);
			if (!pointViewTwist.empty())
				eachNewEdge->pointViewTwist.push_back(pointViewTwist[i]);
			eachNewEdge->whichRoadMesh.push_back(whichRoadMesh[i]);
			eachNewEdge->pointSelected.push_back(false);			
		}		

	}

	if (makingEdge) {
		makingEdge=false;

		if (eachNewEdge->getNumPoints()>=2) {			
			newEdges.push_back(eachNewEdge);
		}
	}
	
	//this handles the case that the edge being broken up was a closed loop
	bool connectedLoop=false;
	for (int i=0;i<newEdges.size();i++) {
		if (newEdges[i]->getPointAttrib(newEdges[i]->getNumPoints()-1,POINT_ATTRIB_CLOSED_LOOP)) {			
			for (int j=0;j<newEdges.size();j++) {
				if (i==j)
					continue;

				if (newEdges[j]->getPoint(0)==firstPoint) {	
					newEdges[i]->setPointAttrib(newEdges[i]->getNumPoints()-1,POINT_ATTRIB_CLOSED_LOOP, false);
					newEdges[i]->concatenateEdge(newEdges[j]);
					newEdges.erase(newEdges.begin()+j);
					if (j<i) 
						i--;
					connectedLoop=true;
					break;
				}
			}
		}
		if (connectedLoop)
			break;
	}	

	//get them all ready
	if (catmullRomSpline!=NULL) {
		for (int i=0;i<newEdges.size();i++) 
			eachNewEdge->useCatmullRomSpline(true);
	}

	return newEdges;

}

void RN_Edge::setHeightMap(HeightMap *h) {
	heightMap=h;
}

void RN_Edge::attachToGround(int startindex) {
		
	float distance;
	int curIndex;
	const float maxdistance=MIN_DIST_BETWEEN_POINTS*5.0;

	for (int i=0;i<2;i++) {

		if (i==0)
			curIndex=startindex;
		else if (i==1)
			curIndex=startindex+1;

		distance=0.0f;

		while (distance<maxdistance&&curIndex>=0&&curIndex<getNumPoints()) {

			heightMap->selectWithRay(suggestedPointSet[curIndex]+Vector(0.0f,100.0f,0.0f),Vector(0.0f,-1.0f,0.0f),suggestedPointSet[curIndex]);			
			setPointAttrib(curIndex,POINT_ATTRIB_BRIDGE, false);

			if (i==0) {
				if (curIndex>0)
					distance+=(getPoint(curIndex)-getPoint(curIndex-1)).GetLength();
				else
					distance=maxdistance;

				curIndex--;
			}
			else if (i==1) {
				if (curIndex<getNumPoints()-1)
					distance+=(getPoint(curIndex)-getPoint(curIndex+1)).GetLength();
				else
					distance=maxdistance;

				curIndex++;
			}

		};

	}	

}

void RN_Edge::addAlongRoad(int segment, float interp, float normalDist, int objectType, ObjectManager *objManager) {

	if (segment<0||segment>=suggestedPointSet.size()-1)
		return;

	Vector normalVec=catmullRomSpline->getRightNormal(segment, interp);
	Vector posVec=catmullRomSpline->interp(segment, interp);

	Vector rotateVec=catmullRomSpline->getTangent(segment, interp);
	float rotateAmt=atan2(-rotateVec.z,rotateVec.x);
	rotateAmt=rotateAmt*180.0f/M_PI+90.0f;
	if (normalDist<0.0f)
		rotateAmt+=180.0f;
	
	if (objectType==OBJ_TYPE_FLARE) {
		Vector yVec=catmullRomSpline->getYSpline()->yInterp(segment,interp);
		objManager->addObject(posVec+Vector(0.0f,yVec.z+0.65f*MIN_DIST_BETWEEN_POINTS,0.0f),rotateAmt,objectType,0.5f*MIN_DIST_BETWEEN_POINTS);
	}
	else if (objectType==OBJ_TYPE_PILLAR) {
		Vector yVec=catmullRomSpline->getYSpline()->yInterp(segment,interp);
		//objManager->plopDownObject(posVec+normalVec*normalDist, rotateAmt, objectType, suggestedPointSet[segment].y);
		objManager->plopDownObject(posVec+normalVec*normalDist, rotateAmt, objectType, yVec.z);
	}
	else
		objManager->plopDownObject(posVec+normalVec*normalDist, rotateAmt, objectType, MIN_DIST_BETWEEN_POINTS);


}

float RN_Edge::getCurvature(int segment) {

    if (curvatureVec.size()!=suggestedPointSet.size()) {
        clothoidSpline->getCurvatures(suggestedPointSet, curvatureVec);
    }

    return curvatureVec[segment];
}

vector <Vector> & RN_Edge::getMeshVertices(int segment) {

        return catmullRomSpline->getMeshVertices(segment);

}

vector <Vector> & RN_Edge::getMeshNormals(int segment) {

	return catmullRomSpline->getMeshNormals(segment);

}

vector <Vector2> & RN_Edge::getMeshUVs(int segment) {

	return catmullRomSpline->getMeshUVs(segment);

}

vector <tFace> & RN_Edge::getMeshFaceIndexes(int segment) {

	return catmullRomSpline->getMeshFaceIndexes(segment);

}

int RN_Edge::getWhichRoadMesh(int segment) {

	if (whichRoadMesh[segment]==PIECE_1_INDEX) 
		return 1;	
	else if (whichRoadMesh[segment]==PIECE_2_INDEX) 
		return 2;	
	else if (whichRoadMesh[segment]==PIECE_3_INDEX) 
		return 3;	
	else if (whichRoadMesh[segment]==PIECE_4_INDEX) 
		return 4;	
	else if (whichRoadMesh[segment]==PIECE_5_INDEX) 
		return 5;	
	else if (whichRoadMesh[segment]==PIECE_6_INDEX) 
		return 5;	
	else
		return -1; //indicates to skip

}

void RN_Edge::export_DAT(FILE *outFile) {

	/*
	outStr << "EDGEPOINTS " << suggestedPointSet.size() << "\n";
	for (int i=0;i<suggestedPointSet.size();i++) 
		outStr << suggestedPointSet[i].x << " " << suggestedPointSet[i].y << " " << suggestedPointSet[i].z << "\n";

	outStr << "EDGEPOINTATTRIBS " << pointAttrib.size() << "\n";
	for (int i=0;i<pointAttrib.size();i++)
		outStr << pointAttrib[i] << "\n";	
		*/

	fprintf(outFile,"EDGEPOINTS %i\n",suggestedPointSet.size());
	for (int i=0;i<suggestedPointSet.size();i++) 
		fprintf(outFile,"%f %f %f\n",suggestedPointSet[i].x,suggestedPointSet[i].y,suggestedPointSet[i].z);

	fprintf(outFile,"EDGEPOINTATTRIBS %i\n",pointAttrib.size());
	for (int i=0;i<pointAttrib.size();i++)
		fprintf(outFile,"%i\n",pointAttrib[i]);		

}
	
void RN_Edge::import_DAT(FILE *inf) {

	char buf[255];

	fgets(buf,255,inf);
	
	int numPoints;
	sscanf(buf,"EDGEPOINTS %i",&numPoints);

	suggestedPointSet.reserve(numPoints);
	pointSelected.reserve(numPoints);
	for (int i=0;i<numPoints;i++) {
		fgets(buf,255,inf);

		Vector eachPoint;
		sscanf(buf,"%f %f %f",&eachPoint.x,&eachPoint.y,&eachPoint.z);
		
		suggestedPointSet.push_back(eachPoint);
		pointSelected.push_back(false);
	}

	fgets(buf,255,inf);
	int numPointAttribs;
	sscanf(buf,"EDGEPOINTATTRIBS %i",&numPointAttribs);
	
	pointAttrib.reserve(numPointAttribs);
	for (int i=0;i<numPointAttribs;i++) {
		fgets(buf,255,inf);

		int eachAttrib;
		sscanf(buf,"%i",&eachAttrib);

		pointAttrib.push_back(eachAttrib);
	}

}

void RN_Edge::makeTwisty() {

	/*
	//now increase the curvatures
	ClothoidSpline *twistySpline=new ClothoidSpline(this,firstPointSelected,lastPointSelected,DEFAULT_CLOTHOID_PENALTY,1.5f);		

	//now update the points with new points sampled from clothoid spline
	for (int i=firstPointSelected;i<=lastPointSelected;i++) {	
		
		//set points to follow clothoid spline
		Vector oldPoint=suggestedPointSet[i];
		suggestedPointSet[i]=twistySpline->getPoint(i-firstPointSelected);

		heightMap->selectWithRay(Vector(suggestedPointSet[i].x,1000.0f,suggestedPointSet[i].z),
								Vector(0.0f,-1.0f,0.0f));
		if (getPointAttrib(i,POINT_ATTRIB_BRIDGE))
			oldPoint.y;
		else
			suggestedPointSet[i].y=heightMap->getSelectLocation().y;
	}
	updateStructure();	
	*/

}
	
void RN_Edge::makeStraight() {

	/*
	//now reduce the curvatures
	ClothoidSpline *twistySpline=new ClothoidSpline(this,firstPointSelected,lastPointSelected,DEFAULT_CLOTHOID_PENALTY,0.5f);
	
	//now update the points with new points sampled from clothoid spline
	for (int i=firstPointSelected;i<=lastPointSelected;i++) {	
		
		//set points to follow clothoid spline
		Vector oldPoint=suggestedPointSet[i];
		suggestedPointSet[i]=twistySpline->getPoint(i-firstPointSelected);

		heightMap->selectWithRay(Vector(suggestedPointSet[i].x,1000.0f,suggestedPointSet[i].z),
								Vector(0.0f,-1.0f,0.0f));
		if (getPointAttrib(i,POINT_ATTRIB_BRIDGE))
			oldPoint.y;
		else
			suggestedPointSet[i].y=heightMap->getSelectLocation().y;
	}
	updateStructure();	
	*/

}
