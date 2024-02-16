/*
 * CatmullRomSpline.h - header file for CatmullRomSpline class
 */

#ifndef _CATMULLROMSPLINE_H
#define _CATMULLROMSPLINE_H

#include <vector>

#include "TriMeshTextured.h"
#include "RN_Edge.h"

#define DEFAULT_DRAWSAMPLERATE 10
#define TANGENT_FINITEDIFFERENCE 0.01
#define TENSION 0.0

using namespace std;

class RN_Edge;

class CatmullRomSpline {

	public:

	CatmullRomSpline(RN_Edge *edge);
	CatmullRomSpline(vector <Vector> set);
	~CatmullRomSpline();

	void draw();
	void drawSegment(int segment);
	void drawTangent(int segment);

	Vector getTangent(int segment, double interpAmt);
	Vector getLeftNormal(int segment, double interpAmt);
	Vector getRightNormal(int segment, double interpAmt);	
	
	Vector interp(int segment, double interpAmt);		
	Vector yInterp(int segment, double interpAmt);		

	void processMeshSegments(vector <TriMeshTextured *> meshes, vector <int> &meshForSegment);
	void drawSegmentWithMesh(int segment);
	void drawSegmentWithHole(int segment, int holeMesh);

	void setTangent(int index, Vector newTangent);

	void getClone();
	CatmullRomSpline *getYSpline();

        vector <Vector> & getMeshVertices(int segment);
        vector <Vector> & getMeshNormals(int segment);
        vector <Vector2> & getMeshUVs(int segment);
        vector <tFace> & getMeshFaceIndexes(int segment);

	protected:

	void processMeshPoint(Vector meshPoint, Vector &warpEachVertex, Vector &warpEachNormal);

	RN_Edge *edgeToFit;	
	vector <Vector> tangentList;
	int drawSampleRate;

	vector <TriMeshTextured *> meshes;
	vector <int> meshForSegment;
        vector <vector <Vector> > warpVertices;
        vector <vector <Vector> > warpNormals;
	vector <unsigned int> meshSegmentDisplayLists;

	CatmullRomSpline *yComponentSpline;

	vector <Vector> pointSet;		

};

#endif 
