#include "TwistLens.h"

TwistLens::TwistLens(QWidget * viewer)
{

    m_pViewer = viewer;

    m_lensCentre=Vector((GLfloat)m_pViewer->width()/2.0f,
                      (GLfloat)m_pViewer->height()/2.0f,
                      0.0f);

    m_outerRadius=(float)m_pViewer->width()/6.0f;
    m_innerRadius=(float)m_pViewer->width()/8.0f;
    m_viewTwist=0.0f;

    int w, h;
    texture_translate=TextureLoader::LoadTexture("menuicons/twistlens_translate.png", w, h);
    texture_radius=TextureLoader::LoadTexture("menuicons/twistlens_radius.png", w, h);
    texture_twist=TextureLoader::LoadTexture("menuicons/twistlens_twist.png", w, h);
    texture_close=TextureLoader::LoadTexture("menuicons/twistlens_close.png", w, h);

    iconSelected=0;

    visible=false;

}

void TwistLens::draw(Camera  & camera) {

    //capture the current projection/modelview matrices,
    //as we must enter orthogonal projection mode
    GLdouble model_view[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    //enter orthogonal mode
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (float)m_pViewer->width(),
            0.0, (GLfloat)m_pViewer->height(), -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //draw the concentric circles
    const float fIncr = (2.0f*M_PI)/50.0f;

    glColor3f(0.95f,0.95f,0.95f);
    glLineWidth(3.0f);
    glTranslatef(m_lensCentre.x,m_lensCentre.y,0.0f);

    glLineStipple(1,0xF0F0);
    glLineWidth(3.0);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINE_LOOP);

    for (float i=0.0f ;i<2.0f*M_PI; i+=fIncr)
        glVertex2f(cos(i)*m_outerRadius,sinf(i)*m_outerRadius);

    glEnd();

    glDisable(GL_LINE_STIPPLE);
    glBegin(GL_LINE_LOOP);

    for (float i=0.0f; i<2.0f*M_PI; i+=fIncr)
        glVertex2f(cos(i)*m_innerRadius,sinf(i)*m_innerRadius);

    glEnd();

    //relative to lens centre!
    drawIcon(translateIconCentre().x,translateIconCentre().y,texture_translate);
    drawIcon(outerRadiusIconCentre().x,outerRadiusIconCentre().y,texture_radius);
    drawIcon(innerRadiusIconCentre().x,innerRadiusIconCentre().y,texture_radius);
    drawIcon(twistIconCentre().x,twistIconCentre().y,texture_twist);
    drawIcon(closeIconCentre().x,closeIconCentre().y,texture_close); 

    //now, enable stencil buffer, and draw only the region of the inner circle
    //for the sky showing thing
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    //draw the breakout stencil (again from position at centre)
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 1);
    glDepthMask( GL_FALSE );					// Turn Off Writing To The Depth-Buffer
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
    glEnable(GL_STENCIL_TEST);

    glBegin(GL_TRIANGLE_FAN);
    for (float i=0.0f;i<=2.0f*M_PI;i+=fIncr) {
        glVertex2f(cosf(i)*m_innerRadius, sinf(i)*m_innerRadius);
    }
    glEnd();

    //now reset the projection/modelview matrices to what was there before
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(projection);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(model_view);

    //now draw the sky showing plane
    glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
    glStencilFunc(GL_EQUAL, 1, 1);

    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


    //REMOVED FOR GEOMETRY MODELLING EXAMPLE
    //if (IsKeyPressed('p'))
    drawTwistPlane(determineSkyPlaneOffset(), camera);

    glDisable(GL_STENCIL_TEST);


}

float TwistLens::determineSkyPlaneOffset() {

    Vector distAxis;
    distAxis.CrossProduct(m_axisOfRotation, Vector(0.0f,1.0f,0.0f));
    distAxis.Normal();

    if (m_viewTwist<0.0f)
        distAxis=distAxis*(-1.0f);

    float maxDist=0.0f;

    for (int i=selectedEdge->getFirstSegmentSelected(); i<=selectedEdge->getLastSegmentSelected(); ++i) {
        float eachDist=distAxis.DotProduct3(selectedEdge->getPoint(i) - m_pointOfRotation);
        if (eachDist>maxDist)
            maxDist=fabs(eachDist);
    }

    return maxDist * 2.0f + MIN_DIST_BETWEEN_POINTS;
}

