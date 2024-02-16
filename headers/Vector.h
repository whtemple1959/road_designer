

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h> 
#include <stdio.h>


#ifndef M_PI
#define M_PI            3.14159265358979323846  /* pi */
#endif

#ifndef M_PI_2
#define M_PI_2          1.57079632679489661923  /* pi/2 */
#endif

#ifndef M_PI_4
#define M_PI_4          0.78539816339744830962  /* pi/4 */
#endif

#ifndef M_PI_6
#define M_PI_6          0.52359877559829887307  /* pi/6 */
#endif

#ifndef M_1_PI
#define M_1_PI          0.31830988618379067154  /* 1/pi */
#endif

#define ONE_BY_ROOT2	0.70710678118654

#define ROOT_3_OVER_2	0.86602540378444


class Vector2
{
public:
	// Construction/Destruction
	Vector2()								{};
        Vector2(float _c)						{ x=y=_c; }
        Vector2(float _x, float _y)			{ x=_x; y=_y; }
	Vector2(const Vector2& _rhs)			{ x=_rhs.x; y=_rhs.y; }
//#if defined(_WIN32)
//	explicit Vector2(const CPoint& _rhs)	{ x=static_cast<float>(_rhs.x); y=static_cast<float>(_rhs.y); }
//#endif
	~Vector2() {};

        float Magnitude() { return sqrt(x*x + y*y); }

	Vector2& operator= (const Vector2& _rhs) { x=_rhs.x; y=_rhs.y; return *this; }
	
	inline Vector2 operator-() const					{ Vector2 _v; _v.x=-x; _v.y=-y; return _v; }
	inline Vector2 operator-(const Vector2& _rhs) const { Vector2 _v; _v.x=x-_rhs.x; _v.y=y-_rhs.y; return _v;}
	inline Vector2 operator+(const Vector2& _rhs) const { return Vector2( x+_rhs.x, y+_rhs.y );	}
        inline Vector2 operator*(const float _rhs) const	{ Vector2 _v; _v.x=x*_rhs; _v.y=y*_rhs; return _v;}
        inline Vector2 operator/(const float _rhs) const	{ Vector2 _v; _v.x=x/_rhs; _v.y=y/_rhs; return _v;}


        Vector2& operator*= (float _c)				{ x*=_c; y*=_c; return *this; }
	Vector2& operator+= (const Vector2& _rhs)	{ x+=_rhs.x; y+=_rhs.y; return *this; }
	Vector2& operator-= (const Vector2& _rhs)	{ x-=_rhs.x; y-=_rhs.y; return *this; }

        inline float Length() const		{ return sqrt(pow(x,2) + pow(y, 2)); }
        inline const Vector2& Normalize()	{ float len = Length(); x = x/len; y = y/len; return *this;}

public:
        float x,y;
};

class Vector
{
   public:
      Vector();                                        // Constructor.
      Vector(float x, float y, float z);      
      Vector(const Vector &v);                       // Another overloaded constructor.

      void operator=(Vector v);                        // Overloaded = sign.
      Vector operator-(const Vector v) const;                    // Overloaded - sign.
      Vector operator+(const Vector v) const;                    // Overloaded + sign.
      Vector operator*(const Vector v) const;                    // Overloaded * sign.
      Vector operator/(const Vector v) const;                    // Overloaded / sign.
      Vector operator+(float f);                       // Overloaded + sign.
      Vector operator-(float f);                       // Overloaded - sign.
      Vector operator*(float f);                       // Overloaded * sign.
      Vector operator/(float f);                       // Overloaded / sign.

      void operator +=(Vector v);                      // Overloaded += sign.
      void operator -=(Vector v);                      // Overloaded -= sign.
      void operator *=(Vector v);                      // Overloaded *= sign.
      void operator /=(Vector v);                      // Overloaded /= sign.
      void operator +=(float f);                         // Overloaded += sign.
      void operator -=(float f);                         // Overloaded -= sign.
      void operator *=(float f);                         // Overloaded *= sign.
      void operator /=(float f);                         // Overloaded /= sign.

      bool operator ==(Vector v);                      // Overloaded == sign.
      bool operator !=(Vector v);                      // Overloaded != sign.

      void CrossProduct(Vector v1, Vector v2);       // Stores cross product of v1, v2.      	  
      float DotProduct3(Vector v1);                    // Dot 3 on v1 and this.
      float GetLength() const;                                 // Return this objects length.
      float GetLengthSquared();
      void SetLength(float newlength);
      void Normal();                                     // Normalize this vector.
	  Vector getNormal();
      void Normalize(Vector Triangle[]);               // Find the normal of a triangle.
      
      Vector GetRotatedX(float angle);                // Rotate along x.
      Vector GetRotatedY(float angle);                // Rotate alone y.
          Vector GetRotatedYRadians(float radians);
      Vector GetRotatedZ(float angle);                // Rotate along z.
          Vector GetRotatedZRadians(float radians);
      Vector GetRotatedAxis(float angle, Vector axis);// Rotate along an axis.
      void CalculateBinormalVector(Vector tangent,
                                   Vector normal);     // Calcluate a binormal.
      float intersect(Vector pOrigin, Vector pNormal, Vector rOrigin, Vector rVector);
      float intersectEllipse(Vector rO, Vector rV, Vector sO, Vector sR);
      Vector closestPointOnTriangle(Vector a, Vector b, Vector c, Vector p);
      Vector closestPointOnLine(Vector a, Vector b, Vector p);
      Vector StretchDimensions(Vector stretchfactors);
      Vector StretchDimensions(float x1, float y1, float z1);
      
      float getComponent(int comp);
      void setComponent(int comp, float val);

      Vector interpolateVector(float interp, Vector v);
      
      void print();
      void print(char *prefix);

      float x, y, z;                                  // vertex's x, y, and z info.
};

#endif
