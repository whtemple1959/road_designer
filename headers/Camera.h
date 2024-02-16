#ifndef _CAMERA_H
#define _CAMERA_H

#include <QtOpenGL>

#include "Vector.h"

#define VAL_PI 3.14159
#define RadToDeg(x) x*360.0/(2*VAL_PI)
#define DegToRad(x) x*(2*VAL_PI)/360.0

class Camera {

public:

    Camera();
    Camera(Vector trackPos, float spin, float tilt);

    Vector getPosition() const;
    float getSpin() const;
    float getTilt() const;
    float getTrackDist() const;
    Vector getTrackPosition() const;

    void setPosition(Vector pos);
    void setSpin(float spin);
    void setTilt(float tilt);
    void setTrackPosition(Vector position);
    void setTrackDist(float dist);

    void setCamera(const Camera & otherCamera);
    void followRollerCoaster(Vector waytolook);

    void interpBetweenCameras(const Camera & cam1, const Camera & cam2, float interp);
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
