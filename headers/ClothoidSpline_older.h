#include <math.h>
#include <GL/glut.h>

#include "Vector.h"

#define NUM_FRESNEL_ITERATIONS 15
#define PI 3.14159
#define DEG2RAD PI/180.0
#define RAD2DEG 180.0/PI

class ClothoidSpline_older {

	public:

	ClothoidSpline_older(int num, Vector *set);
	ClothoidSpline_older(int num, Vector *set, float *curvatures);
	void setupSpline();
	void draw();
	void setCurvatures(float *curvatures);

	private:

	Vector getClothoidPoint(float t, float B, bool curveRight);

	float computeSinFresnelIntegral(float t);
	float computeCosFresnelIntegral(float t);

	float doLineCircleFormula(float newQ_Z, Vector centre, float radius, float K_Q, float &tangent_newQ_x, float &tangent_newQ_z, float &newQ_x);
	float solveForBFormula(float B, Vector P, Vector Q, float K_P, float K_Q, float W);

	Vector *pointSet;
	Vector *pointSetTangents;
	float *pointSetCurvatures;
	int numPoints;

	Vector *piecePointSet;
	int *pieceType;
	int numPieces;

	float *t, *B, *x;
	bool *curveRight;
	
	float L_P, L_Q;

};