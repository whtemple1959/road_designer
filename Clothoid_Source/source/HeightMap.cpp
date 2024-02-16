#include "HeightMap.h"

HeightMap::HeightMap() {	

	width=height=192;	
	
}

int HeightMap::getWidth() {
	return width;
}

int HeightMap::getHeight() {
	return height;
}

void HeightMap::draw(Camera *mainCamera) {

	glColor3f(.7,.7,.7);
	glLineWidth(2.0f);
	glBegin(GL_LINE_LOOP);
		glVertex3f(0,0,0);
		glVertex3f(width,0,0);
		glVertex3f(width,0,height);
		glVertex3f(0,0,height);
	glEnd();

}

Vector HeightMap::getSelectLocation() {
	return selectLocation;
}
	
void HeightMap::selectWithClosedCurve(vector <Vector> pointSet) {
	selected=false;

}

vector <Vector> HeightMap::selectWithRays(Vector pos_xyz, vector <Vector> rays) {

	vector <Vector> returnVec;

	for (int i=0;i<rays.size();i++) {
		selectWithRay(pos_xyz, rays[i]);
		if (selected)
			returnVec.push_back(selectLocation);
	}

	return returnVec;

}

void HeightMap::selectWithRay(Vector pos_xyz, Vector pickray) {
      
	selected=false;    

    Vector v1;
    Vector v2;
    Vector v3;    
    
	float closestselectdist=MAXSELECTDISTANCE;    

	 for (int each=0;each<2;each++) {

		//vertices for each triangle    
		if (each==0) {
			v1=Vector(0, 0, 0);
			v2=Vector(0, 0, height);
			v3=Vector(width, 0, height);                                 
		}
		else {
			v1=Vector(0, 0, 0);			
			v2=Vector(width, 0, height);      
			v3=Vector(width, 0, 0);
		}

        
		//check with each triangle
		Vector *resultThere=Collision::LineTriangleCollide(pos_xyz, pickray, v1, v2, v3);
        
		if (resultThere!=NULL) {                           
                
			if ((pos_xyz-resultThere[0]).GetLength()<=closestselectdist) {

				selectLocation=resultThere[0];                
				closestselectdist=(pos_xyz-resultThere[0]).GetLength();
				selected=true;         

			}
                
			delete [] resultThere;   

		}
                      
	}	
    
}

Vector HeightMap::getDefaultCameraTrackPos() {

	selectWithRay(Vector(width/2.0f,1000.0f,height/2.0f),Vector(0.0f,-1.0f,0.0f));
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
	selectWithRay(Vector(x,1000.0f,z),Vector(0.0f,-1.0f,0.0f));

	if (selected)
		return selectLocation.y;
	else
		return 0.0f;

}