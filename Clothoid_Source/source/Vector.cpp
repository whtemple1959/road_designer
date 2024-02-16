#include "Vector.h"        // Vector class.

Vector::Vector()
{
   // Initialize the variables to 0.
   x = y = z = 0.0;
}


Vector::Vector(float X, float Y, float Z)
{
   // Initialize the varibles to the data in X, Y, and Z.
   x = X;
   y = Y;
   z = Z;
}


Vector::Vector(const Vector &v)
{
   // Initialize this object to v.
   x = v.x;
   y = v.y;
   z = v.z;
}


void Vector::operator =(Vector v)
{
   // Make this objects x, y, and z equal to the object on the right of the = sign.
   x = v.x;
   y = v.y;
   z = v.z;
}


Vector Vector::operator -(Vector v)
{
   // Return the value of this vector - v.
   return Vector(x - v.x, y - v.y, z - v.z);
}


Vector Vector::operator +(Vector v)
{
   // Return the value of this vector + v.
   return Vector(x + v.x, y + v.y, z + v.z);
}


Vector Vector::operator *(Vector v)
{
   // Return the value of this vector * v.
   return Vector(x * v.x, y * v.y, z * v.z);
}


Vector Vector::operator /(Vector v)
{
   // Return the value of this vector / v.
   return Vector(x / v.x, y / v.y, z / v.z);
}


Vector Vector::operator +(float f)
{
   // Return the value of this vector + f.
   return Vector(x + f, y + f, z + f);
}


Vector Vector::operator -(float f)
{
   // Return the value of this vector - f.
   return Vector(x - f, y - f, z - f);
}


Vector Vector::operator *(float f)
{
   // Return the value of this vector * f.
   return Vector(x * f, y * f, z * f);
}


Vector Vector::operator /(float f)
{
   // Return the value of this vector / f.  We do this by multiplying the recip.
   f = 1/f;

   return Vector(x * f, y * f, z * f);
}


void Vector::operator +=(Vector v)
{
   // Add this by object v and store results here.
   x += v.x;
   y += v.y;
   z += v.z;
}


void Vector::operator -=(Vector v)
{
   // Subtract this by object v and store results here.
   x -= v.x;
   y -= v.y;
   z -= v.z;
}


void Vector::operator *=(Vector v)
{
   // Mul this by object v and store results here.
   x *= v.x;
   y *= v.y;
   z *= v.z;
}


void Vector::operator /=(Vector v)
{
   // Divide this by object v and store results here.
   x /= v.x;
   y /= v.y;
   z /= v.z;
}


void Vector::operator +=(float f)
{
   // Add this by object f and store results here.
   x += f;
   y += f;
   z += f;
}


void Vector::operator -=(float f)
{
   // Subract this by object f and store results here.
   x -= f;
   y -= f;
   z -= f;
}


void Vector::operator *=(float f)
{
   // Multiply this by object f and store results here.
   x *= f;
   y *= f;
   z *= f;
}


void Vector::operator /=(float f)
{
   // Divide this by object f and store results here by multiplying by the recip.
   f = 1/f;

   x *= f;
   y *= f;
   z *= f;
}


bool Vector::operator ==(Vector v)
{
   // Return true if all equal each other, false if one or more don't.
   return ((x == v.x) && (y== v.y) && (z == v.z));
}


bool Vector::operator !=(Vector v)
{
   // Return true if one or all don't equal each other, false if they equal.
   return !((x == v.x) && (y== v.y) && (z == v.z));
}


void Vector::CrossProduct(Vector v1, Vector v2)
{
   // Get the cross product of v1 and v2 and store it in this vector.
   x = ((v1.y * v2.z) - (v1.z * v2.y));
   y = ((v1.z * v2.x) - (v1.x * v2.z));
   z = ((v1.x * v2.y) - (v1.y * v2.x));
}

float Vector::DotProduct3(Vector v1)
{
   // Get the dot product of v1 and this object and return it.
   return x * v1.x + y * v1.y + z * v1.z;
}

