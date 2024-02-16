
#include <QtOpenGL>

#include "Vector.h"
#include "textureloader.h"

#define ICONWIDTH 50.0
#define ICONHEIGHT 50.0
#define ICONSPACING 5.0

#define NUM_MODES 6

#define MODE_SELECT 0
#define MODE_ROAD 1
#define MODE_BREAKOUT 2
#define MODE_GLYPHS 3
#define MODE_TWISTLENS 4
#define MODE_PLAYROAD 5

class ModalMenu {

public:

    ModalMenu(QGLWidget * viewer);
    void display();
    int getWidth();
    int getHeight();
    int getMode();
    void setPos(Vector pos_xyz);
    void makeSelection(int mouse_x, int mouse_y);
    bool selected();

private:

    bool lastSelected;
    int mode;
    Vector pos_xyz;

    GLuint selectorindex;
    GLuint *textureindex;

    QGLWidget * m_pViewer;

};
