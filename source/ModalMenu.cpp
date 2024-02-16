#include "ModalMenu.h"

ModalMenu::ModalMenu(QGLWidget * viewer) :
m_pViewer(viewer)
{

    //load the textures
    textureindex=new GLuint[NUM_MODES];
    
    int w, h;

    selectorindex=TextureLoader::LoadTexture("menuicons/selector.png");

    textureindex[MODE_SELECT]=TextureLoader::LoadTexture("menuicons/select.png");
    textureindex[MODE_ROAD]=TextureLoader::LoadTexture("menuicons/road.png");
    textureindex[MODE_BREAKOUT]=TextureLoader::LoadTexture("menuicons/breakout.png");
    textureindex[MODE_GLYPHS]=TextureLoader::LoadTexture("menuicons/tree.png");
    textureindex[MODE_TWISTLENS]=TextureLoader::LoadTexture("menuicons/twistlens.png");
    textureindex[MODE_PLAYROAD]=TextureLoader::LoadTexture("menuicons/play.png");

    //set default mode
    mode=MODE_ROAD;

    //set initial offset
    pos_xyz=Vector(0,0,0);

    //last selected
    lastSelected=false;

}

bool ModalMenu::selected() {
    return lastSelected;
}

void ModalMenu::makeSelection(int mouse_x, int mouse_y) {

    lastSelected=false;

    for (int i=0;i<NUM_MODES;i++) {
        if (mouse_x>pos_xyz.x+(i-1)*ICONSPACING+i*ICONWIDTH&&mouse_x<pos_xyz.x+(i-1)*ICONSPACING+(i+1)*ICONWIDTH&&
            mouse_y>pos_xyz.y&&mouse_y<pos_xyz.y+ICONHEIGHT) {
            mode=i;
            lastSelected=true;
            break;
        }

    }

}

int ModalMenu::getWidth() {
    return ICONWIDTH*NUM_MODES+(ICONSPACING*NUM_MODES-1);
}

int ModalMenu::getHeight() {
    return ICONHEIGHT;
}

int ModalMenu::getMode() {
    return mode;
}

void ModalMenu::setPos(Vector pos_xyz) {
    this->pos_xyz=pos_xyz;
}

void ModalMenu::display() {

    //switch to orthographic mode to render menu icons
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, m_pViewer->width(),
            0.0, m_pViewer->height(), -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(1,1,1);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //for alpha channels for menu icons

    //render selection border
    glPushMatrix();
    glTranslatef(pos_xyz.x+ICONSPACING*mode+ICONWIDTH*mode,pos_xyz.y,pos_xyz.z);
    glBindTexture(GL_TEXTURE_2D, selectorindex);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0,0.0); glVertex3f(-(GLfloat)ICONSPACING,-(GLfloat)ICONSPACING,0.0);
    glTexCoord2f(0.0,1.0); glVertex3f(-(GLfloat)ICONSPACING,(GLfloat)ICONWIDTH+(GLfloat)ICONSPACING,0.0);
    glTexCoord2f(1.0,1.0); glVertex3f((GLfloat)ICONWIDTH+(GLfloat)ICONSPACING,(GLfloat)ICONWIDTH+(GLfloat)ICONSPACING,0.0);
    glTexCoord2f(1.0,0.0); glVertex3f((GLfloat)ICONWIDTH+(GLfloat)ICONSPACING,-(GLfloat)ICONSPACING,0.0);
    glEnd();
    glPopMatrix();

    //render modal menu icons
    glPushMatrix();
    glTranslatef(pos_xyz.x,pos_xyz.y,pos_xyz.z+1.0);
    for (int i=0;i<NUM_MODES;i++) {
        glBindTexture(GL_TEXTURE_2D, textureindex[i]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0,0.0); glVertex3f(0.0,0.0,0.0);
        glTexCoord2f(0.0,1.0); glVertex3f(0.0,(GLfloat)ICONWIDTH,0.0);
        glTexCoord2f(1.0,1.0); glVertex3f((GLfloat)ICONWIDTH,(GLfloat)ICONWIDTH,0.0);
        glTexCoord2f(1.0,0.0); glVertex3f((GLfloat)ICONWIDTH,0.0,0.0);
        glEnd();
        glTranslatef((GLfloat)ICONWIDTH+(GLfloat)ICONSPACING,0,0);
    }
    glPopMatrix();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

}