float Vector::GetLength()
{
   // Return the length for this object.
   return (float)sqrt((x * x + y * y + z * z));
}

float Vector::GetLengthSquared()
{
   // Return the length for this object (without taking its square root)
   return (x * x + y * y + z * z);
}

void Vector::SetLength(float newlength) {
 
    //first normalize
    Normal();
    //then multiply each component by length
    x = x*newlength;
    y = y*newlength;
    z = z*newlength;
}


void Vector::Normal()
{
   // Reduce this object to a unit vector.
   float length = GetLength();

   if(length == 0.0f)
      length = 1.0f;

   x = x/length;
   y = y/length;
   z = z/length;
}

Vector Vector::getNormal() {

	// Reduce this object to a unit vector.
   float length = GetLength();

   if(length == 0.0f)
      length = 1.0f;

   return Vector(x/length,y/length,z/length);
   
}


void Vector::Normalize(Vector Triangle[])
{
   // Normalize a triangle and store results in this object.
   Vector v1, v2;

   v1.x = Triangle[0].x - Triangle[1].x;
   v1.y = Triangle[0].y - Triangle[1].y;
   v1.z = Triangle[0].z - Triangle[1].z;

   v2.x = Triangle[1].x - Triangle[2].x;
   v2.y = Triangle[1].y - Triangle[2].y;
   v2.z = Triangle[1].z - Triangle[2].z;

   CrossProduct(v1, v2);
   Normal();
}

Vector Vector::GetRotatedX(double angle)
{
   float sinAngle = (float)sin(M_PI * angle / 180);
	float cosAngle = (float)cos(M_PI * angle / 180);

	return Vector(x, y * cosAngle - z * sinAngle, y * sinAngle + z * cosAngle);
}


Vector Vector::GetRotatedY(double angle)
{
   float sinAngle = (float)sin(M_PI * angle / 180);
	float cosAngle = (float)cos(M_PI * angle / 180);

	return Vector(x * cosAngle + z * sinAngle, y, -x * sinAngle + z * cosAngle);
}

Vector Vector::GetRotatedYRadians(double radians) {
	float sinRadians=(float)sin(radians);
	float cosRadians=(float)cos(radians);

	return Vector(x * cosRadians + z * sinRadians, y, -x * sinRadians + z * cosRadians);
}

Vector Vector::GetRotatedZ(double angle)
{
   float sinAngle = (float)sin(M_PI * angle / 180);
	float cosAngle = (float)cos(M_PI * angle / 180);

	return Vector(x * cosAngle - y * sinAngle, x * sinAngle + y * cosAngle, z);

}

Vector Vector::GetRotatedZRadians(double radians) {
	float sinRadians=(float)sin(radians);
	float cosRadians=(float)cos(radians);

	return Vector(x * cosRadians - y * sinRadians, x * sinRadians + y * cosRadians, z);
}


Vector Vector::GetRotatedAxis(double angle, Vector axis)
{
	if(angle == 0.0) return(*this);

   axis.Normal();

	Vector RotationRow1, RotationRow2, RotationRow3;

   double newAngle = M_PI * angle / 180;
	float sinAngle = (float)sin(newAngle);
	float cosAngle = (float)cos(newAngle);
	float oneSubCos = 1.0f - cosAngle;

	RotationRow1.x = (axis.x) * (axis.x) + cosAngle * (1 - (axis.x) * (axis.x));
	RotationRow1.y = (axis.x) * (axis.y) * (oneSubCos) - sinAngle * axis.z;
	RotationRow1.z = (axis.x) * (axis.z) * (oneSubCos) + sinAngle * axis.y;

	RotationRow2.x = (axis.x) * (axis.y) * (oneSubCos) + sinAngle * axis.z;
	RotationRow2.y = (axis.y) * (axis.y) + cosAngle * (1 - (axis.y) * (axis.y));
	RotationRow2.z = (axis.y) * (axis.z) * (oneSubCos) - sinAngle * axis.x;
	
	RotationRow3.x = (axis.x) * (axis.z) * (oneSubCos) - sinAngle * axis.y;
	RotationRow3.y = (axis.y) * (axis.z) * (oneSubCos) + sinAngle * axis.x;
	RotationRow3.z = (axis.z) * (axis.z) + cosAngle * (1 - (axis.z) * (axis.z));

	return Vector(this->DotProduct3(RotationRow1),
                   this->DotProduct3(RotationRow2),
                   this->DotProduct3(RotationRow3));
}