Vector TwistLens::translateIconCentre() {
    return Vector(cosf(M_PI/4.0f) * m_outerRadius + 20.0f, sinf(M_PI/4.0f) * m_outerRadius + 20.0f,0.0f);
}

Vector TwistLens::outerRadiusIconCentre() {
    return Vector(m_outerRadius,0.0f,0.0f);
}

Vector TwistLens::innerRadiusIconCentre() {
    return Vector(m_innerRadius,0.0f,0.0f);
}

Vector TwistLens::twistIconCentre() {
    return Vector(cosf(M_PI/2.0f + m_viewTwist * M_PI / 180.0f) * (m_innerRadius + 30.0f),
                  sinf(M_PI/2.0f + m_viewTwist * M_PI / 180.0f) * (m_innerRadius + 30.0f),
                  0.0f);
}

Vector TwistLens::closeIconCentre() {
    return translateIconCentre()+Vector(TWIST_ICON_RADIUS*2.0f+5.0f,0.0f,0.0f);
}

void TwistLens::drawIcon(float xtrans, float ytrans, int textureIndex) {
    //display control icons
    glColor3f(1.0f,1.0f,1.0f);
    glPushMatrix();
    glTranslatef(xtrans-TWIST_ICON_RADIUS,ytrans-TWIST_ICON_RADIUS,0.0f);

    glBindTexture (GL_TEXTURE_2D, textureIndex);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0);
    glVertex2f(0,0);
    glTexCoord2f(0,1);
    glVertex2f(0,TWIST_ICON_RADIUS*2.0f);
    glTexCoord2f(1,1);
    glVertex2f(TWIST_ICON_RADIUS*2.0f,TWIST_ICON_RADIUS*2.0f);
    glTexCoord2f(1,0);
    glVertex2f(TWIST_ICON_RADIUS*2.0f,0);
    glEnd();
    glPopMatrix();
}

void TwistLens::mouseMotion(int x, int y) {

    float xf=(float)x;
    float yf=(float)m_pViewer->height()-(float)y;

    float centreDist = (Vector(xf,yf,0.0f)-m_lensCentre).GetLength();
    float theAngle = atan2(yf - m_lensCentre.y,xf - m_lensCentre.x);

    switch (iconSelected) {

    case 1:

        m_lensCentre=Vector(xf-translateIconCentre().x,yf-translateIconCentre().y,0.0f);

        break;

    case 2:

        if(centreDist > m_innerRadius + TWIST_ICON_RADIUS)
            m_outerRadius=centreDist;

        break;

    case 3:

        if(centreDist > 15.0f && centreDist < m_outerRadius - TWIST_ICON_RADIUS)
            m_innerRadius=centreDist;

        break;

    case 4:

        theAngle=theAngle*180.0f/M_PI-90.0f;

        if (theAngle>-90.0f&&theAngle<90.0f)
            m_viewTwist=theAngle;

        break;

    }

}

bool TwistLens::clickedIcon(int x, int y, Vector cent, float rad) {

    return x > m_lensCentre.x + cent.x - rad &&
            x < m_lensCentre.x + cent.x + rad &&
            y > m_lensCentre.y + cent.y - rad &&
            y < m_lensCentre.y + cent.y + rad;

}

void TwistLens::mouseClick(int x, int y, bool down, RN *roadNetwork, HeightMap *heightMap) {

    //iconselected=1 - translate
    //iconselected=2 - outer radius
    //iconselected=3 - inner radius

    float xf=(float)x;
    float yf=(float)m_pViewer->height()-(float)y;

    iconSelected=0;

    if (down) {

        if (clickedIcon(xf, yf, translateIconCentre(), TWIST_ICON_RADIUS))
            iconSelected=1;
        else if (clickedIcon(xf, yf, outerRadiusIconCentre(), TWIST_ICON_RADIUS))
            iconSelected=2;
        else if (clickedIcon(xf, yf, innerRadiusIconCentre(), TWIST_ICON_RADIUS))
            iconSelected=3;
        else if (clickedIcon(xf, yf, twistIconCentre(), TWIST_ICON_RADIUS))
            iconSelected=4;
        else if (clickedIcon(xf, yf, closeIconCentre(), TWIST_ICON_RADIUS)) {
            iconSelected=5;
            visible=false;
        }

        qDebug() << iconSelected;

    }

}

int TwistLens::getIconSelected() {

    return iconSelected;

}

