#include "ClothoidSpline_older.h"

ClothoidSpline_older::ClothoidSpline_older(int num, Vector *set, float *curvatures) {

	ClothoidSpline_older(num,set);
	pointSetCurvatures=curvatures;

}

ClothoidSpline_older::ClothoidSpline_older(int num, Vector *set) {

	numPoints=num;
	pointSet=set;

}

void ClothoidSpline_older::setupSpline() {

	//assign the tangents
	pointSetTangents=new Vector[numPoints];
	pointSetTangents[0]=(pointSet[1].getNormal()-pointSet[0].getNormal()).getNormal();
	for (int i=1;i<numPoints-1;i++) {
		Vector v1=pointSet[i+1]-pointSet[i];
		v1.y=0.0; v1.Normal();
		Vector v2=pointSet[i]-pointSet[i-1];
		v2.y=0.0; v2.Normal();
		pointSetTangents[i]=(v1+v2).getNormal(); //unnormalized (one vec can contribute way more)
	}
	pointSetTangents[numPoints-1]=(pointSet[numPoints-1]-pointSet[numPoints-2]).getNormal();

	//later, tangents will be provided by the line and circle segments
	pointSetTangents[0]=Vector(1.0,0,0);
	pointSetTangents[1]=Vector(1,0,-.5);
	pointSetTangents[1].Normal();
	pointSetTangents[2]=Vector(.5,0,-1);
	pointSetTangents[2].Normal();
	pointSetTangents[3]=Vector(-1,0,-.5);
	pointSetTangents[4]=Vector(-1,0,0);
	pointSetTangents[5]=Vector(-1,0,0);
	pointSetTangents[6]=Vector(0,0,1);
	pointSetTangents[7]=Vector(0.2,0,1);
	pointSetTangents[7].Normal();
	
	//need to iterate between each neighbouring pair of points in the set,
	//and compose a path out of straight lines, circular segments,
	//and clothoid segments (linear curvature changes)

	//initialize clothoid answer vectors
	t=new float[numPoints-1];
	B=new float[numPoints-1];
	x=new float[numPoints-1];
	curveRight=new bool[numPoints-1];

	for (int i=0;i<numPoints-1;i++) {
		t[i]=0.0;
		B[i]=0.0;
		x[i]=0.0;
		curveRight[i]=true;
	}

	for (int i=0;i<numPoints-1;i++) {

		//first arrange P and Q into "standard form", where P is at the origin,
		//and the tangent vector of P points along the x-axis		
		
		float rotRadians=atan2(pointSetTangents[i].z,pointSetTangents[i].x); //this might have to be rotated the other way!
		Vector P=Vector(0,0,0);
		Vector Q=(pointSet[i+1]-pointSet[i]).GetRotatedYRadians(rotRadians);		
		Vector tangent_P=Vector(1,0,0);
		Vector tangent_Q=pointSetTangents[i+1].GetRotatedYRadians(rotRadians);
		
		float K_P=pointSetCurvatures[i];
		float K_Q=pointSetCurvatures[i+1];						

		//New, intuitive way!

		//Things that must be:
		//Curvature at spiral end must match curvature of circle
		//Tangent at spiral end must match curvature of circle
		//Z Location of spiral end must be Qz

		//Variables:
		//t B Q_z

		//Seems like 3 equations with 3 unknowns

		if (K_P==0.0&&K_Q>0.0) { //LINE TO CIRCLE 

			Vector perpVec;
			if (Q.z>0.0) {
				perpVec=tangent_Q.GetRotatedY(-90.0);
				curveRight[i]=true;
			}
			else if (Q.z<0.0) {
				Q.z=-Q.z;
				tangent_Q.z=-tangent_Q.z;
				perpVec=tangent_Q.GetRotatedY(-90.0);
				curveRight[i]=false;
			}

			//Determine circle centre point
			float radius=(1.0/K_Q);			
			perpVec.SetLength(radius);

			Vector circleCentre=Q+perpVec;

			//Z value of new Q must be kept between perpVec.z-radius and perpVec.z+radius		

			//solve crazy equation for Q_Z
			//i use a newton-like method here and iterate until sorta
			//convergence				
			Vector newQ=Q;
			newQ.z=Q.z;
			float deltaQZ=0.01;
			float errorThreshold=0.000001;
			float deltaQZThreshold=0.000001;

			float leftCurrentQZ=0.0;
			float rightCurrentQZ=0.0;
			float atCurrentQZ=0.0;		

			float tangent_newQ_x;
			float tangent_newQ_z;

			do {			

				leftCurrentQZ=doLineCircleFormula(newQ.z-deltaQZ, circleCentre, radius, K_Q, tangent_newQ_x, tangent_newQ_z, newQ.x);
				atCurrentQZ=doLineCircleFormula(newQ.z, circleCentre, radius, K_Q, tangent_newQ_x, tangent_newQ_z, newQ.x);
				rightCurrentQZ=doLineCircleFormula(newQ.z+deltaQZ, circleCentre, radius, K_Q, tangent_newQ_x, tangent_newQ_z, newQ.x);				

				if (fabs(leftCurrentQZ)<fabs(rightCurrentQZ)&&fabs(leftCurrentQZ)<fabs(atCurrentQZ)) {
					newQ.z=newQ.z-deltaQZ;
				}
				else if (fabs(rightCurrentQZ)<fabs(leftCurrentQZ)&&fabs(rightCurrentQZ)<fabs(atCurrentQZ)){
					newQ.z=newQ.z+deltaQZ;
				}
				else {
					deltaQZ=deltaQZ/2.0;
				}

				if (newQ.z<circleCentre.z-radius||newQ.z>circleCentre.z+radius) {
					printf("Don't think there's a solution here!\n");			
					break;
				}

			} while (fabs(atCurrentQZ)>errorThreshold&&deltaQZ>deltaQZThreshold);	

			doLineCircleFormula(newQ.z, circleCentre, radius, K_Q, tangent_newQ_x, tangent_newQ_z, newQ.x);

			//can now calculate variables with known Q_Z
			t[i]=sqrt(2/PI*atan2(tangent_newQ_z, tangent_newQ_x));
			B[i]=(1/K_Q)*t[i];

			//need the shifted x-value
			x[i]=newQ.x-PI*B[i]*sqrt(2/PI)*computeCosFresnelIntegral(t[i]/sqrt(2/PI));		

			//debugging info
			printf("Solved for t, B: %f %f\n",t[i],B[i]);
			printf("Tangent at point should be: %f %f\n",tangent_newQ_x,tangent_newQ_z);
			printf("atan2 of circle tangent: %f\n",atan2(tangent_newQ_z, tangent_newQ_x));
			printf("atan2 of clothoid tangent: %f\n",t[i]*t[i]*PI/2);

		}
		else if (K_P>0.0&&K_Q==0.0) { //CIRCLE TO LINE

			//in this case though, the circle is probably BEHIND me!
			//to get around this, i'm flipping both tangents
			//also, K_P and K_Q are reversed
			rotRadians=atan2(-pointSetTangents[i+1].z,-pointSetTangents[i+1].x); //this might have to be rotated the other way!
			P=Vector(0,0,0);
			Q=(pointSet[i]-pointSet[i+1]).GetRotatedYRadians(rotRadians);	
			tangent_P=Vector(1,0,0);
			tangent_Q=pointSetTangents[i].GetRotatedYRadians(rotRadians);
			tangent_Q.x=-tangent_Q.x;
			tangent_Q.z=-tangent_Q.z;
			
			K_P=pointSetCurvatures[i+1];
			K_Q=pointSetCurvatures[i];

			//totally reverse these																
						
			Vector perpVec;
			if (Q.z>0.0) {
				perpVec=tangent_Q.GetRotatedY(-90.0);
				curveRight[i]=true;
			}
			else if (Q.z<0.0) {
				Q.z=-Q.z;
				tangent_Q.z=-tangent_Q.z;
				perpVec=tangent_Q.GetRotatedY(-90.0);
				curveRight[i]=false;
			}			

			//Determine circle centre point
			float radius=(1.0/K_Q);			
			perpVec.SetLength(radius);

			Vector circleCentre=Q+perpVec;

			//Z value of new Q must be kept between perpVec.z-radius and perpVec.z+radius		

			//solve crazy equation for Q_Z
			//i use a newton-like method here and iterate until sorta
			//convergence				
			Vector newQ=Q;
			newQ.z=Q.z;
			float deltaQZ=0.01;
			float errorThreshold=0.000001;
			float deltaQZThreshold=0.000001;

			float leftCurrentQZ=0.0;
			float rightCurrentQZ=0.0;
			float atCurrentQZ=0.0;		

			float tangent_newQ_x;
			float tangent_newQ_z;

			do {			

				leftCurrentQZ=doLineCircleFormula(newQ.z-deltaQZ, circleCentre, radius, K_Q, tangent_newQ_x, tangent_newQ_z, newQ.x);
				atCurrentQZ=doLineCircleFormula(newQ.z, circleCentre, radius, K_Q, tangent_newQ_x, tangent_newQ_z, newQ.x);
				rightCurrentQZ=doLineCircleFormula(newQ.z+deltaQZ, circleCentre, radius, K_Q, tangent_newQ_x, tangent_newQ_z, newQ.x);				

				if (fabs(leftCurrentQZ)<fabs(rightCurrentQZ)&&fabs(leftCurrentQZ)<fabs(atCurrentQZ)) {
					newQ.z=newQ.z-deltaQZ;
				}
				else if (fabs(rightCurrentQZ)<fabs(leftCurrentQZ)&&fabs(rightCurrentQZ)<fabs(atCurrentQZ)){
					newQ.z=newQ.z+deltaQZ;
				}
				else {
					deltaQZ=deltaQZ/2.0;
				}

				if (newQ.z<circleCentre.z-radius||newQ.z>circleCentre.z+radius) {
					printf("Don't think there's a solution here!\n");			
					break;
				}

			} while (fabs(atCurrentQZ)>errorThreshold&&deltaQZ>deltaQZThreshold);	

			doLineCircleFormula(newQ.z, circleCentre, radius, K_Q, tangent_newQ_x, tangent_newQ_z, newQ.x);

			//can now calculate variables with known Q_Z
			t[i]=sqrt(2/PI*atan2(tangent_newQ_z, tangent_newQ_x));
			B[i]=(1/K_Q)*t[i];

			//need the shifted x-value
			x[i]=newQ.x-PI*B[i]*sqrt(2/PI)*computeCosFresnelIntegral(t[i]/sqrt(2/PI));		

			//debugging info
			printf("Solved for t, B: %f %f\n",t[i],B[i]);
			printf("Tangent at point should be: %f %f\n",tangent_newQ_x,tangent_newQ_z);
			printf("atan2 of circle tangent: %f\n",atan2(tangent_newQ_z, tangent_newQ_x));
			printf("atan2 of clothoid tangent: %f\n",t[i]*t[i]*PI/2);

		}
		else if (K_P>0.0&&K_Q>0.0) { //CIRCLE TO CIRCLE

			Vector circleCentre1;
			Vector circleCentre2;						

			//Make Q always above the z-axis
			if (Q.z>0.0) {				
				curveRight[i]=true;
			}
			else if (Q.z<0.0) {
				Q.z=-Q.z;
				tangent_Q.z=-tangent_Q.z;				
				curveRight[i]=false;
			}	

			//determine circle centres
			circleCentre1=tangent_P.GetRotatedY(-90.0);
			circleCentre1.SetLength(1.0/K_P);
			circleCentre1=circleCentre1+P;

			circleCentre2=tangent_Q.GetRotatedY(-90.0);
			circleCentre2.SetLength(1.0/K_Q);
			circleCentre2=circleCentre2+Q;			

			float W=2/PI*atan2(tangent_Q.z,tangent_Q.x);
			
			//numerically solve for B
			//i use a newton-like method here and iterate until sorta
			//convergence				
			float newB=1.0;
			float deltaB=0.01;
			float errorThreshold=0.0001;
			float deltaBThreshold=0.0001;

			float leftCurrentB=0.0;
			float rightCurrentB=0.0;
			float atCurrentB=0.0;		

			int maxIter=1000;
			int curIter=0;

			printf("Trying to solve for B in circle/circle case...\n");
			do {		
				curIter++;

				leftCurrentB=solveForBFormula(newB-deltaB, P, Q, K_P, K_Q, W);
				atCurrentB=solveForBFormula(newB, P, Q, K_P, K_Q, W);
				rightCurrentB=solveForBFormula(newB+deltaB, P, Q, K_P, K_Q, W);

				if (fabs(leftCurrentB)<fabs(rightCurrentB)&&fabs(leftCurrentB)<fabs(atCurrentB)) {
					newB=newB-deltaB;
				}
				else if (fabs(rightCurrentB)<fabs(leftCurrentB)&&fabs(rightCurrentB)<fabs(atCurrentB)){
					newB=newB+deltaB;
				}
				else {
					deltaB=deltaB/2.0;
				}

				printf("%f %f %f with %f\n",leftCurrentB,atCurrentB,rightCurrentB,newB);

			} while (fabs(atCurrentB)>errorThreshold&&deltaB>deltaBThreshold&&curIter<maxIter);	
			
			B[i]=newB;

			//with our newly-found B, solve for L_P and L_Q with (14)
			Vector centreDiffVec=circleCentre2-circleCentre1;
			L_P=2/PI*atan2(centreDiffVec.z,centreDiffVec.x)+K_P*K_P*B[i]*B[i];
			L_Q=W-B[i]*B[i]*(K_Q*K_Q-K_P*K_P)-L_P;

			printf("Solved for B, L_P, L_Q: %f %f %f\n",B[i],L_P,L_Q);

		}

	}

}

