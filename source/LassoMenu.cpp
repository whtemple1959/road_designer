#include "LassoMenu.h"

#ifndef M_PI
#define M_PI 3.14159f
#endif

LassoMenu::LassoMenu(QGLWidget * v) :
        m_pViewer(v)
{

    closed=false;
    needComputeClosed=false;
    pieMenuVisible=false;

    mouseButton=-1;
    mouseSubButton=0;

    texView=TextureLoader::LoadTexture("menuicons/lassomenu_view.png");
    texFoliage=TextureLoader::LoadTexture("menuicons/lassomenu_foliage.png");
    texTiming=TextureLoader::LoadTexture("menuicons/lassomenu_timing.png");
    texDeletePath=TextureLoader::LoadTexture("menuicons/lassomenu_deletepath.png");
    texPlay=TextureLoader::LoadTexture("menuicons/lassomenu_play.png");
    texBreakout=TextureLoader::LoadTexture("menuicons/lassomenu_breakout.png");
    texCrossing=TextureLoader::LoadTexture("menuicons/lassomenu_crossing.png");
    texPath=TextureLoader::LoadTexture("menuicons/lassomenu_path.png");

    texClose=TextureLoader::LoadTexture("menuicons/lassomenu_view_close.png");
    texMidway=TextureLoader::LoadTexture("menuicons/lassomenu_view_midway.png");
    texBirdseye=TextureLoader::LoadTexture("menuicons/lassomenu_view_birdseye.png");
    texSparse=TextureLoader::LoadTexture("menuicons/lassomenu_foliage_sparse.png");
    texDense=TextureLoader::LoadTexture("menuicons/lassomenu_foliage_dense.png");
    texSlow=TextureLoader::LoadTexture("menuicons/lassomenu_timing_slow.png");
    texAverage=TextureLoader::LoadTexture("menuicons/lassomenu_timing_average.png");
    texFast=TextureLoader::LoadTexture("menuicons/lassomenu_timing_fast.png");
    texTwisty=TextureLoader::LoadTexture("menuicons/lassomenu_path_twisty.png");
    texStraighten=TextureLoader::LoadTexture("menuicons/lassomenu_path_straighten.png");
    texOverunder=TextureLoader::LoadTexture("menuicons/lassomenu_crossing_over.png");
    texIntersect=TextureLoader::LoadTexture("menuicons/lassomenu_crossing_intersect.png");
    texUnderover=TextureLoader::LoadTexture("menuicons/lassomenu_crossing_under.png");
    texLens=TextureLoader::LoadTexture("menuicons/lassomenu_breakout_lens.png");

    texFree=TextureLoader::LoadTexture("menuicons/lassomenu_drive_free.png");
    texHelicopter=TextureLoader::LoadTexture("menuicons/lassomenu_drive_helicopter.png");
    texHood=TextureLoader::LoadTexture("menuicons/lassomenu_drive_hood.png");
    texInside=TextureLoader::LoadTexture("menuicons/lassomenu_drive_inside.png");
    texStop=TextureLoader::LoadTexture("menuicons/lassomenu_drive_stop.png");
    texBehind=TextureLoader::LoadTexture("menuicons/lassomenu_drive_behind.png");

}

void LassoMenu::addPoint(Vector p2d, Vector pick) {

    points.push_back(p2d);
    pickrays.push_back(pick);

    if (!closed)
        needComputeClosed=true;

}

void LassoMenu::drawAsOpen(int mousex, int mousey) {

    bool tempClosed=closed;
    closed=false;

    draw(mousex, mousey);

    closed=tempClosed;

}