void TwistLens::mouseDragRelease(vector <Vector> lassoPickrays, RN *roadNetwork, HeightMap *heightMap, Camera & mainCamera) {

    if (iconSelected==1||iconSelected==3) { //reselect after movement of lens, or resizing of inner lens radius

        vector <Vector> selectLasso;
        selectLasso.push_back(Vector(m_lensCentre.x, m_lensCentre.y,0.0f));
        const float fIncr = (2.0f*M_PI)/50;
        for (float i=0.0f; i <= 2.1f*M_PI; i += fIncr) {
            Vector p = Vector(cosf(i)*m_innerRadius + m_lensCentre.x, sinf(i) * m_innerRadius + m_lensCentre.y, 0.0f);
            selectLasso.push_back(p);
        }

        selectedEdge->selectWithClosedCurve2D(selectLasso);
        //printf("numpointsselected: %i first: %i last: %i\n",selectedEdge->getNumPointsSelected(),
        //	selectedEdge->getFirstSegmentSelected(),selectedEdge->getLastSegmentSelected());
        selectLasso.clear();

        m_axisOfRotation=selectedEdge->getLastPointSelected() - selectedEdge->getFirstPointSelected();
        m_pointOfRotation=selectedEdge->getFirstPointSelected();


    }
    else if (iconSelected==0) { //update height of selected part of road

        if (lassoPickrays.size()<2)
            return;

        int firstIndex=selectedEdge->getFirstSegmentSelected();
        int lastIndex=selectedEdge->getLastSegmentSelected();

        if (firstIndex == -1 || lastIndex == -1 || firstIndex == lastIndex)
            return;

        //ADDED FOR GEOMETRIC MODELLING
        //RN_Edge *edgeBefore=selectedEdge->getClone();

        //printf("Updating points from %i to %i\n",firstIndex,lastIndex);
        Vector rot1 = Vector(0,-1000.0f,0).GetRotatedAxis(m_viewTwist,m_axisOfRotation);
        Vector rot2 = Vector(0, 1000.0f,0).GetRotatedAxis(m_viewTwist,m_axisOfRotation);

        Vector *planePoints=new Vector[4];
        planePoints[0]=m_pointOfRotation + rot1;
        planePoints[1]=m_pointOfRotation + rot2;
        planePoints[2]=m_pointOfRotation + m_axisOfRotation + rot1;
        planePoints[3]=m_pointOfRotation + m_axisOfRotation + rot2;

        vector <Vector *> lassoPlanePoints;

        for (int i=0;i<lassoPickrays.size();i++) {

            Vector * col1 = Collision::LineTriangleCollide(mainCamera.getPosition(), lassoPickrays[i], planePoints[0], planePoints[1], planePoints[2]);
            Vector * col2 = Collision::LineTriangleCollide(mainCamera.getPosition(), lassoPickrays[i], planePoints[1], planePoints[3], planePoints[2]);

            if (col1!=NULL)
                lassoPlanePoints.push_back(col1);
            else if (col2!=NULL)
                lassoPlanePoints.push_back(col2);

        }

        if (lassoPlanePoints.size()<2)
            return;

        vector <Vector *> roadPlanePoints;
        vector <int> roadPlanePointIndexes;

        for (int i=firstIndex; i<lastIndex; ++i) {

            //use pickray as difference between camera position and the WARPED pointset position
            Vector warpedPos=selectedEdge->getPoint(i);
            warpedPos=warpedPos - m_pointOfRotation;
            warpedPos=warpedPos.GetRotatedAxis(m_viewTwist, m_axisOfRotation);
            warpedPos=warpedPos + m_pointOfRotation;

            Vector *col1=Collision::LineTriangleCollide(mainCamera.getPosition(),(warpedPos-mainCamera.getPosition()).getNormal(),
                                                        planePoints[0],planePoints[1],planePoints[2]);
            Vector *col2=Collision::LineTriangleCollide(mainCamera.getPosition(),(warpedPos-mainCamera.getPosition()).getNormal(),
                                                        planePoints[1],planePoints[3],planePoints[2]);

            if (col1 != NULL) {//ASSUMING THAT ALL ROAD POINTS PROJECT ONTO ORTHO PLANE!
                roadPlanePoints.push_back(col1);
                roadPlanePointIndexes.push_back(i);
            }
            else if (col2 != NULL) {
                roadPlanePoints.push_back(col2);
                roadPlanePointIndexes.push_back(i);
            }

        }

        vector <Vector> lassoPlanePointsProjected; //points are projected onto an XY plane in their own space
        vector <Vector> roadPlanePointsProjected;

        for (int i=0; i < lassoPlanePoints.size(); ++i) {
            Vector p = projectPointOnTwistPlane(*lassoPlanePoints[i],selectedEdge->getPointOfRotation(), m_axisOfRotation);
            lassoPlanePointsProjected.push_back(p);
        }

        for (int i=0; i < roadPlanePoints.size(); ++i) {
            Vector p = projectPointOnTwistPlane(*roadPlanePoints[i],selectedEdge->getPointOfRotation(), m_axisOfRotation);
            roadPlanePointsProjected.push_back(p);
        }

        //sketch curve points and road points are in a local coordinate space, which is the ortho plane
        //to the twistlens

        //new y values for road points are linearly interpolated from the sketch curve points (x-wise)

        for (int i=0;i<roadPlanePointsProjected.size();i++) {
            for (int j=0;j<lassoPlanePointsProjected.size()-1;j++) {
                if (roadPlanePointsProjected[i].x>=lassoPlanePointsProjected[j].x&&
                    roadPlanePointsProjected[i].x<=lassoPlanePointsProjected[j+1].x) {
                    float interp=(roadPlanePointsProjected[i].x-lassoPlanePointsProjected[j].x)/
                                 (lassoPlanePointsProjected[j+1].x-lassoPlanePointsProjected[j].x);
                    float newyval=lassoPlanePointsProjected[j].y*(1.0-interp)+
                                  lassoPlanePointsProjected[j+1].y*interp;

                    selectedEdge->setPoint(roadPlanePointIndexes[i],selectedEdge->getPoint(roadPlanePointIndexes[i])+Vector(0.0f,newyval-roadPlanePointsProjected[i].y,0.0f));

                    //bridge or tunnel based on below/above terrain
                    Vector point1=selectedEdge->getPoint(roadPlanePointIndexes[i]);
                    Vector point2=selectedEdge->getPoint(roadPlanePointIndexes[i]+1);

                    if (point1.y<heightMap->getHeightAt(point1.x,point1.z)||
                        point2.y<heightMap->getHeightAt(point2.x,point2.z)) {
                        selectedEdge->setPointAttrib(roadPlanePointIndexes[i],POINT_ATTRIB_TUNNEL,true);
                        selectedEdge->setPointAttrib(roadPlanePointIndexes[i],POINT_ATTRIB_BRIDGE,false);
                    }
                    else {
                        selectedEdge->setPointAttrib(roadPlanePointIndexes[i],POINT_ATTRIB_BRIDGE,true);
                        selectedEdge->setPointAttrib(roadPlanePointIndexes[i],POINT_ATTRIB_TUNNEL,false);
                    }

                    break;
                }
                else if (roadPlanePointsProjected[i].x<=lassoPlanePointsProjected[j].x&&
                         roadPlanePointsProjected[i].x>=lassoPlanePointsProjected[j+1].x) {
                    float interp=(roadPlanePointsProjected[i].x-lassoPlanePointsProjected[j+1].x)/
                                 (lassoPlanePointsProjected[j].x-lassoPlanePointsProjected[j+1].x);
                    float newyval=lassoPlanePointsProjected[j+1].y*(1.0-interp)+
                                  lassoPlanePointsProjected[j].y*interp;

                    selectedEdge->setPoint(roadPlanePointIndexes[i],selectedEdge->getPoint(roadPlanePointIndexes[i])+Vector(0.0f,newyval-roadPlanePointsProjected[i].y,0.0f));

                    //bridge or tunnel based on below/above terrain
                    Vector point1=selectedEdge->getPoint(roadPlanePointIndexes[i]);
                    Vector point2=selectedEdge->getPoint(roadPlanePointIndexes[i]+1);

                    if (point1.y<heightMap->getHeightAt(point1.x,point1.z)||
                        point2.y<heightMap->getHeightAt(point2.x,point2.z)) {
                        selectedEdge->setPointAttrib(roadPlanePointIndexes[i],POINT_ATTRIB_TUNNEL,true);
                        selectedEdge->setPointAttrib(roadPlanePointIndexes[i],POINT_ATTRIB_BRIDGE,false);
                    }
                    else {
                        selectedEdge->setPointAttrib(roadPlanePointIndexes[i],POINT_ATTRIB_BRIDGE,true);
                        selectedEdge->setPointAttrib(roadPlanePointIndexes[i],POINT_ATTRIB_TUNNEL,false);
                    }

                    break;
                }
            }
        }

        //edge will have to have its structure updated (the spline)
        selectedEdge->updateStructure();

        //ADDED FOR GEOMETRIC MODELLING
        /*
		RN_Edge *edgeAfter=selectedEdge->getClone();

		//approach here: the path has its height modified only, so the change
		//is simplified since its with respect to the Y-axis

		//for each heightmap point, we find a nearest corresponding 
		//point within the road (edgeBefore), the distance will define the amount
		//of influence (2-norm this)

		int numPts=heightMap->geom_GetNumPoints();
		float influenceRad=15.0f;

		for (int i=0;i<numPts;i++) {
			Vector eachPt=heightMap->geom_GetPoint(i);

			if ((eachPt-sphereCentre).GetLength()>sphereInnerRad+influenceRad)
				continue;

                        float eachDist=FLT_MAX;
			int eachDistIndex=0;

			//find the nearest point along path
			for (int j=0;j<edgeBefore->getNumPoints();j++) {
				if ((eachPt-edgeBefore->getPoint(j)).GetLength()<eachDist) {
					eachDist=(eachPt-edgeBefore->getPoint(j)).GetLength();
					eachDistIndex=j;
				}
			}

			//if within volume of influence, then we move the point
			if (eachDist<influenceRad) {
				Vector newEachPt=eachPt;

				newEachPt+=(edgeAfter->getPoint(eachDistIndex)-edgeBefore->getPoint(eachDistIndex))*
					((influenceRad-eachDist)/influenceRad);

				//eachPt.print();
				//newEachPt.print();
				//printf("Updating point %i, with eachDist %f\n",i,eachDist);
				heightMap->geom_SetPoint(newEachPt, i);
			}

		}
		*/

    }

}

