#ifndef _BREAKOUT_H
#define _BREAKOUT_H

#include <QtOpenGL>

#include <vector>

#include "Camera.h"
#include "Collision.h"
#include "RN.h"
#include "Viewer.h"

#include "textureloader.h"

#define ICON_RADIUS 15.0f

using namespace std;

class Viewer;

class BreakoutView {

public:

    BreakoutView(Viewer * v);
    bool isVisible();
    void setVisible(bool v);
    void updateInterpCamera(float timeDiffms);
    void setupView();
    void tearDownView();
    Camera & getCamera();
    bool isCameraMoving();
    Vector *rayIntoView(Vector lastpickray);
    void newBreakoutView(vector <Vector> lassoPoints2D, RN *roadNetwork, Camera & curCamera);

    void drawBreakoutStencil();
    void drawIcon(float xtrans, float ytrans, GLuint textureIndex);

    void mouseClick(int x, int y, bool down);
    void mouseMotion(int x, int y);
    int getIconSelected();
    Vector getCentre();
    RN_Edge *getSelectedEdge();

private:

    void setOrthogonal();

    Camera camera;
    Camera interpCamera;
    float interpVal;

    bool breakoutEnabled;
    vector <Vector> breakoutPoints2D;
    Vector breakoutPointsCentroid;
    Vector breakoutViewCentre;
    int numBreakoutPoints;
    Vector breakoutBound1;
    Vector breakoutBound2;

    bool visible;
    RN_Edge *selectedEdge;

    vector <Vector> originalLasso;

    int iconSelected;

    GLuint texture_translate, texture_close;
    Vector translateIconCentre, closeIconCentre;

    Viewer * m_pViewer;

};

#endif
