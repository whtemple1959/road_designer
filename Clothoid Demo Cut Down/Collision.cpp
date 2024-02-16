#include "Collision.h"

Collision::Collision() {
	
}
Vector *Collision::LineTriangleCollide(Vector pos_xyz, Vector pickray, Vector v1, Vector v2, Vector v3) {
    
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
