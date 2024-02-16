#include "Collision.h"

Collision::Collision() {
	
}

Vector * Collision::LineLineCollideXZ(Vector v1, Vector v2, Vector v3, Vector v4) {

	v1.y=0.0;
	v2.y=0.0;
	v3.y=0.0;
	v4.y=0.0;	

	if (fabs((v2-v1).DotProduct3(v4-v3))<0.001) { //lines parallel, no unique intersection point
		return NULL;
	}
	
	//solve for parametric coordinate u for v=v1+u*d1
	float u=((v4.x-v3.x)*(v1.z-v3.z)-(v4.z-v3.z)*(v1.x-v3.x))/((v4.z-v3.z)*(v2.x-v1.x)-(v4.x-v3.x)*(v2.z-v1.z));
	if (u>1000000.0)
		return NULL;

	Vector v=v1+(v2-v1)*u;
	return new Vector(v.x,0.0,v.z);

}

bool Collision::PointTriangleCollide(Vector P, Vector A, Vector B, Vector C) {

	//use the Barycentric method (cause its fastest)

	// Compute vectors        
	Vector v0 = C - A;
	Vector v1 = B - A;
	Vector v2 = P - A;

	// Compute dot products
	float dot00 = v0.DotProduct3(v0);
	float dot01 = v0.DotProduct3(v1);
	float dot02 = v0.DotProduct3(v2);
	float dot11 = v1.DotProduct3(v1);
	float dot12 = v1.DotProduct3(v2);

	// Compute barycentric coordinates
	double invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
	double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	// Check if point is in triangle
	return (u >= 0.0f) && (v >= 0.0f) && (u + v <= 1.0f);

}

bool Collision::LineSphereCollide(Vector sphereCentre, float sphereRad, Vector lineOrig, Vector lineDir) {

	Vector w=sphereCentre-lineOrig;
	Vector wCrossVL;
	wCrossVL.CrossProduct(w,lineDir);

	float dist=wCrossVL.GetLength()/lineDir.GetLength();
	
	if (dist<=sphereRad)
		return true;
	else 
		return false;

}

Vector * Collision::LineTriangleCollide(Vector pos_xyz, Vector pickray, Vector v1, Vector v2, Vector v3) {
    
    // intersect_RayTriangle(): intersect a ray with a 3D triangle
//    Input:  a ray R, and a triangle T
//    Output: *I = intersection point (when it exists)
//    Return: -1 = triangle is degenerate (a segment or point)
//             0 = disjoint (no intersect)
//             1 = intersect in unique point I1
//             2 = are in the same plane

    Vector    u, v, n;             // triangle vectors
    Vector    dir, w0, w;          // ray vectors
    float     r, a, b;             // params to calc ray-plane intersect

    // get triangle edge vectors and plane normal
    u = v2-v1;
    v = v3-v1;
	n.CrossProduct(u,v);             // cross product
    if (n.GetLength()==0.0)            // triangle is degenerate
        return NULL;                 // do not deal with this case

    dir = pickray;             // ray direction vector
    w0 = pos_xyz - v1;
    a = -n.DotProduct3(w0);
    b = n.DotProduct3(dir);
    if (fabs(b) < 0.00001) {     // ray is parallel to triangle plane
        return NULL;             // ray disjoint from plane
    }

    // get intersect point of ray with triangle plane
    r = a / b;
    //if (r < 0.0)                   // ray goes away from triangle
    //     return NULL;                  // => no intersect
    // for a segment, also test if (r > 1.0) => no intersect

    Vector intersectPoint = pos_xyz + dir.StretchDimensions(r,r,r);           // intersect point of ray and plane

    // is I inside T?
    float    uu, uv, vv, wu, wv, D;
    uu = u.DotProduct3(u);
    uv = u.DotProduct3(v);
    vv = v.DotProduct3(v);
    w = intersectPoint - v1;
    wu = w.DotProduct3(u);
    wv = w.DotProduct3(v);
    D = uv * uv - uu * vv;

    // get and test parametric coords
    float s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)        // I is outside T
        return NULL;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)  // I is outside T
        return NULL;	

    return new Vector(intersectPoint.x,intersectPoint.y,intersectPoint.z); // I is in T
}

bool Collision::LineSegmentsCollide(Vector line1A, Vector line1B, Vector line2A, Vector line2B, Vector &intersection) {	

    float dx = line1B.x - line1A.x;
    float dy = line1B.z - line1A.z;
    float da = line2B.x - line2A.x;
    float db = line2B.z - line2A.z;
	if (da * dy - db * dx==0)
        return false;
    
    float s = (dx * (line2A.z - line1A.z) + dy * (line1A.x - line2A.x)) / (da * dy - db * dx);
    float t = (da * (line1A.z - line2A.z) + db * (line2A.x - line1A.x)) / (db * dx - da * dy);

	intersection.x=line1A.x+t*dx;
	intersection.y=line1A.y+(line1B.y-line1A.y)*t;
	intersection.z=line1A.z+t*dy;

	return (s>=0.0&&s<=1.0&&t>=0.0&&t<=1.0);

}
