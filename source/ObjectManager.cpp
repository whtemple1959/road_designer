#include "ObjectManager.h"

vector <TriMeshTextured *> ObjectManager::meshes;

ObjectManager::ObjectManager(RN *roadNetwork, HeightMap *heightMap) {

	this->roadNetwork=roadNetwork;
	this->heightMap=heightMap;	

	const char *objFilenames[] = {
		"objs/assets/stopsign.obj",
		"objs/assets/lamppost.obj",
		"objs/assets/tree1_trunk.obj",
		"objs/assets/tree1_branch.obj",
		"objs/assets/tree2_trunk.obj",
		"objs/assets/tree2_branch.obj",
		"objs/assets/grass.obj",
		"objs/assets/plant1.obj",
		"objs/assets/plant2_stem.obj",
		"objs/assets/plant2_leaf.obj",
		"objs/assets/pillar.obj",
		"objs/assets/warning_right.obj",
		"objs/assets/warning_left.obj",
		"objs/assets/warning_Scurve.obj",
		"objs/assets/warning_intersect.obj",
		"objs/assets/warning_steep.obj",
		"objs/assets/warning_bump.obj",
		"objs/assets/warning_narrowbridge.obj",
		"objs/assets/warning_bull.obj",
		"objs/assets/warning_stopahead.obj",
		"objs/assets/warning_trafficlights.obj",
		"objs/assets/warning_dip.obj",
		"objs/assets/flare.obj",
		"objs/assets/warning_solidobject2.obj",
		"objs/assets/warning_speed_slow.obj",
		"objs/assets/warning_speed_avg.obj",
		"objs/assets/warning_speed_fast.obj"
	};

	if (meshes.empty()) {

		for (int i=0;i<27;i++)  {
			TriMeshTextured *eachMesh=new TriMeshTextured();
			eachMesh->Read(objFilenames[i]);

			meshes.push_back(eachMesh);
		}

	}	

        clearRange=vector <vector <char> >(heightMap->getWidth(), vector <char> (heightMap->getHeight(), 0));

}

ObjectManager::~ObjectManager() {

    position.clear();
    type.clear();
    scale.clear();
    orientation.clear();

    //cleanup
    for (int i=0;i<clearRange.size();i++) {
        clearRange[i].clear();
    }
    clearRange.clear();

}

void ObjectManager::addObject(Vector pos, float orient, int t, float s) {	

	position.push_back(pos);
	orientation.push_back(orient);
	type.push_back(t);
	scale.push_back(s);

}

void ObjectManager::plopDownObject(Vector pos, float orient, int t, float s) {	

	Vector heightMapCollidePoint;
	if (heightMap->selectWithRay(pos+Vector(0.0f,1000.0f,0.0f),Vector(0.0f,-1.0f,0.0f),heightMapCollidePoint)) {

		if (t==OBJ_TYPE_PILLAR)
			addObject(heightMapCollidePoint, orient, t, s-heightMapCollidePoint.y);
		else 
			addObject(heightMapCollidePoint, orient, t, s);

	}

}

int ObjectManager::getNumObjects() {
	return type.size();
}

