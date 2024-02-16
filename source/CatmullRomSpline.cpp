/*
 * CatmullRomSpline.cpp - body for CatmullRomSpline class
 */

#include "CatmullRomSpline.h"

CatmullRomSpline::CatmullRomSpline(RN_Edge *edge) {

	edgeToFit=edge;
	drawSampleRate=DEFAULT_DRAWSAMPLERATE;

	if (edgeToFit->getNumPoints()<2) {
		edgeToFit=NULL;
		return;
	}

	/*
	 * Calculate list of tangents
	 */
	float tension=TENSION;

	//First and last tangents will just be vector toward line		
	tangentList.push_back(edgeToFit->getPoint(1)-edgeToFit->getPoint(0));	
	
	for (int i=1;i<edgeToFit->getNumPoints()-1;i++)
		tangentList.push_back((edgeToFit->getPoint(i+1)-edgeToFit->getPoint(i-1))*0.5*(1.0-tension));

	tangentList.push_back(edgeToFit->getPoint(edgeToFit->getNumPoints()-1)-
													edgeToFit->getPoint(edgeToFit->getNumPoints()-2));	

	//tangents lie on XZ plane ONLY
	for (int i=0;i<tangentList.size();i++) 
		tangentList[i].y=0.0;	

	yComponentSpline=NULL;

}

//This constructor is called for the Y-component spline
CatmullRomSpline::CatmullRomSpline(vector <Vector> set) {

	pointSet=set;

	/*
	 * Calculate list of tangents
	 */
	float tension=TENSION;

	//First and last tangents will just be vector toward line		
	tangentList.push_back(pointSet[1]-pointSet[0]);	
	
	for (int i=1;i<pointSet.size()-1;i++)
		tangentList.push_back((pointSet[i+1]-pointSet[i-1])*0.5*(1.0-tension));

	tangentList.push_back(pointSet[pointSet.size()-1]-pointSet[pointSet.size()-2]);
	
	//tangents lie on XZ plane ONLY
	for (int i=0;i<tangentList.size();i++) {
		tangentList[i].y=0.0;	
		//if (tangentList[i].z<0.0f) //prevent downward tangents that would dip into terrain
		//	tangentList[i].z=0.0f;
	}

	yComponentSpline=NULL;

}

CatmullRomSpline::~CatmullRomSpline() {

	tangentList.clear();

	for (int i=0;i<warpVertices.size();i++) {
		warpVertices[i].clear();
		warpNormals[i].clear();
	}	
	warpVertices.clear();
	warpNormals.clear();

	if (yComponentSpline!=NULL)
		delete yComponentSpline;
	
}

Vector CatmullRomSpline::yInterp(int segment, double interpAmt) {

	Vector startPoint=pointSet[segment];
	startPoint.y=0.0;
	Vector endPoint=pointSet[segment+1];
	endPoint.y=0.0;	

	Vector returnPoint=startPoint*(2.0*pow(interpAmt,3)-3.0*pow(interpAmt,2)+1.0)+
						tangentList[segment]*(pow(interpAmt,3)-2.0*pow(interpAmt,2)+interpAmt)+
						endPoint*(-2.0*pow(interpAmt,3)+3.0*pow(interpAmt,2))+
						tangentList[segment+1]*(pow(interpAmt,3)-pow(interpAmt,2));

	return returnPoint;

}

Vector CatmullRomSpline::interp(int segment, double interpAmt) {	

	Vector startPoint=edgeToFit->getPoint(segment);
	startPoint.y=0.0;
	Vector endPoint=edgeToFit->getPoint(segment+1);
	endPoint.y=0.0;
		
	/*
	printf("numpoints: %i about to show you points %i, %i\n",edgeToFit->getNumPoints(),segment,segment+1);
	printf("startpoint: %f,%f,%f\n",startPoint.x,startPoint.y,startPoint.z);
	printf("endpoint: %f,%f,%f\n",endPoint.x,endPoint.y,endPoint.z);
	printf("numtangents: %i about to show you tangents %i, %i\n",tangentList.size(),segment,segment+1);
	printf("tangent1: %f,%f,%f\n",tangentList[segment].x,tangentList[segment].y,tangentList[segment].z);
	printf("tangent2: %f,%f,%f\n",tangentList[segment+1].x,tangentList[segment+1].y,tangentList[segment+1].z);
	*/

	Vector returnPoint=startPoint*(2.0*pow(interpAmt,3)-3.0*pow(interpAmt,2)+1.0)+
						tangentList[segment]*(pow(interpAmt,3)-2.0*pow(interpAmt,2)+interpAmt)+
						endPoint*(-2.0*pow(interpAmt,3)+3.0*pow(interpAmt,2))+
						tangentList[segment+1]*(pow(interpAmt,3)-pow(interpAmt,2));

	return returnPoint;


}

