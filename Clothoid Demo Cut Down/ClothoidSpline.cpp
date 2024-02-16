#include "ClothoidSpline.h"

#define DRAW_LINEWIDTH 4.0f

vector <Vector> ClothoidSpline::colorSegments;

ClothoidSpline::ClothoidSpline(vector <Vector> edgePoints, float penalty, bool using_G3, 
	bool using_G1, float g1discontCurvThresh, float endPointWeight, bool using_closed) {	

	this->using_G1=using_G1;
	this->using_G3=using_G3;
	this->using_closed=using_closed;

	if (colorSegments.empty()) {

		colorSegments.clear();

		colorSegments.push_back(Vector(0.8,0,0));
		colorSegments.push_back(Vector(0,.8,0));
		colorSegments.push_back(Vector(0,0,.8));
		colorSegments.push_back(Vector(.8,0,.8));
		colorSegments.push_back(Vector(0,.8,.8));
		colorSegments.push_back(Vector(.8,.8,0));	
		colorSegments.push_back(Vector(.6,.5,.2));			
		
	}

	edgePointSet=edgePoints;

	//Steps:

	//0 - close curve (make startpt=lastpt)
	//1 - construct curvature plot of sketched edge
	//2 - fit small number of connected line segments 
	//		across entire plot (attempting to minimize error)	
	//3 - Flatten slope of line segments whose slope below threshold
	//4 - Find transformation (translation and rotation) that produces
	//		the best fit between clothoid spline and sketched pointset			

	//project to XZ plane
	for (int i=0;i<edgePointSet.size();i++) {
		edgePointSet[i].y=0.0f;
	}

	//close if not closed
	if (using_closed) {

		if ((edgePointSet[edgePointSet.size()-1]-edgePointSet[0]).GetLength()<
			(edgePointSet[edgePointSet.size()-2]-edgePointSet[edgePointSet.size()-1]).GetLength()) 
			edgePointSet.pop_back();
				
		edgePointSet.push_back(edgePointSet[0]);		

	}
	else {

		if (edgePointSet[0]==edgePointSet[edgePointSet.size()-1])
			edgePointSet.pop_back();

	}

	float eachArcLength=0.0f;
	for (int i=0;i<edgePointSet.size();i++) {		
		arcLength.push_back(eachArcLength);
		
		if (i<edgePointSet.size()-1)
			eachArcLength+=(edgePointSet[i+1]-edgePointSet[i]).GetLength();
	}

	//1 - Curvature estimation:	
	estCurv=ClothoidSpline::getCurvatures(edgePointSet);

	//memorize the min/max (for 2d plots drawing later)
	minCurv=1000000.0;
	maxCurv=-1000000.0;
	for (int i=1;i<edgePointSet.size()-1;i++) {
		if (estCurv[i]<minCurv)
			minCurv=estCurv[i];
		if (estCurv[i]>maxCurv)
			maxCurv=estCurv[i];
	}

	//2 - fit small number of connected line segments 
	//		across entire plot (attempting to minimize error)
	//		using dynamic programming approach

	vector <vector <float>> errorMatrix(edgePointSet.size(), vector<float>(edgePointSet.size(),0));
	vector <vector <int>> walkMatrix(edgePointSet.size(), vector<int>(edgePointSet.size(),0));
	vector <vector <float>> AMatrix(edgePointSet.size(), vector<float>(edgePointSet.size(),0));
	vector <vector <float>> BMatrix(edgePointSet.size(), vector<float>(edgePointSet.size(),0));

	for (int i=0;i<edgePointSet.size();i++) {
		for (int j=0;j<edgePointSet.size();j++) {
			errorMatrix[i][j]=0.0;
			walkMatrix[i][j]=-1;
		}
	}

	//populate first diagonal (cost of line segment between neighbouring points)
	float eachSegmentCost=penalty;
	for (int i=0;i+1<edgePointSet.size();i++) {
		errorMatrix[i][i+1]=eachSegmentCost;
		Vector2 *points=new Vector2[2];
		points[0]=Vector2(arcLength[i],estCurv[i]);
		points[1]=Vector2(arcLength[i+1],estCurv[i+1]);

		HeightLineFit2(2, points, AMatrix[i][i+1], BMatrix[i][i+1]); //no longer dependent on wm4lib

	}

	//iterate through remaining diagonals
	for (int j=2;j<edgePointSet.size();j++) { 
		for (int i=0;i+j<edgePointSet.size();i++) {

			//do linear regression on segments between i and i+j inclusive
			//if that error + penalty for segment is less than 
			//	sum of errors [i][i+j-1] and [i+1][i+j], then use
			//	that
			//otherwise, use sum of [i][i+j-1] and [i+1][i+j]
			
			Vector2 *points=new Vector2[j+1];
			for (int each=i;each<=i+j;each++) {				
				points[each-i]=Vector2(arcLength[each],estCurv[each]);
			}

			HeightLineFit2(j+1, points, AMatrix[i][i+j], BMatrix[i][i+j]);
			float fitError=getFitErrors(j+1, points, AMatrix[i][i+j], BMatrix[i][i+j]);

			float minError=fitError+eachSegmentCost;
			int minIndex=-1;

			for (int each=i+1;each<i+j;each++) { //check each partitioning at this level
				if (errorMatrix[i][each]+errorMatrix[each][i+j]<minError) {
					minIndex=each;
					minError=errorMatrix[i][each]+errorMatrix[each][i+j];					
				}
			}
			
			walkMatrix[i][i+j]=minIndex;			
			errorMatrix[i][i+j]=minError;
			
		}
	}		

	//use walk matrix and determine partitions
	bool *segmentEnd=new bool[edgePointSet.size()];
	for (int i=0;i<edgePointSet.size();i++) {
		segmentEnd[i]=false;
	}
	recurseThroughWalkMatrix(walkMatrix, 0, edgePointSet.size()-1, segmentEnd);	

	//intersect linear regressed lines
	int endIndex;
	int endNextIndex;

	for (int j=1;j<edgePointSet.size();j++) {
		if (segmentEnd[j]) {
			endIndex=j;
			break;
		}
	}
	segmentPointSet.push_back(Vector(0.0f,BMatrix[0][endIndex],0.0f));
	for (int i=0;i<edgePointSet.size()-1;i++) {
		if (segmentEnd[i]) {			
			for (int j=i+1;j<edgePointSet.size();j++) {
				if (segmentEnd[j]) {
					endIndex=j;
					break;
				}
			}

			float A1=AMatrix[i][endIndex];
			float B1=BMatrix[i][endIndex];

			endNextIndex=endIndex;
			for (int j=endIndex+1;j<edgePointSet.size();j++) {
				if (segmentEnd[j]) {
					endNextIndex=j;
					break;
				}
			}			

			if (endNextIndex>endIndex) { //this is an intersection between two lines

				float A2=AMatrix[endIndex][endNextIndex];
				float B2=BMatrix[endIndex][endNextIndex];

				float xintersect=(B2-B1)/(A1-A2);
				float yintersect=A1*xintersect+B1;

				segmentPointSet.push_back(Vector(xintersect,yintersect,0.0f));
			}
			else { //this is the last line (hence no intersection)

				segmentPointSet.push_back(Vector(arcLength[edgePointSet.size()-1],A1*arcLength[edgePointSet.size()-1]+B1,0.0f));
				break;
			}

		}
	}

	//some intersection points "overshoot" (causing the linearly interpolated
	//segmentPointSet) not to be a function any more, we have to go through
	//and remove these points
	bool foundOvershoot;
	do {
		foundOvershoot=false;
		for (int i=1;i<segmentPointSet.size()-1;i++) {
			if ((segmentPointSet[i].x>segmentPointSet[i+1].x&&segmentPointSet[i].x>segmentPointSet[i-1].x)||
				(segmentPointSet[i].x<segmentPointSet[i-1].x&&segmentPointSet[i].x<segmentPointSet[i+1].x)) {
				segmentPointSet.erase(segmentPointSet.begin()+i);				
				foundOvershoot=true;
				break;
			}
		}
	} while (foundOvershoot);	

	//3 - Flatten slope of line segments whose slope is below threshold

	for (int i=0;i<segmentPointSet.size()-1;i++) {

		//turn clothoid into circle segment if slope is low enough
		if (fabs(segmentPointSet[i+1].y-segmentPointSet[i].y)<MIN_CURVATURE_SLOPE) {
			float midpoint=(segmentPointSet[i].y+segmentPointSet[i+1].y)/2.0;

			segmentPointSet[i].y=midpoint;
			segmentPointSet[i+1].y=midpoint;
		}

		//turn circle segment into line segment if close enough to x-axis
		if (segmentPointSet[i+1].y==segmentPointSet[i].y&&fabs(segmentPointSet[i].y)<MIN_DISTANCE_FROM_ZERO) {
			segmentPointSet[i].y=0.0f;
			segmentPointSet[i+1].y=0.0f;
		}

	}

	//3.5 - Close clothoid (if desired)
	if (using_closed) {
		//SnapClosed();
		//ConvergeEndpointsToTouch();

		//add patch piece
		Vector newLastPiece=segmentPointSet[segmentPointSet.size()-1];
		segmentPointSet[segmentPointSet.size()-1]=(newLastPiece+segmentPointSet[segmentPointSet.size()-2])*0.5;
		segmentPointSet.push_back(newLastPiece);
	}

	//4 - Find transformation (translation and rotation) that produces
	//		the best fit between clothoid spline and sketched pointset	
		
	g1ClothoidVec.clear();
	if (using_G1) {				

		//1. Determine segmentation regions (curvature spikes)		
		
		float discontThreshold=g1discontCurvThresh;
		int lastIndex=0;

		for (int i=1;i<estCurv.size()-1;i++) {
			if ((fabs(estCurv[i]-estCurv[i-1])>discontThreshold&& //THIS ONE USES CURVATURE VARIATION
				fabs(estCurv[i+1]-estCurv[i])>discontThreshold)||			
				(i==estCurv.size()-2&&
				!g1ClothoidVec.empty())) {				

				vector <Vector> eachClothoidPointSet;

				if (i==estCurv.size()-2) {
					i=estCurv.size()-1;					
				}				

				for (int j=lastIndex;j<=i;j++) 				
					eachClothoidPointSet.push_back(edgePointSet[j]);	

				//2. Create a series of sub-clothoids
				ClothoidSpline *eachClothoid=new ClothoidSpline(eachClothoidPointSet, 
								penalty, using_G3, using_G1, g1discontCurvThresh, endPointWeight);					
				g1ClothoidVec.push_back(eachClothoid);					

				lastIndex=i;

			}
		}				
			
		//3. Interconnect series of sub-clothoids by performing translations

		//segmentPointSet.clear();
		//for (int i=0;i<g1ClothoidVec.size();i++) {
		//	segmentPointSet.insert(segmentPointSet.end(),g1ClothoidVec[i]->getSegmentPointSet().begin(),g1ClothoidVec[i]->getSegmentPointSet().end());
		//}

		if (!g1ClothoidVec.empty()) {

			for (int i=0;i<g1ClothoidVec.size()-1;i++) {

				ClothoidSpline *curClothoid=g1ClothoidVec[i];
				ClothoidSpline *nextClothoid=g1ClothoidVec[i+1];

				Vector curClothoidEnd=curClothoid->getEndPoint();
				Vector nextClothoidBegin=nextClothoid->getStartPoint();				

				nextClothoid->fitTranslate+=(curClothoidEnd-nextClothoidBegin);				

			}

		}

	}

	if (using_G3&&g1ClothoidVec.empty()) {
		/* 
		 * G3 continuity feature
		 */

		for (int i=0;i<segmentPointSet.size()-2;i++) { 						

			float segmentSlope=((segmentPointSet[i+1].y-segmentPointSet[i].y)/(segmentPointSet[i+1].x-segmentPointSet[i].x));	
			float nextSegmentSlope=((segmentPointSet[i+2].y-segmentPointSet[i+1].y)/(segmentPointSet[i+2].x-segmentPointSet[i+1].x));	

			float blend_distance=5.0;
			if ((segmentPointSet[i+1].x-segmentPointSet[i].x)/2.0f<blend_distance)
				blend_distance=(segmentPointSet[i+1].x-segmentPointSet[i].x)/2.0f;
			if ((segmentPointSet[i+2].x-segmentPointSet[i+1].x)/2.0f<blend_distance)
				blend_distance=(segmentPointSet[i+2].x-segmentPointSet[i+1].x)/2.0f;

			Vector newBeforePoint=Vector(segmentPointSet[i+1].x-blend_distance,segmentPointSet[i+1].y+segmentSlope*(-blend_distance),0.0f);
			Vector newAfterPoint=Vector(segmentPointSet[i+1].x+blend_distance,segmentPointSet[i+1].y+nextSegmentSlope*blend_distance,0.0f);

			Vector point=segmentPointSet[i+1];

			segmentPointSet.erase(segmentPointSet.begin()+i+1);
			segmentPointSet.insert(segmentPointSet.begin()+i+1,newAfterPoint);
			segmentPointSet.insert(segmentPointSet.begin()+i+1,newBeforePoint);		

			int numPointsAdded=1;

			for (float j=blend_distance;j>=0.0f;j-=0.2) {					
				Vector startLine=Vector(point.x-blend_distance+j,point.y+segmentSlope*(-blend_distance+j),0.0f);
				Vector endLine=Vector(point.x+j,point.y+nextSegmentSlope*j,0.0f);
				float interp=j/blend_distance;
				
				float eachCurv=startLine.y*(1.0-interp)+endLine.y*interp;
				float nextXval=startLine.x*(1.0-interp)+endLine.x*interp;			

				segmentPointSet.insert(segmentPointSet.begin()+i+2,Vector(nextXval,eachCurv,1.0f));			

				numPointsAdded++;
			}
			
			i+=numPointsAdded;
		}
	}

	setupCanonicalSegments();	
	setupArcLengthSamples();
	setupFitTransform(endPointWeight);	

	maxArcLength=arcLength[edgePointSet.size()-1];

	//Matrix cleanup
	errorMatrix.clear();
	walkMatrix.clear();
	AMatrix.clear();
	BMatrix.clear();	

}

