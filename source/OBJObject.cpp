/*
 * OBJ File Loader (to work specifically with exports from Maya 6.0)
 * (supporting both normals/texture mapping as well)
 *
 * THIS VERSION HAS BEEN MODIFIED FOR USE WITH SKETCHENV...  IT 
 * HAS FUNCTIONALITY FOR MODIFYING THE VERTEX INFORMATION DYNAMICALLY,
 * AND AS A RESULT NO LONGER TAKES ADVANTAGE OF OpenGL's DISPLAY LISTS 
 *
 * Author: James McCrae
 */

#include "OBJObject.h"

#define MAXFILENAMELEN 4000

OBJObject::OBJObject() {    
    
}

OBJObject::~OBJObject() {
    
    if (posVertexes!=NULL) {
        
        delete [] posVertexes;
        delete [] texVertexes;
        delete [] normVertexes;
        
        delete [] posFaceindex;
        delete [] texFaceindex;
        delete [] normFaceindex;
        delete [] faceTriangle;
        
    }
    
}

void OBJObject::load(char *filename) {
 
    load("",filename);
    
}

void OBJObject::load(char *directory, char *filename) {
 
    //Load the OBJ file...
    //Format details:

    /*
    # some text
        Line is a comment until the end of the line
    v float float float
        A single vertex's geometric position in space. The first vertex listed 
        in the file has index 1, and subsequent vertices are numbered 
        sequentially.
    vn float float float
        A normal. The first normal in the file is index 1, and subsequent 
        normals are numbered sequentially.
    vt float float
        A texture coordinate. The first texture coordinate in the file is index 
        1, and subsequent textures are numbered sequentially.
    f int int int ...
        or
    f int/int int/int int/int . . .
        or
    f int/int/int int/int/int int/int/int ...
        A polygonal face. The numbers are indexes into the arrays of vertex 
        positions, texture coordinates, and normals respectively. A number may 
        be omitted if, for example, texture coordinates are not being defined 
        in the model.
        
        There is no maximum number of vertices that a single polygon may 
        contain. The .obj file specification says that each face must be flat 
        and convex.
    */
    
    //Let's load the whole file at once...
    char *filecontents;
    
    FILE *openfilehandle;
    FILE *mtlfilehandle;
    long int filelength;    
    
    //assume no texture offsets, unless we find some!
    texture_uoffset=0;
    texture_voffset=0;
    textureindex=0;
    
    //determine file length
    const int OBJFILENAMELEN=1000;
    char *objfilename=(char *)malloc(OBJFILENAMELEN);
    sprintf(objfilename,"%s/%s",directory,filename);
    openfilehandle = fopen(objfilename,"rb");

    if (openfilehandle==NULL) {
        fprintf(stderr, "File %s could not be opened!\n",objfilename);
        exit(10);
    }

    fseek(openfilehandle,0,SEEK_END);
    filelength=ftell(openfilehandle);
    fseek(openfilehandle,0,SEEK_SET);
    
    //suck it all into "filecontents" array and close
    filecontents=(char *)malloc(sizeof(char)*filelength);
    fread(filecontents,1,filelength,openfilehandle);
    fclose(openfilehandle);
        
    //now we need to read through for the number of:
    //  - vertices
    //  - faces
    
    long int curfileindex=0;
    
    int numposvertices=0;   
    int numtexvertices=0;
    int numnormvertices=0;
    int numfaces=0;
    
    //do a simple vertex/face count
    while (curfileindex<filelength) {
        if (filecontents[curfileindex]=='v'&&
            filecontents[curfileindex+1]==' ')
            numposvertices++;
        else if (filecontents[curfileindex]=='v'&&
            filecontents[curfileindex+1]=='t')
            numtexvertices++;
        else if (filecontents[curfileindex]=='v'&&
            filecontents[curfileindex+1]=='n')
            numnormvertices++;
        else if (filecontents[curfileindex]=='f'&&
            filecontents[curfileindex+1]==' ')
            numfaces++;
            
        //go to next line
        while (filecontents[curfileindex]!=10&&curfileindex<filelength)            
            curfileindex++;
        curfileindex++;
    }
    
    printf("Found vertices: %i %i %i\n",numposvertices,numtexvertices,numnormvertices);
    printf("Found faces: %i\n",numfaces);
    printf("File length: %i\n",filelength);
    
    //allocate the vertex arrays
    posVertexes=new GLfloat[numposvertices*3];
    texVertexes=new GLfloat[numtexvertices*2];
    normVertexes=new GLfloat[numnormvertices*3];
    
    //I (perhaps stupidly) assume here that the number of vertices used
    //for each face won't be greater than 4:
    posFaceindex=new GLint[numfaces*4];
    texFaceindex=new GLint[numfaces*4];
    normFaceindex=new GLint[numfaces*4];
    faceTriangle=new bool[numfaces];
    
    //record the values to the vertex arrays, we need indexes into each array
    //to tell where we are at when filling them...
    posVertexIndex=0;
    texVertexIndex=0;
    normVertexIndex=0;
    
    faceindexIndex=0;
    //all faces are assumed quads, unless otherwise discovered
    for (int i=0;i<numfaces;i++)
        faceTriangle[i]=false;
    
    //start back at the beginning of the OBJ data
    curfileindex=0;    
    int curmtllineindex=0;    	

    while (curfileindex<filelength) {     
		
        if (strncmp(&filecontents[curfileindex],"mtllib",6)==0) {
            //attempt to get the texture this OBJ file wants...
            while (filecontents[curfileindex]!=' ') 
				curfileindex++; 
			curfileindex++; 
            
            //read the texture filename in...
            char *mtlfilename;
            mtlfilename=strtok(&filecontents[curfileindex],"\n");
            char *fullmtlfilename=new char[MAXFILENAMELEN];
            sprintf(fullmtlfilename,"%s/%s",directory,mtlfilename);
            
            mtlfilehandle = fopen(fullmtlfilename,"rb");
            //seek till part with "map_Kd" (.mtl specific thing)
            char *eachline=new char[MAXFILENAMELEN];
            do {
                fgets(eachline,255,mtlfilehandle);
            } while (strncmp(eachline,"map_Kd",6)!=0&&!feof(mtlfilehandle));
            
            //any texture offsets? (specified by -o flag after map_Kd)
            curmtllineindex=7;
            if (eachline[curmtllineindex]=='-'&&eachline[curmtllineindex+1]=='o') {
                //printf("Found texture offset parameters!\n");
                //there are offset parameters for the texture
                curmtllineindex=10;
                texture_uoffset=atof(&eachline[curmtllineindex]);
                while (eachline[curmtllineindex]!=' ') curmtllineindex++; 
                curmtllineindex++; 
                texture_voffset=atof(&eachline[curmtllineindex]);
                while (eachline[curmtllineindex]!=' ') curmtllineindex++; 
                curmtllineindex++; 
            }
            
            //take only the filename portion (without a final newline character!!)
            char *texturefilename=new char[MAXFILENAMELEN];
            int texturefilenamelen=0;
            while (eachline[curmtllineindex+texturefilenamelen]!=10) texturefilenamelen++;
            //make the final character a null (char * ender)
            eachline[curmtllineindex+texturefilenamelen]='\0';
            
            strcpy(texturefilename,&eachline[curmtllineindex]);
            
            char *fulltexturefilename=new char[MAXFILENAMELEN];
            sprintf(fulltexturefilename,"%s/%s",directory,texturefilename);
            
            //close the file...
            fclose(mtlfilehandle);
            
            //load the texture                        
            int w, h;
            textureindex = TextureLoader::LoadTexture(fulltexturefilename, w, h);
            
            //free all the character arrays used here for parsing           
            delete [] fullmtlfilename;
            delete [] eachline;
			delete [] texturefilename;
            delete [] fulltexturefilename;
        }
        if (filecontents[curfileindex]=='v'&&
            filecontents[curfileindex+1]==' ') {
                
            curfileindex+=2;
            
            //change those ASCII representations into GLfloats...
            //I use atof(char *string)...
            //position vertices are in 3-space...            
            posVertexes[3*posVertexIndex+0]=atof(&filecontents[curfileindex]);
            while (filecontents[curfileindex]!=' ') curfileindex++; curfileindex++;            
            posVertexes[3*posVertexIndex+1]=atof(&filecontents[curfileindex]);
            while (filecontents[curfileindex]!=' ') curfileindex++; curfileindex++;
            posVertexes[3*posVertexIndex+2]=atof(&filecontents[curfileindex]);
            
            posVertexIndex++;
        }
        else if (filecontents[curfileindex]=='v'&&
            filecontents[curfileindex+1]=='t') {
                
            curfileindex+=3;
            
            //change those ASCII representations into GLfloats...
            //I use atof(char *string)...
            //texture coordinates are 2-dimensional specifically...
            texVertexes[2*texVertexIndex+0]=atof(&filecontents[curfileindex]);            
            while (filecontents[curfileindex]!=' ') curfileindex++; curfileindex++;
            texVertexes[2*texVertexIndex+1]=atof(&filecontents[curfileindex]); 
            
            texVertexIndex++;
        }
        else if (filecontents[curfileindex]=='v'&&
            filecontents[curfileindex+1]=='n') {
                
            curfileindex+=3;
            
            //change those ASCII representations into GLfloats...
            //I use atof(char *string)...
            //normals are in 3-space...
            normVertexes[3*normVertexIndex+0]=atof(&filecontents[curfileindex]);
            while (filecontents[curfileindex]!=' ') curfileindex++; curfileindex++;
            normVertexes[3*normVertexIndex+1]=atof(&filecontents[curfileindex]);
            while (filecontents[curfileindex]!=' ') curfileindex++; curfileindex++;
            normVertexes[3*normVertexIndex+2]=atof(&filecontents[curfileindex]);
            
            normVertexIndex++;
        }
        else if (filecontents[curfileindex]=='f'&&
            filecontents[curfileindex+1]==' ') {
                
            curfileindex+=2;
            
            //for the faces - they are indexes into our vertex arrays,
            //therefore they are integers... so here I use atoi
            //also note the face may contain either 3 or 4 vertices
            //(an assumption in my implementation, this may bite me in the ass
            //later!)
            
            for (int i=0;i<=3;i++) {
            
                posFaceindex[4*faceindexIndex+i]=atoi(&filecontents[curfileindex])-1;
                while (filecontents[curfileindex]!='/') curfileindex++; curfileindex++;
                texFaceindex[4*faceindexIndex+i]=atoi(&filecontents[curfileindex])-1;
                while (filecontents[curfileindex]!='/') curfileindex++; curfileindex++;
                normFaceindex[4*faceindexIndex+i]=atoi(&filecontents[curfileindex])-1;
                //if we run a space first, there's another vertex: a QUAD
                //if we run a newline, well: a TRIANGLE
                do {
                    curfileindex++;
                    if (filecontents[curfileindex]==10) {
                        if (i==2)
                            faceTriangle[faceindexIndex]=true;
                        i=3; //causes us to exit for loop, ending line                
                    }
                } while(filecontents[curfileindex]!=' '&&filecontents[curfileindex]!=10);
                //only advance if it's a space, otherwise let the code
                //at the end of the loop for newline catch it
                if (filecontents[curfileindex]==' ') curfileindex++;
                
            }       
            
            faceindexIndex++;     
            
        }
        
        //go to next line
        while (filecontents[curfileindex]!=10&&curfileindex<filelength)            
            curfileindex++;
        curfileindex++;
    }        

    //all data is now held in various arrays, waiting to be displayed...                 
    
    //don't forget to de-allocate all we've allocated!!
    free(objfilename);
    free(filecontents);
    
}

