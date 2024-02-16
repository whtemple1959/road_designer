#ifndef _CLOTHOIDSPLINE_H
#define _CLOTHOIDSPLINE_H

#include <windows.h>
#include <GL/glew.h>
#include <glut.h>

#include <float.h>
//#include <Wm4Foundation.h> //no longer using wm4lib

#include <vector>
#include "Vector.h"

#define PI 3.14159265
#define NUM_FRESNEL_ITERATIONS 15

#define CURV_FACTOR sqrt(PI/2.0)

//#define MIN_CURVATURE_SLOPE 0.0075
//#define MIN_DISTANCE_FROM_ZERO 0.005
#define MIN_CURVATURE_SLOPE 0.0015
#define MIN_DISTANCE_FROM_ZERO 0.001

#define DEFAULT_CLOTHOID_PENALTY 0.05

#define DEFAULT_G1DISCONT_CURVTHRESH 0.01f
#define DEFAULT_ENDPOINT_WEIGHT 1.0f

using namespace std;
//using namespace Wm4;

class ClothoidSpline {

	public:

	ClothoidSpline(vector <Vector> edgePoints, float penalty=DEFAULT_CLOTHOID_PENALTY, 
		bool using_G3=false, bool using_G1=false, float g1discontCurvThresh=DEFAULT_G1DISCONT_CURVTHRESH, float endPointWeight=DEFAULT_ENDPOINT_WEIGHT,
		bool using_closed=false);	
	~ClothoidSpline();

	void draw();	
	void drawCurvaturePlot2D();

	vector <Vector> getFineSamples();

	void outputToMayaMEL(char *theFilename);
	void outputToIllustratorAI(char *theFilename);
	void outputToSVG(char *theFilename);

	vector <Vector> getPointSet();
	
	unsigned int GetNumCurvaturePoints();
	double GetCurvatureIntegral(unsigned int start, unsigned int end);	
	void SnapClosed();
	void ConvergeEndpointsToTouch();
	double DoOneConvergeIter(double tangentFadeIn=1.0);
	double GetEndpointError(double tangentFadeIn=1.0);

	void setupCanonicalSegments();
	void setupArcLengthSamples();
	void setupFitTransform(float endpointWeight);
	void setupFineSamples(float sampleDistance);

	private:

	float getPiecePenalty();
	static vector <float> getCurvatures(vector <Vector> theEdgePointSet);

	Vector getPoint(int segment);	
	Vector getTangent(int segment, float interp);	

	Vector getStartPoint();
	Vector getEndPoint();	

	vector <Vector> getSegmentPointSet(); //for use with G1 discont feature when concatenating clothoid curves	

	float sinFresnel(float t);  //the y component
	float cosFresnel(float t);  //the x component

	Vector getClothoidPiecePoint(float t1, float t, float t2);

	float getFitErrors(int num, Vector2 *points, float A, float B);
	void recurseThroughWalkMatrix(vector <vector <int>>, int begin, int end, bool *segmentEnd);	

	void HeightLineFit2(int numPoints, const Vector2 *points, float &m, float &b);

	bool using_G1;
	bool using_G3;
	bool using_closed;
	bool using_drawCurvaturePlot;	

	vector <Vector> edgePointSet;
	vector <float> estCurv;
	vector <float> arcLength;	

	vector <Vector> segmentPointSet;

	vector <Vector> segmentTranslate;
	vector <float> segmentRotate;
	vector <Vector> arcLengthSamples;
	unsigned int arcLengthSamplesStart, arcLengthSamplesEnd;
	vector <Vector> fineSampleVector;	

	//translation/rotation to fit sketch
	Vector centreSpline;
	Vector centreSketch;
	Vector fitTranslate;
	float fitRotate;

	float piecePenalty;

	float minCurv;
	float maxCurv;

	static vector <Vector> colorSegments;
	float maxArcLength;
	
	vector <ClothoidSpline *> g1ClothoidVec;

};

#endif