ClothoidSpline::~ClothoidSpline() {

	estCurv.clear();
	arcLength.clear();

	segmentPointSet.clear();
	segmentTranslate.clear();
	segmentRotate.clear();
	fineSampleVector.clear();

	edgePointSet.clear();

}

vector <float> ClothoidSpline::getCurvatures(vector <Vector> theEdgePointSet) {	

	vector <float> curvVec;

	curvVec.reserve(theEdgePointSet.size());
	curvVec.push_back(0.0f);
	for (int i=1;i<theEdgePointSet.size()-1;i++) {

		//curvature given by: 2*sin(theta/2), and where sign is given by sign of cross product

		/*
		Vector vec1=theEdgePointSet[i]-theEdgePointSet[i-1];
		Vector vec2=theEdgePointSet[i+1]-theEdgePointSet[i];
		vec1.Normal();
		vec2.Normal();

		float dotProd=vec1.DotProduct3(vec2);
		float theta;
		if (dotProd>1.0)
			theta=0.0f;
		else if (dotProd<-1.0)
			theta=3.14159f;
		else
			theta=acosf(dotProd);
		
		Vector signVec;
		signVec.CrossProduct(vec1,vec2);	
	
		float curvVal=2.0*sin(theta/2.0f)/sqrt((theEdgePointSet[i]-theEdgePointSet[i-1]).GetLength())/sqrt((theEdgePointSet[i+1]-theEdgePointSet[i]).GetLength());

		curvVec.push_back(curvVal);
		if (signVec.y>0.0f)
			curvVec[i]=-curvVec[i];
			*/

		//this curvature definition from schneider/kobbelt paper
		Vector vec1=theEdgePointSet[i]-theEdgePointSet[i-1];
		Vector vec2=theEdgePointSet[i+1]-theEdgePointSet[i];

		double det=vec1.x*vec2.z-vec2.x*vec1.z;

		float curvVal=2.0*det/(vec1.GetLength()*vec2.GetLength()*(vec1+vec2).GetLength());		
		curvVec.push_back(curvVal);

	}
	curvVec.push_back(0.0f);

	curvVec[0]=curvVec[1];
	curvVec[curvVec.size()-1]=curvVec[curvVec.size()-2];

	return curvVec;

}

float ClothoidSpline::getPiecePenalty() {

	return piecePenalty;

}