void OBJObject::draw() {
  
    //loop through and draw all faces    
    if (textureindex>0) {
        glBindTexture(GL_TEXTURE_2D, textureindex);                              
    }
            
    for (int i=0;i<faceindexIndex;i++) {

        if (faceTriangle[i]) {
            //triangles...
            glBegin(GL_TRIANGLES);
                glNormal3f(normVertexes[3*normFaceindex[4*i+0]+0],
                    normVertexes[3*normFaceindex[4*i+0]+1],
                    normVertexes[3*normFaceindex[4*i+0]+2]);
                glTexCoord2f(texVertexes[2*texFaceindex[4*i+0]+0]+texture_uoffset,
                    -texVertexes[2*texFaceindex[4*i+0]+1]+texture_voffset);
                glVertex3d(posVertexes[3*posFaceindex[4*i+0]+0],
                    posVertexes[3*posFaceindex[4*i+0]+1],
                    posVertexes[3*posFaceindex[4*i+0]+2]);
                glNormal3f(normVertexes[3*normFaceindex[4*i+1]+0],
                    normVertexes[3*normFaceindex[4*i+1]+1],
                    normVertexes[3*normFaceindex[4*i+1]+2]);                    
                glTexCoord2f(texVertexes[2*texFaceindex[4*i+1]+0]+texture_uoffset,
                    -texVertexes[2*texFaceindex[4*i+1]+1]+texture_voffset);                    
                glVertex3d(posVertexes[3*posFaceindex[4*i+1]+0],
                    posVertexes[3*posFaceindex[4*i+1]+1],
                    posVertexes[3*posFaceindex[4*i+1]+2]);
                glNormal3f(normVertexes[3*normFaceindex[4*i+2]+0],
                    normVertexes[3*normFaceindex[4*i+2]+1],
                    normVertexes[3*normFaceindex[4*i+2]+2]);
                glTexCoord2f(texVertexes[2*texFaceindex[4*i+2]+0]+texture_uoffset,
                    -texVertexes[2*texFaceindex[4*i+2]+1]+texture_voffset); 
                glVertex3d(posVertexes[3*posFaceindex[4*i+2]+0],
                    posVertexes[3*posFaceindex[4*i+2]+1],
                    posVertexes[3*posFaceindex[4*i+2]+2]);
            glEnd();            

        }
        else {
            //do quads too... 
            glBegin(GL_QUADS);
                glNormal3f(normVertexes[3*normFaceindex[4*i+0]+0],
                    normVertexes[3*normFaceindex[4*i+0]+1],
                    normVertexes[3*normFaceindex[4*i+0]+2]);
                glTexCoord2f(texVertexes[2*texFaceindex[4*i+0]+0]+texture_uoffset,
                    -texVertexes[2*texFaceindex[4*i+0]+1]+texture_voffset);
                glVertex3d(posVertexes[3*posFaceindex[4*i+0]+0],
                    posVertexes[3*posFaceindex[4*i+0]+1],
                    posVertexes[3*posFaceindex[4*i+0]+2]);
                glNormal3f(normVertexes[3*normFaceindex[4*i+1]+0],
                    normVertexes[3*normFaceindex[4*i+1]+1],
                    normVertexes[3*normFaceindex[4*i+1]+2]);                    
                glTexCoord2f(texVertexes[2*texFaceindex[4*i+1]+0]+texture_uoffset,
                    -texVertexes[2*texFaceindex[4*i+1]+1]+texture_voffset);  
                glVertex3d(posVertexes[3*posFaceindex[4*i+1]+0],
                    posVertexes[3*posFaceindex[4*i+1]+1],
                    posVertexes[3*posFaceindex[4*i+1]+2]);
                glNormal3f(normVertexes[3*normFaceindex[4*i+2]+0],
                    normVertexes[3*normFaceindex[4*i+2]+1],
                    normVertexes[3*normFaceindex[4*i+2]+2]);                    
                glTexCoord2f(texVertexes[2*texFaceindex[4*i+2]+0]+texture_uoffset,
                    -texVertexes[2*texFaceindex[4*i+2]+1]+texture_voffset); 
                glVertex3d(posVertexes[3*posFaceindex[4*i+2]+0],
                    posVertexes[3*posFaceindex[4*i+2]+1],
                    posVertexes[3*posFaceindex[4*i+2]+2]);
                glNormal3f(normVertexes[3*normFaceindex[4*i+3]+0],
                    normVertexes[3*normFaceindex[4*i+3]+1],
                    normVertexes[3*normFaceindex[4*i+3]+2]);
                glTexCoord2f(texVertexes[2*texFaceindex[4*i+3]+0]+texture_uoffset,
                    -texVertexes[2*texFaceindex[4*i+3]+1]+texture_voffset); 
                glVertex3d(posVertexes[3*posFaceindex[4*i+3]+0],
                    posVertexes[3*posFaceindex[4*i+3]+1],
                    posVertexes[3*posFaceindex[4*i+3]+2]);
            glEnd();
        }

    }
    
}