void ObjectManager::drawObjects() {
	
	glColor3f(1,1,1);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);	

	for (int i=0;i<type.size();i++) {
				
		glPushMatrix();	
			
			//perform twist if vector is not empty	
			glTranslatef(position[i].x,position[i].y,position[i].z);
			glRotatef(orientation[i],0,1,0);			

			if (type[i]==OBJ_TYPE_PILLAR) 
				glScalef(MIN_DIST_BETWEEN_POINTS, scale[i], MIN_DIST_BETWEEN_POINTS);
			else
				glScalef(scale[i],scale[i],scale[i]);

			if (type[i]==OBJ_TYPE_STOPSIGN) {				
				meshes[OBJ_MESH_STOPSIGN]->Draw();
			}
			else if (type[i]==OBJ_TYPE_LAMPPOST) 
				meshes[OBJ_MESH_LAMPPOST]->Draw();
			else if (type[i]==OBJ_TYPE_TREE1) {				
				meshes[OBJ_MESH_TREE1_TRUNK]->Draw();
				meshes[OBJ_MESH_TREE1_BRANCHES]->Draw();
			}
			else if (type[i]==OBJ_TYPE_TREE2){				
				meshes[OBJ_MESH_TREE2_TRUNK]->Draw();
				meshes[OBJ_MESH_TREE2_BRANCHES]->Draw();
			}
			else if (type[i]==OBJ_TYPE_GRASS){				
				meshes[OBJ_MESH_GRASS]->Draw();
				meshes[OBJ_MESH_GRASS]->Draw();
			}
			else if (type[i]==OBJ_TYPE_PLANT1){				
				meshes[OBJ_MESH_PLANT1]->Draw();				
			}
			else if (type[i]==OBJ_TYPE_PLANT2){				
				meshes[OBJ_MESH_PLANT2_STEM]->Draw();
				meshes[OBJ_MESH_PLANT2_LEAF]->Draw();				
			}
			else if (type[i]==OBJ_TYPE_PILLAR) {
				meshes[OBJ_MESH_PILLAR]->Draw();
			}
			else if (type[i]==OBJ_TYPE_WARNING_RIGHT) 
				meshes[OBJ_MESH_WARNING_RIGHT]->Draw();
			else if (type[i]==OBJ_TYPE_WARNING_LEFT) 
				meshes[OBJ_MESH_WARNING_LEFT]->Draw();
			else if (type[i]==OBJ_TYPE_WARNING_SCURVE) 
				meshes[OBJ_MESH_WARNING_SCURVE]->Draw();
			else if (type[i]==OBJ_TYPE_WARNING_INTERSECT) 
				meshes[OBJ_MESH_WARNING_INTERSECT]->Draw();
			else if (type[i]==OBJ_TYPE_WARNING_STEEP) 
				meshes[OBJ_MESH_WARNING_STEEP]->Draw();
			else if (type[i]==OBJ_TYPE_WARNING_BUMP) 
				meshes[OBJ_MESH_WARNING_BUMP]->Draw();
			else if (type[i]==OBJ_TYPE_WARNING_NARROWBRIDGE) 
				meshes[OBJ_MESH_WARNING_NARROWBRIDGE]->Draw();
			else if (type[i]==OBJ_TYPE_WARNING_BULL) 
				meshes[OBJ_MESH_WARNING_BULL]->Draw();
			else if (type[i]==OBJ_TYPE_WARNING_STOPAHEAD) 
				meshes[OBJ_MESH_WARNING_STOPAHEAD]->Draw();
			else if (type[i]==OBJ_TYPE_WARNING_TRAFFICLIGHTS) 
				meshes[OBJ_MESH_WARNING_TRAFFICLIGHTS]->Draw();
			else if (type[i]==OBJ_TYPE_WARNING_DIP) 
				meshes[OBJ_MESH_WARNING_DIP]->Draw();
			else if (type[i]==OBJ_TYPE_FLARE) {
				meshes[OBJ_MESH_FLARE]->Draw();
			}
			else if (type[i]==OBJ_TYPE_WARNING_SOLIDOBJECT) 
				meshes[OBJ_MESH_WARNING_SOLIDOBJECT]->Draw();
			else if (type[i]==OBJ_TYPE_WARNING_SPEED_SLOW) 
				meshes[OBJ_MESH_WARNING_SPEED_SLOW]->Draw();
			else if (type[i]==OBJ_TYPE_WARNING_SPEED_AVG) 
				meshes[OBJ_MESH_WARNING_SPEED_AVG]->Draw();
			else if (type[i]==OBJ_TYPE_WARNING_SPEED_FAST) 
				meshes[OBJ_MESH_WARNING_SPEED_FAST]->Draw();
	
		glPopMatrix();

	}

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

}

void ObjectManager::drawObjectsUnderwater() {

	///* REMOVED FOR GEOMETRY MODELLING EXAMPLE
	
	glColor3f(1,1,1);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	for (int i=0;i<type.size();i++) {

		if (position[i].y>heightMap->getWaterHeight()) //only render when beneath water level
			continue;

		if (type[i]==OBJ_TYPE_PILLAR&&position[i].y<heightMap->getWaterHeight()) {
			glPushMatrix();
				

				glTranslatef(position[i].x,position[i].y,position[i].z);			
				glRotatef(orientation[i],0,1,0);					
				glScalef(MIN_DIST_BETWEEN_POINTS, scale[i], MIN_DIST_BETWEEN_POINTS);
				
				meshes[OBJ_MESH_PILLAR]->Draw();
			glPopMatrix();
		}	

	}
	//*/

}

ObjectManager *ObjectManager::getClone(RN *newRoadNetwork) {

	ObjectManager *newObjManager=new ObjectManager(newRoadNetwork, heightMap);

	newObjManager->position=vector <Vector> (position.size());
	newObjManager->orientation=vector <float> (orientation.size());
	newObjManager->scale=vector <float> (scale.size());
	newObjManager->type=vector <int> (type.size());

	std::copy(position.begin(), position.end(), newObjManager->position.begin());
	std::copy(orientation.begin(), orientation.end(), newObjManager->orientation.begin());
	std::copy(scale.begin(), scale.end(), newObjManager->scale.begin());
	std::copy(type.begin(), type.end(), newObjManager->type.begin());		

	newObjManager->updateClearRange();

	return newObjManager;
}

