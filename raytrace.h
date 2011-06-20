#ifndef __RAYTRACE_H
#define __RAYTRACE_H
#include <cmath>
#include "SimpleString.h"

/* Piover180 is simply a conversion factor for converting between degrees and radians. */
const double PIOVER180 = 0.017453292519943295769236907684886;


struct point {
	float x, y, z;
};


struct vector2 {
	float x, y, z;

    vector2& operator += (const vector2 &v2){
	    this->x += v2.x;
        this->y += v2.y;
        this->z += v2.z;
	    return *this;
    }
	
	vector2& operator = (const vector2 &v2){
	    this->x = v2.x;
        this->y = v2.y;
        this->z = v2.z;
	    return *this;
    }
};




inline point operator + (const point&p, const vector2 &v){
	point p2={p.x + v.x, p.y + v.y, p.z + v.z };
	return p2;
}


inline point operator - (const point&p, const vector2 &v){
	point p2={p.x - v.x, p.y - v.y, p.z - v.z };
	return p2;
}


inline vector2 operator + (const vector2&v1, const vector2 &v2){
	vector2 v={v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
	return v;
}


inline vector2 operator - (const point&p1, const point &p2){
	vector2 v={p1.x - p2.x, p1.y - p2.y, p1.z - p2.z };
	return v;
}


inline vector2 operator * (float c, const vector2 &v)
{
	vector2 v2={v.x *c, v.y * c, v.z * c };
	return v2;
}


inline vector2 operator - (const vector2&v1, const vector2 &v2){
	vector2 v={v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	return v;
}


inline float operator * (const vector2&v1, const vector2 &v2 ) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}


struct color {
    enum OFFSET 
    {
        OFFSET_RED = 0,
        OFFSET_GREEN = 1,
        OFFSET_BLUE = 2,
        OFFSET_MAX  = 3
    };
    float red, green, blue;

    inline color & operator += (const color &c2 ) {
	    this->red +=  c2.red;
        this->green += c2.green;
        this->blue += c2.blue;
	    return *this;
    }

    inline float & getChannel(OFFSET offset )
    {
        return reinterpret_cast<float*>(this)[offset];
    }

    inline float getChannel(OFFSET offset ) const
    {
        return reinterpret_cast<const float*>(this)[offset];
    }
};


inline color operator * (const color&c1, const color &c2 ) {
	color c = {c1.red * c2.red, c1.green * c2.green, c1.blue * c2.blue};
	return c;
}


inline color operator + (const color&c1, const color &c2 ) {
	color c = {c1.red + c2.red, c1.green + c2.green, c1.blue + c2.blue};
	return c;
}


inline color operator * (float coef, const color &c ) {
	color c2 = {c.red * coef, c.green * coef, c.blue * coef};
	return c2;
}


struct material {
	float reflection;
	float refraction;
	float density;
	float power;
	color diffuse;
	color specular;
};


struct sphere {
	point pos;
	float size;
	int materialId;
};


struct light {
	point pos;
	color intensity;
};


struct ray {
	point start;
	vector2 dir;
};

inline float normalize(ray viewRay){
	point start = viewRay.start;
	point end =viewRay.start + viewRay.dir;
	float n= (end.x-start.x)*(end.x-start.x)+(end.y-start.y)*(end.y-start.y)+(end.z-start.z)*(end.z-start.z);
	
	return sqrtf(n);
}

struct cubemap
{
	enum {
		up = 0,
		down = 1,
		right = 2,
		left = 3,
		forward = 4,
		backward = 5
	};
    SimpleString name[6];
    int sizeX, sizeY;
	color *texture; 
    float exposure;
    bool bExposed;
    bool bsRGB;
    cubemap() : sizeX(0), sizeY(0), texture(0), exposure(1.0f), bExposed(false), bsRGB(false) {};
    bool Init();
    void setExposure(float newExposure) {exposure = newExposure; }
    ~cubemap() { if (texture) delete []texture; }
};

#define invsqrtf(x) (1.0f / sqrtf(x))

#endif // __RAYTRACE_H