void ClothoidSpline::setupCanonicalSegments() {

	//set up the translate/rotates for each segment

	segmentTranslate.clear();
	segmentRotate.clear();

	segmentTranslate.push_back(Vector(0,0,0));
	segmentRotate.push_back(0.0f);

	for (int i=1;i<segmentPointSet.size();i++) {

		Vector eachTranslate=segmentTranslate[i-1];
		float eachRotate=segmentRotate[i-1];

		float curv1=segmentPointSet[i-1].y;
		float curv2=segmentPointSet[i].y;	

		if (!(fabs(curv2-curv1)<MIN_CURVATURE_SLOPE)) {//CLOTHOID CASE		

			float eachLength=segmentPointSet[i].x-segmentPointSet[i-1].x;
			float B=sqrt(eachLength/(PI*fabs(curv2-curv1)));

			float t1=curv1*B*CURV_FACTOR;
			float t2=curv2*B*CURV_FACTOR;			
			
			//printf("t1: %f t2: %f B: %f\n",t1,t2,B);

			Vector transVec=getClothoidPiecePoint(t1,t2,t2);
			transVec=transVec*PI*B;		
			
			//rotate for the end of this piece
			float rotAmount;
			
			if (t2>t1) {
				rotAmount=(t2*t2-t1*t1)*180.0/PI;				
			}
			else {
				rotAmount=(t1*t1-t2*t2)*180.0/PI;				
			}

			segmentTranslate.push_back(eachTranslate+transVec.GetRotatedY(eachRotate));
			segmentRotate.push_back(eachRotate-rotAmount);			

		}
		else if (fabs(curv2-curv1)<MIN_CURVATURE_SLOPE&&(fabs(curv1)<MIN_CURVATURE_SLOPE||fabs(curv2)<MIN_CURVATURE_SLOPE)) { //LINE SEGMENT CASE

			float eachLength=segmentPointSet[i].x-segmentPointSet[i-1].x;
			Vector transVec=Vector(eachLength,0,0);

			segmentTranslate.push_back(eachTranslate+transVec.GetRotatedY(eachRotate));
			segmentRotate.push_back(eachRotate);		

		}
		else { //CIRCLE SEGMENT CASE

			float eachLength=segmentPointSet[i].x-segmentPointSet[i-1].x;
			float radius=(curv1+curv2)/2.0;
			radius=1.0/radius;
			bool negCurvature;

			if (radius<0.0) {
				negCurvature=true;
				radius=fabs(radius);
			}
			else
				negCurvature=false;
			
			float circumference=2*PI*radius;
			float anglesweep_rad=(eachLength*2*PI)/circumference;
			Vector transVec;

			if (!negCurvature) {
				transVec=Vector(0,0,-radius);
				transVec=transVec.GetRotatedY(-anglesweep_rad*180.0/PI);
				transVec.z=transVec.z+radius;
			}
			else {
				transVec=Vector(0,0,radius);
				transVec=transVec.GetRotatedY(anglesweep_rad*180.0/PI);
				transVec.z=transVec.z-radius;
			}

			float rotAmount;
			if (!negCurvature)
				rotAmount=anglesweep_rad*180.0/PI;
			else
				rotAmount=-anglesweep_rad*180.0/PI;
				
			segmentTranslate.push_back(eachTranslate+transVec.GetRotatedY(eachRotate));			
			segmentRotate.push_back(eachRotate-rotAmount);
		}
		
	}

}

void ClothoidSpline::setupFitTransform(float endpointWeight) {
	
	//0.  Assign weights to each point that define how much 
	//		each point counts in the transformation
	vector <float> weighting;
	float totalWeight=0.0f;	
	
	for (int i=arcLengthSamplesStart;i<arcLengthSamplesEnd;i++) { 

		if (i==arcLengthSamplesStart||i==arcLengthSamplesEnd-1) 
			weighting.push_back(endpointWeight);
		else
			weighting.push_back(1.0f);

		totalWeight+=weighting[i-arcLengthSamplesStart];		
	}	

	//1.  Translation is given by the centres of mass between the two curves

	centreSketch=Vector(0,0,0);
	centreSpline=Vector(0,0,0);

	for (int i=arcLengthSamplesStart;i<arcLengthSamplesEnd;i++) {
		centreSketch+=edgePointSet[i]*weighting[i-arcLengthSamplesStart];
	}
	centreSketch=centreSketch/totalWeight;

	for (int i=arcLengthSamplesStart;i<arcLengthSamplesEnd;i++) { //JAMES CHANGE FROM EDGEPOINTSET TO ARCLENGTHSAMPLES
		centreSpline+=arcLengthSamples[i-arcLengthSamplesStart]*weighting[i-arcLengthSamplesStart];
	}
	centreSpline=centreSpline/totalWeight;

	fitTranslate=centreSketch-centreSpline;	

	//2.  Rotation if found from rotation component of best linear transformation
	//		(after performing the translation - which leaves only scaling and rotation)

	//Set up matrix A_pq
	double *A_pq=new double[4];	
	for (int i=0;i<4;i++) {
		A_pq[i]=0.0f;
	}

	for (int i=arcLengthSamplesStart;i<arcLengthSamplesEnd;i++) { //JAMES CHNAGE FROM EDGEPOINTSET TO arcLengthSamples
		Vector p_i=edgePointSet[i]-centreSketch;
		Vector q_i=arcLengthSamples[i-arcLengthSamplesStart]-centreSpline;
		
		A_pq[0]+=p_i.x*q_i.x*weighting[i-arcLengthSamplesStart];
		A_pq[1]+=p_i.x*q_i.z*weighting[i-arcLengthSamplesStart];
		A_pq[2]+=p_i.z*q_i.x*weighting[i-arcLengthSamplesStart];
		A_pq[3]+=p_i.z*q_i.z*weighting[i-arcLengthSamplesStart];		
	}

	//Solve for S, where S=sqrt(A_pq^TA_pq)
	double *A_pqTA_pq=new double[4];
	A_pqTA_pq[0]=A_pq[0]*A_pq[0]+A_pq[2]*A_pq[2];
	A_pqTA_pq[1]=A_pq[0]*A_pq[1]+A_pq[2]*A_pq[3];
	A_pqTA_pq[2]=A_pq[0]*A_pq[1]+A_pq[2]*A_pq[3];
	A_pqTA_pq[3]=A_pq[1]*A_pq[1]+A_pq[3]*A_pq[3];

	//square root can be found using eigenvalues of this 2x2 matrix	
	//Direct method to obtain eigenvalues
	double a, b, c, d;
	a=A_pqTA_pq[0];
	b=A_pqTA_pq[1];
	c=A_pqTA_pq[2];
	d=A_pqTA_pq[3];

	double r_1=(a+d)/2.0+sqrt(((a+d)*(a+d))/4.0+b*c-a*d);
	double r_2=(a+d)/2.0-sqrt(((a+d)*(a+d))/4.0+b*c-a*d);

	double *sqrtA=new double[4]; //sqrt(A) where A_pq^T A_pq
	double *Sinv=new double[4];
	double *R=new double[4];

	if (r_1!=0.0f&&r_2!=0.0f) { //If matrix is not RANK DEFICIENT

		double m;
		double p;

		if (r_2!=r_1) {
			m=(sqrt(r_2)-sqrt(r_1))/(r_2-r_1);
			p=(r_2*sqrt(r_1)-r_1*sqrt(r_2))/(r_2-r_1);
		}
		else if (r_2==r_1) {
			m=1/(4*r_1);
			p=sqrt(r_1)/2;
		}
		
		//sqrt(A)=m*A+p*I
		sqrtA[0]=m*A_pqTA_pq[0]+p;
		sqrtA[1]=m*A_pqTA_pq[1];
		sqrtA[2]=m*A_pqTA_pq[2];
		sqrtA[3]=m*A_pqTA_pq[3]+p;		

		//S^(-1) = (1/ad-bc)(d -b; -c a)	
		float determinant=(1/(sqrtA[0]*sqrtA[3]-sqrtA[1]*sqrtA[2]));
		Sinv[0]=determinant*sqrtA[3];
		Sinv[1]=determinant*(-sqrtA[1]);
		Sinv[2]=determinant*(-sqrtA[2]);
		Sinv[3]=determinant*sqrtA[0];		

		//finally, R=A_pq*S^(-1)		
		R[0]=A_pq[0]*Sinv[0]+A_pq[1]*Sinv[2];
		R[1]=A_pq[0]*Sinv[1]+A_pq[1]*Sinv[3];
		R[2]=A_pq[2]*Sinv[0]+A_pq[3]*Sinv[2];
		R[3]=A_pq[2]*Sinv[1]+A_pq[3]*Sinv[3];		

		if (fabs(R[0]-R[3])<0.001f&&fabs(R[1]-R[2])>0.001f) {

			if (R[1]<0.0) {
				fitRotate=-acos(R[0])*180.0/PI;			
			}
			else {
				fitRotate=acos(R[0])*180.0/PI;			
			}
		}
		else {

			//printf("\n\n\n R0 NOT EQUAL to R3!!\n\n\n");
			if (R[1]<0.0) {
				fitRotate=acos(R[0])*180.0/PI;
			}
			else {
				fitRotate=-acos(R[0])*180.0/PI;
			}			

		}

		//printf("\nR[0]: %f R[1]: %f R[2]: %f R[3]: %f det: %f\n",R[0],R[1],R[2],R[3],determinant);

		//printf("fitRotate %f found VIA R\n",fitRotate);		

	}
	else { //MATRIX A_pq is RANK DEFICIENT	

		//use arctangent of 1st tangent to approximate
		fitRotate=-atan2(edgePointSet[edgePointSet.size()-1].z-edgePointSet[0].z,
			edgePointSet[edgePointSet.size()-1].x-edgePointSet[0].x)*180.0f/PI;	

		//printf("fitRotate %f found VIA arctan2\n",fitRotate);

	}		

	//Matrix cleanup
	delete [] A_pq;
	delete [] A_pqTA_pq;	
	delete [] sqrtA;
	delete [] Sinv;
	delete [] R;

}

