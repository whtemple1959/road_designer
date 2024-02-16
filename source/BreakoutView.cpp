#include "BreakoutView.h"

BreakoutView::BreakoutView(Viewer * v) :
m_pViewer(v)
{   

    visible=false;
    iconSelected=0;

    texture_translate=TextureLoader::LoadTexture("menuicons/twistlens_translate.png");
    texture_close=TextureLoader::LoadTexture("menuicons/twistlens_close.png");

}

void BreakoutView::drawBreakoutStencil() {

    glColor4f(.6,.6,.7,1);
    glBegin(GL_TRIANGLE_FAN);
    for (int i=0;i<breakoutPoints2D.size();i++) {
        glVertex3f(breakoutPoints2D[i].x-(breakoutPointsCentroid.x-breakoutViewCentre.x)*(interpVal),
                   breakoutPoints2D[i].y-(breakoutPointsCentroid.y-breakoutViewCentre.y)*(interpVal),0.0f);
    }
    glEnd();

}

void BreakoutView::setOrthogonal() {

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, m_pViewer->width(),
            0.0, m_pViewer->height(), -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);

}

bool BreakoutView::isVisible() {
    return visible;
}

void BreakoutView::updateInterpCamera(float timeDiffms) {

    if (interpVal<1.0) {
        interpVal+=(timeDiffms/1000.0f);
        interpCamera.interpBetweenCameras(m_pViewer->getCamera(), camera, interpVal);
    }
    else if (interpVal>=1.0) {
        interpVal=1.0;
        interpCamera.setCamera(camera);
    }

}

void BreakoutView::setupView() {	

    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    GLdouble model_view[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    setOrthogonal();

    //draw original lasso
    glColor3f(1.0,1.0,1.0);
    glLineStipple(1,0xF0F0);
    glLineWidth(3.0);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINE_LOOP);
    for (int i=0;i<originalLasso.size();i++)
        glVertex3f(originalLasso[i].x,originalLasso[i].y,0.0f);
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    //draw the icons to use it
    drawIcon(breakoutViewCentre.x+translateIconCentre.x,breakoutViewCentre.y+translateIconCentre.y,texture_translate);
    drawIcon(breakoutViewCentre.x+closeIconCentre.x,breakoutViewCentre.y+closeIconCentre.y,texture_close);

    //draw breakout outline
    glColor3f(1.0,1.0,1.0);
    glLineStipple(1,0xF0F0);
    glLineWidth(5.0);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINE_LOOP);
    for (int i=0;i<breakoutPoints2D.size();i++) {
        glVertex3f(breakoutPoints2D[i].x-(breakoutPointsCentroid.x-breakoutViewCentre.x)*(interpVal),
                   breakoutPoints2D[i].y-(breakoutPointsCentroid.y-breakoutViewCentre.y)*(interpVal),1.0);
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    //draw the breakout stencil
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 1);
    glEnable(GL_STENCIL_TEST);  
    glDisable(GL_DEPTH_TEST);

    drawBreakoutStencil();

    glEnable(GL_DEPTH_TEST);
    glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
    glStencilFunc(GL_EQUAL, 1, 1);    
    glClear(GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(projection);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(model_view);



}

void BreakoutView::drawIcon(float xtrans, float ytrans, GLuint textureIndex) {

    //display control icons
    glColor3f(1.0f,1.0f,1.0f);
    glPushMatrix();
    glTranslatef(xtrans-ICON_RADIUS,ytrans-ICON_RADIUS,0.0f);

    glBindTexture (GL_TEXTURE_2D, textureIndex);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0);
    glVertex2f(0,0);
    glTexCoord2f(0,1);
    glVertex2f(0,ICON_RADIUS*2.0f);
    glTexCoord2f(1,1);
    glVertex2f(ICON_RADIUS*2.0f,ICON_RADIUS*2.0f);
    glTexCoord2f(1,0);
    glVertex2f(ICON_RADIUS*2.0f,0);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

}

void BreakoutView::tearDownView() {

    //draw the border for the breakout view
    glClear(GL_DEPTH_BUFFER_BIT);
    //drawBreakoutBorder();

    glDisable(GL_STENCIL_TEST);

    glViewport(0, 0, m_pViewer->width(), m_pViewer->height());

}

Camera & BreakoutView::getCamera() {
    return interpCamera;
}

void BreakoutView::setVisible(bool v) {
    visible=v;
}

bool BreakoutView::isCameraMoving() {
    return (interpVal<1.0f);
}

Vector *BreakoutView::rayIntoView(Vector lastpickray) {

    //intersect the plane (either of 2 triangles)
    Vector *collisionPoint;
    collisionPoint=Collision::LineTriangleCollide(camera.getPosition(),lastpickray,
                                                  breakoutBound1+Vector(0,-1000,0),
                                                  breakoutBound1+Vector(0,1000,0),
                                                  breakoutBound2+Vector(0,-1000,0));
    if (collisionPoint==NULL) {
        collisionPoint=Collision::LineTriangleCollide(camera.getPosition(),lastpickray,
                                                      breakoutBound1+Vector(0,1000,0),
                                                      breakoutBound2+Vector(0,-1000,0),
                                                      breakoutBound2+Vector(0,1000,0));
    }

    return collisionPoint;

}

