#ifndef _UI_SELECTABLE_H
#define _UI_SELECTABLE_H

#include <vector>
#include "Vector.h"
#define MAXSELECTDISTANCE 5000.0f

using namespace std;

class UI_Selectable {

	public:

	virtual void selectWithRay(Vector pos_xyz, Vector rayDir) = 0;
	virtual void selectWithClosedCurve(vector <Vector> pointSet) = 0;
	bool isSelected() { return selected; };
	void setSelected(bool sel) { selected=sel; };

	protected:

	bool selected;

};

#endif