void ClothoidSpline::setupFineSamples(float sampleDistance) {	

	vector <float> fineSamples;
	for (float f=segmentPointSet[0].x;f<=segmentPointSet[segmentPointSet.size()-1].x;f+=sampleDistance)
		fineSamples.push_back(f);

	fineSampleVector.clear();

	for (int i=0;i<fineSamples.size();i++) {

		bool pointFound=false;
		for (int j=0;j<segmentPointSet.size()-1;j++) {			

			if (fineSamples[i]>=segmentPointSet[j].x&&fineSamples[i]<=segmentPointSet[j+1].x) {
				
				float curv1=segmentPointSet[j].y;
				float curv2=segmentPointSet[j+1].y;	
				float interp=(fineSamples[i]-segmentPointSet[j].x)/(segmentPointSet[j+1].x-segmentPointSet[j].x);

				Vector transVec;

				if (!(fabs(curv2-curv1)<MIN_CURVATURE_SLOPE)) {//CLOTHOID CASE	

					float eachLength=segmentPointSet[j+1].x-segmentPointSet[j].x;
					float B=sqrt(eachLength/(PI*fabs(curv2-curv1)));
					float t1=curv1*B*CURV_FACTOR;
					float t2=curv2*B*CURV_FACTOR;
					float eacht=t1*(1.0-interp)+t2*(interp);

					transVec=getClothoidPiecePoint(t1,eacht,t2);
					transVec=transVec*PI*B;

				}
				else if (fabs(curv2-curv1)<MIN_CURVATURE_SLOPE&&(fabs(curv1)<MIN_CURVATURE_SLOPE||fabs(curv2)<MIN_CURVATURE_SLOPE)) { //LINE SEGMENT CASE	

					transVec=Vector(fineSamples[i]-segmentPointSet[j].x,0,0);					

				}
				else { //CIRCLE SEGMENT CASE

					float eachLength=segmentPointSet[j+1].x-segmentPointSet[j].x;
					float radius=2.0f/(curv1+curv2);					
					bool negCurvature;

					if (radius<0.0) {
						negCurvature=true;
						radius=fabs(radius);
					}
					else
						negCurvature=false;
			
					float circumference=2*PI*radius;
					float anglesweep_rad=(eachLength*2*PI)/circumference;

					if (!negCurvature) {
						transVec=Vector(0,0,-radius);
						transVec=transVec.GetRotatedY(-interp*anglesweep_rad*180.0/PI);
						transVec.z=transVec.z+radius;
					}
					else {
						transVec=Vector(0,0,radius);
						transVec=transVec.GetRotatedY(interp*anglesweep_rad*180.0/PI);
						transVec.z=transVec.z-radius;
					}						
				}

				Vector eachPoint=segmentTranslate[j]+transVec.GetRotatedY(segmentRotate[j]);
				
				eachPoint=eachPoint-centreSpline;
				eachPoint=eachPoint.GetRotatedY(fitRotate);
				eachPoint+=centreSpline;
				eachPoint+=fitTranslate;

				eachPoint.y=j; //y component is which segment this is!!

				fineSampleVector.push_back(eachPoint);
				
				pointFound=true;
				break;
			}

			if (pointFound)
				break;

		}
	}

}

vector <Vector> ClothoidSpline::getFineSamples() {

	return fineSampleVector;

}

void ClothoidSpline::setupArcLengthSamples() {

	arcLengthSamples.clear();

	arcLengthSamplesStart=INT_MAX;
	arcLengthSamplesEnd=0;

	for (int i=0;i<edgePointSet.size();i++) {

		for (int j=0;j<segmentPointSet.size()-1;j++) {
			if (arcLength[i]>=segmentPointSet[j].x&&arcLength[i]<=segmentPointSet[j+1].x) {
				
				float curv1=segmentPointSet[j].y;
				float curv2=segmentPointSet[j+1].y;	
				float interp=(arcLength[i]-segmentPointSet[j].x)/(segmentPointSet[j+1].x-segmentPointSet[j].x);

				Vector transVec;

				if (!(fabs(curv2-curv1)<MIN_CURVATURE_SLOPE)) {//CLOTHOID CASE	

					float eachLength=segmentPointSet[j+1].x-segmentPointSet[j].x;
					float B=sqrt(eachLength/(PI*fabs(curv2-curv1)));
					float t1=curv1*B*CURV_FACTOR;
					float t2=curv2*B*CURV_FACTOR;
					float eacht=t1*(1.0-interp)+t2*(interp);

					transVec=getClothoidPiecePoint(t1,eacht,t2);
					transVec=transVec*PI*B;

				}
				else if (fabs(curv2-curv1)<MIN_CURVATURE_SLOPE&&(fabs(curv1)<MIN_CURVATURE_SLOPE||fabs(curv2)<MIN_CURVATURE_SLOPE)) { //LINE SEGMENT CASE	

					transVec=Vector(arcLength[i]-segmentPointSet[j].x,0,0);					

				}
				else { //CIRCLE SEGMENT CASE

					float eachLength=segmentPointSet[j+1].x-segmentPointSet[j].x;
					float radius=2.0f/(curv1+curv2);					
					bool negCurvature;

					if (radius<0.0) {
						negCurvature=true;
						radius=fabs(radius);
					}
					else
						negCurvature=false;
			
					float circumference=2*PI*radius;
					float anglesweep_rad=(eachLength*2*PI)/circumference;

					if (!negCurvature) {
						transVec=Vector(0,0,-radius);
						transVec=transVec.GetRotatedY(-interp*anglesweep_rad*180.0/PI);
						transVec.z=transVec.z+radius;
					}
					else {
						transVec=Vector(0,0,radius);
						transVec=transVec.GetRotatedY(interp*anglesweep_rad*180.0/PI);
						transVec.z=transVec.z-radius;
					}						
				}

				arcLengthSamples.push_back(segmentTranslate[j]+transVec.GetRotatedY(segmentRotate[j]));
				if (i<arcLengthSamplesStart)
					arcLengthSamplesStart=i;
				if (i>arcLengthSamplesEnd)
					arcLengthSamplesEnd=i;

				break;
			}
		}

	}

}

void ClothoidSpline::recurseThroughWalkMatrix(vector <vector <int>> walkMatrix, int begin, int end, bool *segmentEnd) {

	if (begin+1>=end) {
		segmentEnd[begin]=true;
		segmentEnd[end]=true;
	}

	if (walkMatrix[begin][end]==-1) {
		segmentEnd[begin]=true;
		segmentEnd[end]=true;
	}
	else {
		recurseThroughWalkMatrix(walkMatrix, begin, walkMatrix[begin][end], segmentEnd);
		recurseThroughWalkMatrix(walkMatrix, walkMatrix[begin][end], end, segmentEnd);
	}

}

float ClothoidSpline::getFitErrors(int num, Vector2 *points, float A, float B) {

	float totalError=0.0;

	for (int i=0;i<num;i++) 	
		totalError+=fabs(B+(A*points[i].x)-points[i].y);

	return totalError;

}

Vector ClothoidSpline::getPoint(int segment) {

	Vector returnPoint=arcLengthSamples[segment];	

	returnPoint=returnPoint-centreSpline;
	returnPoint=returnPoint.GetRotatedY(fitRotate);
	returnPoint=returnPoint+centreSketch;	

	return returnPoint;
}

Vector ClothoidSpline::getTangent(int segment, float interp) {
	return Vector(1,0,0);
}

void ClothoidSpline::draw() {
		
	if (segmentPointSet.empty()||segmentPointSet.size()<2) {		
		return;
	}	

	glColor3f(.95,.95,.95);
	glLineWidth(0.5);
	glBegin(GL_LINE_STRIP);	
		for (int i=0;i<edgePointSet.size();i++) {
			glVertex3f(edgePointSet[i].x,0.0f,edgePointSet[i].z);
		}
	glEnd();	

	glColor3f(0,0,0);
	glPointSize(DRAW_LINEWIDTH);
	
	glBegin(GL_POINTS);	
		for (int i=0;i<edgePointSet.size();i++) {
			glVertex3f(edgePointSet[i].x,0.0f,edgePointSet[i].z);
		}
	glEnd();

	if (!g1ClothoidVec.empty()) {		

		for (int i=0;i<g1ClothoidVec.size();i++) {			
			g1ClothoidVec[i]->draw();			
		}		

		return;
	}

	//for each spline segment

	if (fineSampleVector.empty()) 
		setupFineSamples(0.1f);

	glLineWidth(DRAW_LINEWIDTH);
	glBegin(GL_LINE_STRIP);
	for (int i=0;i<fineSampleVector.size();i++) {
		glColor3f(colorSegments[(unsigned int)(fineSampleVector[i].y)%colorSegments.size()].x,
			colorSegments[(unsigned int)(fineSampleVector[i].y)%colorSegments.size()].y,
			colorSegments[(unsigned int)(fineSampleVector[i].y)%colorSegments.size()].z);
		glVertex3f(fineSampleVector[i].x,0.0f,fineSampleVector[i].z);
	}
	glEnd();	

}

