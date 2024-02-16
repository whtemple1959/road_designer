/*
 * Viewer.h - header file for Viewer,
 * a class which encompasses the interface's 3D view of the currently
 * generated track
 */ 

#ifndef _VIEWER_H
#define _VIEWER_H

#include <QtCore>
#include <QtOpenGL>
#include <GL/glu.h>

#include <stdlib.h>
#include <vector>

#include "Vector.h"
#include "ModalMenu.h"
#include "OBJObject.h"
#include "HeightMap.h"
#include "Collision.h"
#include "RN.h"
#include "Camera.h"
#include "TwistLens.h"
#include "BreakoutView.h"
#include "TriMeshTextured.h"
#include "LassoMenu.h"

#define DEFAULT_CAR_TRACK_DIST 6.0f*MIN_DIST_BETWEEN_POINTS
#define MENU_DISPLAY_DELAY 0.75f

#define PLAYBACK_SPEED_SLOW 0.025f
#define PLAYBACK_SPEED_FAST 0.125f //0.1f
#define PLAYBACK_SPEED_AVERAGE 0.05f


using namespace std;

class BreakoutView;
class LassoMenu;

class Viewer : public QGLWidget {
    
public:
    
    Viewer(QWidget * parent, int screenWidth, int screenHeight, QString heightFile, QString textureFile);
    
    void initializeGL(void);
    void paintGL(void);
    void mouseMoveEvent(QMouseEvent * e);
    void mousePressEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent * e);

    void keyPressEvent(QKeyEvent * e);
    void keyReleaseEvent(QKeyEvent * e);

    void resizeGL(int width, int height);

    void DrawText(GLint x, GLint y, char* s, GLfloat r, GLfloat g, GLfloat b);
    void makeSelection();
    Vector *getCollision(Vector pickray);
    Camera & getCamera();
    void getMouseXY(int &x, int &y);

    void setupTheMenu();

private:

    void setPerspective();
    void setOrthogonal();

    void trackPosition(Vector position);
    void updatePickray();
    void updateCamera(Camera & whichCam);

    void drawPlayModeCar();

    double rotationAngle, viewAngle;
    long lastFrameUpdate;

    int screenWidth;
    int screenHeight;

    HeightMap *heightMap;

    //camera parameters
    Camera lastCamera;
    Camera interpCamera;
    Camera mainCamera;
    Camera playModeLastCamera;
    QTime playModeTimer;
    QTime playModeStartTimer;
    QTime cameraTimer;

    int mouse_x;
    int mouse_y;
    bool mouse_b1;
    bool mouse_b2;
    bool mouse_b3;

    vector <Vector> lassoPoints3D;

    Vector lastCollision;
    Vector lastpickray;

    RN *roadNetwork;
    RN *previewRoadNetwork;

    QGLShaderProgram * breakoutLensShader;    

    //for "playing" the road (to see if you like it)
    TriMeshTextured *playRoadCar;
    TriMeshTextured *playRoadTire;
    TriMeshTextured *playRoadInterior;
    TriMeshTextured *playRoadInteriorSeats;
    TriMeshTextured *playRoadSteeringWheel;

    bool playRoadEnabled;
    int playRoadSegment;
    float playRoadInterp;
    RN_Edge *playRoadEdge;
    int playRoadCamMode;
    float playRoadLastSpin;
    Vector carPosition;
    Vector carDirection;
    float carWheelRotate;
    float carSpeed_Current;
    float carSpeed_Desired;
    float carSpeed_AccelRate;

    float curClothoidPenalty;

    //twist lens
    TwistLens *twistLens;

    //breakout view
    BreakoutView *breakoutView;

    //lasso menu
    LassoMenu *lassoMenu;

    //timer
    double timeDiffms;

    int specialModifiers;

    bool m_bActiveAlt;
    QString m_heightFile;
    QString m_textureFile;

    QTimer timer;

};

#endif