void CatmullRomSpline::draw() {

	if (edgeToFit==NULL)
		return;	

	glBegin(GL_LINE_STRIP);

	for (int i=0;i<edgeToFit->getNumPoints()-1;i++) {					

			for (int j=0;j<=drawSampleRate;j++) {
				Vector samplePoint=interp(i,(float)j/(float)drawSampleRate);
				glVertex3f(samplePoint.x,
							(edgeToFit->getPoint(i).y*(1.0-(float)j/(float)drawSampleRate))+
								(edgeToFit->getPoint(i+1).y*((float)j/(float)drawSampleRate)),
							samplePoint.z);	
			}

	}
	glEnd();		

}

void CatmullRomSpline::drawSegment(int segment) {

	if (segment<0||segment>=edgeToFit->getNumPoints()-1) 
		return;

	
	glPushMatrix();
		glTranslatef(edgeToFit->getPoint(segment).x,
						edgeToFit->getPoint(segment).y,
						edgeToFit->getPoint(segment).z);
                gluSphere(gluNewQuadric(), 1.0, 5, 5);
	glPopMatrix();

	glBegin(GL_LINE_STRIP);
	for (int j=0;j<=drawSampleRate;j++) {

		Vector samplePoint=interp(segment,(float)j/(float)drawSampleRate);
		glVertex3f(samplePoint.x,
					(edgeToFit->getPoint(segment).y*(1.0-(float)j/(float)drawSampleRate))+
								(edgeToFit->getPoint(segment+1).y*((float)j/(float)drawSampleRate)),
					samplePoint.z);

	}
	glEnd();	

}

void CatmullRomSpline::drawTangent(int segment) {

	//tangent
	glColor3f(0,0,1);
	glBegin(GL_LINES);	
	glVertex3f(edgeToFit->getPoint(segment).x,
						edgeToFit->getPoint(segment).y,
						edgeToFit->getPoint(segment).z);
	glVertex3f(edgeToFit->getPoint(segment).x+tangentList[segment].x,
						edgeToFit->getPoint(segment).y+tangentList[segment].y,
						edgeToFit->getPoint(segment).z+tangentList[segment].z);	
	glEnd();

}

Vector CatmullRomSpline::getTangent(int segment, double interpAmt) {

	/*
	if (interpAmt==0.0f)
		return tangentList[segment];
	else if (interpAmt==1.0f)
		return tangentList[segment+1];
	else {
		float magnitude=tangentList[segment].GetLength()*(1.0-interpAmt)+
						tangentList[segment+1].GetLength()*interpAmt;
		Vector returnVec=interp(segment,interpAmt+TANGENT_FINITEDIFFERENCE)-
						interp(segment,interpAmt-TANGENT_FINITEDIFFERENCE);
		returnVec.SetLength(magnitude);

		return returnVec;
	}	
	*/
	Vector returnVec=interp(segment,interpAmt+TANGENT_FINITEDIFFERENCE)-
						interp(segment,interpAmt-TANGENT_FINITEDIFFERENCE);		

	return returnVec;

}

Vector CatmullRomSpline::getLeftNormal(int segment, double interpAmt) {
	
	Vector tangentVec=getTangent(segment,interpAmt);
	Vector normalVec=Vector(tangentVec.z,0,-tangentVec.x);

	tangentVec.CrossProduct(tangentVec,normalVec);
	normalVec.Normal();
	if (tangentVec.y>0) {
		return normalVec;
	}
	else {
		return normalVec*(-1.0);
	}

}

Vector CatmullRomSpline::getRightNormal(int segment, double interpAmt) {
	return getLeftNormal(segment,interpAmt)*(-1.0);
}

