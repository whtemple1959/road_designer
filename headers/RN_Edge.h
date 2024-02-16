/*
* RN_Edge.h.  Header file for Road Network edge.  The edge connects exactly two vertices.
*/

#pragma once

#include <QtOpenGL>
#include <QtCore>

#include <algorithm>

#include "Vector.h"
#include "UI_Selectable.h"
#include "HeightMap.h"
#include "RN.h"
#include "ObjectManager.h"

#include "ClothoidSpline.h"
#include "CatmullRomSpline.h"

#include "TriMeshTextured.h"

///* MODIFIED FOR GEOMETRY MODELLING EXAMPLE
#define MIN_DIST_BETWEEN_POINTS 4.0 //default 4.0
//*/ #define MIN_DIST_BETWEEN_POINTS 0.5

#define POINT_ATTRIB_CROSS 0x1
#define POINT_ATTRIB_DONOTRENDER 0x2
#define POINT_ATTRIB_INTERSECT 0x4
#define POINT_ATTRIB_CLOSED_LOOP 0x8
#define POINT_ATTRIB_BRIDGE 0x10
#define POINT_ATTRIB_TUNNEL 0x20
#define POINT_ATTRIB_SPEED_SLOW 0x40
#define POINT_ATTRIB_SPEED_FAST 0x80

enum {
	PIECE_1_INDEX,
	PIECE_2_INDEX,
	PIECE_3_INDEX,
	PIECE_4_INDEX,
	PIECE_5_INDEX,
	PIECE_6_INDEX,
	PIECE_7_INDEX,
	NUM_ROADMESH_PIECES
};

using namespace std;

class RN;
class CatmullRomSpline;
class ClothoidSpline;
class ObjectManager;

class RN_Edge : public UI_Selectable {

	public:

	RN_Edge();
	~RN_Edge();

	void selectWithRay(Vector pos_xyz, Vector rayDir);
        void selectWithClosedCurve(vector <Vector> & pointSet);
        void selectWithClosedCurve2D(vector <Vector> & pointSet);

	int getNumPointsSelected();

        void setPointSet(const vector <Vector> & set);
	void setPoint(int index, Vector newPoint);
	Vector getPoint(int index);
	int getNumPoints();

	void insertPoint(int index, Vector pos, bool sel, int attrib, float twist, int whichMesh);
	void removePoint(int index);
	void printSizes();

	bool getPointAttrib(int segment, int attrib);
	int getPointAttribVal(int segment);
	void setPointAttrib(int segment, int attrib, bool val);

	void useClothoidSpline(HeightMap *heightMap);
	void useCatmullRomSpline(bool useCatmullRom);		

	void edgesCross(RN_Edge *otherEdge, int &mySegment, int &otherSegment, Vector &intersectVec);
	void concatenateEdge(RN_Edge *otherEdge);
	void concatenatePoint(Vector newPoint);
	void reverse();

	void draw();
	void drawTunnelHoles();
	void updateStructure();

	float getLength();

	Vector getFirstPointSelected();
	Vector getLastPointSelected();	
	bool getPointSelected(int i);

	int getFirstSegmentSelected();
	int getLastSegmentSelected();

	Vector getInterpPosition(int segment, float interp);
	Vector getInterpTangent(int segment, float interp);

	void setSelected(bool sel); //overriding inherited method
	void setPointSelected(int index, bool sel);
	void unselectAll();

	void replaceRangeWith(RN_Edge *newEdge, int start, int end);
	void replaceHeightOfSelected(vector <Vector> pointSet2D, vector <Vector> pointSet3D);

	void setViewTwists(vector <float> newViewTwists, Vector pointOfRotation, Vector axisOfRotation);
	vector <float> getViewTwists();
	Vector getPointOfRotation();
	Vector getAxisOfRotation();

	RN_Edge *getClone();
	vector <RN_Edge *> getEdgesFromUnselected();

	static void setHeightMap(HeightMap *h);
	void attachToGround(int startindex);

	void addAlongRoad(int segment, float interp, float normalDist, int objectType, ObjectManager *objManager);
	float getCurvature(int segment);

	void updateWhichRoadMesh();

        vector <Vector> & getMeshVertices(int segment);
        vector <Vector> & getMeshNormals(int segment);
        vector <Vector2> & getMeshUVs(int segment);
        vector <tFace> & getMeshFaceIndexes(int segment);
	int getWhichRoadMesh(int segment);

	void export_DAT(FILE *outFile);
	void import_DAT(FILE *inf);

	//for the "path" slice of the main menu
	void makeTwisty();
	void makeStraight();

	protected:	

	int numPointsSelected;
	int firstPointSelected;
	int lastPointSelected;
	
	void filterThisEdge();

	vector <Vector> suggestedPointSet;	
	vector <bool> pointSelected;	
	vector <int> pointAttrib;	
	vector <float> pointViewTwist;
	vector <int> whichRoadMesh;

	Vector pointOfRotation;
	Vector axisOfRotation;
	
	static vector <TriMeshTextured *> roadMesh;	
	static HeightMap *heightMap;		

	CatmullRomSpline *catmullRomSpline;		

	vector <float> curvatureVec;

	//clothoids
	ClothoidSpline *clothoidSpline;
	//static int exportClothoidFilename;
	//static float piecePenalty;
	//static float endpointWeight;

};
