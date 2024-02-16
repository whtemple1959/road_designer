#ifndef _TWISTLENS_H
#define _TWISTLENS_H

#include <QtOpenGL>
#include <GL/glu.h>

#include "Vector.h"
#include "RN.h"
#include "HeightMap.h"
#include "Collision.h"
#include "Camera.h"

#include "textureloader.h"

#define TWIST_ICON_RADIUS 15.0f
#define TWIST_START_ALPHA 30.0f
#define TWIST_END_ALPHA 75.0f

class TwistLens {

	public:

        TwistLens(QWidget * viewer);

        void draw(Camera & camera);
        void drawSkyPlane(Camera & camera);

	void mouseMotion(int x, int y);
	void mouseClick(int x, int y, bool down, RN *roadNetwork, HeightMap *heightMap);
        void mouseDragRelease(vector <Vector> lassoPickrays, RN *roadNetwork, HeightMap *heightMap, Camera & mainCamera);

	int getIconSelected();

        void drawTwistPlane(float offset, Camera & camera);

	bool isVisible();
	void setVisible(bool v);

	void setOnSelectedEdge(RN_Edge *selectedEdge);	

	void getShaderAttribs(float &twistVal, float &innerRad, float &outerRad, Vector &mainPoint,Vector &axisPoint,Vector &midPoint);

	private:
	
	float determineSkyPlaneOffset();
	Vector projectPointOnTwistPlane(Vector point, Vector pointOfRotation, Vector axisOfRotation);
	void drawIcon(float xtrans, float ytrans, int textureIndex);

        bool clickedIcon(int x, int y, Vector cent, float rad);

	Vector translateIconCentre();
	Vector outerRadiusIconCentre();
	Vector innerRadiusIconCentre();
	Vector twistIconCentre();
	Vector closeIconCentre();

        Vector m_lensCentre;
        float m_outerRadius;
        float m_innerRadius;

        float m_viewTwist; //(in degrees)
        Vector m_axisOfRotation;
        Vector m_pointOfRotation;

	//textures for widget
	GLuint texture_translate;
	GLuint texture_radius;
	GLuint texture_twist;
	GLuint texture_close;

	int iconSelected;

	bool visible;
	RN_Edge *selectedEdge;

	//matrices for projection
	GLdouble model_view[16];
	GLdouble projection[16];
	GLint viewport[4];

	//sphere attributes
	Vector sphereCentre;
	float sphereInnerRad;
	float sphereOuterRad;

        QWidget * m_pViewer;

};

#endif
