// ObjFile.h: interface for the ObjFile class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <QtOpenGL>

#include <vector>
#include <map>
#include <string>
#include <math.h>
#include <float.h>
#include <string>
#include <stdio.h>
#include <stdarg.h>

#include "textureloader.h"
#include "Vector.h"

using namespace std;

struct tFace
{
    std::vector<int> vertIndex;
    std::vector<int> coordIndex;
    std::vector<int> normalIndex;
    Vector faceNormal;
};

class ObjFile  
{
protected:

    struct tMaterialInfo
    {
        char			strName[255];		// The texture name
        char			strFile[255];		// The texture file name (If this is set it's a texture map)
        float			color[3];			// The color of the object (R, G, B)
        float			specColor[3];		// The specular color of the object
        unsigned int	textureId;			// the texture ID
        float			uTile;				// u tiling of texture  (Currently not used)
        float			vTile;				// v tiling of texture	(Currently not used)
        float			uOffset;	 	    // u offset of texture	(Currently not used)
        float			vOffset;			// v offset of texture	(Currently not used)
    };

    struct t3DObject
    {
        int  materialID;			// The texture ID to use, which is the index into our texture array
        char materialName[255];
        bool bHasTexture;			// This is TRUE if there is a texture map for this object
        char strName[255];			// The name of the object
        vector <Vector> pVerts;			// The object's vertices
        vector <Vector> pNormals;			// The object's normals
        vector <Vector2> pTexVerts;		// The texture's UV coordinates
        vector <tFace> pFaces;				// The faces information of the object
    };

public:
    ObjFile();
    virtual ~ObjFile();

    bool Read(const char *sFilePath);

    //required to subclass GeometryFile class:
    void Draw();
    void DrawUnshaded();
    inline double GetUnitScale() { return 1.0; }

    float GetBoundingRadius() const { return m_fBoundingRadius; }
    Vector GetBoundingCenter() const { return Vector(0.0, 0.0, 0.0); }

    unsigned int getTextureID(int whichObject);

    //Basic functions
    int getNumObjects();
    vector <tFace> &getFaces(int whichObject);
    vector <Vector> &getVerts(int whichObject);
    vector <Vector> &getNormals(int whichObject);
    vector <Vector2> &getTexVerts(int whichObject);

    static float ms_defaultDiffuse[4];
    static float ms_defaultWireDiffuse[4];
    static float ms_defaultWireWidth;

private:

    bool FaceIsInside( int& whichObj, tFace * pFace, float lx, float ly, float lz, float width );

    // Obj reading functions
    bool ImportObj(const char *sFilePath);
    void ReadObjFile(FILE* _f);
    void ReadVertexInfo(FILE* _f);
    void ReadFaceInfo(FILE* _f);
    void FillInObjectInfo();
    void ReadMaterialInfo(FILE* _f);
    void ReadMaterialToUse(FILE* _f);


    void CenterModel(double xOff, double yOff, double zOff);
    float ComputeBoundingSphereRadius(double xDist, double yDist, double zDist);

protected:
    std::vector<tMaterialInfo>	m_vMaterials;		//The list of material information (Textures and colors)
    std::vector<t3DObject>		m_vObjects;			//The object list for our model

    bool						m_bObjectHasUV;
    bool						m_bHasTexture;
    bool						m_bObjectHasNormals;
    bool						m_bJustReadAFace;
    int							m_iMaterialID;
    float						m_fBoundingRadius;

    //Temp stuff
    std::vector<Vector>		m_vVertices;		//Temp vertices vector
    std::vector<tFace>			m_vFaces;			//Temp Faces vector
    std::vector<Vector2>		m_vTextureCoords;	//Temp Texture Coords vector
    std::vector<Vector>		m_vNormals;			//Temp Normals vector

    unsigned int objDisplayList;
    QString baseDirectory;

};