void ObjectManager::addFoliage(vector <Vector> lassoPoints3D, float foliageDensity) {

	//foliage will be randomly added within triangles of the 3d lasso
	//probability of being in a particular triangle is proportional to area of triangle
	
	if (lassoPoints3D.size()<3)
		return;

	for (int i=0;i<lassoPoints3D.size();i++) 
		lassoPoints3D[i].y=0.0f;	

	vector <float> triangleAreas;
	vector <float> shortChanged;

	triangleAreas.push_back(0.0f);
	shortChanged.push_back(0.0f);
	float totalTriangleAreas=0.0f;
	for (int i=1;i<lassoPoints3D.size()-1;i++) {
		triangleAreas.push_back(triangleArea(lassoPoints3D[0],lassoPoints3D[i],lassoPoints3D[i+1]));
		shortChanged.push_back(triangleAreas[triangleAreas.size()-1]);
		totalTriangleAreas+=triangleAreas[triangleAreas.size()-1];
	}

	//int numObjects=(int)ceil(totalTriangleAreas/(64.0f*MIN_DIST_BETWEEN_POINTS))*foliageDensity;
	int numObjects=(int)ceil(totalTriangleAreas/(256*MIN_DIST_BETWEEN_POINTS))*foliageDensity; //modified for just trees case
	float component1, component2;

	//printf("lassopoints: %i numobjects: %i\n",lassoPoints3D.size(), numObjects);

        for (int j=0; j<numObjects; ++j) {

		//find most needy triangle (for uniform distribution)
		int biggestIndex=0;
		float mostInNeed=0.0;

		for (int i=1;i<lassoPoints3D.size()-1;i++) {		
			if (shortChanged[i]>mostInNeed) {
				mostInNeed=shortChanged[i];
				biggestIndex=i;					
			}
		}	

		//sample particle in this triangle
		//with the biggest index, we give it a particle
		float component1, component2;
		int rand1, rand2;

		rand1=rand()%100;
		rand2=rand()%100-rand1;

		component1=(float)(rand1)/100.0;
		component2=(float)(rand2)/100.0;
			
		Vector component1Vec=lassoPoints3D[biggestIndex]-lassoPoints3D[0];
		Vector component2Vec=lassoPoints3D[biggestIndex+1]-lassoPoints3D[0];
		component1Vec.StretchDimensions(component1,component1,component1);
		component2Vec.StretchDimensions(component2,component2,component2);

		//this is foliage position
		Vector projectVec=lassoPoints3D[0]+component1Vec+component2Vec+Vector(0.0f,256.0f,0.0f);
		Vector newPos;	

                heightMap->selectWithRay(projectVec, Vector(0.0f,-1.0f,0.0f), newPos);

                if (newPos.x < 0 || newPos.x >= heightMap->getWidth() || newPos.z < 0 || newPos.z >= heightMap->getHeight()) {
                    continue;
                }

		//make sure that its not too close to an existing road		
                if (clearRange[(int)newPos.x][(int)newPos.z]%2 == 1) {
			numObjects--;
			continue;
		}

		//make sure its not underwater
                if (newPos.y<=heightMap->getWaterHeight()) {
			numObjects--;
			continue;
		}

		position.push_back(newPos);
		orientation.push_back((float)(rand()%360));
		scale.push_back(MIN_DIST_BETWEEN_POINTS/2.0f+((float)(rand()%100)/100.0f)*MIN_DIST_BETWEEN_POINTS);
		int foliageType=rand()%2; //modified for TREES ONLY!
		if (foliageType==0)
			type.push_back(OBJ_TYPE_TREE1);
		else if (foliageType==1)
			type.push_back(OBJ_TYPE_TREE2);
		else if (foliageType==2)
			type.push_back(OBJ_TYPE_GRASS);
		else if (foliageType==3)
			type.push_back(OBJ_TYPE_PLANT1);
		else if (foliageType==4)
			type.push_back(OBJ_TYPE_PLANT2);

		//update how much this triangle needs a particle
		shortChanged[biggestIndex]=triangleAreas[biggestIndex]/((1.0/shortChanged[biggestIndex])*triangleAreas[biggestIndex]+1.0f);

		numObjects--;

	};

}

float ObjectManager::triangleArea(Vector v1, Vector v2, Vector v3) {
	
	Vector a=v2-v1;
	Vector b=v3-v1;

	//sine of angle between vectors
	float sineOfAngle=acos(a.DotProduct3(b)/(a.GetLength()*b.GetLength()));
	return (sineOfAngle*a.GetLength()*b.GetLength())/2.0;

}