GLfloat *OBJObject::getPosVertexes() {    
    return posVertexes;    
}
    
void OBJObject::setPosVertexes(GLfloat *posVertexes) {
    this->posVertexes=posVertexes;
}

int OBJObject::getNumPosVertexes() {
    return posVertexIndex;
}

int OBJObject::GetNumPotentialColliders() {
	return faceindexIndex;
}
   
Vector *OBJObject::GetPotentialColliders() {

	Vector *colliderList=new Vector[faceindexIndex*3];

	for (int i=0;i<faceindexIndex;i++) {

        colliderList[i*3]=Vector(posVertexes[3*posFaceindex[4*i+0]+0],
                    posVertexes[3*posFaceindex[4*i+0]+1],
                    posVertexes[3*posFaceindex[4*i+0]+2]);
        colliderList[i*3+1]=Vector(posVertexes[3*posFaceindex[4*i+1]+0],
                    posVertexes[3*posFaceindex[4*i+1]+1],
                    posVertexes[3*posFaceindex[4*i+1]+2]);
        colliderList[i*3+2]=Vector(posVertexes[3*posFaceindex[4*i+2]+0],
                    posVertexes[3*posFaceindex[4*i+2]+1],
                    posVertexes[3*posFaceindex[4*i+2]+2]);
            
    }
    

	return colliderList;

}