Vector ClothoidSpline::getClothoidPiecePoint(float t1, float t, float t2) {

	Vector point;		

	//translate and rotate point
	if (t2>t1) {
		point=Vector(cosFresnel(t),0.0,sinFresnel(t));
		point=point-Vector(cosFresnel(t1),0.0,sinFresnel(t1));
		point=point.GetRotatedY(t1*t1*180.0/PI);		
	}
	else {		
		point=Vector(cosFresnel(t),0.0,sinFresnel(t));
		point=point-Vector(cosFresnel(t1),0.0,sinFresnel(t1));		
		point=point.GetRotatedY(t1*t1*180.0/PI+180.0);
		point.z=-point.z;
	}

	point.StretchDimensions(Vector(1.0/CURV_FACTOR,1.0/CURV_FACTOR,1.0/CURV_FACTOR));

	return point;


}

void ClothoidSpline::drawCurvaturePlot2D() {

	if (!using_drawCurvaturePlot) return;		

	glPushMatrix();

		glLoadIdentity();		
		
		glTranslatef((GLfloat)glutGet(GLUT_WINDOW_WIDTH)*0.25,(GLfloat)glutGet(GLUT_WINDOW_HEIGHT)*0.2,0.0f);
		glScalef(.5,.5,1);

		if (maxArcLength>100)
			glScalef((GLfloat)glutGet(GLUT_WINDOW_WIDTH)/maxArcLength,
					(GLfloat)glutGet(GLUT_WINDOW_HEIGHT)/(maxCurv-minCurv)/2.0,
					1.0);
		else
			glScalef((GLfloat)glutGet(GLUT_WINDOW_WIDTH)/100.0,
					(GLfloat)glutGet(GLUT_WINDOW_HEIGHT)/(maxCurv-minCurv)/2.0,
					1.0);

		glLineWidth(2.0f);
		glColor3f(0,0,0);
		glBegin(GL_LINES);
		glVertex3f(0,0.0-minCurv,-1.0);
		glVertex3f(maxArcLength+20.0f,0.0-minCurv,-1.0);
		glVertex3f(0,0,-1.0);
		glVertex3f(0,maxCurv-minCurv,-1.0);
		glEnd();
		
		glColor3f(0.0,0.0,0.0);		
		glPointSize(DRAW_LINEWIDTH);
		glBegin(GL_POINTS);
		for (int i=0;i<edgePointSet.size();i++) { 			
			glVertex3f(arcLength[i],estCurv[i]-minCurv,1.0);			
		}		
		glEnd();

		glLineWidth(DRAW_LINEWIDTH);
		
		for (int i=0;i<segmentPointSet.size()-1;i++) { 	
			if (using_G3) {
				if (segmentPointSet[i].z>0.5) 
					glColor3f(colorSegments[1].x,colorSegments[1].y,colorSegments[1].z);
				else
					glColor3f(colorSegments[0].x,colorSegments[0].y,colorSegments[0].z);				
			}
			else {
				int colIndex=i%colorSegments.size();
				glColor3f(colorSegments[colIndex].x,colorSegments[colIndex].y,colorSegments[colIndex].z);
			}
			glBegin(GL_LINES);
			glVertex3f(segmentPointSet[i].x,segmentPointSet[i].y-minCurv,0.0);
			glVertex3f(segmentPointSet[i+1].x,segmentPointSet[i+1].y-minCurv,0.0);
			glEnd();
		}		

	glPopMatrix();	

}

float ClothoidSpline::sinFresnel(float t) {

	t=t/CURV_FACTOR;

	float returnVal;
	if (t>=0.0) {
		float R=(0.506f*t+1.0f)/(1.79f*t*t+2.054f*t+sqrt(2.0f));
		float A=1.0f/(0.803f*pow(t,3)+1.886f*t*t+2.524f*t+2.0f);

		returnVal=0.5f-R*cos(0.5f*PI*(A-t*t));
	}
	else {
		t=-t;

		float R=(0.506f*t+1.0f)/(1.79f*t*t+2.054f*t+sqrt(2.0f));
		float A=1.0f/(0.803f*pow(t,3)+1.886f*t*t+2.524f*t+2.0f);

		returnVal=-(0.5f-R*cos(0.5f*PI*(A-t*t)));
	}
	return returnVal*CURV_FACTOR;

}

float ClothoidSpline::cosFresnel(float t) {

	t=t/CURV_FACTOR;

	float returnVal;
	if (t>=0.0) {
		float R=(0.506f*t+1.0f)/(1.79f*t*t+2.054f*t+sqrt(2.0f));
		float A=1.0f/(0.803f*pow(t,3)+1.886f*t*t+2.524f*t+2.0f);

		returnVal=0.5f-R*sin(0.5f*PI*(A-t*t));
	}
	else {
		t=-t;
	
		float R=(0.506f*t+1.0f)/(1.79f*t*t+2.054f*t+sqrt(2.0f));
		float A=1.0f/(0.803f*pow(t,3)+1.886f*t*t+2.524f*t+2.0f);

		returnVal=-(0.5f-R*sin(0.5f*PI*(A-t*t)));
	}
	return returnVal*CURV_FACTOR;

}

vector <Vector> ClothoidSpline::getPointSet() {
	return edgePointSet;
}

void ClothoidSpline::outputToMayaMEL(char *theFilename) {

	FILE *fp=fopen(theFilename,"w");	

	if (!fp)
		return;

	fprintf(fp,"requires maya \"4.0\";\n");
	fprintf(fp,"//curve count %d\n",2);
	fprintf(fp,"\n");      
		
	long numPoints = fineSampleVector.size();

	fprintf(fp,"createNode nurbsCurve -n ClothoidSpline%d;\n",0);
	fprintf(fp,"setAttr -k off \".v\";\n");
	fprintf(fp,"setAttr \".cc\" -type \"nurbsCurve\"\n");
	fprintf(fp,"1 %d 0 no 3\n%d",numPoints-1,numPoints);

	for ( int i = 0; i < numPoints; i++) {
		  fprintf(fp," %d",i);         
	}

	fprintf(fp,"\n%d\n",numPoints);          	

	for (int i = 0; i < numPoints; i++) {         
		fprintf(fp,"%f %f %f\n",fineSampleVector[i].x,0.0f,fineSampleVector[i].z);          
	}

	fprintf(fp,";\n");  
	fprintf(fp,"\n");       

	fclose(fp);
	
}