float ClothoidSpline_older::solveForBFormula(float B, Vector P, Vector Q, float K_P, float K_Q, float W) {

	float firstPart=pow(Q.x-(1.0/K_Q)*sin(PI/2*W),2)+pow(Q.z-(1.0/K_P)+(1.0/K_Q)*cos(PI/2*W),2);
	float secondPart=pow((PI*computeCosFresnelIntegral(K_Q*B))/(K_Q)-(PI*computeCosFresnelIntegral(K_P*B))/(K_P),2)+
					pow((PI*computeSinFresnelIntegral(K_Q*B)+1)/K_Q-(PI*computeSinFresnelIntegral(K_P*B)+1)/K_P,2);

	return firstPart-secondPart;

}

float ClothoidSpline_older::doLineCircleFormula(float newQ_Z, Vector centre, float radius, float K_Q, float &tangent_newQ_x, float &tangent_newQ_z, float &newQ_x) {

	float returnVal;	

	//determine newQ_x
	newQ_x=centre.x+sqrt(radius*radius-(newQ_Z-centre.z)*(newQ_Z-centre.z));

	//determine tangents
	Vector tanVector=Vector(newQ_x-centre.x,0,newQ_Z-centre.z);
	tanVector=tanVector.GetRotatedY(-90.0);
	tanVector.Normal();
	tangent_newQ_x=tanVector.x;
	tangent_newQ_z=tanVector.z;
	
	float sqrtoftanstuff=sqrt(2/PI*atan2(tangent_newQ_z,tangent_newQ_x));
	returnVal=PI*(1/K_Q)*sqrtoftanstuff*sqrt(2/PI)*computeSinFresnelIntegral(sqrtoftanstuff/sqrt(2/PI))-newQ_Z;

	return returnVal;

}