void CatmullRomSpline::processMeshSegments(vector <TriMeshTextured *> meshes, vector <int> & meshForSegment) {

	//save the meshes (need to index the faces later when drawing)
	this->meshes=meshes;
	this->meshForSegment=meshForSegment;

	warpVertices.clear();
	warpNormals.clear();
	meshSegmentDisplayLists.clear();

        warpVertices=vector<vector <Vector> >(edgeToFit->getNumPoints()-1);
        warpNormals=vector<vector <Vector> >(edgeToFit->getNumPoints()-1);

	//vector <float> viewTwists=edgeToFit->getViewTwists();
	Vector pointOfRotation=edgeToFit->getPointOfRotation();
	Vector axisOfRotation=edgeToFit->getAxisOfRotation();

	//genreate y-axis spline	
	if (yComponentSpline==NULL) {
		vector <Vector> yPointSet;
		for (int i=0;i<edgeToFit->getNumPoints();i++) {
			yPointSet.push_back(Vector(MIN_DIST_BETWEEN_POINTS*i,0.0f,edgeToFit->getPoint(i).y));			
		}		
		yComponentSpline=new CatmullRomSpline(yPointSet);
	}
	

	//warp object to follow each spline piece
	for (int i=0;i<edgeToFit->getNumPoints()-1;i++) { //for i components (points), you have i-1 segments
		
		meshSegmentDisplayLists.push_back(0);

		//this segment may be skipped
                if (meshForSegment[i] < 0)  {
                    continue;
                }

		warpVertices[i]=meshes[meshForSegment[i]]->getVerts(0);
		warpNormals[i]=meshes[meshForSegment[i]]->getNormals(0);

		for (int j=0;j<warpVertices[i].size();j++) {
			Vector eachVertex=warpVertices[i][j];
			Vector rightNormal=getRightNormal(i,eachVertex.z);	//param: segment, arc-length distance
			Vector upNormal=Vector(0.0f,1.0f,0.0f);		
			float yvalue=yComponentSpline->yInterp(i,eachVertex.z).z;

			warpVertices[i][j]=interp(i,eachVertex.z)+
				rightNormal*eachVertex.x*MIN_DIST_BETWEEN_POINTS+
				upNormal*eachVertex.y*MIN_DIST_BETWEEN_POINTS+
				upNormal*yvalue;
		}

	}	

}

/* THIS FUNCTION SHOULD ONLY BE CALLED WHEN segment>=0 ! */
void CatmullRomSpline::drawSegmentWithMesh(int segment) {		

	if (edgeToFit==NULL||edgeToFit->getNumPoints()<2)
		return;			

	if (meshSegmentDisplayLists[segment]>0)
		glCallList(meshSegmentDisplayLists[segment]);
	else {

		vector <tFace> meshFaces=meshes[meshForSegment[segment]]->getFaces(0);
		vector <Vector> meshVerts=meshes[meshForSegment[segment]]->getVerts(0);
		vector <Vector> meshNorms=meshes[meshForSegment[segment]]->getNormals(0);
		vector <Vector2> meshTexVerts=meshes[meshForSegment[segment]]->getTexVerts(0);	

		meshSegmentDisplayLists[segment]=glGenLists(1);
		glNewList(meshSegmentDisplayLists[segment],GL_COMPILE_AND_EXECUTE);

		glColor3f(1,1,1);
		glBindTexture(GL_TEXTURE_2D, meshes[meshForSegment[segment]]->getTextureID(0));

		glBegin(GL_TRIANGLES);	

		//_RMSInfo("Drawing mesh segment %i\n",segment);
		for (int i=0;i<meshFaces.size();i++) {

			for (int j=0;j<3;j++) {
									

				glTexCoord2f(meshTexVerts[meshFaces[i].coordIndex[j]-1].x,
					meshTexVerts[meshFaces[i].coordIndex[j]-1].y);
				glNormal3f(warpNormals[segment][meshFaces[i].normalIndex[j]-1].x,
							warpNormals[segment][meshFaces[i].normalIndex[j]-1].y,
							warpNormals[segment][meshFaces[i].normalIndex[j]-1].z);
				glVertex3f(warpVertices[segment][meshFaces[i].vertIndex[j]-1].x,
					warpVertices[segment][meshFaces[i].vertIndex[j]-1].y,
					warpVertices[segment][meshFaces[i].vertIndex[j]-1].z);	
		
				/*
				glTexCoord2f(meshes[meshForSegment[segment]]->getTexturePoint(i,j).x,
							meshes[meshForSegment[segment]]->getTexturePoint(i,j).y);
				glNormal3f(warpNormals[segment][i*3+j].x,
							warpNormals[segment][i*3+j].y,
							warpNormals[segment][i*3+j].z);
				glVertex3f(warpVertices[segment][(meshes[meshForSegment[segment]]->faces[i][j])].x,
					warpVertices[segment][(meshes[meshForSegment[segment]]->faces[i][j])].y,
					warpVertices[segment][(meshes[meshForSegment[segment]]->faces[i][j])].z);	
					*/
			}
			
		}	
		glEnd();

		glEndList();

	}

}