void LassoMenu::draw(int mousex, int mousey) {

    GLdouble model_view[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    setOrthogonal();

    glDisable(GL_TEXTURE_2D);

    glColor3f(1.0,1.0,1.0);
    //glColor3f(0,0,0); //for clothoid demo
    glLineStipple(1,0xF0F0);
    glLineWidth(3.0);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINE_STRIP);
    for (int i=0;i<points.size();i++)
        glVertex3f(points[i].x,points[i].y,41.0f);
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    if (closed) { //its a lasso

        //draw the triangle fan into the STENCIL BUFFER (so selected region is actually BRIGHTER)

        glClearStencil(0);
        glClear(GL_STENCIL_BUFFER_BIT);

        glEnable(GL_STENCIL_TEST);

        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 1);
        glDepthMask( GL_FALSE );					// Turn Off Writing To The Depth-Buffer
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

        glColor4f(1,1,1,1);
        glBegin(GL_TRIANGLE_FAN);
        for (int i=closeStartIndex;i<=closeEndIndex;i++) {
            glVertex3f(points[i].x,points[i].y,42.0f);
        }
        glEnd();

        glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
        glStencilFunc(GL_NOTEQUAL, 1, 1);
        glDepthMask(GL_TRUE);
        glColorMask( GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

        glColor4f(0,0,0,0.3);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
        glVertex3f(0,0,42.0f);
        glVertex3f(m_pViewer->width(),0,42.0f);
        glVertex3f(m_pViewer->width(),m_pViewer->height(),42.0f);
        glVertex3f(0,m_pViewer->height(),42.0f);
        glEnd();
        glDisable(GL_BLEND);

        glDisable(GL_STENCIL_TEST);

    }

    //pie menu
    if (pieMenuVisible) {
        drawPieMenu(mousex, mousey);
    }

    //restore old projection/modelview
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(projection);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(model_view);

}

void LassoMenu::clear() {
    points.clear();
    pickrays.clear();

    closed=false;

    pieMenuVisible=false;
    pieMenuButtons.clear();
}

Vector LassoMenu::getPoint(int index) {
    return points[index];
}

Vector LassoMenu::getPickray(int index) {
    return pickrays[index];
}

vector <Vector> LassoMenu::getAllPoints() {
    return points;
}

void LassoMenu::getLassoPoints(vector <Vector> & lassoPoints) {

    if (!closed) {
        return;
    }

    for (int i=closeStartIndex;i<closeEndIndex;i++) {
        lassoPoints.push_back(points[i]);
    }

}

Vector LassoMenu::getLassoCentroid() {
    Vector centroid=Vector(0.0f,0.0f,0.0f);
    for (int i=0;i<points.size();i++) {
        centroid+=points[i];
    }
    centroid=centroid/points.size();

    return centroid;
}

vector <Vector> LassoMenu::getAllPickrays() {
    return pickrays;
}

vector <Vector> LassoMenu::getLassoPickrays() {
    vector <Vector> lassoPoints;

    if (!closed)
        return lassoPoints;

    for (int i=closeStartIndex;i<closeEndIndex;i++)
        lassoPoints.push_back(pickrays[i]);

    return lassoPoints;
}

int LassoMenu::getNumPoints() {
    return points.size();
}

void LassoMenu::setOrthogonal() {

    glViewport(0, 0, m_pViewer->width(), m_pViewer->height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, m_pViewer->width(),
            0.0, m_pViewer->height(), -50.0, 50.0);

    glMatrixMode(GL_MODELVIEW);

}

bool LassoMenu::isClosed() {

    //return false; //for clothoid build

    if (!needComputeClosed)
        return closed;

    int sampleInterval=(int)ceil(points.size()/50.0f);
    int minLassoDist=8;

    for (int i=0;i+sampleInterval<points.size();i+=sampleInterval) {
        for (int j=i+sampleInterval*2;j+sampleInterval<points.size();j+=sampleInterval) {
            if (Collision::LineSegmentsCollide(Vector(points[i].x,0.0f,points[i].y),
                                               Vector(points[i+sampleInterval].x,0.0f,points[i+sampleInterval].y),
                                               Vector(points[j].x,0.0f,points[j].y),
                                               Vector(points[j+sampleInterval].x,0.0f,points[j+sampleInterval].y),
                                               closedPoint)&&j-i>minLassoDist) {
                closedPoint.y=closedPoint.z; closedPoint.z=0.0f;
                closeStartIndex=i+sampleInterval;
                closeEndIndex=j;

                //used closedpoint as centre of piemenu
                pieMenuCentre=closedPoint;

                needComputeClosed=false;
                closed=true;
                return closed;
            }

        }
    }

    needComputeClosed=false;
    closed=false;
    return closed;

}

void LassoMenu::setPieMenuButtons(vector <vector <char *> > buttons, vector <vector <bool> > enabled) {
    pieMenuButtons=buttons;
    pieMenuButtonEnabled=enabled;
    mouseButton=-1;
    buttonChanged=true;
}

void LassoMenu::setPieMenuVisible(bool v) {
    pieMenuVisible=v;
}

bool LassoMenu::getPieMenuVisible() {
    return pieMenuVisible;
}