void ClothoidSpline_older::setCurvatures(float *curvatures) {

	pointSetCurvatures=curvatures;

}

float ClothoidSpline_older::computeSinFresnelIntegral(float t) {

	float fresnelIntegral=0.0;	

	for (int i=0;i<NUM_FRESNEL_ITERATIONS;i++) {

		float twoiplus1factorial=1.0;
		for (int j=1;j<=2*i+1;j++) {
			twoiplus1factorial=twoiplus1factorial*j;
		}

		fresnelIntegral+=pow(-1.0,(double)i)*(pow((double)t,(double)(4*i+3))/((4*i+3)*twoiplus1factorial));
	}

	return fresnelIntegral;

}

float ClothoidSpline_older::computeCosFresnelIntegral(float t) {

	float fresnelIntegral=0.0;	

	for (int i=0;i<NUM_FRESNEL_ITERATIONS;i++) {

		float twoifactorial=1.0;
		for (int j=1;j<=2*i;j++) {
			twoifactorial=twoifactorial*j;
		}

		fresnelIntegral+=pow(-1.0,(double)i)*(pow((double)t,(double)(4*i+1))/((4*i+1)*twoifactorial));
	}
	
	return fresnelIntegral;

}

Vector ClothoidSpline_older::getClothoidPoint(float t, float B, bool curveRight) {

	float sqrt2overPI=sqrt(2/PI);
	Vector returnVec=Vector(PI*B*sqrt2overPI*computeCosFresnelIntegral(t/sqrt2overPI),0,PI*B*sqrt2overPI*computeSinFresnelIntegral(t/sqrt2overPI));

	if (!curveRight) {
		returnVec.z=-returnVec.z;
	}

	return returnVec;

}

