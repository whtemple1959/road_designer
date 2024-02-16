/*
 * Header for class HeightMap - represents terrain in this program 
 */

#ifndef _HEIGHTMAP_H
#define _HEIGHTMAP_H

#include <windows.h>
#include <gl/glew.h>
#include <glut.h>
#include <IL/ilut.h>

#include "Vector.h"
#include "Collision.h"
#include "UI_Selectable.h"
#include "Camera.h"

class HeightMap : public UI_Selectable {

	public:

	HeightMap();	

	int getWidth();
	int getHeight();		
	Vector getDefaultCameraTrackPos();
	float getDefaultCameraTrackDist();

	void draw(Camera *mainCamera);
	float getHeightAt(float x, float z);

	//selectable interface
	void selectWithRay(Vector pos_xyz, Vector rayDir);
	vector <Vector> selectWithRays(Vector pos_xyz, vector <Vector> rays);
	void selectWithClosedCurve(vector <Vector> pointSet);

	Vector getSelectLocation();			

	//heightmap change stuff (for road/terrain interpenetration)
	float getMeshHeight(int x, int z);
	void setMeshHeight(int x, int z, float newHeight);

	private:

	Vector selectLocation;	
	
	unsigned int width;
	unsigned int height;	

};

#endif