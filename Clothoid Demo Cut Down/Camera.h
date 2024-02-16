#ifndef _CAMERA_H
#define _CAMERA_H

#include <windows.h>
#include <gl/glew.h>
#include <glut.h>
#include "Vector.h"

#define VAL_PI 3.14159
#define RadToDeg(x) x*360.0/(2*VAL_PI)
#define DegToRad(x) x*(2*VAL_PI)/360.0

class Camera {

	public:

	Camera();
	Camera(Vector trackPos, float spin, float tilt);

	Vector getPosition();
	float getSpin();
	float getTilt();

	void setPosition(Vector pos);
	void setSpin(float spin);
	void setTilt(float tilt);

	void setCamera(Camera *otherCamera);	
	void followRollerCoaster(Vector waytolook);

	void interpBetweenCameras(Camera *cam1, Camera *cam2, float interp);

	Vector getTrackPosition();
	float getTrackDist();
	void setTrackPosition(Vector position);
	void setTrackDist(float dist);
	void updateTrackPosition();
	void rotateLookAtTrackPosition();

	private:

	Vector pos_xyz;
	float pos_spin;
	float pos_tilt;	

	Vector trackPosition;
	float trackDistance;

};

#endif
