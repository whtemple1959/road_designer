#include "Viewer.h"

Viewer::Viewer(QWidget * parent, int screenWidth, int screenHeight, QString heightFile, QString textureFile) :
        QGLWidget(parent),
        m_heightFile(heightFile),
        m_textureFile(textureFile)
{
    
    m_bActiveAlt = false;

    curClothoidPenalty=DEFAULT_CLOTHOID_PENALTY;

    this->screenWidth=screenWidth;
    this->screenHeight=screenHeight;  	   

    m_heightFile = heightFile;
    m_textureFile = textureFile;

    timer.setInterval(25);
    timer.start();
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));

}

void Viewer::initializeGL(void) {
    

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    //load heightmap
    heightMap=new HeightMap();

    if (m_heightFile != QString::null && m_textureFile != QString::null)
        heightMap->loadFromImages(m_heightFile, m_textureFile);
    else {
        heightMap->generateGrid(256, 256);
    }

    cameraTimer.restart();
    mainCamera.setPosition(Vector(0, 0, 0));
    mainCamera.setSpin(0);
    mainCamera.setTilt(55);
    mainCamera.setTrackDist(150.0);
    mainCamera.setTrackPosition(Vector(128, 64, 128));

    mainCamera.updateTrackPosition();

    //set road network
    roadNetwork=new RN(heightMap);
    roadNetwork->setHeightMap(heightMap);

    previewRoadNetwork=new RN(heightMap);

    //setup twist lens
    twistLens=new TwistLens(this);

    //setup breakout view, lasso
    breakoutView=new BreakoutView(this);
    lassoMenu=new LassoMenu(this);

    //QGLShader breakoutShader(QGLShader::Vertex);
    //breakoutShader.compileSourceFile("breakoutLens.vert");
    //breakoutLensShader = SM.loadfromFile( "breakoutLens.vert", "breakoutLens.frag" );
    breakoutLensShader = new QGLShaderProgram(QGLContext::currentContext());

    breakoutLensShader->addShaderFromSourceFile(QGLShader::Vertex, "shaders/breakoutLens.vert");
    breakoutLensShader->addShaderFromSourceFile(QGLShader::Fragment, "shaders/breakoutLens.frag");

    breakoutLensShader->link();

    qDebug() << "breakoutLensShader: " << breakoutLensShader;

    playRoadEnabled=false;

    //set perspective of viewer
    setPerspective();

    //set clear/active colours
    glColor3f(1,1,1);
    glClearColor(0.6,0.6,0.7,1);    

    carSpeed_Current=0.0f;
    carSpeed_AccelRate=1.0f;
    carSpeed_Desired=0.0f;

    playRoadCamMode=0;

    //firstTime=now();
    //_RMSTUNE_start(0);
    //_RMSTUNE_start(1);

    ///* REMOVED FOR GEOMETRIC MODELLING EXAMPLE
    /*
	playRoadCar=new TriMeshTextured("objs/assets/delorean_car.obj","objs/assets/delorean.tga");
	playRoadTire=new TriMeshTextured("objs/assets/delorean_tire.obj","objs/assets/delorean.tga");
	playRoadInterior=new TriMeshTextured("objs/assets/car_interior.obj","objs/assets/car_interior.tga");
	playRoadSteeringWheel=new TriMeshTextured("objs/assets/car_steeringwheel.obj","objs/assets/car_steeringwheel.tga");
	playRoadInteriorSeats=new TriMeshTextured("objs/assets/car_interior_seats.obj","objs/assets/car_interior_seats.tga");
	*/

    playRoadCar=new TriMeshTextured();
    playRoadCar->Read("objs/assets/delorean_car.obj");
    playRoadTire=new TriMeshTextured();
    playRoadTire->Read("objs/assets/delorean_tire.obj");
    playRoadInterior=new TriMeshTextured();
    playRoadInterior->Read("objs/assets/car_interior.obj");
    playRoadSteeringWheel=new TriMeshTextured();
    playRoadSteeringWheel->Read("objs/assets/car_steeringwheel.obj");
    playRoadInteriorSeats=new TriMeshTextured();
    playRoadInteriorSeats->Read("objs/assets/car_interior_seats.obj");   
    
}

void Viewer::setPerspective() {

    //sets perspective to current screen width/height
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(80.0, float(width()) / float(height()), 0.05, MAXSELECTDISTANCE);
    glMatrixMode(GL_MODELVIEW);

}

void Viewer::setOrthogonal() {

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, float(width()),
            0.0, float(height()), -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);

}

void Viewer::DrawText(GLint x, GLint y, char* s, GLfloat r, GLfloat g, GLfloat b)
{

    QPen pen;
    pen.setColor(QColor(r, g, b));

    QPainter painter(this);

    painter.setPen(pen);
    painter.drawText(QPoint(x,y), s);

    painter.end();

}

void Viewer::updatePickray() {

    //update pickray
    GLdouble proj[16];
    GLdouble modelview[16];
    GLint viewport[4];

    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetIntegerv(GL_VIEWPORT,viewport);

    GLdouble objx1;
    GLdouble objy1;
    GLdouble objz1;

    GLdouble objx2;
    GLdouble objy2;
    GLdouble objz2;

    if (!breakoutView->isVisible()) {
        gluUnProject(mouse_x,viewport[3]-mouse_y-1,0.0,modelview,proj,viewport,&objx1,&objy1,&objz1);
        gluUnProject(mouse_x,viewport[3]-mouse_y-1,1.0,modelview,proj,viewport,&objx2,&objy2,&objz2);
    }
    else {
        Vector breakout2dVec=Vector(float(mouse_x-(breakoutView->getCentre().x - width()/2.0f)),
                                    float(screenHeight - mouse_y - height()/2.0f),
                                    0.0f);

        gluUnProject(breakout2dVec.x,breakout2dVec.y-1,0.0,modelview,proj,viewport,&objx1,&objy1,&objz1);
        gluUnProject(breakout2dVec.x,breakout2dVec.y-1,1.0,modelview,proj,viewport,&objx2,&objy2,&objz2);
    }

    lastpickray=Vector(objx2-objx1, objy2-objy1, objz2-objz1);
    lastpickray.Normal();

}

