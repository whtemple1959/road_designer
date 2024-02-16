// ObjFile.cpp: implementation of the ObjFile class.
//
//////////////////////////////////////////////////////////////////////

#include "ObjFile.h"

float ObjFile::ms_defaultDiffuse[4] = { 0.43f, 0.63f, 0.75f, 1.0f };
float ObjFile::ms_defaultWireDiffuse[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // { 0.5f, 0.5f, 0.5f, 1.0f };
float ObjFile::ms_defaultWireWidth = 2.0f;

/////////////////////////////////////////////////////////////////////////////
// ObjFile construction/destruction

ObjFile::ObjFile()
{	

    objDisplayList=0;

}

ObjFile::~ObjFile()
{
    // Go through all the objects in the scene
    size_t nObjCount = m_vObjects.size();
    for(size_t i = 0; i < nObjCount; i++)
    {
        // Free the faces, normals, vertices, and texture coordinates.
        m_vObjects[i].pFaces.clear();
        m_vObjects[i].pNormals.clear();
        m_vObjects[i].pVerts.clear();
        m_vObjects[i].pTexVerts.clear();
    }
}


/////////////////////////////////////////////////////////////////////////////
// ObjFile member functions

bool ObjFile::Read(const char * sFilePath)
{	

    /*
	 * Determine base path from file path (to be used later for mtl/texture files)
	 */	
    QFileInfo qfile(sFilePath);
    baseDirectory = qfile.path() + QString("/");
    qDebug() << "Reading OBJ" << sFilePath << "with base path" << baseDirectory;

    m_fBoundingRadius = 0.0f;

    m_bObjectHasUV	= false;
    m_bHasTexture = false;
    m_bObjectHasNormals = false;
    m_bJustReadAFace = false;
    m_iMaterialID = 0;

    bool bSuccess = ImportObj(sFilePath);

    m_vVertices.clear();
    m_vFaces.clear();
    m_vNormals.clear();
    m_vTextureCoords.clear();

    return bSuccess;
}

void ObjFile::Draw() 
{

    if (objDisplayList>0) {
        glCallList(objDisplayList);
    }
    else {

        objDisplayList=glGenLists(1);
        glNewList(objDisplayList, GL_COMPILE_AND_EXECUTE);       

        int totalFaces = 0;

        size_t nObjCount = m_vObjects.size();

        qDebug() << "Sub-objects loaded: " << nObjCount;
        for(size_t nObj=0; nObj<nObjCount; nObj++) {

            const t3DObject* pObj = &(m_vObjects[nObj]);

            if(m_vMaterials.size()) {

                if (strcmp(m_vMaterials[pObj->materialID].strFile, " ")) {
                    // Turn on texture mapping and turn off color
                    glEnable(GL_TEXTURE_2D);
                    // Reset the color to normal again

                    unsigned int texId = m_vMaterials[pObj->materialID].textureId;
                    glBindTexture(GL_TEXTURE_2D, texId);
                }
                else {
                    glDisable(GL_TEXTURE_2D);
                    float pColor[4];
                    float sColor[4];
                    pColor[0] = m_vMaterials[pObj->materialID].color[0];
                    pColor[1] = m_vMaterials[pObj->materialID].color[1];
                    pColor[2] = m_vMaterials[pObj->materialID].color[2];
                    sColor[0] = m_vMaterials[pObj->materialID].specColor[0];
                    sColor[1] = m_vMaterials[pObj->materialID].specColor[1];
                    sColor[2] = m_vMaterials[pObj->materialID].specColor[2];
                    pColor[3] = 1;
                    sColor[3] = 1;
                    glColor4fv(pColor);
                    // now set the specular
                    glMateriali(GL_FRONT, GL_SHININESS, 3); // random number (I picked it out of a hat, but looks good)
                    glMaterialfv(GL_FRONT, GL_SPECULAR, sColor);
                }
            }
            else {
                //use default material
                //static const float specularcolor[4] = { 0.9f, 0.9f, 0.9f, 1.0f };
                //static const float defaultcolor[4] = ms_defaultDiffuse; //{ 0.33f, 0.53f, 0.75f, 1.0f };
                glDisable(GL_TEXTURE_2D);

                glColor4fv(ms_defaultDiffuse);
                //glMateriali(GL_FRONT, GL_SHININESS, 100); // random number (I picked it out of a hat, but looks good)
                //glMaterialfv(GL_FRONT, GL_SPECULAR, specularcolor);
            }

            glBegin(GL_TRIANGLES);

            for(int nFace=0; nFace<pObj->pFaces.size(); nFace++) {

                /*
                if(pObj->pFaces[nFace].vertIndex.size() == 3)
                    glBegin(GL_TRIANGLES);
                else if(pObj->pFaces[nFace].vertIndex.size() == 4)
                    glBegin(GL_QUADS);
                else
                    glBegin(GL_POLYGON);
                    */


                int vertIndex, normalIndex, coordIndex;

                for(unsigned int whichVertex = 0; whichVertex < pObj->pFaces[nFace].vertIndex.size(); whichVertex++) { // go through each corner and draw it

                    normalIndex = -1;
                    coordIndex = -1;

                    // Get the vertex index for each point of the face
                    vertIndex = pObj->pFaces[nFace].vertIndex[whichVertex] - 1; // -1 is new

                    if(pObj->pFaces[nFace].normalIndex.size() > 0) {
                        //face has normal data
                        normalIndex = pObj->pFaces[nFace].normalIndex[whichVertex] - 1;
                        if(normalIndex <= pObj->pNormals.size()) {
                            glNormal3d(pObj->pNormals[ normalIndex ].x, pObj->pNormals[ normalIndex ].y, pObj->pNormals[ normalIndex ].z);
                            // Give OpenGL the normal for this vertex.  have to multiply by m_scale to re-normalize the normals
                        }
                        else {
                            //TRACE (_T("Buggered normal,object# %d, normalIndex# %d\n"), nObj, normalIndex);
                            //ASSERT(0);
                        }
                    }

                    if(pObj->pTexVerts.size()>0) {
                        // Get the texture coordinate index
                        int coordIndex = pObj->pFaces[nFace].coordIndex[whichVertex] -1;

                        // Assign the UV coordinates to the current vertex being rendered
                        glTexCoord2d(pObj->pTexVerts[ coordIndex ].x, pObj->pTexVerts[ coordIndex ].y);
                    }

                    // Pass in the current vertex of the object (Corner of current face)
                    //assert (vertIndex <= pObject->numOfVerts);
                    glVertex3d(pObj->pVerts[ vertIndex ].x, pObj->pVerts[ vertIndex ].y, pObj->pVerts[ vertIndex ].z);
                }

                //glEnd();

            }

            glEnd();

            totalFaces += pObj->pFaces.size();
            qDebug() << "Number of Faces: " << pObj->pFaces.size();
        }

        qDebug() << "Total Number of Faces: " << totalFaces;

        glEndList();

    }

} // end draw

void ObjFile::DrawUnshaded() 
{	

    int totalFaces = 0;

    size_t nObjCount = m_vObjects.size();

    for(size_t nObj=0; nObj<nObjCount; nObj++) {

        const t3DObject* pObj = &(m_vObjects[nObj]);

        //skip any materials...

        for(int nFace=0; nFace<pObj->pFaces.size(); nFace++) {

            if(pObj->pFaces[nFace].vertIndex.size() == 3)
                glBegin(GL_TRIANGLES);
            else if(pObj->pFaces[nFace].vertIndex.size() == 4)
                glBegin(GL_QUADS);
            else
                glBegin(GL_POLYGON);


            int vertIndex, normalIndex, coordIndex;

            for(unsigned int whichVertex = 0; whichVertex < pObj->pFaces[nFace].vertIndex.size(); whichVertex++) { // go through each corner and draw it

                normalIndex = -1;
                coordIndex = -1;

                // Get the vertex index for each point of the face
                vertIndex = pObj->pFaces[nFace].vertIndex[whichVertex] - 1; // -1 is new

                if(pObj->pFaces[nFace].normalIndex.size() > 0) {
                    //face has normal data
                    normalIndex = pObj->pFaces[nFace].normalIndex[whichVertex] - 1;
                    if(normalIndex <= pObj->pNormals.size()) {
                        glNormal3d(pObj->pNormals[ normalIndex ].x, pObj->pNormals[ normalIndex ].y, pObj->pNormals[ normalIndex ].z);
                        // Give OpenGL the normal for this vertex.  have to multiply by m_scale to re-normalize the normals
                    }
                    else {
                        //TRACE (_T("Buggered normal,object# %d, normalIndex# %d\n"), nObj, normalIndex);
                        //ASSERT(0);
                    }
                }

                //skip textures...

                // Pass in the current vertex of the object (Corner of current face)
                //assert (vertIndex <= pObject->numOfVerts);
                glVertex3d(pObj->pVerts[ vertIndex ].x, pObj->pVerts[ vertIndex ].y, pObj->pVerts[ vertIndex ].z);
            }

            glEnd();
        }

        totalFaces += pObj->pFaces.size();
        //TRACE(_T("Number of Faces: %d\n"), pObj->pFaces.size());
    }

    //TRACE(_T("Total Number of Faces: %d\n"), totalFaces);

} // end draw

/**********************************************************************/
// This function loads a .obj file into a specified model by a .obj file name
bool ObjFile::ImportObj(const char *sFilePath)
{
    // Here we open the desired file for read only and return the file pointer
    FILE* f;

    f = fopen(sFilePath, "r");

    // Check to make sure we have a valid file pointer
    if(!f) {
        // Create an error message for the attempted file
        //sprintf(strMessage, "Unable to find or open the file: %s", strFileName);
        //MessageBox(NULL, strMessage, "Error", MB_OK);
        //TRACE(_T("Cannot open the file"));
        qDebug() << "!!! Unable to load the OBJ" << sFilePath;
        return false;

    }

    // query and store the current locale
    //char * locale = _strdup(setlocale(LC_ALL, NULL));
    // reset the locale to "C" so that scanf() works correctly
    //setlocale(LC_ALL, "C");

    // Now that we have a valid file and it's open, let's read in the info!
    ReadObjFile(f);

    // restore the locale
    //setlocale(LC_ALL, locale);
    //free(locale);

    // Close the .obj file that we opened
    fclose(f);


    // Return a success!
    return true;
}

/**********************************************************************/
//	This function is the main loop for reading in the .obj file
void ObjFile::ReadObjFile(FILE* _f)
{
    char strLine[255] = {0};
    char ch = 0;

    int		maxVertex, maxCoord, maxNormal;	// maximum
    int		minVertex, minCoord, minNormal;

    maxVertex = maxCoord = maxNormal = 0;
    minVertex = minCoord = minNormal = 2147483647;//std::numeric_limits<int>::max();//


    while(!feof(_f))
    {
        //float x = 0.0f, y = 0.0f, z = 0.0f;

        // Get the beginning character of the current line in the .obj file
        ch = (char)fgetc(_f);

        switch(ch)
        {
        case 'v':						// Check if we just read in a 'v' (Could be a vertice/normal/textureCoord)

            // If we just read in a face line, then we have gone to another object,
            // so we need to save the last object's data before moving onto the next one.
            if(m_bJustReadAFace) {
                // Save the last object's info into our model structure
                FillInObjectInfo();

                // print out some debug information and reset the counters
                //	TRACE(_T("%d/%d \t%d/%d \t%d/%d\n"), m_minVertex, m_maxVertex, m_minCoord, m_maxCoord, m_minNormal, m_maxNormal);
                maxVertex = maxCoord = maxNormal = 0;
                minVertex = minCoord = minNormal = 100000;
            }

            // Decipher this line to see if it's a vertex ("v"), normal ("vn"), or UV coordinate ("vt")
            ReadVertexInfo(_f);
            break;

		case 'f':						// Check if we just read in a face header ('f')

                    // If we get here we then we need to read in the face information.
                    // The face line holds the vertex indices into our vertex array, or if
                    // the object has texture coordinates then it holds those too.
                    ReadFaceInfo(_f);
                    break;

		case 'm':						// Check if we are looking at a mtllib line
                    // read the material information
                    ReadMaterialInfo(_f);
                    break;

		case 'u':						// Check if we are looking at a usemtl line
                    ReadMaterialToUse(_f);
                    break;

		case '\n':

                    // If we read in a newline character, we've encountered a blank line in
                    // the .obj file.  We don't want to do the default case and skip another
                    // line, so we just break and do nothing.
                    break;

		default:

                    // If we get here then we don't care about the line being read, so read past it.
                    fgets(strLine, 100, _f);
                    break;
		}

    }

    // Now that we are done reading in the file, we have need to save the last object read.
    FillInObjectInfo();

    // print out some debug information and reset the counters
    //	TRACE(_T("%d/%d \t%d/%d \t%d/%d\n"), m_minVertex, m_maxVertex, m_minCoord, m_maxCoord, m_minNormal, m_maxNormal);
    maxVertex = maxCoord = maxNormal = 0;
    minVertex = minCoord = minNormal = 2147483647;

}

/**********************************************************************/
void ObjFile::ReadVertexInfo(FILE* _f)
{
    Vector vNewVertex;
    Vector2 vNewTexCoord;
    Vector vNewNormal;
    char strLine[255]		= {0};
    char ch = 0;

    // Read the next character in the file to see if it's a vertice/normal/UVCoord
    ch = (char)fgetc(_f);

    if(ch == ' ')				// If we get a space it must have been a vertex ("v")
    {
        // Here we read in a vertice.  The format is "v x y z"
        fscanf(_f, "%f %f %f", &vNewVertex.x, &vNewVertex.y, &vNewVertex.z);
        //_RMSInfo("vertices %f, %f, %f\n", vNewVertex.x, vNewVertex.y, vNewVertex.z);

        // Read the rest of the line so the file pointer returns to the next line.
        fgets(strLine, 100, _f);

        // Add a new vertice to our list
        m_vVertices.push_back(vNewVertex);

    }
    else if(ch == 't')			// If we get a 't' then it must be a texture coordinate ("vt")
    {
        // Here we read in a texture coordinate.  The format is "vt u v"
        fscanf(_f, "%f %f", &vNewTexCoord.x, &vNewTexCoord.y);

        // Read the rest of the line so the file pointer returns to the next line.
        fgets(strLine, 100, _f);

        // Add a new texture coordinate to our list
        m_vTextureCoords.push_back(vNewTexCoord);

        // Set the flag that tells us this object has texture coordinates.
        // Now we know that the face information will list the vertice AND UV index.
        // For example, ("f 1 3 2" verses "f 1/1 2/2 3/3")
        m_bObjectHasUV = true;
    }
    else if(ch == 'n')			// If we get a 'n' then it must be a normal vector
    {
        // Here we read in a texture coordinate.  The format is "vt u v"
        fscanf(_f, "%f %f %f", &vNewNormal.x, &vNewNormal.y, &vNewNormal.z);

        // Read the rest of the line so the file pointer returns to the next line.
        fgets(strLine, 100, _f);

        // Add a new texture coordinate to our list
        m_vNormals.push_back(vNewNormal);

        // Set the flag that tells us this object has normals
        // Now we know that the face information will list the vertice AND UV index.
        // For example, ("f 1 3 2" verses "f 1/1 2/2 3/3")
        m_bObjectHasNormals = true;
    }
    else
    {
        fgets(strLine, 100, _f);
    }

}

/**********************************************************************/
void ObjFile::ReadFaceInfo(FILE* _f)
{
    tFace newFace;
    newFace.coordIndex.resize(0);
    newFace.vertIndex.resize(0);
    newFace.normalIndex.resize(0);

    //char strLine[255]		= {0};

    // This function reads in the face information of the object.
    // A face is a polygon (a triangle in this case) that has information about it.
    // It has the 3D points that make up the polygon and may also have texture coordinates.
    // When reading in an .obj, objects don't have to have UV texture coordinates so we
    // need to read in the face information differently in that case.  If the object does have
    // UV coordinates, then the format will look like this:
    // "f vertexIndex1/coordIndex1 vertexIndex2/coordIndex2 vertexIndex3/coordIndex3"
    // otherwise the format will look like this:"f vertexIndex1 vertexIndex2 vertexIndex3"
    // The index values index into our vertice and texture coordinate arrays.  More explained in RenderScene().
    // *Note* Make sure if you cut this code out for your own use you minus 1 from the indices.
    // This is because arrays are zero based and the .obj indices start at 1.  Look at FillInObjectInfo().

    // Check if this object has texture coordinates before reading in the values


    // generalized parsing code
    char ch = 0;
    int vert = 0;
    int currVal = 0;
    int indexCounter = 0;

    do
    {
        do
        {
            if(fscanf(_f, "%d", &currVal)) // only assign if we get a number
            {
                if (indexCounter == 2)		// just got a normal index
                {
                    newFace.normalIndex.push_back(currVal);
                }
                else if (indexCounter == 1)	// just got a coord index
                {
                    newFace.coordIndex.push_back(currVal);
                }
                else						// just got a vert index
                {
                    newFace.vertIndex.push_back(currVal);
                }
            }
            ch = (char)fgetc(_f);
            indexCounter++;

        } while (ch != ' ' && ch != '\n');
        vert++;
        indexCounter = 0;
    }while (ch != '\n' && vert<3);

    // Add the new face to our face list
    m_vFaces.push_back(newFace);

    // We set this flag to TRUE so we know that we just read in some face information.
    // Since face information is the last thing we read in for an object we will check
    // this flag when we read in a vertice.  If it's true, then we just finished reading
    // in an object and we need to save off the information before going to the next object.
    // Since there is no standard header for objects in a .obj file we do it this way.
    m_bJustReadAFace = true;
}

/**********************************************************************/
//	This function is called after an object is read in to fill in the model structure
void ObjFile::FillInObjectInfo()
{
    t3DObject newObject = {0};
    int textureOffset = 0, vertexOffset = 0, normalOffset = 0;
    int i = 0;

    // Add a new object to the list of objects in our model
    m_vObjects.push_back(newObject);

    // Get a pointer to the current object so our code is easier to read
    t3DObject *pObject = &(m_vObjects[m_vObjects.size() - 1]);

    // Now that we have our list's full of information, we can get the size
    // of these lists by calling size() from our vectors.  That is one of the
    // wonderful things that the Standard Template Library offers us.  Now you
    // never need to write a link list or constantly call malloc()/new.

    // Here we get the number of faces, vertices and texture coordinates
    //pObject->numOfFaces   = static_cast<int>(m_vFaces.size());
    //pObject->numOfVerts   = static_cast<int>(m_vVertices.size());
    //pObject->numTexVertex = static_cast<int>(m_vTextureCoords.size());
    //pObject->numNormals	  = static_cast<int>(m_vNormals.size());

    // If we read in any faces for this object (required)
    //if(pObject->pFaces.size()>0) {

    // Allocate enough memory to store all the faces in our object
    //pObject->pFaces = new tFace [pObject->numOfFaces];
    //}

    // If we read in any vertices for this object (required)
    //if(pObject->numOfVerts) {

    // Allocate enough memory to store all the vertices in our object
    //pObject->pVerts = new Vector [pObject->numOfVerts];
    //}

    // If we read in any texture coordinates for this object (optional)
    //if(pObject->numTexVertex) {
    //pObject->pTexVerts = new Vector2 [pObject->numTexVertex];
    //}

    // set texture flag

    pObject->bHasTexture = m_bHasTexture;

    if(pObject->pNormals.size()>0)
    {
        //pObject->pNormals = new Vector [pObject->numNormals];
    }

    // Go through all of the faces in the object
    for(i = 0; i < m_vFaces.size(); i++)
    {
        // Copy the current face from the temporary list to our Model list
        pObject->pFaces.push_back(m_vFaces[i]);

        // Because of the problem with .obj files not being very object friendly,
        // if a new object is found in the file, the face and texture indices start
        // from the last index that was used in the last object.  That means that if
        // the last one was 8, it would then go to 9 for the next object.  We need to
        // bring that back down to 1, so we just create an offset that we subtract from
        // the vertex and UV indices.

        // Check if this is the first face of the object
        if(i == 0)
        {
            //go through each previous object and count the verticies etc
            for (unsigned int i = 0; i < this->m_vObjects.size() - 1; i++)
            {
                vertexOffset += this->m_vObjects[i].pVerts.size();
                textureOffset += this->m_vObjects[i].pTexVerts.size();
                normalOffset += this->m_vObjects[i].pNormals.size();
            }
        }

        // Because the face indices start at 1, we need to minus 1 from them due
        // to arrays being zero based.  This is VERY important!
        for(unsigned int j = 0; j < pObject->pFaces[i].vertIndex.size(); j++)
        {
            // For each index, minus 1 to conform with zero based arrays.
            // We also need to add the vertex and texture offsets to subtract
            // the total amount necessary for this to work.  The first object
            // will have a offset of 0 for both since it starts at 1.
            pObject->pFaces[i].vertIndex[j]  -= vertexOffset;
            if (pObject->pFaces[i].normalIndex.size())
                pObject->pFaces[i].normalIndex[j] -= normalOffset;
            if (pObject->pFaces[i].coordIndex.size())
                pObject->pFaces[i].coordIndex[j] -= textureOffset;
        }
    }

    // Go through all the vertices in the object
    for(i = 0; i < m_vVertices.size(); i++)
    {
        // Copy the current vertice from the temporary list to our Model list
        pObject->pVerts.push_back(m_vVertices[i]);
    }

    // Go through all of the texture coordinates in the object (if any)
    for(i = 0; i < m_vTextureCoords.size(); i++)
    {
        // Copy the current UV coordinate from the temporary list to our Model list
        pObject->pTexVerts.push_back(m_vTextureCoords[i]);
    }

    // Go through all of the normals
    for(i = 0; i < m_vNormals.size(); i++)
    {
        // Copy the current UV coordinate from the temporary list to our Model list
        pObject->pNormals.push_back(m_vNormals[i]);
    }

    // this is new, this is trying to match up the actual material
    pObject->materialID = m_iMaterialID;

    // Build normal for each face
    for(int nFace=0; nFace<pObject->pFaces.size(); nFace++)
    {
        int vertIndex;
        double triangle[3][3];

        for(unsigned int whichVertex = 0; whichVertex < pObject->pFaces[nFace].vertIndex.size(); whichVertex++)
        {
            // Get the vertex index for each point of the face
            vertIndex = pObject->pFaces[nFace].vertIndex[whichVertex] - 1; // -1 is new

            triangle[whichVertex][0] = pObject->pVerts[vertIndex].x;
            triangle[whichVertex][1] = pObject->pVerts[vertIndex].y;
            triangle[whichVertex][2] = pObject->pVerts[vertIndex].z;
        }

        double edge1[3], edge2[3];
        edge1[0] = triangle[1][0] - triangle[0][0];
        edge1[1] = triangle[1][1] - triangle[0][1];
        edge1[2] = triangle[1][2] - triangle[0][2];
        edge2[0] = triangle[2][0] - triangle[0][0];
        edge2[1] = triangle[2][1] - triangle[0][1];
        edge2[2] = triangle[2][2] - triangle[0][2];

        pObject->pFaces[nFace].faceNormal.x = (edge1[1]*edge2[2])-(edge1[2]*edge2[1]);
        pObject->pFaces[nFace].faceNormal.y = (edge1[2]*edge2[0])-(edge1[0]*edge2[2]);
        pObject->pFaces[nFace].faceNormal.z = (edge1[0]*edge2[1])-(edge1[1]*edge2[0]);

        // TODO: build proper normals for each vertex on the face
        //for ( whichVertex
    }

    // Now that we have all the information from out list's, we need to clear them
    // so we can be ready for the next object that we read in.
    m_vVertices.clear();
    m_vFaces.clear();
    m_vTextureCoords.clear();
    m_vNormals.clear();

    // Reset these booleans to be prepared for the next object
    m_bObjectHasUV   = false;
    m_bJustReadAFace = false;

}

/**********************************************************************/
void ObjFile::ReadMaterialInfo(FILE* _f)
{
    /*	it appears that the Maya OBJ exporter is only giving material information in the .mtl
	file for lambert shading.  Anisotropic and Blinn are certainly not giving the info
	for either the colour properties or the texture map files.  Lambert is producing a copy
	of the image file that was used for the texturing and placing it in the same directory
	as the .obj adn .mtl files*/

    char* temp = new char[1000];
    char* filename = new char[PATH_MAX];

    char space[] = " ";
    char endline[] = "\n";

    fgets(temp, 7, _f);				// read the rest of the word "mtllib"
    fgets(filename, PATH_MAX, _f);	// read the material filename
    // remove the trailing newline
    filename = strtok(filename, endline);

    QString completeFilename = baseDirectory + QString(filename);

    //modified by JAMES to use basedirectory for materials/textures
    FILE* materialFilePointer = NULL;

    materialFilePointer = fopen(completeFilename.toAscii().data(), "r");

    //ASSERT(materialFilePointer);
    // now parse the material file
    // it is in a format as follows:
    // newmtl materialName1
    // newmtl materialName2
    // illum 4
    // Ni 1.00
    // Kd 1.00 0.00 0.00
    // Ka 0.00 0.00 0.00
    // Tf 1.00 1.00 1.00
    //
    // the materials may or may not have information following their name

    tMaterialInfo newMaterial = {0};
    newMaterial.specColor[0] = 0;
    newMaterial.specColor[1] = 0;
    newMaterial.specColor[2] = 0;

    char* materialName		= new char[100];
    char* startMaterialPtr	= materialName;			// we need to keep track of this to clean up resources
    char* textureFile		= new char[255];
    char* startTexturePtr	= textureFile;
    char* colorValues		= new char[50];
    char* startColorPtr		= colorValues;
    m_bHasTexture			= false;
    bool keepGoing			= false;

    // read the first texture name
    fgets(temp, 1000, materialFilePointer);
    temp = strtok(temp, space);						// read the newmtl
    materialName = strtok(NULL, endline);	// read the material name

    if ( materialName == NULL )
    {
        // clean up resources and get out
        delete [] temp;
        delete [] filename;
        delete [] startMaterialPtr;
        delete [] startTexturePtr;
        delete [] startColorPtr;

        return;
    }


    strcpy(newMaterial.strName, materialName);	// set the material name

    do
    {
        keepGoing = false;
        m_bHasTexture = false;
        strcpy(newMaterial.strFile, " ");
        strcpy(newMaterial.strName, materialName);	// set the material name

        while (fgets(temp, 1000, materialFilePointer))
        {
            // now get the Kd line, this has the diffuse color information
            // also look for a map_Kd line, this has the texture image information
            strtok(temp, space);					// get the first word in that line
            if (!strcmp(temp, "newmtl"))
            {
                materialName = strtok(NULL, endline);	// read the material name
                keepGoing = true;
                break; // found the next material
            }
            else if (!strcmp(temp, "Kd")||!strcmp(temp, "\tKd"))		// check if it is "Kd"
            {
                colorValues = strtok(NULL, endline);		// grab the colour line
                float r, g, b;
                r = g = b = 0;
                sscanf(colorValues, "%f %f %f", &r, &g, &b);	// use the values
                newMaterial.color[0] = r;
                newMaterial.color[1] = g;
                newMaterial.color[2] = b;
            }
            else if (!strcmp(temp, "Ks")||!strcmp(temp, "\tKs"))		// check if it is "Ks" (specular)
            {
                colorValues = strtok(NULL, endline);		// grab the colour line
                float r, g, b;
                r = g = b = 0;
                sscanf(colorValues, "%f %f %f", &r, &g, &b);	// use the values
                newMaterial.specColor[0] = r;
                newMaterial.specColor[1] = g;
                newMaterial.specColor[2] = b;
            }
            else if (!strcmp(temp, "map_Kd"))					// check if it is "map_Kd"
            {
                textureFile = strtok(NULL, endline);		// get the texture file name
                strcpy(newMaterial.strFile, textureFile);		// copy the name to the material info

                QString textureWithPath = baseDirectory + QString(textureFile);

                //use Qt image loader
                newMaterial.textureId=TextureLoader::LoadTexture(textureWithPath);

            }
        }
        m_vMaterials.push_back(newMaterial);
    } while (keepGoing);

    //close material file (close open file handle)
    fclose(materialFilePointer);

    // clean up resources!
    delete [] temp;
    delete [] filename;
    delete [] startMaterialPtr;
    delete [] startTexturePtr;
    delete [] startColorPtr;
}

/**********************************************************************/
void ObjFile::ReadMaterialToUse(FILE* _f)
{
    char temp[10];
    char* materialName		= new char[255];
    char* startMaterialPtr	= materialName;
#if defined(_WIN32)
    char* next_token;
#endif
    char endline[] = "\n";

    fgets(temp, 7, _f);					// read the rest of the word "usemtl"
    fgets(materialName, PATH_MAX, _f);	// read the material filename
    // remove the trailing newline
    materialName = strtok(materialName, endline);

    // match up the material name with the materialID
    m_iMaterialID = -1;
    for (unsigned int i=0; i<m_vMaterials.size(); i++)
    {
        if (!strcmp(materialName, m_vMaterials[i].strName))
        {
            m_iMaterialID = i;
            break;
        }
    }

    delete[] startMaterialPtr;
}

/**********************************************************************/
void ObjFile::CenterModel(double xOff, double yOff, double zOff)
{
    size_t nNumObjects = m_vObjects.size();
    for(size_t i = 0; i < nNumObjects; i++) {

        t3DObject *pObject = &m_vObjects[i];
        for(int k = 0; k < pObject->pVerts.size(); k++)					// Go through each vertex
        {
            // offset the verticies, so they come to the origin
            pObject->pVerts[k].x -= xOff;
            pObject->pVerts[k].y -= yOff;
            pObject->pVerts[k].z -= zOff;
        }
    }
}

/**********************************************************************/
float ObjFile::ComputeBoundingSphereRadius(double, double, double)
{    

    float maxRad = 0;
    size_t nNumObjects = m_vObjects.size();
    for(size_t i = 0; i < nNumObjects; i++)
    {
        t3DObject *pObject = &m_vObjects[i];
        for(int k = 0; k < pObject->pVerts.size(); k++)					// Go through each vertex
        {
            // find the maximum distance for a vertex to the origin
            maxRad = qMax(maxRad, sqrtf(pow((float)pObject->pVerts[k].x, 2) + pow((float)pObject->pVerts[k].y, 2) + pow((float)pObject->pVerts[k].z, 2)));
        }
    }
    return maxRad;
}

int ObjFile::getNumObjects() {
    return (int)m_vObjects.size();
}

vector <tFace> &ObjFile::getFaces(int whichObject) {
    return m_vObjects[whichObject].pFaces;
}

vector <Vector> &ObjFile::getVerts(int whichObject) {
    return m_vObjects[whichObject].pVerts;
}

vector <Vector2> &ObjFile::getTexVerts(int whichObject) {
    return m_vObjects[whichObject].pTexVerts;
}

vector <Vector> &ObjFile::getNormals(int whichObject) {
    return m_vObjects[whichObject].pNormals;
}

unsigned int ObjFile::getTextureID(int whichObject) {

    return m_vMaterials[m_vObjects[whichObject].materialID].textureId;

}
