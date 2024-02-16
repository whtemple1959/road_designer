#ifndef _TRIMESH_TEXTURED_H
#define _TRIMESH_TEXTURED_H

#include <QtOpenGL>
#include <QtCore>

#include "textureloader.h"
#include "Vector.h"
#include "ObjFile.h"

using namespace std;

class TriMeshTextured: public ObjFile {

	public:
	
	TriMeshTextured();
        void loadFromImages(QString heightFile, QString texFile, int &width, int &height);
	void generateGrid(int gridwidth, int gridheight);

};

#endif