void CatmullRomSpline::drawSegmentWithHole(int segment, int holeMesh) {

	TriMeshTextured *eachMesh=meshes[holeMesh];

	vector <Vector> warpHoleMesh=eachMesh->getVerts(0);	

	for (int j=0;j<warpHoleMesh.size();j++) {
		//warp XZ components to conform to spline
		//Z axis of mesh defines arc-length axis

		Vector eachVertex=warpHoleMesh[j];

		Vector rightNormal=getRightNormal(segment,eachVertex.z);	//param: segment, arc-length distance
		Vector upNormal=Vector(0.0f,1.0f,0.0f);				
		
		float yvalue=yComponentSpline->yInterp(segment,eachVertex.z).z;

		Vector warpEachVertex=interp(segment,eachVertex.z)+rightNormal*eachVertex.x*MIN_DIST_BETWEEN_POINTS+upNormal*eachVertex.y*MIN_DIST_BETWEEN_POINTS+upNormal*yvalue;

		//twist!
		if (!edgeToFit->getViewTwists().empty()&&segment<edgeToFit->getViewTwists().size()-1) {	
			warpEachVertex=warpEachVertex-edgeToFit->getPointOfRotation();
			warpEachVertex=warpEachVertex.GetRotatedAxis(edgeToFit->getViewTwists()[segment]*(1.0f-eachVertex.z)+
												edgeToFit->getViewTwists()[segment+1]*(eachVertex.z),
												edgeToFit->getAxisOfRotation());
			warpEachVertex=warpEachVertex+edgeToFit->getPointOfRotation();
		}

		warpHoleMesh[j]=warpEachVertex;

	}	

	vector <tFace> meshFaces=meshes[holeMesh]->getFaces(0);

	glBegin(GL_TRIANGLES);	
	for (int i=0;i<meshFaces.size();i++) {

		for (int j=0;j<3;j++) {	
			
			glVertex3f(warpHoleMesh[meshFaces[i].vertIndex[j]].x,
						warpHoleMesh[meshFaces[i].vertIndex[j]].y,
						warpHoleMesh[meshFaces[i].vertIndex[j]].z);			
		}
		
	}	
	glEnd();

}

void CatmullRomSpline::setTangent(int index, Vector newTangent) {	
	tangentList[index]=newTangent;
}

CatmullRomSpline *CatmullRomSpline::getYSpline() {

	return yComponentSpline;

}

vector <Vector> & CatmullRomSpline::getMeshVertices(int segment) {

	/*
	vector <Vector> meshVertices;
	
	for (int j=0;j<warpVertices[segment].size();j+=3) {
		meshVertices.push_back(Vector(warpVertices[segment][j],warpVertices[segment][j+1],warpVertices[segment][j+2]));
	}	

	return meshVertices;
	*/

	return warpVertices[segment];

}

vector <Vector> & CatmullRomSpline::getMeshNormals(int segment) {

	vector <Vector> meshNormals;
	
	for (int j=0;j<warpNormals[segment].size();j++) {
		meshNormals.push_back(warpNormals[segment][j]);
	}	

	return meshNormals;

}

vector <Vector2> & CatmullRomSpline::getMeshUVs(int segment) {

	/*
	vector <Vector> meshUVs;

	//printf("meshforsegment: %i segment: %i numsegments: %i\n",meshForSegment[segment],segment, edgeToFit->getNumPoints());	
	for (int i=0;i<meshes[meshForSegment[segment]]->faces.size();i++) {
		for (int j=0;j<3;j++) {						
			meshUVs.push_back(Vector(meshes[meshForSegment[segment]]->getTexturePoint(i,j).x,
						meshes[meshForSegment[segment]]->getTexturePoint(i,j).y,
						0.0f));			
		}
	}

	return meshUVs;
	*/

	return meshes[meshForSegment[segment]]->getTexVerts(0);

}

vector <tFace> & CatmullRomSpline::getMeshFaceIndexes(int segment) {

	/*
	vector <int> meshFaceIndexes;
	
	for (int j=0;j<meshes[meshForSegment[segment]]->faces.size();j++) {

		for (int k=0;k<3;k++) 				
			meshFaceIndexes.push_back(meshes[meshForSegment[segment]]->faces[j][k]);			

	}	

	return meshFaceIndexes;
	*/

	return meshes[meshForSegment[segment]]->getFaces(0);

}