void ClothoidSpline_older::draw() {
	
	float tanFactor=5.0;
	
	glColor3f(0.0,0.2,0.8);						

	//draw the points and tangents in blue
	for (int i=0;i<numPoints;i++) {
		glPushMatrix();
			glTranslatef(pointSet[i].x,pointSet[i].y,pointSet[i].z);
			glutSolidSphere(.5,5,5);
		glPopMatrix();
		glBegin(GL_LINES);
			glVertex3f(pointSet[i].x,pointSet[i].y,pointSet[i].z);
			glVertex3f(pointSet[i].x+pointSetTangents[i].x*tanFactor,pointSet[i].y+pointSetTangents[i].y*tanFactor,pointSet[i].z+pointSetTangents[i].z*tanFactor);
		glEnd();
	}		
	
	//for each segment
	for (int i=0;i<numPoints-1;i++) {

		if (B[i]>0.0&&pointSetCurvatures[i]==0.0&&pointSetCurvatures[i+1]>0.0) { //LINE TO CIRCLE CLOTHOID SEGMENT

			glPushMatrix();			
							
				float rotAmount=atan2(pointSetTangents[i].z,pointSetTangents[i].x);
				rotAmount=rotAmount*RAD2DEG;			

				//draw the line segment
				glTranslatef(pointSet[i].x,pointSet[i].y,pointSet[i].z);
				glRotatef(-rotAmount,0,1,0);
				glColor3f(0.6,0,0);
				glBegin(GL_LINES);
					glVertex3f(0,0,0);
					glVertex3f(x[i],0,0);
				glEnd();		

				//gotta draw newQ
				glDisable(GL_DEPTH_TEST);
				glColor3f(1,1,0);
				glPushMatrix();
					Vector translatePt=getClothoidPoint(t[i],B[i],curveRight[i]);
					glTranslatef(translatePt.x+x[i],translatePt.y,translatePt.z);
					glutSolidSphere(0.5,5,5);				
				glPopMatrix();
				glEnable(GL_DEPTH_TEST);
				
				//New!  Draw the clothoid from 0 to t
				glTranslatef(x[i],0,0);

				glColor3f(1,0,0);
				glBegin(GL_LINE_STRIP);
					for (int j=0;j<=30;j++) {

						float eacht=(float)j/30.0*t[i];

						//clothoid defined by formula: PI * B ( C(t), S(t) )
						//   where C and S are the Fresnel integrals
						Vector eachPoint=getClothoidPoint(eacht,B[i],curveRight[i]);
						glVertex3f(eachPoint.x,0,eachPoint.z);

					}
				glEnd();

				//new, draw clothoid's tangent
				Vector translatePoint=getClothoidPoint(t[i],B[i],curveRight[i]);
				glTranslatef(translatePoint.x,0,translatePoint.z);
				glColor3f(.8,.8,0);
				glBegin(GL_LINES);
					glVertex3f(0,0,0);
					if (curveRight[i])
						glVertex3f(cos(t[i]*t[i]*PI/2.0)*tanFactor,0,sin(t[i]*t[i]*PI/2.0)*tanFactor);
					else
						glVertex3f(cos(t[i]*t[i]*PI/2.0)*tanFactor,0,-sin(t[i]*t[i]*PI/2.0)*tanFactor);
				glEnd();
			
			glPopMatrix();

			//draw the circle (and segment defined by L) corresponding to pointSet[i+1]	
			if (pointSetCurvatures[i+1]!=0.0) {
				glPushMatrix();		
					
					float radius=1.0/pointSetCurvatures[i+1];

					glTranslatef(pointSet[i+1].x,pointSet[i+1].y,pointSet[i+1].z);		

					if (curveRight[i]) {
						float rotateAmount=atan2(pointSetTangents[i+1].z,pointSetTangents[i+1].x);
						rotateAmount=rotateAmount*RAD2DEG;					
						glRotatef(-rotateAmount,0,1,0);		
					}
					else {
						float rotateAmount=atan2(-pointSetTangents[i+1].z,pointSetTangents[i+1].x);
						rotateAmount=rotateAmount*RAD2DEG;	
						glRotatef(rotateAmount+180.0,0,1,0);	
					}

					glTranslatef(0,0,radius);

					glBegin(GL_LINE_LOOP);
			 
					for (int i=0; i < 360; i++) {

						float degInRad = i*DEG2RAD;				
						glColor3f(0,0,0);
						glVertex3f(cos(degInRad-PI/2.0)*radius,0,sin(degInRad-PI/2.0)*radius);

					}
				 
				   glEnd();

				glPopMatrix();	
			}
		}
		else if (pointSetCurvatures[i]==0.0&&pointSetCurvatures[i+1]==0.0) { //LINE SEGMENT
			glColor3f(1,0,0);
			glBegin(GL_LINES);
				glVertex3f(pointSet[i].x,pointSet[i].y,pointSet[i].z);
				glVertex3f(pointSet[i+1].x,pointSet[i+1].y,pointSet[i+1].z);
			glEnd();

		}
		else if (B[i]>0.0&&pointSetCurvatures[i]>0.0&&pointSetCurvatures[i+1]==0.0) { //CIRCLE TO LINE CLOTHOID SEGMENT

			glPushMatrix();			
							
				float rotAmount=atan2(-pointSetTangents[i+1].z,-pointSetTangents[i+1].x); //work with the flipped tangents!
				rotAmount=rotAmount*RAD2DEG;			

				//draw the line segment
				glTranslatef(pointSet[i+1].x,pointSet[i+1].y,pointSet[i+1].z);
				glRotatef(-rotAmount,0,1,0);
				glColor3f(0.6,0,0);
				glBegin(GL_LINES);
					glVertex3f(0,0,0);
					glVertex3f(x[i],0,0);
				glEnd();		

				//gotta draw newQ
				glDisable(GL_DEPTH_TEST);
				glColor3f(1,1,0);
				glPushMatrix();
					Vector translatePt=getClothoidPoint(t[i],B[i],curveRight[i]);
					glTranslatef(translatePt.x+x[i],translatePt.y,translatePt.z);
					glutSolidSphere(0.5,5,5);				
				glPopMatrix();
				glEnable(GL_DEPTH_TEST);
				
				//New!  Draw the clothoid from 0 to t
				glTranslatef(x[i],0,0);

				glColor3f(1,0,0);
				glBegin(GL_LINE_STRIP);
					for (int j=0;j<=30;j++) {

						float eacht=(float)j/30.0*t[i];

						//clothoid defined by formula: PI * B ( C(t), S(t) )
						//   where C and S are the Fresnel integrals
						Vector eachPoint=getClothoidPoint(eacht,B[i],curveRight[i]);
						glVertex3f(eachPoint.x,0,eachPoint.z);

					}
				glEnd();

				//new, draw clothoid's tangent
				Vector translatePoint=getClothoidPoint(t[i],B[i],curveRight[i]);
				glTranslatef(translatePoint.x,0,translatePoint.z);
				glColor3f(.8,.8,0);
				glBegin(GL_LINES);
					glVertex3f(0,0,0);
					if (curveRight[i])
						glVertex3f(-cos(t[i]*t[i]*PI/2.0)*tanFactor,0,-sin(t[i]*t[i]*PI/2.0)*tanFactor);
					else
						glVertex3f(-cos(t[i]*t[i]*PI/2.0)*tanFactor,0,sin(t[i]*t[i]*PI/2.0)*tanFactor);
				glEnd();
			
			glPopMatrix();

			//draw the circle (and segment defined by L) corresponding to pointSet[i]	(it's I this time!)
			if (pointSetCurvatures[i]!=0.0) {
				glPushMatrix();		
					
					float radius=1.0/pointSetCurvatures[i];

					glTranslatef(pointSet[i].x,pointSet[i].y,pointSet[i].z);		

					if (curveRight[i]) {
						float rotateAmount=atan2(-pointSetTangents[i].z,-pointSetTangents[i].x); //work with flipped tangents!
						rotateAmount=rotateAmount*RAD2DEG;					
						glRotatef(-rotateAmount,0,1,0);		
					}
					else {
						float rotateAmount=atan2(pointSetTangents[i].z,-pointSetTangents[i].x); //work with flipped tangents!
						rotateAmount=rotateAmount*RAD2DEG;	
						glRotatef(rotateAmount+180.0,0,1,0);	
					}

					glTranslatef(0,0,radius);

					glBegin(GL_LINE_LOOP);
			 
					for (int i=0; i < 360; i++) {

						float degInRad = i*DEG2RAD;				
						glColor3f(0,0,0);
						glVertex3f(cos(degInRad-PI/2.0)*radius,0,sin(degInRad-PI/2.0)*radius);

					}
				 
				   glEnd();

				glPopMatrix();	
			}
		}
		else if (pointSetCurvatures[i]>0.0&&pointSetCurvatures[i+1]>0.0) { //CIRCLE TO CIRCLE SEGMENT

			float rotRadians=atan2(pointSetTangents[i].z,pointSetTangents[i].x); //this might have to be rotated the other way!
			Vector P=Vector(0,0,0);
			Vector Q=(pointSet[i+1]-pointSet[i]).GetRotatedYRadians(rotRadians);		
			Vector tangent_P=Vector(1,0,0);
			Vector tangent_Q=pointSetTangents[i+1].GetRotatedYRadians(rotRadians);
			
			float K_P=pointSetCurvatures[i];
			float K_Q=pointSetCurvatures[i+1];				

			//New!  Draw the clothoid from 0 to t
			glPushMatrix();															

				//draw the line segment
				glTranslatef(pointSet[i].x,pointSet[i].y,pointSet[i].z);
				glRotatef(-rotRadians*RAD2DEG,0,1,0);
				//we need to do this rotation with the CENTRE POINT OF CIRCLE 1

				if (curveRight[i]) {
					glTranslatef(0,0,1.0/pointSetCurvatures[i]);				
					glRotatef(-(PI/2*L_P)*RAD2DEG,0,1,0);
					glTranslatef(0,0,-1.0/pointSetCurvatures[i]);				
				}
				else {
					glTranslatef(0,0,-1.0/pointSetCurvatures[i]);				
					glRotatef(-(PI/2*L_P)*RAD2DEG,0,1,0);
					glTranslatef(0,0,1.0/pointSetCurvatures[i]);		
				}

				glColor3f(1,0,0);
				glBegin(GL_LINE_STRIP);
					for (int j=0;j<=20;j++) {

						float eacht=(float)j/20.0;
						eacht=(1.0-eacht)*pointSetCurvatures[i]*B[i]+eacht*pointSetCurvatures[i+1]*B[i]; //interpolate from K_P*B to K_Q*B

						//clothoid defined by formula: PI * B ( C(t), S(t) )
						//   where C and S are the Fresnel integrals
						Vector eachPoint=getClothoidPoint(eacht,B[i],curveRight[i]);
						glVertex3f(eachPoint.x,0,eachPoint.z);

					}
				glEnd();

			glPopMatrix();

			//draw circle for i+1
			glPushMatrix();		
					
				float radius=1.0/pointSetCurvatures[i+1];

				glTranslatef(pointSet[i+1].x,pointSet[i+1].y,pointSet[i+1].z);		

				if (curveRight[i]) {
					float rotateAmount=atan2(pointSetTangents[i+1].z,pointSetTangents[i+1].x);
					rotateAmount=rotateAmount*RAD2DEG;					
					glRotatef(-rotateAmount,0,1,0);		
				}
				else {
					float rotateAmount=atan2(-pointSetTangents[i+1].z,pointSetTangents[i+1].x);
					rotateAmount=rotateAmount*RAD2DEG;	
					glRotatef(rotateAmount+180.0,0,1,0);	
				}

				glTranslatef(0,0,radius);

				glBegin(GL_LINE_LOOP);
		 
				for (int i=0; i < 360; i++) {

					float degInRad = i*DEG2RAD;				
					glColor3f(0,0,0);
					glVertex3f(cos(degInRad-PI/2.0)*radius,0,sin(degInRad-PI/2.0)*radius);

				}
			 
			   glEnd();

			glPopMatrix();	
		}

	}

}