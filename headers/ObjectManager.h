#ifndef _OBJECT_MANAGER_H
#define _OBJECT_MANAGER_H

#include <QtOpenGL>

#include <vector>

#include "RN.h"
#include "RN_Edge.h"
#include "TriMeshTextured.h"
#include "Vector.h"

enum {
	OBJ_TYPE_STOPSIGN,
	OBJ_TYPE_LAMPPOST,
	OBJ_TYPE_TREE1,
	OBJ_TYPE_TREE2,
	OBJ_TYPE_GRASS,
	OBJ_TYPE_PLANT1,
	OBJ_TYPE_PLANT2,
	OBJ_TYPE_PILLAR,
	OBJ_TYPE_WARNING_RIGHT,
	OBJ_TYPE_WARNING_LEFT,
	OBJ_TYPE_WARNING_SCURVE,
	OBJ_TYPE_WARNING_INTERSECT,
	OBJ_TYPE_WARNING_STEEP,
	OBJ_TYPE_WARNING_BUMP,
	OBJ_TYPE_WARNING_NARROWBRIDGE,
	OBJ_TYPE_WARNING_BULL,
	OBJ_TYPE_WARNING_STOPAHEAD,
	OBJ_TYPE_WARNING_TRAFFICLIGHTS,
	OBJ_TYPE_WARNING_DIP,
	OBJ_TYPE_FLARE,
	OBJ_TYPE_WARNING_SOLIDOBJECT,
	OBJ_TYPE_WARNING_SPEED_SLOW,
	OBJ_TYPE_WARNING_SPEED_AVG,
	OBJ_TYPE_WARNING_SPEED_FAST

};

enum {
	OBJ_MESH_STOPSIGN,
	OBJ_MESH_LAMPPOST,
	OBJ_MESH_TREE1_TRUNK,
	OBJ_MESH_TREE1_BRANCHES,
	OBJ_MESH_TREE2_TRUNK,
	OBJ_MESH_TREE2_BRANCHES,
	OBJ_MESH_GRASS,
	OBJ_MESH_PLANT1,
	OBJ_MESH_PLANT2_STEM,
	OBJ_MESH_PLANT2_LEAF,
	OBJ_MESH_PILLAR,
	OBJ_MESH_WARNING_RIGHT,
	OBJ_MESH_WARNING_LEFT,
	OBJ_MESH_WARNING_SCURVE,
	OBJ_MESH_WARNING_INTERSECT,
	OBJ_MESH_WARNING_STEEP,
	OBJ_MESH_WARNING_BUMP,
	OBJ_MESH_WARNING_NARROWBRIDGE,
	OBJ_MESH_WARNING_BULL,
	OBJ_MESH_WARNING_STOPAHEAD,
	OBJ_MESH_WARNING_TRAFFICLIGHTS,
	OBJ_MESH_WARNING_DIP,
	OBJ_MESH_FLARE,
	OBJ_MESH_WARNING_SOLIDOBJECT,
	OBJ_MESH_WARNING_SPEED_SLOW,
	OBJ_MESH_WARNING_SPEED_AVG,
	OBJ_MESH_WARNING_SPEED_FAST
};

using namespace std;

class RN;
class RN_Edge;

class ObjectManager {

	public:

	ObjectManager(RN *roadNetwork, HeightMap *heightMap);
	~ObjectManager();

	void addObject(Vector pos, float orient, int t, float s);
	void plopDownObject(Vector pos, float orient, int t, float s);
	int getNumObjects();
	void drawObjects();
	void drawObjectsUnderwater();
	ObjectManager *getClone(RN *newNetwork);	

	void addFoliage(vector <Vector> lassoPoints3D, float foliageDensity);
	void updateClearRange();
	void removePillars();
	
	void remove(int index);
	void removeType(int theType);
	void removeAll();

	Vector getObjectPos(int index);

	void export_DAT(FILE *outFile);
	void import_DAT(FILE *inf);

	protected:	

	float triangleArea(Vector v1, Vector v2, Vector v3);

	vector <Vector> position;
	vector <float> orientation;
	vector <float> scale;
	vector <int> type;

	static vector <TriMeshTextured *> meshes;
	RN *roadNetwork;
	HeightMap *heightMap;

        vector <vector <char> > clearRange;

};

#endif
