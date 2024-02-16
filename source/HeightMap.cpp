#include "HeightMap.h"

HeightMap::HeightMap() {	

    //generate a plane by default
    textureIndex=0;
    detailTextureIndex=0;
    width=height=DEFAULT_GRIDSIZE;

    //waterHeight=55.0f; //good for 256 mesh
    //waterHeight=35.0f; //good for 1024 mesh
    waterHeight=42.0f; //good for river mesh
    //waterHeight=-1.0f;

    width=DEFAULT_GRIDSIZE;
    height=DEFAULT_GRIDSIZE;

    minimumAltitude=0.0f;

}

void HeightMap::generateGrid(int w, int h) {

    heightMapMesh = new TriMeshTextured();
    heightMapMesh->generateGrid(w,h);
    waterMesh = new TriMeshTextured();
    waterMesh->generateGrid(w,h);

    width=w;
    height=h;

}

void HeightMap::loadFromImages(QString heightFile, QString texFile) {

    minimumAltitude=255.0f;

    heightMapMesh = new TriMeshTextured();
    heightMapMesh->loadFromImages(heightFile, texFile, width, height);
    waterMesh = new TriMeshTextured();
    waterMesh->generateGrid(width, height);

}

int HeightMap::getWidth() {
    return width;
}

int HeightMap::getHeight() {
    return height;
}

void HeightMap::Draw() {

    glColor3f(1,1,1);
    heightMapMesh->Draw();

}

void HeightMap::setWaterViewTwists(vector <vector <float> > &twists) {

    waterTwists=twists;

}

void HeightMap::drawWater() {

    glDisable(GL_TEXTURE_2D);

    glColor4f(0.0f,0.3f,0.6f,0.4f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();
    glTranslatef(0,waterHeight,0);
    waterMesh->Draw();
    glPopMatrix();

    glDisable(GL_BLEND);

}

vector <Vector> HeightMap::selectWithRays(Vector pos_xyz, vector <Vector> rays) {

    vector <Vector> returnVec;

    for (int i=0;i<rays.size();i++) {

        Vector eachVec;

        if (selectWithRay(pos_xyz, rays[i], eachVec))
            returnVec.push_back(eachVec);
    }

    return returnVec;

}

vector <Vector> HeightMap::selectWithRaysUseDepthBuffer(Vector pos_xyz, vector <Vector> points2D) {

    glClear(GL_DEPTH_BUFFER_BIT);

    //drawFullDetail();
    Draw();
    drawWater();

    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLdouble posX, posY, posZ;
    float mouseZ;

    glGetIntegerv( GL_VIEWPORT, viewport );
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );

    //h = viewport[3] - viewport[1];

    //pick each point
    vector <Vector> returnVec;
    for (int i=0;i<points2D.size();i++) {

        glReadPixels( (GLint)points2D[i].x, points2D[i].y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &mouseZ );

        if (mouseZ>0.0&&mouseZ<1.0) {
            gluUnProject( points2D[i].x, points2D[i].y, mouseZ, modelview, projection, viewport,
                          &posX, &posY, &posZ);
            returnVec.push_back(Vector(posX,posY,posZ));
        }
    }

    return returnVec;

}

bool HeightMap::selectWithRay(Vector pos_xyz, Vector pickray, Vector &collidePoint) {

    GLdouble modelviewOrig[16];
    GLdouble projectionOrig[16];
    GLint viewportOrig[4];
    GLfloat depthVal;

    glGetDoublev( GL_MODELVIEW_MATRIX, modelviewOrig );
    glGetDoublev( GL_PROJECTION_MATRIX, projectionOrig );
    glGetIntegerv( GL_VIEWPORT, viewportOrig );

    //Finding closest point using depth-buffer is 2-pass... Once for coarse estimation, one to refine
    for (int i=0;i<2;i++) {

        if (i==0)
            gluLookAt(pos_xyz.x,pos_xyz.y,pos_xyz.z,
                      pos_xyz.x+pickray.x,pos_xyz.y+pickray.y,pos_xyz.z+pickray.z,
                      0,0,1);
        else
            gluLookAt(collidePoint.x-pickray.x,collidePoint.y-pickray.y,collidePoint.z-pickray.z,
                      collidePoint.x,collidePoint.y,collidePoint.z,
                      0,0,1);
        //glOrtho(-1,1,-1,1,0.1,1000.0);
        //glViewport(0,0,64,64);

        glClear(GL_DEPTH_BUFFER_BIT);

        glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);

        Draw();

        glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

        GLdouble modelview[16];
        GLdouble projection[16];
        GLint viewport[4];
        GLdouble posX, posY, posZ;

        glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
        glGetDoublev( GL_PROJECTION_MATRIX, projection );
        glGetIntegerv( GL_VIEWPORT, viewport );

        glReadPixels(viewport[2]/2.0,viewport[3]/2.0,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&depthVal);

        gluUnProject( viewport[2]/2.0,
                      viewport[3]/2.0,
                      depthVal,
                      modelview, projection, viewport,
                      &posX, &posY, &posZ );

        collidePoint=Vector(posX,posY,posZ);

        glViewport(viewportOrig[0],viewportOrig[1],viewportOrig[2],viewportOrig[3]);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(projectionOrig);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixd(modelviewOrig);

        if (depthVal>=1.0) break;

    }

    if (depthVal<1.0)
        return true;
    else
        return false;   
    
}

void HeightMap::setViewTwists(int bw, int bh, vector <float> &twists, Vector pointOfRotation, Vector axisOfRotation) {

    viewTwists[bw][bh].clear();
    viewTwists[bw][bh]=twists;
    if (!viewTwists[bw][bh].empty()) {
        this->pointOfRotation=pointOfRotation;
        this->axisOfRotation=axisOfRotation;
    }

}

float HeightMap::getWidthHeightMax() {
    if (width>height)
        return width;
    else
        return height;
}

float HeightMap::getWaterHeight() {
    return waterHeight;
}

float HeightMap::getMeshHeight(int x, int z) {

    //return heightMapMesh->vertices[z*width+x].y;
    return 0.0;

}

void HeightMap::setMeshHeight(int x, int z, float newHeight) {

    //heightMapMesh->vertices[z*width+x].y=newHeight;

}

Vector HeightMap::getDefaultCameraTrackPos() {

    Vector selectLocation;
    selectWithRay(Vector(width/2.0f,256.0f,height/2.0f),Vector(0.0f,-1.0f,0.0f),selectLocation);

    return selectLocation;

}

float HeightMap::getDefaultCameraTrackDist() {

    float maxDist;

    if (width>height)
        maxDist=(float)width;
    else
        maxDist=(float)height;

    return maxDist/1.5f;

}

float HeightMap::getHeightAt(float x, float z) {

    Vector selectLocation;

    if (selectWithRay(Vector(x,256.0f,z),Vector(0.0f,-1.0f,0.0f),selectLocation))
        return selectLocation.y;
    else
        return 0.0f;

}

float HeightMap::getMinimumAltitude() {

    return minimumAltitude;

}
