#include "TriMeshTextured.h"

/*
 * Constructor with nothing.
 */
TriMeshTextured::TriMeshTextured() {
}



void TriMeshTextured::loadFromImages(QString heightFile, QString texFile, int & width, int & height) {

    QImage heightImg = QImage(heightFile);
    width = heightImg.width();
    height = heightImg.height();

    t3DObject terrainObject;
    terrainObject.pVerts.reserve(width*height);

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            terrainObject.pVerts.push_back(Vector(i, float(qGray(heightImg.pixel(i, height - j - 1))), float(j)));
            terrainObject.pNormals.push_back(Vector(0,1,0));
            terrainObject.pTexVerts.push_back(Vector2(float(i)/width, float(j)/height));
        }
    }

    for (int j = 0; j < height-1; j++)
        for (int i = 0; i < width-1; i++) {
        tFace face1;
        tFace face2;

        int ind = i + j*width + 1; //the +1 is 1-indexing... matches OBJ format

        face1.vertIndex.push_back(ind);
        face1.vertIndex.push_back(ind+1);
        face1.vertIndex.push_back(ind+width);
        face1.normalIndex.push_back(ind);
        face1.normalIndex.push_back(ind+1);
        face1.normalIndex.push_back(ind+width);
        face1.coordIndex.push_back(ind);
        face1.coordIndex.push_back(ind+1);
        face1.coordIndex.push_back(ind+width);

        face2.vertIndex.push_back(ind+1);
        face2.vertIndex.push_back(ind+width+1);
        face2.vertIndex.push_back(ind+width);
        face2.normalIndex.push_back(ind+1);
        face2.normalIndex.push_back(ind+width+1);
        face2.normalIndex.push_back(ind+width);
        face2.coordIndex.push_back(ind+1);
        face2.coordIndex.push_back(ind+width+1);
        face2.coordIndex.push_back(ind+width);

        terrainObject.pFaces.push_back(face1);
        terrainObject.pFaces.push_back(face2);
    }

    tMaterialInfo terrainMaterialInfo;
    int w, h;
    terrainMaterialInfo.textureId=TextureLoader::LoadTexture(texFile, w, h);
    if (terrainMaterialInfo.textureId<=0)
        qDebug() << "Could not load texture " << texFile;

    strcpy(terrainObject.materialName,"terrainTexture");
    terrainObject.materialID=0;

    m_vMaterials.push_back(terrainMaterialInfo);
    m_vObjects.push_back(terrainObject);


}


/*
 * Generates triangular mesh grid of gridsize by gridsize on the XZ plane
 */
void TriMeshTextured::generateGrid(int width, int height) {

    t3DObject terrainObject;
    terrainObject.pVerts.reserve(width*height);

    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++) {
        terrainObject.pVerts.push_back(Vector(i,0,(float)(height-j)));
        terrainObject.pNormals.push_back(Vector(0,1,0));
    }

    for (int j = 0; j < height-1; j++)
        for (int i = 0; i < width-1; i++) {
        tFace face1;
        tFace face2;

        int ind = i + j*width + 1; //the +1 is 1-indexing... matches OBJ format

        face1.vertIndex.push_back(ind);
        face1.vertIndex.push_back(ind+1);
        face1.vertIndex.push_back(ind+width);
        face1.normalIndex.push_back(ind);
        face1.normalIndex.push_back(ind+1);
        face1.normalIndex.push_back(ind+width);

        face2.vertIndex.push_back(ind+1);
        face2.vertIndex.push_back(ind+width+1);
        face2.vertIndex.push_back(ind+width);
        face2.normalIndex.push_back(ind+1);
        face2.normalIndex.push_back(ind+width+1);
        face2.normalIndex.push_back(ind+width);

        terrainObject.pFaces.push_back(face1);
        terrainObject.pFaces.push_back(face2);
    }

    m_vObjects.push_back(terrainObject);

}
