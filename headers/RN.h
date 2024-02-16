#ifndef _RN_H
#define _RN_H

#include <vector>
#include <fstream>

#include "RN_Edge.h"
#include "RN_Intersection.h"
#include "Collision.h"
#include "ObjectManager.h"

using namespace std;

class RN_Edge;
class RN_Intersection;
class ObjectManager;

class RN: public UI_Selectable {

	public:

	RN(HeightMap *heightMap);
	~RN();	

	//ui selectable interface
	void selectWithRay(Vector pos_xyz, Vector rayDir);
        void selectWithClosedCurve(vector <Vector> & pointSet);
        void selectWithClosedCurve2D(vector <Vector> & pointSet);

	//all operations for modifying road network go here
	void addRoad(vector <Vector> lassoPoints3D, HeightMap *heightMap);
	void unSelectAll();

	int addEdge(RN_Edge *newEdge);	

	RN_Edge *getEdge(int edgeIndex);
	unsigned int getNumVertexes();
	unsigned int getNumEdges();

	unsigned int getNumIntersects();
	RN_Intersection *getIntersect(int index);

	void draw();
	void drawObjects();	
	void drawTunnelHoles();
	
	bool doOversketch(RN_Edge *newEdge);
	void doIntersections();
	bool doOverUnders();	
	bool doJoins();
	void doIntersectionUpdates();
	void doWaterBridges(RN_Edge *newEdge, HeightMap *heightMap);	
	void doPrepareTerrain(RN_Edge *newEdge, HeightMap *heightMap);
	void doDepthRelations();
	void doObjects();
	void doDeleteRoads();

	void doEdgeReversal(RN_Edge *edge);

	vector <RN_Edge *> getSelectedEdges();
	RN_Edge *getMostSelectedEdge(); //returns edge with max # of vertices selected, NULL otherwise
	vector <RN_Intersection *> getSelectedIntersects();	

	RN *getCloneNetwork(HeightMap *heightMap);	

	void setHeightMap(HeightMap *heightMap);	
	void updateClearRange();

	void addFoliage(vector <Vector> lassoPoints3D, float foliageDensity);
	void changeIntersect(char *buttonText, HeightMap *heightMap);
	void changeTiming(char *buttonText);

	//special case
	RN_Edge *getPlayModeLongEdge();

	ObjectManager *getObjectManager();

	void export_COLLADA(char *exportFilename, HeightMap *heightMap=NULL);
	void export_DAT(char *outfilename);
	void import_DAT(char *infilename, HeightMap *heightMap);

	protected:	

	void addIntersect(RN_Intersection *intersect);

	//what the road network consists of
	vector <RN_Edge *> edgeList;
	vector <RN_Intersection *> intersectList;

	//elements of the road network currently selected
	vector <int> selectedEdgeList;		
	vector <int> selectedIntersectList;	

	ObjectManager *objManager;

};

#endif