void ClothoidSpline::outputToIllustratorAI(char *theFilename) {	
	
	FILE *fp=fopen(theFilename,"w");	

	if (!fp)
		return;

	fprintf(fp,"%%!PS-Adobe-3.0\n");
	fprintf(fp,"%%%%Creator: Rhinoceros\n");
	fprintf(fp,"%%%%Title: (%s)\n",theFilename);
	fprintf(fp,"%%%%BoundingBox: -23 -10 26 32\n");
	fprintf(fp,"%%%%DocumentProcessColors: Black\n");
	fprintf(fp,"%%%%DocumentNeededResources: procset Adobe_packedarray 2.0 0\n");
	fprintf(fp,"%%%%+ procset Adobe_cmykcolor 1.1 0\n");
	fprintf(fp,"%%%%+ procset Adobe_cshow 1.1 0\n");
	fprintf(fp,"%%%%+ procset Adobe_customcolor 1.0 0\n");
	fprintf(fp,"%%%%+ procset Adobe_typography_AI3 1.0 0\n");
	fprintf(fp,"%%%%+ procset Adobe_IllustratorA_AI3 1.0 0\n");
	fprintf(fp,"%%AI3_ColorUsage: Color\n");
	fprintf(fp,"%%AI3_TemplateBox: 288 384 288 384\n");
	fprintf(fp,"%%AI3_TileBox: 0 0 576 768\n");
	fprintf(fp,"%%AI3_DocumentPreview: None\n");
	fprintf(fp,"%%%%Template:\n");
	fprintf(fp,"%%%%PageOrigin:0 0\n");
	fprintf(fp,"%%%%EndComments\n");
	fprintf(fp,"%%%%BeginProlog\n");
	fprintf(fp,"%%%%IncludeResource: procset Adobe_packedarray 2.0 0\n");
	fprintf(fp,"Adobe_packedarray /initialize get exec\n");
	fprintf(fp,"%%%%IncludeResource: procset Adobe_cmykcolor 1.1 0\n");
	fprintf(fp,"%%%%IncludeResource: procset Adobe_cshow 1.1 0\n");
	fprintf(fp,"%%%%IncludeResource: procset Adobe_customcolor 1.0 0\n");
	fprintf(fp,"%%%%IncludeResource: procset Adobe_typography_AI3 1.0 0\n");
	fprintf(fp,"%%%%IncludeResource: procset Adobe_IllustratorA_AI3 1.0 0\n");
	fprintf(fp,"%%%%EndProlog\n");
	fprintf(fp,"%%%%BeginSetup\n");
	fprintf(fp,"Adobe_cmykcolor /initialize get exec\n");
	fprintf(fp,"Adobe_cshow /initialize get exec\n");
	fprintf(fp,"Adobe_customcolor /initialize get exec\n");
	fprintf(fp,"Adobe_typography_AI3 /initialize get exec\n");
	fprintf(fp,"Adobe_IllustratorA_AI3 /initialize get exec\n");
	fprintf(fp,"%%%%EndSetup\n");
	fprintf(fp,"0 A\n");
	fprintf(fp,"0 R\n");
	fprintf(fp,"0 0 0 1 K\n");
	fprintf(fp,"0 i 0 J 0 j 1 w 4 M []0 d\n");
	fprintf(fp,"0 D\n");
	fprintf(fp,"1 1 1 1 0 0 -1 0 0 0 Lb\n");

	fprintf(fp,"(0) Ln\n");
	bool firstOfSegment=true;
	for (int i=0;i<fineSampleVector.size();i++) {
		if (firstOfSegment) {
			fprintf(fp,"%f %f m\n",fineSampleVector[i].x,128.0f-fineSampleVector[i].z);
			firstOfSegment=false;
		}
		else
			fprintf(fp,"%f %f L\n",fineSampleVector[i].x,128.0f-fineSampleVector[i].z);

		if (i<fineSampleVector.size()-1&&fineSampleVector[i].y!=fineSampleVector[i+1].y) {
			fprintf(fp,"S\n");
			firstOfSegment=true;
		}
	}
	fprintf(fp,"S\n");
		
	for (int i=0;i<edgePointSet.size();i++) {
		if (i==0)
			fprintf(fp,"%f %f m\n",edgePointSet[i].x,128.0f-edgePointSet[i].z);
		else
			fprintf(fp,"%f %f L\n",edgePointSet[i].x,128.0f-edgePointSet[i].z);
	}
	fprintf(fp,"S\n");

	fprintf(fp,"LB\n");
	fprintf(fp,"%%%%PageTrailer\n");
	fprintf(fp,"gsave annotatepage grestore showpage\n");
	fprintf(fp,"%%%%Trailer\n");
	fprintf(fp,"Adobe_IllustratorA_AI3 /terminate get exec\n");
	fprintf(fp,"Adobe_typography_AI3 /terminate get exec\n");
	fprintf(fp,"Adobe_customcolor /terminate get exec\n");
	fprintf(fp,"Adobe_cshow /terminate get exec\n");
	fprintf(fp,"Adobe_cmykcolor /terminate get exec\n");
	fprintf(fp,"Adobe_packedarray /terminate get exec\n");
	fprintf(fp,"%%%%EOF\n");

	fclose(fp);

}