void TwistLens::drawTwistPlane(float offset, Camera & camera) {

    Vector cameraAxisProjectVec=camera.getPosition() - m_pointOfRotation;
    float projectAmount=cameraAxisProjectVec.DotProduct3(m_axisOfRotation)/m_axisOfRotation.GetLength();
    cameraAxisProjectVec -= m_axisOfRotation.getNormal()*projectAmount;
    cameraAxisProjectVec.Normal();

    float cameraIncidentAngle = acosf(Vector(0,1,0).DotProduct3(cameraAxisProjectVec));
    cameraIncidentAngle *= 180.0f/3.14159f;

    //have to give the incident angle a sign (based on if its on the left or
    //right side of the plane defined by up vector and axisofrotation
    Vector cameraAxisProjectVecUpVecCrossProd;
    cameraAxisProjectVecUpVecCrossProd.CrossProduct(Vector(0,1,0),cameraAxisProjectVec);
    if (m_axisOfRotation.DotProduct3(cameraAxisProjectVecUpVecCrossProd)>0.0f)
        cameraIncidentAngle=-cameraIncidentAngle;

    if (fabs(m_viewTwist+cameraIncidentAngle)<=TWIST_START_ALPHA)
        return;

    Vector offsetVec;
    offsetVec.CrossProduct(m_axisOfRotation,Vector(0.0f,1.0f,0.0f));
    offsetVec.SetLength(offset);

    if (m_viewTwist+cameraIncidentAngle>0.0f) {
        offsetVec=offsetVec.GetRotatedAxis(m_viewTwist, m_axisOfRotation);
    }
    else {
        offsetVec=offsetVec.GetRotatedAxis(m_viewTwist, m_axisOfRotation);
        offsetVec=offsetVec*(-1.0f);
    }

    Vector widthVec = m_axisOfRotation;
    widthVec.SetLength(200.0f);

    glPushMatrix();

    glTranslatef(offsetVec.x,offsetVec.y,offsetVec.z);

    Vector rot1 = Vector(0, 0, 0).GetRotatedAxis(m_viewTwist, m_axisOfRotation);
    Vector rot2 = Vector(0, 200.0f,0).GetRotatedAxis(m_viewTwist, m_axisOfRotation);

    Vector * planePoints=new Vector[4];
    planePoints[0]=m_pointOfRotation + rot1;
    planePoints[1]=m_pointOfRotation + rot2;
    planePoints[2]=m_pointOfRotation + m_axisOfRotation + rot1;
    planePoints[3]=m_pointOfRotation + m_axisOfRotation + rot2;

    //make it wide enough to cover complete view
    planePoints[0]-=widthVec;
    planePoints[1]-=widthVec;
    planePoints[2]+=widthVec;
    planePoints[3]+=widthVec;

    glEnable(GL_BLEND);

    float skyPlaneAlpha = 1.0f;
    if (fabs(m_viewTwist+cameraIncidentAngle) <= TWIST_END_ALPHA) {
        skyPlaneAlpha=(fabs(m_viewTwist+cameraIncidentAngle)-TWIST_START_ALPHA)/(TWIST_END_ALPHA-TWIST_START_ALPHA);
    }

    glColor4f(.6, .6, .7, skyPlaneAlpha);

    glBegin(GL_TRIANGLES);
    glVertex3f(planePoints[0].x,planePoints[0].y,planePoints[0].z);
    glVertex3f(planePoints[1].x,planePoints[1].y,planePoints[1].z);
    glVertex3f(planePoints[2].x,planePoints[2].y,planePoints[2].z);
    glVertex3f(planePoints[1].x,planePoints[1].y,planePoints[1].z);
    glVertex3f(planePoints[3].x,planePoints[3].y,planePoints[3].z);
    glVertex3f(planePoints[2].x,planePoints[2].y,planePoints[2].z);
    glEnd();

    glDisable(GL_BLEND);

    glPopMatrix();

}