void ObjectManager::updateClearRange() {	

	for (int i=0;i<clearRange.size();i++) {
		for (int j=0;j<clearRange[i].size();j++)
			clearRange[i][j]=0;
	}

	//construct clear range
	for (int e=0;e<roadNetwork->getNumEdges();e++) {
		for (int i=0;i<roadNetwork->getEdge(e)->getNumPoints();i++) {
			int eachx=roadNetwork->getEdge(e)->getPoint(i).x;
			int eachy=roadNetwork->getEdge(e)->getPoint(i).z;

			//printf("width: %i height: %i\n",heightMap->getWidth(),heightMap->getHeight());
			//printf("eachx: %i eachy: %i\n",eachx,eachy);
			//printf("clearrange sizes: %i %i\n",clearRange.size(),clearRange[0].size());

			//set up where foliage can't be
			for (int j=eachx-(int)MIN_DIST_BETWEEN_POINTS;j<=eachx+(int)MIN_DIST_BETWEEN_POINTS;j++) {
				for (int k=eachy-(int)MIN_DIST_BETWEEN_POINTS;k<=eachy+(int)MIN_DIST_BETWEEN_POINTS;k++) {
					if (j>=0&&k>=0&&j<heightMap->getWidth()&&k<heightMap->getHeight()) {
						if (clearRange[j][k]==0)
							clearRange[j][k]=1;
						else if (clearRange[j][k]==2)
							clearRange[j][k]=3;
					}
				}
			}

			//set up where stop signs CAN be (should use the pillar method, its easier)
			if (roadNetwork->getEdge(e)->getPointAttrib(i,POINT_ATTRIB_INTERSECT)) {
				for (int j=eachx-(int)MIN_DIST_BETWEEN_POINTS;j<=eachx+(int)MIN_DIST_BETWEEN_POINTS;j++) {
					for (int k=eachy-(int)MIN_DIST_BETWEEN_POINTS;k<=eachy+(int)MIN_DIST_BETWEEN_POINTS;k++) {
						if (j>=0&&k>=0&&j<heightMap->getWidth()&&k<heightMap->getHeight()) {

							if (clearRange[j][k]==1)
								clearRange[j][k]=3;
							else 
								clearRange[j][k]=2;
							
						}
					}		
				}
			}
		}
	}	

	//check if anything needs clearing
	for (int i=0;i<type.size();i++) {		

		int eachx=position[i].x;
		int eachy=position[i].z;

		if (type[i]==OBJ_TYPE_TREE1||
			type[i]==OBJ_TYPE_TREE2||
			type[i]==OBJ_TYPE_GRASS||
			type[i]==OBJ_TYPE_PLANT1||
			type[i]==OBJ_TYPE_PLANT2) {			

			//remove plant if within clear range
			if ((clearRange[eachx][eachy]%2)==1) {
				remove(i);
				i--;
			}
		}
	}		

}

void ObjectManager::remove(int index) {
	position.erase(position.begin()+index);
	orientation.erase(orientation.begin()+index);
	scale.erase(scale.begin()+index);
	type.erase(type.begin()+index);
}

void ObjectManager::removeAll() {

	position.clear();
	orientation.clear();
	scale.clear();
	type.clear();

}

void ObjectManager::removeType(int theType) {

	for (int i=0;i<type.size();i++) {
		if (type[i]==theType) {
			remove(i);
			i--;
		}
	}

}

Vector ObjectManager::getObjectPos(int index) {
	return position[index];
}

void ObjectManager::export_DAT(FILE *outFile) {

	/*
	outStr << "OBJECTS " << type.size() << "\n";

	for (int i=0;i<type.size();i++) {

		outStr << type[i] << " " << position[i].x << " " << position[i].y << " " << position[i].z << " " << orientation[i] << " " << scale[i] << "\n";

	}
	*/

	fprintf(outFile,"OBJECTS %i\n",type.size());
	for (int i=0;i<type.size();i++) 
		fprintf(outFile,"%i %f %f %f %f %f\n",type[i],position[i].x,position[i].y,position[i].z,orientation[i],scale[i]);

}
	
void ObjectManager::import_DAT(FILE *inf) {

	removeAll();

	char buf[256];

	fgets(buf,256,inf);

	int numObjects;
	sscanf(buf,"OBJECTS %i",&numObjects);
	
	for (int i=0;i<numObjects;i++) {
		fgets(buf,256,inf);

		int eachType;
		Vector eachPos;
		float eachOrient;
		float eachScale;
		sscanf(buf,"%i %f %f %f %f %f",&eachType,&eachPos.x,&eachPos.y,&eachPos.z,&eachOrient,&eachScale);

		type.push_back(eachType);
		position.push_back(eachPos);
		orientation.push_back(eachOrient);
		scale.push_back(eachScale);

	}

}
