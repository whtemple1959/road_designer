/*
 * Header for class HeightMap - represents terrain in this program 
 */

#ifndef _HEIGHTMAP_H
#define _HEIGHTMAP_H

#include <QtOpenGL>
#include <GL/glu.h>

//#include "GLutils/glsl.h"

#include "TriMeshTextured.h"

#include "Vector.h"
#include "Collision.h"
#include "UI_Selectable.h"
#include "Camera.h"

#define DEFAULT_GRIDSIZE 128

class HeightMap {

	public:

	HeightMap();	

        void loadFromImages(QString heightFile, QString texFile);
	void generateGrid(int w, int h);

	int getWidth();
	int getHeight();		
	Vector getDefaultCameraTrackPos();
	float getDefaultCameraTrackDist();

	void Draw();
	void drawWater();
	float getWaterHeight();
	float getHeightAt(float x, float z);

	//selectable interface
	bool selectWithRay(Vector pos_xyz, Vector rayDir, Vector &collidePoint);
	vector <Vector> selectWithRays(Vector pos_xyz, vector <Vector> rays);
	vector <Vector> selectWithRaysUseDepthBuffer(Vector pos_xyz, vector <Vector> points2D);
	void selectWithClosedCurve(vector <Vector> pointSet);

	void setViewTwists(int bw, int bh, vector <float> &twists, Vector pointOfRotation, Vector axisOfRotation);
        void setWaterViewTwists(vector <vector <float> > &twists);

	float getWidthHeightMax();

	//heightmap change stuff (for road/terrain interpenetration)
	float getMeshHeight(int x, int z);
	void setMeshHeight(int x, int z, float newHeight);

	//COLLADA EXPORT STUFF
        //vector <TiXmlElement *> export_COLLADA_geometry();
	float getMinimumAltitude();

	private:

	TriMeshTextured *heightMapMesh;	
	TriMeshTextured *waterMesh;

	//view twist stuff
        vector <vector <vector <float> > > viewTwists;
        vector <vector <float> > waterTwists;
	Vector pointOfRotation;
	Vector axisOfRotation;

	//for texture-mapped heightmaps
	int width;
	int height;
	unsigned int textureIndex;
	unsigned int detailTextureIndex;	
	float waterHeight;
        vector <vector <bool> > waterVisible;
	char *heightmapTextureFilename;

	float minimumAltitude;

};

#endif
