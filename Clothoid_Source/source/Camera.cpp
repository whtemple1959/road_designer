#include "Camera.h"

Camera::Camera() {

	pos_xyz=Vector(0,0,0);
	pos_spin=0.0;
	pos_tilt=0.0;

}

Camera::Camera(Vector trackPos, float spin, float tilt) {
	trackPosition=trackPos;
	trackDistance=64.0f;
	pos_spin=spin;
	pos_tilt=tilt;
}

Vector Camera::getPosition() {
	return pos_xyz;
}

float Camera::getSpin() {
	return pos_spin;
}

float Camera::getTilt() {
	return pos_tilt;
}

void Camera::setPosition(Vector pos) {
	pos_xyz=pos;
}

void Camera::setSpin(float spin) {
	pos_spin=spin;

	//mod spin
	if (pos_spin>360.0) pos_spin+=-360.0f;
	if (pos_spin<0.0) pos_spin+=360.0f;
	
}

void Camera::setTilt(float tilt) {
	pos_tilt=tilt;

	//clamp tilt

	if (pos_tilt<-45.0f) pos_tilt=-45.0f;
	if (pos_tilt>90.0f) pos_tilt=90.0f;	
}

void Camera::setCamera(Camera *otherCamera) {
	if (otherCamera==NULL) return;

	pos_xyz=otherCamera->getPosition();
	pos_spin=otherCamera->getSpin();
	pos_tilt=otherCamera->getTilt();
	trackPosition=otherCamera->getTrackPosition();
	trackDistance=otherCamera->getTrackDist();
}

Vector Camera::getTrackPosition() {
	return trackPosition;
}

float Camera::getTrackDist() {
	return trackDistance;
}

void Camera::setTrackPosition(Vector position) {
	trackPosition=position;
}

/*
 * Input:
 * pos_spin, pos_tilt, trackPosition, trackDistance
 *
 * Output:
 * pos_xyz
 */
void Camera::updateTrackPosition() {
 
    //this will set pos_spin and pos_tilt to point toward the desired
    //position from this one
    
	/*
    Vector pathVector=trackPosition-pos_xyz;
    pathVector.Normal();
    
    if (pathVector.z>=0)
        pos_spin=-RadToDeg(atanf(pathVector.x/pathVector.z))+180.0;
    else
        pos_spin=-RadToDeg(atanf(pathVector.x/pathVector.z));
        	
	pos_tilt=-RadToDeg(asinf(pathVector.y));   
	*/

	//spin affects xz plane
	//tilt affects ySpinVec plane
	Vector theSpinVec=Vector(cosf(DegToRad(pos_spin)),0.0f,sin(DegToRad(pos_spin)));
	theSpinVec.SetLength(trackDistance*cos(DegToRad(pos_tilt))); 

	Vector theTiltVec=Vector(0.0f,1.0f,0.0f);
	theTiltVec.SetLength(trackDistance*sin(DegToRad(pos_tilt)));

	Vector cameraMoveVec=theSpinVec+theTiltVec;
	cameraMoveVec.SetLength(trackDistance);	

	pos_xyz=trackPosition+cameraMoveVec;	
    
}

void Camera::setTrackDist(float dist) {
	trackDistance=dist;
	if (trackDistance<1.0f)
		trackDistance=1.0f;
}

void Camera::followRollerCoaster(Vector waytolook) {

	//this will set pos_spin and pos_tilt to point toward the desired
    //position from this one
    
    Vector spinPathVector=Vector(waytolook.x,0,waytolook.z)-Vector(pos_xyz.x,0,pos_xyz.z);
    spinPathVector.Normal();
    
    if (spinPathVector.z>=0)
        pos_spin=-RadToDeg(atanf(spinPathVector.x/spinPathVector.z))+180.0;
    else
        pos_spin=-RadToDeg(atanf(spinPathVector.x/spinPathVector.z));
        
	pos_tilt=-RadToDeg(asinf(waytolook.y-pos_xyz.y));  	

}

void Camera::interpBetweenCameras(Camera *cam1, Camera *cam2, float interp) {

	if (interp<0.0) interp=0.0;
	if (interp>1.0) interp=1.0;
	
	pos_tilt=cam1->getTilt()*(1-interp)+cam2->getTilt()*interp;

	//account for going between 360-0 degrees for spin
	float spin1=cam1->getSpin();
	float spin2=cam2->getSpin();
	
	if (spin2-spin1>180.0f)
		spin1+=360.0f;
	else if (spin1-spin2>180.0f)
		spin2+=360.0f;

	pos_spin=spin1*(1-interp)+spin2*interp;	

	trackDistance=cam1->getTrackDist()*(1.0f-interp)+cam2->getTrackDist()*interp;
	trackPosition=cam1->getTrackPosition()*(1.0f-interp)+cam2->getTrackPosition()*interp;

}

void Camera::rotateLookAtTrackPosition() {

	gluLookAt(pos_xyz.x,pos_xyz.y,pos_xyz.z,trackPosition.x,trackPosition.y,trackPosition.z,0.0f,1.0f,0.0f);

}