void LassoMenu::drawPieMenu(int mousex, int mousey) {

    float MENU_ANGLESHIFT=2.0f*M_PI/pieMenuButtons.size()/2.0f;

    float menuRadius=115.0f;
    float menuTransparency=0.8f;

    GLdouble model_view[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    //render the circle
    //menu will be centred on intersection point
    glPushMatrix();

    float mouseAngle=atan2((float)mousey-pieMenuCentre.y,(float)mousex-pieMenuCentre.x);
    if ((float)mousey-pieMenuCentre.y<0.0f) {
        mouseAngle=2.0f*M_PI-fabs(mouseAngle);
    }
    float mouseDistance=Vector((float)mousex-pieMenuCentre.x,(float)mousey-pieMenuCentre.y,0.0f).GetLength();

    int curMouseButton=mouseButton;
    int curMouseSubButton=mouseSubButton;

    if (mouseDistance>menuRadius||mouseDistance<8.0f) {
        curMouseButton=-1;

    }
    else {
        for (int i=0;i<pieMenuButtons.size();i++) {
            if (mouseAngle>=i*2.0f*M_PI/pieMenuButtons.size()+MENU_ANGLESHIFT&&
                mouseAngle<(i+1)*2.0f*M_PI/pieMenuButtons.size()+MENU_ANGLESHIFT) {

                curMouseButton=i;
                curMouseSubButton=0;

                if (pieMenuButtons[i].size()>=1) {
                    curMouseSubButton=pieMenuButtons[curMouseButton].size()-1;
                    for (int eachsubbutton=1;eachsubbutton<pieMenuButtons[curMouseButton].size();eachsubbutton++) {
                        float eachsubradius=menuRadius*((float)eachsubbutton/(float)(pieMenuButtons[i].size()-1));
                        if (mouseDistance<eachsubradius) {
                            curMouseSubButton=eachsubbutton;
                            break;
                        }
                    }

                }

                break;
            }
            else if (mouseAngle<MENU_ANGLESHIFT) {
		
                curMouseButton=pieMenuButtons.size()-1;
                curMouseSubButton=0;

                if (pieMenuButtons[i].size()>=1)  {
                    curMouseSubButton=pieMenuButtons[curMouseButton].size()-1;
                    for (int eachsubbutton=1;eachsubbutton<pieMenuButtons[curMouseButton].size();eachsubbutton++) {
                        float eachsubradius=menuRadius*((float)eachsubbutton/(float)(pieMenuButtons[i].size()-1));
                        if (mouseDistance<eachsubradius) {
                            curMouseSubButton=eachsubbutton;
                            break;
                        }
                    }
                }

                break;
            }
        }
    }

    if (curMouseButton!=mouseButton||
        curMouseSubButton!=mouseSubButton)
        buttonChanged=true;

    mouseButton=curMouseButton;
    mouseSubButton=curMouseSubButton;

    //the base of the menu
    if (pieMenuCentre.x<menuRadius)
        pieMenuCentre.x=menuRadius+20.0f;
    if (pieMenuCentre.x+menuRadius>m_pViewer->width())
        pieMenuCentre.x=m_pViewer->width()-menuRadius-20.0f;
    if (pieMenuCentre.y<menuRadius)
        pieMenuCentre.y=menuRadius+20.0f;
    if (pieMenuCentre.y+menuRadius>m_pViewer->height())
        pieMenuCentre.y=m_pViewer->height()-menuRadius-20.0f;

    glTranslatef(pieMenuCentre.x,pieMenuCentre.y,0.0f);
    glColor4f(curColourDark.x,curColourDark.y,curColourDark.z,menuTransparency);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //render the menu disc
    glColor4f(curColourDark.x,curColourDark.y,curColourDark.z,menuTransparency);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f,0.0f,43.0f);
    for (float i=MENU_ANGLESHIFT;i<=MENU_ANGLESHIFT+2.0f*M_PI;i+=(2.0f*M_PI)/96.0f) {
        glVertex3f(cos(i)*menuRadius,sin(i)*menuRadius,43.0f);
    }
    glEnd();

    //render highlight of selection
    if (mouseButton>=0&&pieMenuButtonEnabled[mouseButton][mouseSubButton]) {

        float startRad;
        float endRad;

        if (pieMenuButtons[mouseButton].size()==1) {
            startRad=0.0f;
            endRad=menuRadius;
        }
        else {
            startRad=(mouseSubButton-1)*menuRadius/(pieMenuButtons[mouseButton].size()-1);
            endRad=(mouseSubButton)*menuRadius/(pieMenuButtons[mouseButton].size()-1);
        }

        glColor4f(curColourLight.x,curColourLight.y,curColourLight.z,menuTransparency);
        glBegin(GL_QUADS);
        for (float i=(float)mouseButton*2.0f*M_PI/pieMenuButtons.size()+MENU_ANGLESHIFT;
        i<(float)(mouseButton+1)*2.0f*M_PI/pieMenuButtons.size()+MENU_ANGLESHIFT;
        i+=(2.0f*M_PI)/256.0f) {

            glVertex3f(cos(i)*startRad,sin(i)*startRad,44.0f);
            glVertex3f(cos(i)*endRad,sin(i)*endRad,44.0f);
            glVertex3f(cos(i+(2.0f*M_PI)/256.0f)*endRad,sin(i+(2.0f*M_PI)/256.0f)*endRad,44.0f);
            glVertex3f(cos(i+(2.0f*M_PI)/256.0f)*startRad,sin(i+(2.0f*M_PI)/256.0f)*startRad,44.0f);

        }
        glEnd();
    }

    //division lines between buttons
    glLineWidth(3.0f);
    glColor4f(curColourLight.x,curColourLight.y,curColourLight.z,menuTransparency);
    if (pieMenuButtons.size()>1) {
        glBegin(GL_LINES);
        for (int i=0;i<pieMenuButtons.size();i++) {
            float eachAngle=i*2.0f*M_PI/pieMenuButtons.size()+MENU_ANGLESHIFT;
            glVertex3f(0.0f,0.0f,44.0f);
            glVertex3f(cos(eachAngle)*menuRadius,sin(eachAngle)*menuRadius,44.0f);
        }
        glEnd();
    }

    //division lines between sub-buttons
    glLineWidth(1.0f);
    for (int i=0;i<pieMenuButtons.size();i++) {

        for (int eachsubbutton=1;eachsubbutton<pieMenuButtons[i].size();eachsubbutton++) {

            float eachsubradius=menuRadius*((float)eachsubbutton/(float)(pieMenuButtons[i].size()-1));

            glBegin(GL_LINE_STRIP);
            for (float j=i*2.0f*M_PI/pieMenuButtons.size()+MENU_ANGLESHIFT;j<=(i+1)*2.0f*M_PI/pieMenuButtons.size()+MENU_ANGLESHIFT;j+=(2.0f*M_PI)/256.0f) {
                glVertex3f(cos(j)*eachsubradius,sin(j)*eachsubradius,44.0f);
            }
            glEnd();

        }
    }

    if (pieMenuUseSpecialLabels==0) {

        glColor3f(1.0f,1.0f,1.0f);
        for (int i=0;i<pieMenuButtons.size();i++) {
            float eachAngle=((float)i+0.5f)*2.0f*M_PI/pieMenuButtons.size()+MENU_ANGLESHIFT;

            //set enabled colour
            if (pieMenuButtonEnabled[i][0]&&pieMenuButtons[i].size()>1)
                glColor3f(curColourLight.x,curColourLight.y,curColourLight.z);
            else if (pieMenuButtonEnabled[i][0])
                glColor3f(1,1,1);
            else
                glColor3f(.5,.5,.5);

            m_pViewer->renderText((cos(eachAngle)*(menuRadius+15.0f)), (sin(eachAngle)*(menuRadius+15.0f)), QString(pieMenuButtons[i][0]));

            for (int eachsubbutton=1;eachsubbutton<pieMenuButtons[i].size();eachsubbutton++) {

                float eachsubradius=menuRadius*((float)eachsubbutton/(float)(pieMenuButtons[i].size()-1));
                eachsubradius-=10.0f;

                if (pieMenuButtonEnabled[i][eachsubbutton])
                    glColor3f(1,1,1);
                else
                    glColor3f(.5,.5,.5);

                m_pViewer->renderText(cos(eachAngle)*eachsubradius, sin(eachAngle)*eachsubradius, QString(pieMenuButtons[i][eachsubbutton]));

            }
        }

    }
    else if (pieMenuUseSpecialLabels==1) {

        glColor4f(1,1,1,1);
        for (int i=0;i<8;i++) {

            int texIndex;

            if (i==0)
                texIndex=texTiming;
            else if (i==1)
                texIndex=texFoliage;
            else if (i==2)
                texIndex=texView;
            else if (i==3)
                texIndex=texDeletePath;
            else if (i==4)
                texIndex=texBreakout;
            else if (i==5)
                texIndex=texCrossing;
            else if (i==6)
                texIndex=texPath;
            else
                texIndex=texPlay;

            //if (pieMenuButtonEnabled[i][0]&&pieMenuButtons[i].size()>1)
            //	glColor3f(curColourLight.x,curColourLight.y,curColourLight.z);
            if (pieMenuButtonEnabled[i][0])
                glColor3f(1,1,1);
            else
                glColor3f(.5,.5,.5);

            drawPieMenuLabel((float)i*2.0f*M_PI/pieMenuButtons.size()+MENU_ANGLESHIFT,
                             (float)(i+1)*2.0f*M_PI/pieMenuButtons.size()+MENU_ANGLESHIFT,
                             texIndex,
                             menuRadius-10.0f,
                             menuRadius+10.0f);
        }

        //draw sublabels (if selected)
        for (int i=0;i<8;i++) {

            if (pieMenuButtons[i].size()>1&&i==mouseButton&&pieMenuButtonEnabled[i][mouseSubButton]) {

                int texIndex;

                switch (i) {

                case 0:

                    if (mouseSubButton==1)
                        texIndex=texSlow;
                    else if (mouseSubButton==2)
                        texIndex=texAverage;
                    else if (mouseSubButton==3)
                        texIndex=texFast;

                    break;

                case 1:

                    if (mouseSubButton==1)
                        texIndex=texSparse;
                    else if (mouseSubButton==2)
                        texIndex=texDense;

                    break;

                case 2:

                    if (mouseSubButton==1)
                        texIndex=texClose;
                    else if (mouseSubButton==2)
                        texIndex=texMidway;
                    else if (mouseSubButton==3)
                        texIndex=texBirdseye;

                    break;

                case 4:

                    if (mouseSubButton==1)
                        texIndex=texView;
                    else if (mouseSubButton==2)
                        texIndex=texLens;

                    break;

                case 5:

                    if (mouseSubButton==1)
                        texIndex=texOverunder;
                    else if (mouseSubButton==2)
                        texIndex=texIntersect;
                    else if (mouseSubButton==3)
                        texIndex=texUnderover;

                    break;

                case 6:

                    if (mouseSubButton==1)
                        texIndex=texTwisty;
                    else if (mouseSubButton==2)
                        texIndex=texStraighten;

                    break;

                default:
                    break;

                }

                glColor4f(1,1,1,1);
                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                glBindTexture(GL_TEXTURE_2D, texIndex);

                glBegin(GL_QUADS);
                glTexCoord2f(0,0);
                glVertex3f(-menuRadius/2.0f,-menuRadius/4.0f,46.0f);
                glTexCoord2f(0,1);
                glVertex3f(-menuRadius/2.0f,menuRadius/4.0f,46.0f);
                glTexCoord2f(1,1);
                glVertex3f(menuRadius/2.0f,menuRadius/4.0f,46.0f);
                glTexCoord2f(1,0);
                glVertex3f(menuRadius/2.0f,-menuRadius/4.0f,46.0f);
                glEnd();

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);

            }
        }
    }
    else if (pieMenuUseSpecialLabels==2) {

        glColor4f(1,1,1,1);
        for (int i=0;i<6;i++) {

            int texIndex;

            if (i==0)
                texIndex=texFree;
            else if (i==1)
                texIndex=texHelicopter;
            else if (i==2)
                texIndex=texHood;
            else if (i==3)
                texIndex=texBehind;
            else if (i==4)
                texIndex=texInside;
            else
                texIndex=texStop;

            //if (pieMenuButtonEnabled[i][0]&&pieMenuButtons[i].size()>1)
            //	glColor3f(curColourLight.x,curColourLight.y,curColourLight.z);
            if (pieMenuButtonEnabled[i][0])
                glColor3f(1,1,1);
            else
                glColor3f(.5,.5,.5);

            drawPieMenuLabel((float)i*2.0f*M_PI/pieMenuButtons.size()+MENU_ANGLESHIFT,
                             (float)(i+1)*2.0f*M_PI/pieMenuButtons.size()+MENU_ANGLESHIFT,
                             texIndex,
                             menuRadius-10.0f,
                             menuRadius+10.0f);
        }

    }

    glPopMatrix();

    //now reset the projection/modelview matrices to what was there before
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(projection);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(model_view);

}