void Vector::CalculateBinormalVector(Vector tangent, Vector normal)
{
   this->CrossProduct(tangent, normal);
}

// Inputs: plane origin, plane normal, ray origin ray vector.
// NOTE: both vectors are assumed to be normalized

double Vector::intersect(Vector pOrigin, Vector pNormal, Vector rOrigin, Vector rVector)
{
    double d = -(pNormal.DotProduct3(pOrigin));
    double numer = pNormal.DotProduct3(rOrigin) + d;
    double denom = pNormal.DotProduct3(rVector);
    return -(numer / denom);
}

double Vector::intersectEllipse(Vector rO, Vector rV, Vector sO, Vector sR)
{
    Vector Q = sO - rO;
    double c = Q.GetLength();
    double v = Q.DotProduct3(rV);
    double d = sR.DotProduct3(sR)-(c*c-v*v);

   // If there was no intersection, return -1

   if (d < 0.0) return -1.0;

   // Return the distance to the [first] intersecting point

   return v - sqrt(d);
} 

Vector Vector::closestPointOnTriangle(Vector a, Vector b, Vector c, Vector p)
{
    
    Vector  Rab = closestPointOnLine(a, b, p);
    Vector  Rbc = closestPointOnLine(b, c, p);
    Vector  Rca = closestPointOnLine(c, a, p);
   
    double d_ap, d_bp, d_cp;
   
    d_ap=(Rab-p).GetLength();
    d_bp=(Rbc-p).GetLength();
    d_cp=(Rca-p).GetLength();
   
    if (d_ap<d_bp&&d_ap<d_cp)
        return Rab;
    else if (d_bp<d_ap&&d_bp<d_cp)
        return Rbc;
    else
        return Rca;       
}

 

Vector Vector::closestPointOnLine(Vector a, Vector b, Vector p)
{
    // Determine t (the length of the vector from ‘a’ to ‘p’)

    Vector c = p - a;
    Vector V = b - a;
    V.Normal();
    float d = (a-b).GetLength();
    float t = V.DotProduct3(c);

    // Check to see if ‘t’ is beyond the extents of the line segment

    if (t < 0) return a;
    if (t > d) return b;
 
    // Return the point between ‘a’ and ‘b’

    V.SetLength(t);
    return a + V;
    
}


Vector Vector::StretchDimensions(float x1, float y1, float z1) {
 
    return StretchDimensions(Vector(x1,y1,z1));
    
}

Vector Vector::StretchDimensions(Vector stretchfactors) {
 
    x=x*stretchfactors.x;
    y=y*stretchfactors.y;
    z=z*stretchfactors.z;
    
    return *this;
    
}

float Vector::getComponent(int comp) {
 
    if (comp==0) return x;
    else if (comp==1) return y;
    else return z;
    
}

void Vector::setComponent(int comp, float val) {

    if (comp==0) x=val;
    else if (comp==1) y=val;
    else z=val;
    
}

Vector Vector::interpolateVector(float interp, Vector v) {
    
    float vx;
    float vy;
    float vz;
    
    vx = (float)(1.0-interp)*x + interp*v.x;
    vy = (float)(1.0-interp)*y + interp*v.y;
    vz = (float)(1.0-interp)*z + interp*v.z;
    
    return Vector(vx,vy,vz);
    
}

void Vector::print() {
    print("");
}

void Vector::print(char *prefix) {
    
    printf("%s (%3.2f,%3.2f,%3.2f)\n",prefix,x,y,z);
    
}