void Viewer::updateCamera(Camera & whichCam) {

    //translate/rotate to camera view
    whichCam.updateTrackPosition();
    whichCam.rotateLookAtTrackPosition();

}

void Viewer::paintGL(void) {

    GLenum glErr;

    if (twistLens->isVisible()) {

        float twistVal, innerRad, outerRad;
        Vector mainPoint, axisPoint, midPoint;

        twistLens->getShaderAttribs(twistVal ,innerRad, outerRad, mainPoint, axisPoint, midPoint);

        breakoutLensShader->bind();

        float rotVal = float(M_PI * twistVal/180.0f);
        breakoutLensShader->setUniformValue("rotVal", rotVal);
        breakoutLensShader->setUniformValue("innerRad", innerRad);
        breakoutLensShader->setUniformValue("outerRad", outerRad);
        breakoutLensShader->setUniformValue("mainPoint",mainPoint.x,mainPoint.y,mainPoint.z);
        breakoutLensShader->setUniformValue("axisPoint",axisPoint.x,axisPoint.y,axisPoint.z);
        breakoutLensShader->setUniformValue("midPoint",midPoint.x,midPoint.y,midPoint.z);

    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glPushMatrix();

    //if play mode enabled, do main camera update
    if (playRoadEnabled) {

        //modify speed of car
        if (playRoadEdge->getPointAttrib(playRoadSegment,POINT_ATTRIB_SPEED_SLOW))
                carSpeed_Desired=PLAYBACK_SPEED_SLOW;
        else if (playRoadEdge->getPointAttrib(playRoadSegment,POINT_ATTRIB_SPEED_FAST))
                carSpeed_Desired=PLAYBACK_SPEED_FAST;
        else
                carSpeed_Desired=PLAYBACK_SPEED_AVERAGE;

        float timeDiff = float(playModeTimer.elapsed()) / 1000.0f;
        playModeTimer.restart();

        float carSpeed_Displace=carSpeed_AccelRate * timeDiff;

        if (fabs(carSpeed_Desired-carSpeed_Current)<carSpeed_Displace)
                carSpeed_Current=carSpeed_Desired;
        else if (carSpeed_Desired>carSpeed_Current)
                carSpeed_Current+=carSpeed_Displace;
        else
                carSpeed_Current-=carSpeed_Displace;

        carPosition=playRoadEdge->getInterpPosition(playRoadSegment,playRoadInterp);
        carDirection=playRoadEdge->getInterpTangent(playRoadSegment,playRoadInterp);
        carWheelRotate+=(float)timeDiffms;

        Vector displaceRight=carDirection.GetRotatedY(90.0f);
        displaceRight.y=0.0f;
        displaceRight.SetLength(MIN_DIST_BETWEEN_POINTS/4.0f);

        Vector displaceUp=Vector(0.0f,0.5f*MIN_DIST_BETWEEN_POINTS,0.0f);

        Vector displaceForward=carDirection;
        displaceForward.SetLength(MIN_DIST_BETWEEN_POINTS);

        float curTime = float(playModeStartTimer.elapsed()) / 1000.0f;

        if (playRoadCamMode==0) {
                mainCamera.setTrackPosition(carPosition-displaceRight+displaceUp);
        }
        else if (playRoadCamMode==1) { //HELICOPTER - spin, tilt and zoom about
                mainCamera.setTrackPosition(carPosition-displaceRight+displaceUp);
                mainCamera.setSpin(atan2(carDirection.z,carDirection.x)*180.0/M_PI+180.0f+30.0f*sinf(curTime*0.5f));
                mainCamera.setTilt(60.0f+10.0f*sinf(curTime*1.5f));
                mainCamera.setTrackDist(MIN_DIST_BETWEEN_POINTS*(5.0f+2.0f*sinf(curTime)));
        }
        else if (playRoadCamMode==2) {
                mainCamera.setTrackPosition(carPosition-displaceRight+displaceUp);
                mainCamera.setSpin(atan2(carDirection.z,carDirection.x)*180.0/M_PI+180.0f);
                mainCamera.setTilt(-RadToDeg(asinf(carDirection.y)));
                mainCamera.setTrackDist(0.05f);
        }
        else if (playRoadCamMode==3) {
                mainCamera.setTrackPosition(carPosition-displaceRight+displaceUp);
                mainCamera.setSpin(atan2(carDirection.z,carDirection.x)*180.0/M_PI+180.0f);
                mainCamera.setTilt(20.0f);
                mainCamera.setTrackDist(MIN_DIST_BETWEEN_POINTS);
        }
        else if (playRoadCamMode==4) {

                mainCamera.setTrackPosition(carPosition-displaceRight*0.72f+displaceUp*0.75);

                float curSpin=atan2(carDirection.z,carDirection.x)*180.0/M_PI+180.0f;
                float spinDiff=curSpin-playRoadLastSpin;
                mainCamera.setSpin(mainCamera.getSpin()+spinDiff);
                mainCamera.setTilt(-RadToDeg(asinf(carDirection.y)));
                mainCamera.setTrackDist(0.01);
        }

        playRoadLastSpin=atan2(carDirection.z,carDirection.x)*180.0/M_PI+180.0f;

        if (cameraTimer.elapsed() > 1000) { //stuttering caused if update main camera only
                interpCamera.setTrackPosition(mainCamera.getTrackPosition());
        }

        //distance to travel is based on timeDiffms now
        float translateDist=carSpeed_Current*MIN_DIST_BETWEEN_POINTS;

        while (translateDist>0.0f&&playRoadSegment<playRoadEdge->getLastSegmentSelected()) { //move along the road!

                float segmentDist=(playRoadEdge->getPoint(playRoadSegment+1)-
                        playRoadEdge->getPoint(playRoadSegment)).GetLength();
                float distLeftInSegment=segmentDist*(1.0-playRoadInterp);

                if (translateDist>distLeftInSegment) {
                        translateDist-=distLeftInSegment;
                        playRoadSegment++;
                        playRoadInterp=0.0f;
                }
                else {
                        playRoadInterp+=translateDist/segmentDist;
                        translateDist=0.0f;
                }
        }

        if (playRoadSegment>=playRoadEdge->getLastSegmentSelected()) { //done yet?
                playRoadEnabled=false;

                //move back to origin view
                cameraTimer.restart();
                mainCamera.setCamera(playModeLastCamera);
                lastCamera.setCamera(interpCamera);

        }

        /*
        Vector cameraDisplace=Vector(0, 2.0 ,0);
        Vector camInterpPos = playRoadEdge->getInterpPosition(playRoadSegment, playRoadInterp) + cameraDisplace;
        Vector camInterpTangent = playRoadEdge->getInterpTangent(playRoadSegment,playRoadInterp);

        mainCamera.setTrackPosition(camInterpPos);
        mainCamera.followRollerCoaster(camInterpPos + camInterpTangent);

        playRoadInterp = float(playModeTimer.elapsed()) / 1000.0f;

        if (playRoadInterp>1.0f) {

            playRoadInterp=0.0f;
            playModeTimer.restart();

            playRoadSegment++;

            if (playRoadSegment >= playRoadEdge->getLastSegmentSelected()) {

                playRoadEnabled=false;
                cameraTimer.restart();
                lastCamera.setCamera(interpCamera);
                mainCamera.setCamera(playModeLastCamera);

            }

        }
        */

    }

    float camInterp = float(cameraTimer.elapsed()) / 1000.0f;
    interpCamera.interpBetweenCameras(lastCamera, mainCamera, camInterp);
    updateCamera(interpCamera);
    //updateCamera(mainCamera);

    heightMap->Draw(); //display heightmap
    heightMap->drawWater();   

    glClear(GL_DEPTH_BUFFER_BIT);

    roadNetwork->draw(); //otherwise, display usual road network
    roadNetwork->drawObjects(); //otherwise, display usual road network   

    if (playRoadEnabled) {
        drawPlayModeCar();
    }

    if (twistLens->isVisible()) {

        glDisable(GL_TEXTURE_2D);
        breakoutLensShader->release();
        twistLens->draw(mainCamera);

    }

    glPopMatrix();

    //draw the breakout view (if active)
    if (breakoutView->isVisible()) {

        breakoutView->setupView();

        glViewport(breakoutView->getCentre().x-width()/2.0f,breakoutView->getCentre().y-height()/2.0f,width(),height());

        glPushMatrix();

        updateCamera(breakoutView->getCamera());
        heightMap->Draw();
        roadNetwork->draw();
        roadNetwork->drawObjects();

        glPopMatrix();
        glViewport(0,0,width(),height());

        breakoutView->tearDownView();

    }

    //display lasso    
    if (breakoutView->isVisible()) {
        if (breakoutView->getIconSelected()==0)
            lassoMenu->drawAsOpen(mouse_x, mouse_y);
    }
    else if (twistLens->isVisible()) {
        if (twistLens->getIconSelected()==0)
            lassoMenu->drawAsOpen(mouse_x, mouse_y);
    }
    else {
        lassoMenu->draw(mouse_x, height() - mouse_y);
    }

    //display other debugging stuff
    /*
	glClear(GL_DEPTH_BUFFER_BIT);
	char *infostr=new char[150];
	sprintf_s(infostr,150,"FPS: %i Pickray: (%.1f,%.1f,%.1f)",(int)floor(1000.0f/(float)timeDiffms),lastpickray.x,lastpickray.y,lastpickray.z);
	DrawText(5, 5, infostr, 1.0, 1.0, 1.0);	
	Vector curTrackPos=interpCamera->getTrackPosition();	
	Vector curPos=interpCamera->getPosition();	
	sprintf_s(infostr,150,"TrackPos: (%.1f,%.1f,%.1f) Pos: (%.1f,%.1f,%.1f) TrackDist: %f Spin: %.1f Tilt: %.1f",
		curTrackPos.x,curTrackPos.y,curTrackPos.z,curPos.x,curPos.y,curPos.z,
		interpCamera->getTrackDist(),interpCamera->getSpin(),interpCamera->getTilt());
	DrawText(5, 15, infostr, 1.0, 1.0, 1.0);	
	*/

    glFinish();
    glErr = glGetError();
    while (glErr != GL_NO_ERROR) {
        qDebug("Theres error: GL Error # %i (%s) \n", glErr, gluErrorString(glErr));
        glErr = glGetError();
    }
    
}

void Viewer::mouseMoveEvent(QMouseEvent * e) {

    makeCurrent();

    int x = e->pos().x();
    int y = e->pos().y();

    int mouse_x_diff = mouse_x - x;
    int mouse_y_diff = mouse_y - y;
    mouse_x=x;
    mouse_y=y;

    //modify spin/tilt accordingly    	
    if (e->buttons() == Qt::LeftButton && m_bActiveAlt) {

        mainCamera.setSpin(mainCamera.getSpin() - mouse_x_diff);
        mainCamera.setTilt(mainCamera.getTilt() - mouse_y_diff);

    }
    else if (e->buttons() == Qt::LeftButton) {

        glPushMatrix();

        if (breakoutView->isVisible())
            updateCamera(breakoutView->getCamera());
        else
            updateCamera(mainCamera);

        updatePickray();

        if (twistLens->isVisible()) {
            twistLens->mouseMotion(x,y);
            lassoMenu->addPoint(Vector(x,screenHeight-y,0),lastpickray);

        }
        else if (breakoutView->isVisible()) {
            if (breakoutView->getIconSelected()==0) {

                lassoMenu->addPoint(Vector(x,screenHeight-y,0),lastpickray);

            }
            else if (breakoutView->getIconSelected()==1) {
                breakoutView->mouseMotion(mouse_x,mouse_y);
            }
        }
        else {

            if (!lassoMenu->isClosed()&&!lassoMenu->getPieMenuVisible()) {
                lassoMenu->addPoint(Vector(x,screenHeight-y,0),lastpickray);
            }
            else if (lassoMenu->isClosed()&&!lassoMenu->getPieMenuVisible()) {
                setupTheMenu();
                glPushMatrix();
                updateCamera(mainCamera);
                vector <Vector> lassoPoints;
                lassoMenu->getLassoPoints(lassoPoints);
                lassoPoints3D = heightMap->selectWithRaysUseDepthBuffer(mainCamera.getPosition(), lassoPoints);
                glPopMatrix();
            }

            if (lassoMenu->getPieMenuVisible()) {

                if (lassoMenu->getButtonChanged()) {

                    char *buttonText=lassoMenu->getSelectedButtonText();

                    delete previewRoadNetwork;
                    previewRoadNetwork=roadNetwork->getCloneNetwork(heightMap);

                    if (strcmp(buttonText,"Sparse")==0||
                        strcmp(buttonText,"Dense")==0) {

                        if (strcmp(buttonText,"Sparse")==0)
                            previewRoadNetwork->addFoliage(lassoPoints3D, 1.0f);
                        else
                            previewRoadNetwork->addFoliage(lassoPoints3D, 6.0f);
                    }
                    else if (strcmp(buttonText,"Delete Road")==0||
                             strcmp(buttonText,"Delete Roads")==0) {
                        previewRoadNetwork->doDeleteRoads();
                    }
                    else if (strcmp(buttonText,"Twisty")==0) {
                        previewRoadNetwork->getMostSelectedEdge()->makeTwisty();
                        previewRoadNetwork->doPrepareTerrain(previewRoadNetwork->getMostSelectedEdge(), heightMap);
                    }
                    else if (strcmp(buttonText,"Straight")==0) {
                        previewRoadNetwork->getMostSelectedEdge()->makeStraight();
                        previewRoadNetwork->doPrepareTerrain(previewRoadNetwork->getMostSelectedEdge(), heightMap);
                    }
                    else if (strcmp(buttonText,"Intersect")==0||
                             strcmp(buttonText,"Over- Under")==0||
                             strcmp(buttonText,"Under-Over")==0) {

                        previewRoadNetwork->changeIntersect(buttonText, heightMap);

                    }
                    else if (strcmp(buttonText,"Slow")==0||
                             strcmp(buttonText,"Average")==0||
                             strcmp(buttonText,"Fast")==0) {
                        previewRoadNetwork->changeTiming(buttonText);
                    }

                }

            }

        }
        glPopMatrix();

    }
    else if (mouse_b2 && m_bActiveAlt) {

        mainCamera.setTrackDist(mainCamera.getTrackDist() * pow(1.02f,mouse_y_diff));
        mainCamera.updateTrackPosition();

    }
    else if (mouse_b3 && m_bActiveAlt) {

        Vector xPlane;
        mainCamera.updateTrackPosition();
        xPlane.CrossProduct(Vector(0.0f,1.0f,0.0f) ,mainCamera.getTrackPosition() - mainCamera.getPosition());
        Vector yPlane=Vector(0.0f,1.0f,0.0f);
        yPlane=yPlane.GetRotatedAxis(mainCamera.getTilt(),xPlane);
        xPlane.Normal();
        yPlane.Normal();

        mainCamera.setTrackPosition(mainCamera.getTrackPosition() - xPlane * mouse_x_diff - yPlane * mouse_y_diff);
        mainCamera.setPosition(mainCamera.getPosition() - xPlane * mouse_x_diff - yPlane * mouse_y_diff);

    }
    
}

void Viewer::keyPressEvent(QKeyEvent * e)
{

    if (e->key() == Qt::Key_Shift) {
        m_bActiveAlt = true;
    }

}

void Viewer::keyReleaseEvent(QKeyEvent * e)
{

    if (e->key() == Qt::Key_Shift)
        m_bActiveAlt = false;

    if (e->key() == Qt::Key_Escape) {

        breakoutView->setVisible(false);

        if (twistLens->isVisible()) {
            twistLens->setVisible(false);
        }

        playRoadEnabled=false;

    }

    if (e->key() == Qt::Key_C) {
        roadNetwork->export_COLLADA("test.dae");
    }

    if (e->key() == Qt::Key_T) {
        roadNetwork->export_COLLADA("terrainTest.dae",heightMap);
    }


}

void Viewer::resizeGL(int width, int height)
{

    screenWidth=width;
    screenHeight=height;

    //reset position of twist lens
    if (twistLens->isVisible()) {
        twistLens->setVisible(false);
    }
    breakoutView->setVisible(false);

    const float ar = (float) width / (float) height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //reset the perspective
    setPerspective();

}

void Viewer::mousePressEvent(QMouseEvent * e)
{

    makeCurrent();

    int x = e->pos().x();
    int y = e->pos().y();

    mouse_x=x;
    mouse_y=y;

    if (e->button() == Qt::LeftButton) {

        mouse_b1=true;      

        if (twistLens->isVisible()) {
            twistLens->mouseClick(mouse_x, mouse_y, mouse_b1, roadNetwork, heightMap);
            roadNetwork->doObjects();
        }
        if (breakoutView->isVisible()) {
            breakoutView->mouseClick(mouse_x,mouse_y,mouse_b1);
        }

    }
    else if (e->button() == Qt::RightButton)
        mouse_b2 = true;
    else if (e->button() == Qt::MiddleButton)
        mouse_b3 = true;

}

void Viewer::mouseReleaseEvent(QMouseEvent * e)
{

    makeCurrent();

    int x = e->pos().x();
    int y = e->pos().y();

    mouse_x=x;
    mouse_y=y;

    if (e->button() == Qt::LeftButton) {

        mouse_b1=false;

        if (twistLens->isVisible()) {
            //perhaps the user wants to edit the height of a road
            glPushMatrix();
            updateCamera(mainCamera);

            twistLens->mouseDragRelease(lassoMenu->getAllPickrays(), roadNetwork, heightMap, mainCamera);
            twistLens->mouseClick(x,y,mouse_b1, roadNetwork, heightMap);
            if (twistLens->getIconSelected()==0) {
                roadNetwork->doObjects();
            }
            glPopMatrix();
        }
        else if (breakoutView->isVisible()) {

            if (breakoutView->getIconSelected()==0) {

                //construct 2d and 3d pointsets
                vector <Vector> set2d;
                vector <Vector> set3d;
                Vector *eachVec;

                glPushMatrix();
                updateCamera(breakoutView->getCamera());

                for (int i=0;i<lassoMenu->getNumPoints();i++) {
                    eachVec=breakoutView->rayIntoView(lassoMenu->getPickray(i));
                    if (eachVec!=NULL) {

                        Vector xShifted=lassoMenu->getPoint(i);
                        xShifted.x-=breakoutView->getCentre().x-width()/2.0f;

                        set2d.push_back(xShifted);
                        set3d.push_back(Vector(eachVec->x,eachVec->y,eachVec->z));
                    }
                }

                breakoutView->getSelectedEdge()->replaceHeightOfSelected(set2d, set3d);
                glPopMatrix();

                set2d.clear();
                set3d.clear();

                roadNetwork->doObjects();

            }
            else if (breakoutView->getIconSelected()==1) {
                breakoutView->mouseClick(mouse_x,mouse_y,mouse_b1);
            }

        }
        else if (!lassoMenu->getPieMenuVisible()) {

            //draw a road in this case
            if (lassoMenu->getNumPoints()>=2) {
                //lassoPoints3D=heightMap->selectWithRays(mainCamera->getPosition(), lassoMenu->getAllPickrays());

                //put camera into right mode
                //pass in lassoPoints2D, use unprojected points...

                glPushMatrix();
                updateCamera(mainCamera);
                lassoPoints3D=heightMap->selectWithRaysUseDepthBuffer(mainCamera.getPosition(), lassoMenu->getAllPoints());
                glPopMatrix();

                if (lassoPoints3D.size()>=2) {
                    roadNetwork->addRoad(lassoPoints3D, heightMap);
                }
            }

        }
        else {

            char *buttonText=lassoMenu->getSelectedButtonText();

            if (strcmp(buttonText,"Lens")==0) {

                if (roadNetwork->getSelectedEdges().size()>0) {

                    glPushMatrix();
                    updateCamera(mainCamera);

                    twistLens->setOnSelectedEdge(roadNetwork->getMostSelectedEdge());
                    glPopMatrix();

                    twistLens->setVisible(true);
                }

            }
            else if (strcmp(buttonText,"View")==0) {
                vector <Vector> lassoPoints;
                lassoMenu->getLassoPoints(lassoPoints);
                breakoutView->newBreakoutView(lassoPoints, roadNetwork, mainCamera);
                breakoutView->setVisible(true);
            }
            else if (strcmp(buttonText, "Play") == 0) {

                qDebug() << "selected play";
                RN_Edge * edge = roadNetwork->getMostSelectedEdge();

                if (edge != NULL) {

                    playRoadEnabled = true;
                    playRoadSegment = edge->getFirstSegmentSelected();
                    playRoadEdge = edge;
                    playRoadInterp = 0.0;

                    playModeLastCamera.setCamera(mainCamera);
                    lastCamera.setCamera(interpCamera);
                    cameraTimer.restart();
                    playModeTimer.restart();
                    playModeStartTimer.restart();

                }
                else {
                    playRoadEnabled = false;
                }

            }
            else if (strcmp(buttonText,"Close")==0||
                     strcmp(buttonText,"Midway")==0||
                     strcmp(buttonText,"Bird's Eye")==0) {

                glPushMatrix();
                updateCamera(mainCamera);
                Vector centroidPoint=lassoMenu->getLassoCentroid();
                mouse_x=centroidPoint.x;
                mouse_y=height()-centroidPoint.y;
                updatePickray();
                glPopMatrix();

                Vector heightMapSelectPoint;

                if (heightMap->selectWithRay(mainCamera.getPosition(), lastpickray, heightMapSelectPoint)) { //only if view is on terrain should we do anything

                    lastCamera = mainCamera;
                    interpCamera = mainCamera;
                    cameraTimer.restart();

                    mainCamera.setTrackPosition(heightMapSelectPoint+Vector(0.0f,MIN_DIST_BETWEEN_POINTS,0.0f));

                    if (strcmp(buttonText,"Close")==0) {
                        mainCamera.setTilt(0.0f);
                        mainCamera.setTrackDist(1.0f);
                    }
                    else if (strcmp(buttonText,"Midway")==0) {
                        mainCamera.setTilt(45.0f);
                        mainCamera.setTrackDist(20.0f);
                    }
                    else if (strcmp(buttonText,"Bird's Eye")==0) {
                        mainCamera.setTilt(90.0f);
                        mainCamera.setTrackDist(50.0f);
                    }                                        

                }
            }
            else if (strcmp(buttonText,"Free")==0) {
                    playRoadCamMode=0;
                    cameraTimer.restart();
                    lastCamera.setCamera(interpCamera);
            }
            else if (strcmp(buttonText,"Helicopter")==0) {
                    playRoadCamMode=1;
                    cameraTimer.restart();
                    lastCamera.setCamera(interpCamera);
            }
            else if (strcmp(buttonText,"Hood")==0) {
                    playRoadCamMode=2;
                    cameraTimer.restart();
                    lastCamera.setCamera(interpCamera);
            }
            else if (strcmp(buttonText,"Behind")==0) {
                    playRoadCamMode=3;
                    cameraTimer.restart();
                    lastCamera.setCamera(interpCamera);
            }
            else if (strcmp(buttonText,"Inside")==0) {
                    playRoadCamMode=4;
                    cameraTimer.restart();
                    lastCamera.setCamera(interpCamera);
            }
            else if (strcmp(buttonText,"Stop")==0) {
                    playRoadEnabled=false;
                    //move back to origin view
                    cameraTimer.restart();
                    mainCamera.setCamera(playModeLastCamera);
                    lastCamera.setCamera(interpCamera);
            }
            else {
                //change roadnetwork to what's in preview
                roadNetwork=previewRoadNetwork;
                previewRoadNetwork=new RN(heightMap);
            }

        }

        //delete lasso list
        lassoMenu->clear();
        lassoPoints3D.clear();

    }
    else if (e->button() == Qt::RightButton)
        mouse_b2 = false;
    else if (e->button() == Qt::MiddleButton)
        mouse_b3 = false;

}

Camera & Viewer::getCamera() {
    return mainCamera;
}

void Viewer::getMouseXY(int &x, int &y) {
    x=mouse_x;
    y=mouse_y;
}

void Viewer::setupTheMenu() {

    vector <vector <char *> > pieMenuButtons;
    vector <char *> eachPieMenuButton;
    vector <vector <bool> > buttonEnabled;
    vector <bool> eachButtonEnabled;

    /*
    if (IsKeyPressed('s')||IsKeyPressed('S')) {

        eachButtonEnabled.push_back(true);

        eachPieMenuButton.push_back("Save Slot 1");
        pieMenuButtons.push_back(eachPieMenuButton);
        buttonEnabled.push_back(eachButtonEnabled);
        eachPieMenuButton.clear();
        eachPieMenuButton.push_back("Save Slot 2");
        pieMenuButtons.push_back(eachPieMenuButton);
        buttonEnabled.push_back(eachButtonEnabled);
        eachPieMenuButton.clear();
        eachPieMenuButton.push_back("Save Slot 3");
        pieMenuButtons.push_back(eachPieMenuButton);
        buttonEnabled.push_back(eachButtonEnabled);
        eachPieMenuButton.clear();
        eachPieMenuButton.push_back("Save Slot 4");
        pieMenuButtons.push_back(eachPieMenuButton);
        buttonEnabled.push_back(eachButtonEnabled);
        eachPieMenuButton.clear();

        lassoMenu->setPieMenuButtons(pieMenuButtons,  buttonEnabled);
        lassoMenu->setPieMenuVisible(true);
        lassoMenu->setPieMenuColour(Vector(0.5,0.1,0.0),Vector(1.0,0.4,0.3));
        lassoMenu->setUseSpecialLabels(0);

        return;

    }


    if (IsKeyPressed('l')||IsKeyPressed('L')) {

        eachButtonEnabled.push_back(true);

        eachPieMenuButton.push_back("Load Slot 1");
        pieMenuButtons.push_back(eachPieMenuButton);
        buttonEnabled.push_back(eachButtonEnabled);
        eachPieMenuButton.clear();
        eachPieMenuButton.push_back("Load Slot 2");
        pieMenuButtons.push_back(eachPieMenuButton);
        buttonEnabled.push_back(eachButtonEnabled);
        eachPieMenuButton.clear();
        eachPieMenuButton.push_back("Load Slot 3");
        pieMenuButtons.push_back(eachPieMenuButton);
        buttonEnabled.push_back(eachButtonEnabled);
        eachPieMenuButton.clear();
        eachPieMenuButton.push_back("Load Slot 4");
        pieMenuButtons.push_back(eachPieMenuButton);
        buttonEnabled.push_back(eachButtonEnabled);
        eachPieMenuButton.clear();

        lassoMenu->setPieMenuButtons(pieMenuButtons, buttonEnabled);
        lassoMenu->setPieMenuVisible(true);
        lassoMenu->setPieMenuColour(Vector(0.0,0.5,0.2),Vector(0.3,1.0,0.5));
        lassoMenu->setUseSpecialLabels(0);

        return;
    }
    */

    if (playRoadEnabled) {

        eachPieMenuButton.push_back("Free");
        if (playRoadCamMode==0)
            eachButtonEnabled.push_back(false);
        else
            eachButtonEnabled.push_back(true);
        pieMenuButtons.push_back(eachPieMenuButton);
        buttonEnabled.push_back(eachButtonEnabled);
        eachPieMenuButton.clear();
        eachButtonEnabled.clear();

        eachPieMenuButton.push_back("Helicopter");
        if (playRoadCamMode==1)
            eachButtonEnabled.push_back(false);
        else
            eachButtonEnabled.push_back(true);
        pieMenuButtons.push_back(eachPieMenuButton);
        buttonEnabled.push_back(eachButtonEnabled);
        eachPieMenuButton.clear();
        eachButtonEnabled.clear();

        eachPieMenuButton.push_back("Hood");
        if (playRoadCamMode==2)
            eachButtonEnabled.push_back(false);
        else
            eachButtonEnabled.push_back(true);
        pieMenuButtons.push_back(eachPieMenuButton);
        buttonEnabled.push_back(eachButtonEnabled);
        eachPieMenuButton.clear();
        eachButtonEnabled.clear();

        eachPieMenuButton.push_back("Behind");
        if (playRoadCamMode==3)
            eachButtonEnabled.push_back(false);
        else
            eachButtonEnabled.push_back(true);
        pieMenuButtons.push_back(eachPieMenuButton);
        buttonEnabled.push_back(eachButtonEnabled);
        eachPieMenuButton.clear();
        eachButtonEnabled.clear();

        eachPieMenuButton.push_back("Inside");
        if (playRoadCamMode==4)
            eachButtonEnabled.push_back(false);
        else
            eachButtonEnabled.push_back(true);
        pieMenuButtons.push_back(eachPieMenuButton);
        buttonEnabled.push_back(eachButtonEnabled);
        eachPieMenuButton.clear();
        eachButtonEnabled.clear();

        eachPieMenuButton.push_back("Stop");
        eachButtonEnabled.push_back(true);
        pieMenuButtons.push_back(eachPieMenuButton);
        buttonEnabled.push_back(eachButtonEnabled);
        eachPieMenuButton.clear();
        eachButtonEnabled.clear();

        lassoMenu->setPieMenuButtons(pieMenuButtons, buttonEnabled);
        lassoMenu->setPieMenuVisible(true);
        lassoMenu->setPieMenuColour(Vector(0.3,0.15,0.3),Vector(0.6,0.3,0.6));
        lassoMenu->setUseSpecialLabels(2);

        return;

    }

    if (lassoMenu->getNumPoints()>2) {
        vector <Vector> lassoPoints;
        lassoMenu->getLassoPoints(lassoPoints);
        roadNetwork->selectWithClosedCurve2D(lassoPoints);
    }
    else {
        roadNetwork->unSelectAll();
    }

    vector <RN_Edge *> selectedEdges=roadNetwork->getSelectedEdges();
    vector <RN_Intersection *> selectedIntersects=roadNetwork->getSelectedIntersects();

    eachPieMenuButton.push_back("Timing");
    if (selectedEdges.size()>=1)
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    eachPieMenuButton.push_back("Slow");
    if (selectedEdges.size()>=1)
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    eachPieMenuButton.push_back("Average");
    if (selectedEdges.size()>=1)
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    eachPieMenuButton.push_back("Fast");
    if (selectedEdges.size()>=1)
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    pieMenuButtons.push_back(eachPieMenuButton);
    buttonEnabled.push_back(eachButtonEnabled);
    eachPieMenuButton.clear();
    eachButtonEnabled.clear();

    eachPieMenuButton.push_back("Foliage");
    eachPieMenuButton.push_back("Sparse");
    eachPieMenuButton.push_back("Dense");
    if (lassoMenu->getNumPoints()>2) {
        eachButtonEnabled.push_back(true);
        eachButtonEnabled.push_back(true);
        eachButtonEnabled.push_back(true);
    }
    else {
        eachButtonEnabled.push_back(false);
        eachButtonEnabled.push_back(false);
        eachButtonEnabled.push_back(false);
    }
    pieMenuButtons.push_back(eachPieMenuButton);
    buttonEnabled.push_back(eachButtonEnabled);
    eachPieMenuButton.clear();
    eachButtonEnabled.clear();

    eachPieMenuButton.push_back("View");
    eachButtonEnabled.push_back(true);
    eachPieMenuButton.push_back("Close");
    eachButtonEnabled.push_back(true);
    eachPieMenuButton.push_back("Midway");
    eachButtonEnabled.push_back(true);
    eachPieMenuButton.push_back("Bird's Eye");
    eachButtonEnabled.push_back(true);
    pieMenuButtons.push_back(eachPieMenuButton);
    buttonEnabled.push_back(eachButtonEnabled);
    eachPieMenuButton.clear();
    eachButtonEnabled.clear();

    if (selectedEdges.size()==1)
        eachPieMenuButton.push_back("Delete Road");
    else
        eachPieMenuButton.push_back("Delete Roads");
    pieMenuButtons.push_back(eachPieMenuButton);
    eachPieMenuButton.clear();
    if (selectedEdges.size()>=1)
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    buttonEnabled.push_back(eachButtonEnabled);
    eachButtonEnabled.clear();

    eachPieMenuButton.push_back("Breakout");
    if (selectedEdges.size()>=1) //single road options
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    eachPieMenuButton.push_back("View");
    if (selectedEdges.size()>=1) //single road options
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    eachPieMenuButton.push_back("Lens");
    if (selectedEdges.size()>=1) //single road options
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    pieMenuButtons.push_back(eachPieMenuButton);
    buttonEnabled.push_back(eachButtonEnabled);
    eachPieMenuButton.clear();
    eachButtonEnabled.clear();

    eachPieMenuButton.push_back("Crossing");
    eachPieMenuButton.push_back("Over- Under");
    eachPieMenuButton.push_back("Intersect");
    eachPieMenuButton.push_back("Under-Over");
    pieMenuButtons.push_back(eachPieMenuButton);
    eachPieMenuButton.clear();

    if (selectedIntersects.size()==1)
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    //if (selectedIntersects.size()==1&&selectedIntersects[0]->getRelation()!=RELATION_OVERUNDER)
    if (selectedIntersects.size()==1)
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    //if (selectedIntersects.size()==1&&selectedIntersects[0]->getRelation()!=RELATION_INTERSECT)
    if (selectedIntersects.size()==1)
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    //if (selectedIntersects.size()==1&&selectedIntersects[0]->getRelation()!=RELATION_UNDEROVER)
    if (selectedIntersects.size()==1)
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    buttonEnabled.push_back(eachButtonEnabled);
    eachButtonEnabled.clear();

    eachPieMenuButton.push_back("Path");
    if (selectedEdges.size()>=1) //single road options
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    eachPieMenuButton.push_back("Twisty");
    if (selectedEdges.size()>=1) //single road options
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    eachPieMenuButton.push_back("Straight");
    if (selectedEdges.size()>=1) //single road options
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    pieMenuButtons.push_back(eachPieMenuButton);
    buttonEnabled.push_back(eachButtonEnabled);
    eachPieMenuButton.clear();
    eachButtonEnabled.clear();

    eachPieMenuButton.push_back("Play");
    if (selectedEdges.size()>=1) //single road options
        eachButtonEnabled.push_back(true);
    else
        eachButtonEnabled.push_back(false);
    pieMenuButtons.push_back(eachPieMenuButton);
    buttonEnabled.push_back(eachButtonEnabled);
    eachPieMenuButton.clear();
    eachButtonEnabled.clear();

    lassoMenu->setPieMenuButtons(pieMenuButtons, buttonEnabled);
    lassoMenu->setPieMenuVisible(true);
    lassoMenu->setPieMenuColour(Vector(0.0,0.2,0.5),Vector(0.3,0.5,1.0));
    lassoMenu->setUseSpecialLabels(1);

    delete previewRoadNetwork;
    previewRoadNetwork=roadNetwork->getCloneNetwork(heightMap);

}

void Viewer::drawPlayModeCar() {


    glEnable(GL_TEXTURE_2D);
    //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat white_light[]={1.0f,1.0f,1.0f};

    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);

    GLfloat lmodel_ambient[]={0.75,0.75,0.75,1.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, lmodel_ambient);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

    glPushMatrix();

    if (playRoadCamMode==4 && cameraTimer.elapsed() > 1000) {
        glTranslatef(carPosition.x,carPosition.y+0.36f*MIN_DIST_BETWEEN_POINTS,carPosition.z);
        glRotatef(-atan2(carDirection.z,carDirection.x)*180.0/M_PI+90.0f,0.0f,1.0f,0.0f);
        glTranslatef(-MIN_DIST_BETWEEN_POINTS/4.0f,0.0f,0.0f);

        float carTilt=-RadToDeg(asinf(carDirection.y));
        glRotatef(carTilt,1.0f,0.0f,0.0f);

        glScalef(MIN_DIST_BETWEEN_POINTS,MIN_DIST_BETWEEN_POINTS,MIN_DIST_BETWEEN_POINTS);

        playRoadInterior->Draw();
        playRoadInteriorSeats->Draw();

        glTranslatef(0.075f,-0.04f,0.075f);
        glRotatef((playRoadEdge->getCurvature(playRoadSegment)*(1.0f-playRoadInterp)+
                   playRoadEdge->getCurvature(playRoadSegment+1)*playRoadInterp)*180.0f*6.0f,0.0f,0.0f,1.0f);
        playRoadSteeringWheel->Draw();

        glPopMatrix();
        return;
    }
    else {
        glTranslatef(carPosition.x,carPosition.y+0.1f*MIN_DIST_BETWEEN_POINTS,carPosition.z);
        glRotatef(-atan2(carDirection.z,carDirection.x)*180.0/M_PI+90.0f,0.0f,1.0f,0.0f);
        glTranslatef(-MIN_DIST_BETWEEN_POINTS/4.0f,0.0f,0.0f);

        float carTilt=-RadToDeg(asinf(carDirection.y));
        glRotatef(carTilt,1.0f,0.0f,0.0f);

        glScalef(MIN_DIST_BETWEEN_POINTS,MIN_DIST_BETWEEN_POINTS,MIN_DIST_BETWEEN_POINTS);
        playRoadCar->Draw();
    }

    float tireXmove=0.135f;
    float tireZmove=0.21f;
    float tireZoffset=0.02f;

    glTranslatef(0.0f,0.05f,0.0f);
    for (int tire=0;tire<4;tire++) {
        glPushMatrix();
        if (tire<2)
            glRotatef(180.0f,0.0f,1.0f,0.0f);

        glTranslatef(-tireXmove,0.0f,0.0f);
        if (tire<2)
            glTranslatef(0.0f,0.0f,-tireZoffset);
        else
            glTranslatef(0.0f,0.0f,tireZoffset);

        if (tire==0||tire==2)
            glTranslatef(0.0f,0.0f,-tireZmove);
        else
            glTranslatef(0.0f,0.0f,tireZmove);

        //turning of front tires
        if (tire==0||tire==3)
            glRotatef(-(playRoadEdge->getCurvature(playRoadSegment)*(1.0f-playRoadInterp)+
                        playRoadEdge->getCurvature(playRoadSegment+1)*playRoadInterp)*180.0f*0.75f,0.0f,1.0f,0.0f);

        //rotation of tires
        //tire 0 front left
        //tire 1 rear left
        //tire 2 front right
        //tire 3 rear right
        if (tire==0||tire==1)
            glRotatef(-carWheelRotate,1.0f,0.0f,0.0f);
        else
            glRotatef(carWheelRotate,1.0f,0.0f,0.0f);

        playRoadTire->Draw();
        glPopMatrix();

    }

    glPopMatrix();

    glDisable(GL_LIGHTING);


}
