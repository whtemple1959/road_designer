#include <GL/glut.h>
#include <IL/ilut.h>

#include "Vector.h"
#include "RN.h"

class TwistLens {

	public:

	TwistLens();

	void draw();
	void setViewTwists(RN *roadNetwork);

	void mouseMotion(int x, int y);
	void mouseClick(int x, int y, bool down);

	private:

	void drawIcon(float xtrans, float ytrans, int textureIndex);

	Vector translateIconCentre();
	Vector outerRadiusIconCentre();
	Vector innerRadiusIconCentre();
	Vector twistIconCentre();

	Vector lensCentre;
	float outerRadius;
	float innerRadius;
	float viewTwist; //(in degrees)

	//textures for widget
	GLuint texture_translate;
	GLuint texture_radius;
	GLuint texture_twist;

	int iconSelected;

};