void ClothoidSpline::outputToSVG(char *theFilename) {

	FILE *fp=fopen(theFilename,"w");	

	if (!fp)
		return;

	fprintf(fp,"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
	fprintf(fp,"<svg\n");
	fprintf(fp,"xmlns:svg=\"http://www.w3.org/2000/svg\"\n");
	fprintf(fp,"xmlns=\"http://www.w3.org/2000/svg\"\n");
	fprintf(fp,"version=\"1.0\"\n");
	fprintf(fp,"width=\"744.09448\"\n");
	fprintf(fp,"height=\"1052.3622\"\n");
	fprintf(fp,"id=\"svg2\">\n");
	fprintf(fp,"<defs\n");
	fprintf(fp,"id=\"defs4\" />\n");	
	
	int curColorSegment=0;
	bool firstOfSegment=true;

	if (!using_G1) {
		
		fprintf(fp,"<g\n");
		fprintf(fp,"id=\"layer1\">\n");
		fprintf(fp,"<path\n");
		fprintf(fp,"d=\"");

		for (int i=0;i<fineSampleVector.size();i++) {
			if (firstOfSegment) {
				fprintf(fp,"M %f %f",-fineSampleVector[i].z,fineSampleVector[i].x);
				firstOfSegment=false;
			}
			else
				fprintf(fp," L %f %f",-fineSampleVector[i].z,fineSampleVector[i].x);

			if (i<fineSampleVector.size()-1&&fineSampleVector[i].y!=fineSampleVector[i+1].y) {
				
				firstOfSegment=true;

				fprintf(fp,"\" id=\"path%i\"\n",i);
				fprintf(fp,"style=\"fill:none;fill-rule:evenodd;stroke:#%02x%02x%02x;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" />\n",
					(unsigned int)(colorSegments[curColorSegment].x*255.0),(unsigned int)(colorSegments[curColorSegment].y*255.0),(unsigned int)(colorSegments[curColorSegment].z*255.0));
				fprintf(fp,"</g>\n");

				fprintf(fp,"<g\n");
				fprintf(fp,"id=\"layer1\">\n");
				fprintf(fp,"<path\n");
				fprintf(fp,"d=\"");

				curColorSegment++;
				curColorSegment=curColorSegment%colorSegments.size();
			}
		}

		fprintf(fp,"\" id=\"path%i\"\n",fineSampleVector.size());
		fprintf(fp,"style=\"fill:none;fill-rule:evenodd;stroke:#%02x%02x%02x;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" />\n",
					(unsigned int)(colorSegments[curColorSegment].x*255.0),(unsigned int)(colorSegments[curColorSegment].y*255.0),(unsigned int)(colorSegments[curColorSegment].z*255.0));
		fprintf(fp,"</g>\n");

	}
	else if (!g1ClothoidVec.empty()) {

		for (int c=0;c<g1ClothoidVec.size();c++) {
			vector <Vector> eachFineSampleVector=g1ClothoidVec[c]->getFineSamples();

			fprintf(fp,"<g\n");
			fprintf(fp,"id=\"layer1\">\n");
			fprintf(fp,"<path\n");
			fprintf(fp,"d=\"");

			firstOfSegment=true;
			curColorSegment=0;
			for (int i=0;i<eachFineSampleVector.size();i++) {
				if (firstOfSegment) {
					fprintf(fp,"M %f %f",-eachFineSampleVector[i].z,eachFineSampleVector[i].x);
					firstOfSegment=false;
				}
				else
					fprintf(fp," L %f %f",-eachFineSampleVector[i].z,eachFineSampleVector[i].x);

				if (i<eachFineSampleVector.size()-1&&eachFineSampleVector[i].y!=eachFineSampleVector[i+1].y) {
					
					firstOfSegment=true;

					fprintf(fp,"\" id=\"path%i\"\n",i);
					fprintf(fp,"style=\"fill:none;fill-rule:evenodd;stroke:#%02x%02x%02x;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" />\n",
						(unsigned int)(colorSegments[curColorSegment].x*255.0),(unsigned int)(colorSegments[curColorSegment].y*255.0),(unsigned int)(colorSegments[curColorSegment].z*255.0));
					fprintf(fp,"</g>\n");

					fprintf(fp,"<g\n");
					fprintf(fp,"id=\"layer1\">\n");
					fprintf(fp,"<path\n");
					fprintf(fp,"d=\"");

					curColorSegment++;
					curColorSegment=curColorSegment%colorSegments.size();
				}
			}

			fprintf(fp,"\" id=\"path%i\"\n",fineSampleVector.size());
			fprintf(fp,"style=\"fill:none;fill-rule:evenodd;stroke:#%02x%02x%02x;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" />\n",
						(unsigned int)(colorSegments[curColorSegment].x*255.0),(unsigned int)(colorSegments[curColorSegment].y*255.0),(unsigned int)(colorSegments[curColorSegment].z*255.0));
			fprintf(fp,"</g>\n");

		}

	}
	
	fprintf(fp,"<g\n");
	fprintf(fp,"id=\"layer1\">\n");
	fprintf(fp,"<path\n");
	fprintf(fp,"d=\"");

	for (int i=0;i<edgePointSet.size();i++) {
		if (i==0)
			fprintf(fp,"M %f %f",-edgePointSet[i].z,edgePointSet[i].x);
		else
			fprintf(fp," L %f %f",-edgePointSet[i].z,edgePointSet[i].x);

		if (using_closed&&i>=edgePointSet.size()-2)
			break;
	}

	fprintf(fp,"\" id=\"path%i\"\n",fineSampleVector.size());
	fprintf(fp,"style=\"fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.5;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" />\n");
	fprintf(fp,"</g>\n");			

	double minCurv=DBL_MAX;
	double maxCurv=-DBL_MAX;

	for (int i=0;i<segmentPointSet.size();i++) {
		if (segmentPointSet[i].y<minCurv)
			minCurv=segmentPointSet[i].y;
		if (segmentPointSet[i].y>maxCurv)
			maxCurv=segmentPointSet[i].y;
	}

	/*
	 * Draw original curvature plot
	 */

	fprintf(fp,"<g\n");
	fprintf(fp,"id=\"layer1\">\n");
	fprintf(fp,"<path\n");
	fprintf(fp,"d=\"");
	fprintf(fp,"M %f %f",arcLength[0],-(estCurv[0]-minCurv)/(maxCurv-minCurv)*100.0);	
	for (int i=1;i<estCurv.size();i++)
		fprintf(fp," L %f %f",arcLength[i],-(estCurv[i]-minCurv)/(maxCurv-minCurv)*100.0);

	fprintf(fp,"\" id=\"path%i\"\n",0);
	fprintf(fp,"style=\"fill:none;fill-rule:evenodd;stroke:#%02x%02x%02x;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" />\n",
		(unsigned int)(0),(unsigned int)(0),(unsigned int)(0));
	fprintf(fp,"</g>\n");

	/*
	 * Draw colour coded curvature plot
	 */

	if (!using_G1) {
		curColorSegment=0;
		for (int i=0;i<segmentPointSet.size()-1;i++) {
			
			fprintf(fp,"<g\n");
			fprintf(fp,"id=\"layer1\">\n");
			fprintf(fp,"<path\n");
			fprintf(fp,"d=\"");
			fprintf(fp,"M %f %f",segmentPointSet[i].x,-(segmentPointSet[i].y-minCurv)/(maxCurv-minCurv)*100.0);		
			fprintf(fp," L %f %f",segmentPointSet[i+1].x,-(segmentPointSet[i+1].y-minCurv)/(maxCurv-minCurv)*100.0);			

			fprintf(fp,"\" id=\"path%i\"\n",i);
			fprintf(fp,"style=\"fill:none;fill-rule:evenodd;stroke:#%02x%02x%02x;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" />\n",
				(unsigned int)(colorSegments[curColorSegment].x*255.0),(unsigned int)(colorSegments[curColorSegment].y*255.0),(unsigned int)(colorSegments[curColorSegment].z*255.0));
			fprintf(fp,"</g>\n");

			curColorSegment++;
			curColorSegment=curColorSegment%colorSegments.size();
			
		}
	}
	else {
		float baseArcLength=0.0f;		
		for (int c=0;c<g1ClothoidVec.size();c++) {
			vector <Vector> eachSegmentPointSet=g1ClothoidVec[c]->getSegmentPointSet();

			curColorSegment=0;			
			for (int i=0;i<eachSegmentPointSet.size()-1;i++) {
				
				fprintf(fp,"<g\n");
				fprintf(fp,"id=\"layer1\">\n");
				fprintf(fp,"<path\n");
				fprintf(fp,"d=\"");
				fprintf(fp,"M %f %f",baseArcLength+eachSegmentPointSet[i].x,-(eachSegmentPointSet[i].y-minCurv)/(maxCurv-minCurv)*100.0);		
				fprintf(fp," L %f %f",baseArcLength+eachSegmentPointSet[i+1].x,-(eachSegmentPointSet[i+1].y-minCurv)/(maxCurv-minCurv)*100.0);			

				fprintf(fp,"\" id=\"path%i\"\n",i);
				fprintf(fp,"style=\"fill:none;fill-rule:evenodd;stroke:#%02x%02x%02x;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" />\n",
					(unsigned int)(colorSegments[curColorSegment].x*255.0),(unsigned int)(colorSegments[curColorSegment].y*255.0),(unsigned int)(colorSegments[curColorSegment].z*255.0));
				fprintf(fp,"</g>\n");

				curColorSegment++;
				curColorSegment=curColorSegment%colorSegments.size();
				
			}

			baseArcLength+=eachSegmentPointSet[eachSegmentPointSet.size()-1].x;

		}
	}

	//draw axes
	fprintf(fp,"<g\n");
	fprintf(fp,"id=\"layer1\">\n");
	fprintf(fp,"<path\n");
	fprintf(fp,"d=\"");
	fprintf(fp,"M %f %f",segmentPointSet[0].x,-(0-minCurv)/(maxCurv-minCurv)*100.0);		
	fprintf(fp," L %f %f",segmentPointSet[segmentPointSet.size()-1].x,-(0-minCurv)/(maxCurv-minCurv)*100.0);			
	fprintf(fp,"\" id=\"path0\"\n");
	fprintf(fp,"style=\"fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" />\n");
	fprintf(fp,"</g>\n");

	fprintf(fp,"<g\n");
	fprintf(fp,"id=\"layer1\">\n");
	fprintf(fp,"<path\n");
	fprintf(fp,"d=\"");
	fprintf(fp,"M 0.0 -100.0");		
	fprintf(fp," L 0.0 0.0");			
	fprintf(fp,"\" id=\"path0\"\n");
	fprintf(fp,"style=\"fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" />\n");
	fprintf(fp,"</g>\n");

	fprintf(fp,"</svg>\n");
	
	fclose(fp);

}

vector <Vector> ClothoidSpline::getSegmentPointSet() {

	return segmentPointSet;

}

Vector ClothoidSpline::getStartPoint() {	

	Vector point=segmentTranslate[0];
	
	point=point-centreSpline;
	point=point.GetRotatedY(fitRotate);
	point+=centreSpline;

	point+=fitTranslate;

	return point;

}

Vector ClothoidSpline::getEndPoint() {

	Vector point=segmentTranslate[segmentTranslate.size()-1];

	point=point-centreSpline;
	point=point.GetRotatedY(fitRotate);
	point+=centreSpline;

	point+=fitTranslate;

	return point;

}

double ClothoidSpline::GetCurvatureIntegral(unsigned int start, unsigned int end) {

	double curvInt=0.0;

	for (unsigned int i=start;i<end;i++) {
		curvInt+=(segmentPointSet[i+1].x-segmentPointSet[i].x)*
					(segmentPointSet[i].y+segmentPointSet[i+1].y)/2.0;
		
	}
	
	return curvInt;

}

void ClothoidSpline::SnapClosed() {

	/*
	 * Curvature-space conditions for curve closure:
	 *
	 * 1.  Something to do with length					-	(G0 continuous)
	 * 2.  Curvature integral = k*2*PI for some int k	-	(G1 continuous)
	 * 3.  segmentPoint[0].y = segmentPoint[n-1].y		-	(G2 continuous)
	 */

	if (segmentPointSet.size()<2)
		return;
	
	printf("curv Before: %f\n",GetCurvatureIntegral(0,segmentPointSet.size()-1));
	printf("without Ends: %f\n",GetCurvatureIntegral(1,segmentPointSet.size()-2));
	double targetDiff=2*PI-GetCurvatureIntegral(1,segmentPointSet.size()-2);
	printf("goal: %f targetDiff: %f\n",2*PI,targetDiff);

	for (int i=0;i<segmentPointSet.size()-1;i++)
		printf("from %i to %i: %f\n",i,i+1,GetCurvatureIntegral(i,i+1));

	double ax=0.0;
	double bx=segmentPointSet[1].x;
	double by=segmentPointSet[1].y;
	double cx=segmentPointSet[segmentPointSet.size()-2].x;
	double cy=segmentPointSet[segmentPointSet.size()-2].y;
	double dx=segmentPointSet[segmentPointSet.size()-1].x;

	double ay=(2*targetDiff-bx*by+ax*by-dx*cy+cx*cy)/(bx-ax+dx-cx);

	segmentPointSet[0].y=ay;
	segmentPointSet[segmentPointSet.size()-1].y=ay;

	printf("curv After: %f\n",GetCurvatureIntegral(0,segmentPointSet.size()-1));
	for (int i=0;i<segmentPointSet.size()-1;i++)
		printf("from %i to %i: %f\n",i,i+1,GetCurvatureIntegral(i,i+1));


}

unsigned int ClothoidSpline::GetNumCurvaturePoints() {
	return segmentPointSet.size();
}

double ClothoidSpline::GetEndpointError(double tangentFadeIn) {

	setupCanonicalSegments();
	Vector startPoint=getStartPoint();
	Vector endPoint=getEndPoint();

	double theCurvInt=GetCurvatureIntegral(0,segmentPointSet.size()-1);

	//return (startPoint-endPoint).GetLengthSquared()+
	//	5.0*min(abs(theCurvInt-2*PI),
	//					min(abs(theCurvInt+2*PI), abs(theCurvInt)));

	// this does position+tangent (without tangent fade-in)
	return (startPoint-endPoint).GetLengthSquared()+
		tangentFadeIn*min(abs(theCurvInt-2*PI),
						min(abs(theCurvInt+2*PI), abs(theCurvInt)));

	//this does tangent only
	//return min(abs(theCurvInt-2*PI),
	//					min(abs(theCurvInt+2*PI), abs(theCurvInt)));
}

double ClothoidSpline::DoOneConvergeIter(double tangentFadeIn) {

	double *errors=new double[11];
	errors[0]=GetEndpointError(tangentFadeIn);

	double curCurv1=segmentPointSet[0].y;
	//double downCurv1=curCurv1-errors[0]/1000.0;
	//double upCurv1=curCurv1+errors[0]/1000.0;
	double downCurv1=curCurv1-1.0/1000.0;
	double upCurv1=curCurv1+1.0/1000.0;

	double curCurv2=segmentPointSet[segmentPointSet.size()-2].y;
	//double downCurv2=curCurv2-errors[0]/1000.0;
	//double upCurv2=curCurv2+errors[0]/1000.0;
	double downCurv2=curCurv2-1.0/1000.0;
	double upCurv2=curCurv2+1.0/1000.0;

	double curStartArclength=segmentPointSet[0].x;								
	//double upStartArclength=curStartArclength+errors[0]/100.0;
	//double downStartArclength=curStartArclength-errors[0]/100.0;
	double upStartArclength=curStartArclength+1.0/100.0;
	double downStartArclength=curStartArclength-1.0/100.0;

	double curArclength=segmentPointSet[segmentPointSet.size()-2].x;								
	//double upArclength=curArclength+errors[0]/100.0;
	//double downArclength=curArclength-errors[0]/100.0;
	double upArclength=curArclength+1.0/100.0;
	double downArclength=curArclength-1.0/100.0;
	double curArclengthOther=segmentPointSet[segmentPointSet.size()-1].x;								
	//double upArclengthOther=curArclengthOther+errors[0]/100.0;
	//double downArclengthOther=curArclengthOther-errors[0]/100.0;
	double upArclengthOther=curArclengthOther+1.0/100.0;
	double downArclengthOther=curArclengthOther-1.0/100.0;

	double curEndArclength=segmentPointSet[segmentPointSet.size()-1].x;								
	//double upEndArclength=curEndArclength+errors[0]/100.0;
	//double downEndArclength=curEndArclength-errors[0]/100.0;
	double upEndArclength=curEndArclength+1.0/100.0;
	double downEndArclength=curEndArclength-1.0/100.0;

	/*
	 * Adjust final curvature 1
	 */		
	
	segmentPointSet[0].y=upCurv1;
	segmentPointSet[segmentPointSet.size()-1].y=upCurv1;
	errors[1]=GetEndpointError(tangentFadeIn);		
	segmentPointSet[0].y=curCurv1;
	segmentPointSet[segmentPointSet.size()-1].y=curCurv1;
	
	segmentPointSet[0].y=downCurv1;
	segmentPointSet[segmentPointSet.size()-1].y=downCurv1;
	errors[2]=GetEndpointError(tangentFadeIn);	
	segmentPointSet[0].y=curCurv1;
	segmentPointSet[segmentPointSet.size()-1].y=curCurv1;	

	/*
	 * Adjust final curvature 2
 	*/			
	
	segmentPointSet[segmentPointSet.size()-2].y=upCurv2;
	errors[3]=GetEndpointError(tangentFadeIn);		
	segmentPointSet[segmentPointSet.size()-2].y=curCurv2;
	
	segmentPointSet[segmentPointSet.size()-2].y=downCurv2;
	errors[4]=GetEndpointError(tangentFadeIn);
	segmentPointSet[segmentPointSet.size()-2].y=curCurv2;	


	/*
	 * Adjust start arc length
	 */

	segmentPointSet[0].x=upStartArclength;
	errors[5]=GetEndpointError(tangentFadeIn);
	segmentPointSet[0].x=curStartArclength;

	segmentPointSet[0].x=downStartArclength;
	errors[6]=GetEndpointError(tangentFadeIn);
	segmentPointSet[0].x=curStartArclength;

	/*
	 * Adjust end piece arc length
	 */

	segmentPointSet[segmentPointSet.size()-2].x=upArclength;
	segmentPointSet[segmentPointSet.size()-1].x=upArclengthOther;
	errors[7]=GetEndpointError(tangentFadeIn);
	segmentPointSet[segmentPointSet.size()-2].x=curArclength;
	segmentPointSet[segmentPointSet.size()-1].x=curArclengthOther;

	segmentPointSet[segmentPointSet.size()-2].x=downArclength;
	segmentPointSet[segmentPointSet.size()-1].x=downArclengthOther;
	errors[8]=GetEndpointError(tangentFadeIn);
	segmentPointSet[segmentPointSet.size()-2].x=curArclength;
	segmentPointSet[segmentPointSet.size()-1].x=curArclengthOther;	

	/*
	 * Adjust patch piece arc length
	 */

	segmentPointSet[segmentPointSet.size()-1].x=upEndArclength;
	errors[9]=GetEndpointError(tangentFadeIn);
	segmentPointSet[segmentPointSet.size()-1].x=curEndArclength;

	segmentPointSet[segmentPointSet.size()-1].x=downEndArclength;
	errors[10]=GetEndpointError(tangentFadeIn);
	segmentPointSet[segmentPointSet.size()-1].x=curEndArclength;	

	int minIndex=0;
	double minError=errors[0];
	
	for (int i=0;i<11;i++) {
		//printf("errors[%i]: %f\n",i,errors[i]);
		if (errors[i]<minError) {
			minIndex=i;
			minError=errors[i];
		}
	}
	//printf("WINNER: minIndex: %i minError: %f\n\n",minIndex,minError);

	if (errors[1]<errors[0]) { 

		segmentPointSet[0].y=upCurv1;
		segmentPointSet[segmentPointSet.size()-1].y=upCurv1;
	}

	if (errors[2]<errors[0]) { 
		segmentPointSet[0].y=downCurv1;
		segmentPointSet[segmentPointSet.size()-1].y=downCurv1;
	}

	if (errors[3]<errors[0]) {  
		segmentPointSet[segmentPointSet.size()-2].y=upCurv2;	
	}

	if (errors[4]<errors[0]) {  
		segmentPointSet[segmentPointSet.size()-2].y=downCurv2;
	}

	if (errors[5]<errors[0]) { 
		segmentPointSet[0].x=upStartArclength;
	}

	if (errors[6]<errors[0]) { 
		segmentPointSet[0].x=downStartArclength;
	}

	if (errors[7]<errors[0]) { 
		segmentPointSet[segmentPointSet.size()-2].x=upArclength;
		segmentPointSet[segmentPointSet.size()-1].x=upArclengthOther;
	}

	if (errors[8]<errors[0]) { 
		segmentPointSet[segmentPointSet.size()-2].x=downArclength;
		segmentPointSet[segmentPointSet.size()-1].x=downArclengthOther;
	}

	if (errors[9]<errors[0]) { 
		segmentPointSet[segmentPointSet.size()-1].x=upEndArclength;
	}

	if (errors[10]<errors[0]) { 
		segmentPointSet[segmentPointSet.size()-1].x=downEndArclength;
	}	

	/*
	switch (minIndex) {

		case (1): 

			segmentPointSet[0].y=upCurv1;
			segmentPointSet[segmentPointSet.size()-1].y=upCurv1;
			printf("increasing curvature 1\n");			
			break;

		case(2):
	
			segmentPointSet[0].y=downCurv1;
			segmentPointSet[segmentPointSet.size()-1].y=downCurv1;
			printf("lowering curvature 1\n");
			break;

		case (3):

			segmentPointSet[segmentPointSet.size()-2].y=upCurv2;
			printf("increasing curvature 2\n");			
			break;

		case (4):

			segmentPointSet[segmentPointSet.size()-2].y=downCurv2;
			printf("lowering curvature 2\n");
			break;

		case (5):

			segmentPointSet[0].x=upStartArclength;
			printf("increase start arc length\n");
			break;

		case (6):

			segmentPointSet[0].x=downStartArclength;
			printf("lower start arc length\n");
			break;

		case (7):

			segmentPointSet[segmentPointSet.size()-2].x=upArclength;
			segmentPointSet[segmentPointSet.size()-1].x=upArclengthOther;
			printf("increase end arc length\n");			
			break;
	
		case (8):

			segmentPointSet[segmentPointSet.size()-2].x=downArclength;
			segmentPointSet[segmentPointSet.size()-1].x=downArclengthOther;
			printf("lower end arc length\n");
			break;

		case (9):

			segmentPointSet[segmentPointSet.size()-1].x=upEndArclength;
			printf("increase patch arc length\n");
			break;

		case (10):

			segmentPointSet[segmentPointSet.size()-1].x=downEndArclength;
			printf("lower patch arc length\n");
			break;

	}
	*/

	delete [] errors;
	return (GetEndpointError(tangentFadeIn));

}

void ClothoidSpline::ConvergeEndpointsToTouch() {

	while (DoOneConvergeIter()) { };

}

void ClothoidSpline::HeightLineFit2(int numPoints, const Vector2 *points, float &m, float &b) {

	// compute sums for linear system
    float fSumX = 0.0;
	float fSumY = 0.0;
    float fSumXX = 0.0;
	float fSumXY = 0.0;    

	float n=numPoints;

    for (int i = 0; i < numPoints; i++)
    {
        fSumX += points[i].x;
        fSumY += points[i].y;
        fSumXX += points[i].x*points[i].x;
        fSumXY += points[i].x*points[i].y;
    }

	m=(n*fSumXY-fSumX*fSumY)/(n*fSumXX-fSumX*fSumX);
	b=(fSumY-m*fSumX)/n;

}