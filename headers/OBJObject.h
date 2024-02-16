/*
 * Header file for OBJ file loader/displayer.
 *
 * THIS VERSION HAS BEEN MODIFIED FOR USE WITH SKETCHENV...  IT 
 * HAS FUNCTIONALITY FOR MODIFYING THE VERTEX INFORMATION DYNAMICALLY,
 * AND AS A RESULT NO LONGER TAKES ADVANTAGE OF OpenGL's DISPLAY LISTS
 *
 * Author: James McCrae
 */

#ifndef _OBJObject
#define _OBJObject

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <QtOpenGL>

#include "Vector.h"
#include "textureloader.h"

class OBJObject {

public:
    
    OBJObject();
    ~OBJObject();
    void load(char *filename);
    void load(char *directory, char *filename);
    void draw();

    GLfloat *getPosVertexes();
    void setPosVertexes(GLfloat *posVertexes);
    int getNumPosVertexes();

    int GetNumPotentialColliders();
    Vector *GetPotentialColliders();   

private:
    
    GLuint textureindex;
    
    //texture offsets (u and v coords)
    GLfloat texture_uoffset;
    GLfloat texture_voffset;
    
    //vertex arrays
    GLfloat *posVertexes;
    GLfloat *texVertexes;
    GLfloat *normVertexes;
    
    //faces (sequences of indexes to the vertex arrays)
    GLint *posFaceindex;
    GLint *texFaceindex;
    GLint *normFaceindex;
    bool *faceTriangle;
    
    //max values of the sizes of the arrays
    int posVertexIndex;
    int texVertexIndex;
    int normVertexIndex;    
    int faceindexIndex;
    
};

#endif