void LassoMenu::drawPieMenuLabel(float startAngle, float endAngle, int texIndex, float startRad, float endRad) {

    //the button labels
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, texIndex);

    bool shouldFlip=false;

    if (startAngle<M_PI&&endAngle>M_PI) {

        float theAngle=(endAngle+startAngle)/2.0f;
        float theRadius=(startRad+endRad)/2.0f;

        glBegin(GL_QUADS);
        glTexCoord2f(0,0);
        glVertex3f(cos(theAngle)*theRadius/2.0f-theRadius/2.0f,sin(theAngle)*theRadius/2.0f-10.0f,45.0f);
        glTexCoord2f(0,1);
        glVertex3f(cos(theAngle)*theRadius/2.0f-theRadius/2.0f,sin(theAngle)*theRadius/2.0f+10.0f,45.0f);
        glTexCoord2f(1,1);
        glVertex3f(cos(theAngle)*theRadius/2.0f+theRadius/4.0f,sin(theAngle)*theRadius/2.0f+10.0f,45.0f);
        glTexCoord2f(1,0);
        glVertex3f(cos(theAngle)*theRadius/2.0f+theRadius/4.0f,sin(theAngle)*theRadius/2.0f-10.0f,45.0f);
        glEnd();

    }
    else if (startAngle<2.0f*M_PI&&endAngle>2.0f*M_PI) {
        float theAngle=(endAngle+startAngle)/2.0f;
        float theRadius=(startRad+endRad)/2.0f;

        glBegin(GL_QUADS);
        glTexCoord2f(0,0);
        glVertex3f(cos(theAngle)*theRadius/2.0f-theRadius/4.0f,sin(theAngle)*theRadius/2.0f-10.0f,45.0f);
        glTexCoord2f(0,1);
        glVertex3f(cos(theAngle)*theRadius/2.0f-theRadius/4.0f,sin(theAngle)*theRadius/2.0f+10.0f,45.0f);
        glTexCoord2f(1,1);
        glVertex3f(cos(theAngle)*theRadius/2.0f+theRadius/2.0f,sin(theAngle)*theRadius/2.0f+10.0f,45.0f);
        glTexCoord2f(1,0);
        glVertex3f(cos(theAngle)*theRadius/2.0f+theRadius/2.0f,sin(theAngle)*theRadius/2.0f-10.0f,45.0f);
        glEnd();
    }
    else {

        if (startAngle>M_PI&&startAngle<2.0f*M_PI&&
            endAngle>M_PI&&endAngle<2.0f*M_PI)
            shouldFlip=true;

        glBegin(GL_QUADS);
        for (float i=startAngle;i<endAngle;i+=(2.0f*M_PI)/256.0f) {

            float xInterp=(i-startAngle)/(endAngle-startAngle);
            float xInterp2=(i+(2.0f*M_PI)/256.0f-startAngle)/(endAngle-startAngle);

            if (!shouldFlip)
                glTexCoord2f(1.0f-xInterp,0.0f);
            else
                glTexCoord2f(xInterp,1.0f);

            glVertex3f(cos(i)*startRad,sin(i)*startRad,45.0f);

            if (!shouldFlip)
                glTexCoord2f(1.0f-xInterp,1.0f);
            else
                glTexCoord2f(xInterp,0.0f);

            glVertex3f(cos(i)*endRad,sin(i)*endRad,45.0f);

            if (!shouldFlip)
                glTexCoord2f(1.0f-xInterp2,1.0f);
            else
                glTexCoord2f(xInterp2,0.0f);

            glVertex3f(cos(i+(2.0f*M_PI)/256.0f)*endRad,sin(i+(2.0f*M_PI)/256.0f)*endRad,45.0f);

            if (!shouldFlip)
                glTexCoord2f(1.0f-xInterp2,0.0f);
            else
                glTexCoord2f(xInterp2,1.0f);

            glVertex3f(cos(i+(2.0f*M_PI)/256.0f)*startRad,sin(i+(2.0f*M_PI)/256.0f)*startRad,45.0f);

        }
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

}

char *LassoMenu::getSelectedButtonText() {

    if (mouseButton<0)
        return "";
    else {
        if (pieMenuButtonEnabled[mouseButton][mouseSubButton])
            return pieMenuButtons[mouseButton][mouseSubButton];
        else
            return "";
    }

}

//returns true of button change, but only once!  it resets after
bool LassoMenu::getButtonChanged() {

    if (buttonChanged) {
        buttonChanged=false;
        return true;
    }

    return false;
}

void LassoMenu::setPieMenuColour(Vector dark, Vector light) {

    curColourDark=dark;
    curColourLight=light;

}

void LassoMenu::setPieMenuCentre(Vector v) {
    pieMenuCentre=v;
}

void LassoMenu::setUseSpecialLabels(int labels) {

    pieMenuUseSpecialLabels=labels;

}