Vector TwistLens::projectPointOnTwistPlane(Vector point, Vector pointOfRotation, Vector axisOfRotation) {

    Vector v1=point-pointOfRotation;
    Vector axisNormalized=axisOfRotation.getNormal();

    float Xlength=v1.DotProduct3(axisNormalized);

    Vector ortho=Vector(0,1,0);
    ortho=ortho.GetRotatedAxis(m_viewTwist, axisNormalized);
    ortho.Normal();

    float Ylength=v1.DotProduct3(ortho);

    return Vector(Xlength, Ylength, 0.0f);

}

bool TwistLens::isVisible() {
    return visible;
}

void TwistLens::setVisible(bool v) {
    visible=v;
}

void TwistLens::setOnSelectedEdge(RN_Edge *selectedEdge) {

    m_viewTwist=0.0f;

    //first, gluProject this vertex
    GLdouble model_view[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLdouble eachxval, eachyval, eachzval;

    vector <Vector> projectPoints;

    m_lensCentre=Vector(0.0f,0.0f,0.0f);

    for (int i=selectedEdge->getFirstSegmentSelected();i<=selectedEdge->getLastSegmentSelected();i++) {
        gluProject(selectedEdge->getPoint(i).x,selectedEdge->getPoint(i).y,selectedEdge->getPoint(i).z,
                   model_view, projection, viewport, &eachxval, &eachyval, &eachzval);
        projectPoints.push_back(Vector(eachxval,eachyval,0.0f));
        m_lensCentre+=Vector(eachxval,eachyval,0.0f);
    }
    m_lensCentre = m_lensCentre / (selectedEdge->getLastSegmentSelected() - selectedEdge->getFirstSegmentSelected() + 1 );

    m_innerRadius=0.0f;
    for (int i=0;i<projectPoints.size()-1;i++) {
        for (int j=i+1;j<projectPoints.size();j++) {
            if ((projectPoints[i]-projectPoints[j]).GetLength()>m_innerRadius)
                m_innerRadius=(projectPoints[i]-projectPoints[j]).GetLength();
        }
    }
    m_innerRadius=m_innerRadius/2.0f;
    m_outerRadius=m_innerRadius*1.5f;

    this->selectedEdge=selectedEdge;
    m_axisOfRotation=selectedEdge->getLastPointSelected() - selectedEdge->getFirstPointSelected();
    m_pointOfRotation=selectedEdge->getFirstPointSelected();



}

void TwistLens::getShaderAttribs(float &twistVal, float &innerRad, float &outerRad, Vector &mainPoint, Vector &axisPoint, Vector &midPoint) {

    twistVal = m_viewTwist;

    innerRad = m_axisOfRotation.GetLength()/2.0f;
    outerRad = m_axisOfRotation.GetLength()/2.0f*(m_outerRadius/m_innerRadius);

    mainPoint = m_pointOfRotation;
    axisPoint = m_axisOfRotation;
    midPoint = m_pointOfRotation+m_axisOfRotation*0.5;   

}