void BreakoutView::newBreakoutView(vector <Vector> lassoPoints2D, RN *roadNetwork, Camera & curCamera) {

    breakoutPoints2D.clear();
    originalLasso.clear();

    breakoutPoints2D.reserve(lassoPoints2D.size());
    originalLasso.reserve(lassoPoints2D.size());
    for (int i=0;i<lassoPoints2D.size();i++) {
        breakoutPoints2D.push_back(lassoPoints2D[i]);
        originalLasso.push_back(lassoPoints2D[i]);
    }

    //are a vector, place camera along the axis perpendicular to it and the vector (0,1,0)
    //the distance to move along that vector will be determined by the distance between those selected points

    breakoutBound1=roadNetwork->getMostSelectedEdge()->getFirstPointSelected();
    breakoutBound2=roadNetwork->getMostSelectedEdge()->getLastPointSelected();

    breakoutPointsCentroid=Vector(0.0f,0.0f,0.0f);
    translateIconCentre=Vector(0.0f,0.0f,0.0f);
    closeIconCentre=Vector(0.0f,0.0f,0.0f);

    for (int i=0;i<breakoutPoints2D.size();i++) {
        breakoutPointsCentroid+=Vector(breakoutPoints2D[i].x,breakoutPoints2D[i].y,0.0f);
        if (breakoutPoints2D[i].x>translateIconCentre.x) {
            translateIconCentre.x=breakoutPoints2D[i].x;
            closeIconCentre.x=breakoutPoints2D[i].x;
        }
        if (breakoutPoints2D[i].y>translateIconCentre.y) {
            translateIconCentre.y=breakoutPoints2D[i].y;
            closeIconCentre.y=breakoutPoints2D[i].y;
        }
    }
    breakoutPointsCentroid=breakoutPointsCentroid/breakoutPoints2D.size();
    translateIconCentre.x=closeIconCentre.x-ICON_RADIUS*2.0f-5.0f;
    translateIconCentre.y+=ICON_RADIUS;
    closeIconCentre.y+=ICON_RADIUS;

    translateIconCentre=translateIconCentre-breakoutPointsCentroid;
    closeIconCentre=closeIconCentre-breakoutPointsCentroid;

    selectedEdge=roadNetwork->getMostSelectedEdge();

    int firstSegment=selectedEdge->getFirstSegmentSelected();
    int lastSegment=selectedEdge->getLastSegmentSelected();
    float maxDist=0.0f;
    Vector midPoint=Vector(0.0f,0.0f,0.0f);

    for (int i=firstSegment;i<=lastSegment;i++) {
        midPoint+=selectedEdge->getPoint(i);
        if ((selectedEdge->getPoint(i)-breakoutBound1).GetLength()>maxDist) {
            maxDist=(selectedEdge->getPoint(i)-breakoutBound1).GetLength();
        }
    }
    midPoint=midPoint/(float)(lastSegment-firstSegment+1);

    camera.setTrackPosition(midPoint);
    camera.setTrackDist(maxDist);
    camera.setTilt(8.0f);

    //spin is based on perpvec
    Vector perpVec;
    perpVec.CrossProduct(breakoutBound2-breakoutBound1,Vector(0,1,0));
    float spinAngle=RadToDeg(atan2(perpVec.z,perpVec.x));

    //get it on the same angle as the camera right now
    if (fabs(spinAngle-curCamera.getSpin())>90.0f) {
        if (!(spinAngle<90.0f&&fabs(spinAngle+360.0f-curCamera.getSpin())<90.0f)&&
            !(spinAngle>270.0f&&fabs(spinAngle-360.0f-curCamera.getSpin())<90.0f))
            spinAngle+=180.0f;
    }

    camera.setSpin(spinAngle);
    camera.updateTrackPosition();

    //set the interpolant
    interpVal=0.0;

    //set where to move breakout view to
    Vector displaceCentre=Vector(m_pViewer->width()/2.0f,m_pViewer->height()/2.0f,0.0f)-breakoutPointsCentroid;
    displaceCentre.SetLength(m_pViewer->height()/2.0f);

    breakoutViewCentre=breakoutPointsCentroid+displaceCentre;

}

void BreakoutView::mouseClick(int x, int y, bool down) {
    //iconselected=1 - translate
    //iconselected=2 - outer radius
    //iconselected=3 - inner radius
    float xf=(float)x;
    float yf=(float)m_pViewer->height()-(float)y;

    iconSelected=0;

    if (down) {
        if (xf>breakoutViewCentre.x+translateIconCentre.x-ICON_RADIUS&&
            xf<breakoutViewCentre.x+translateIconCentre.x+ICON_RADIUS&&
            yf>breakoutViewCentre.y+translateIconCentre.y-ICON_RADIUS&&
            yf<breakoutViewCentre.y+translateIconCentre.y+ICON_RADIUS)
            iconSelected=1;
        else if (xf>breakoutViewCentre.x+closeIconCentre.x-ICON_RADIUS&&
                 xf<breakoutViewCentre.x+closeIconCentre.x+ICON_RADIUS&&
                 yf>breakoutViewCentre.y+closeIconCentre.y-ICON_RADIUS&&
                 yf<breakoutViewCentre.y+closeIconCentre.y+ICON_RADIUS) {
            iconSelected=2;
            visible=false;
        }
    }

}

void BreakoutView::mouseMotion(int x, int y) {

    float xf=(float)x;
    float yf=(float)m_pViewer->height()-(float)y;

    if (iconSelected==1) { //translate the breakout view
        breakoutViewCentre=Vector(xf,yf,0.0f)-translateIconCentre;
    }

}

int BreakoutView::getIconSelected() {
    return iconSelected;
}

Vector BreakoutView::getCentre() {
    return breakoutViewCentre;
}

RN_Edge *BreakoutView::getSelectedEdge() {
    return selectedEdge;
}
