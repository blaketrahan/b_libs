#ifndef B_VEC_234_H
#define B_VEC_234_H

#include <iostream>
#include <iomanip>
#include "math.h"

#ifndef M_DEFINE_ANGLES
#define M_DEFINE_ANGLES

#define M_PI32 3.14159265359f
#define M_DEGTORAD32 M_PI32 / 180.0f
#define M_RADTODEG32 180.0f / M_PI32
#define M_PI64 3.1415926535897932384626433832795028841971693993751
#define M_DEGTORAD64 M_PI64 / 180.0
#define M_RADTODEG64 180.0 / M_PI64

f4 identity[] =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

f4 RMAT[4][16] = 
{
    { // 0 degrees (identity)
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    },
    { // 90 degrees
        (f4)cos( M_PI32/2.0f ), (f4)-sin( M_PI32/2.0f ), 0.0f, 0.0f,
        (f4)sin( M_PI32/2.0f ), (f4) cos( M_PI32/2.0f ), 0.0f, 0.0f, // 
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    },
    { // 180 degrees
        (f4)cos( M_PI32 ), (f4)-sin( M_PI32 ), 0.0f, 0.0f,
        (f4)sin( M_PI32 ), (f4) cos( M_PI32 ), 0.0f, 0.0f, // top
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    },
    { // 270 degrees
        (f4)cos( 3.0f*M_PI32/2.0f ), (f4)-sin( 3.0f*M_PI32/2.0f ), 0.0f, 0.0f,
        (f4)sin( 3.0f*M_PI32/2.0f ), (f4) cos( 3.0f*M_PI32/2.0f ), 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    }
};
#endif

typedef union vec2
{
    struct { f4 x,y; };
    f4 p[2];

    vec2(f4 mx = 0.0f, f4 my = 0.0f)
        : x(mx),y(my) {}

    inline vec2& operator=(const vec2 v)
    {
        p[0] = v[0];
        p[1] = v[1];
        return *this;
    }
    inline f4 operator[](u4 index) const
    {
        return p[index];
    }
    inline f4& operator[](u4 index)
    {
        return p[index];
    }
    inline vec2& operator-=(const vec2 v)
    {
        x -= v.x;
        y -= v.y; 
        return *this;
    }
    inline vec2& operator+=(const vec2 v)
    {
        x += v.x;
        y += v.y; 
        return *this;
    }
} vec2;

typedef union vec3
{
    struct { f4 x,y,z; };
    struct { f4 r,g,b; };
    f4 p[3];

    vec3(f4 mx = 0.0f, f4 my = 0.0f, f4 mz = 0.0f)
        : x(mx),y(my), z(mz) {}

    inline vec3& operator=(const vec3 v)
    {
        p[0] = v[0];
        p[1] = v[1];
        p[2] = v[2];
        return *this;
    }
    inline f4 operator[](u4 index) const
    {
        return p[index];
    }
    inline f4& operator[](u4 index)
    {
        return p[index];
    }
    inline vec3& operator-=(const vec3 v)
    {
        x -= v.x;
        y -= v.y; 
        z -= v.z; 
        return *this;
    }
    inline vec3& operator+=(const vec3 v)
    {
        x += v.x;
        y += v.y; 
        z += v.z; 
        return *this;
    }
} vec3;

typedef union vec4
{
    struct { f4 x,y,z,w; };
    struct { f4 r,g,b,a; };
    f4 p[4];

    vec4(f4 mx = 0.0f, f4 my = 0.0f, f4 mz = 0.0f, f4 mw = 0.0f)
        : x(mx),y(my), z(mz), w(mw) {}

    inline vec4& operator=(const vec4 v)
    {
        p[0] = v[0];
        p[1] = v[1];
        p[2] = v[2];
        p[3] = v[3];
        return *this;
    }
    inline f4 operator[](u4 index) const
    {
        return p[index];
    }
    inline f4& operator[](u4 index)
    {
        return p[index];
    }
    inline vec4& operator-=(const vec4 v)
    {
        x -= v.x;
        y -= v.y; 
        z -= v.z; 
        w -= v.w; 
        return *this;
    }
    inline vec4& operator+=(const vec4 v)
    {
        x += v.x;
        y += v.y; 
        z += v.z; 
        w += v.w; 
        return *this;
    }
} vec4;

inline vec4 operator-(vec4 A, vec4 B);
inline vec4 operator+(vec4 A, vec4 B);
inline vec3 operator-(vec3 A, vec3 B);
inline vec3 operator+(vec3 A, vec3 B);
inline vec2 operator-(vec2 A, vec2 B);
inline vec2 operator+(vec2 A, vec2 B);
inline b4 operator!=(vec2 A, vec2 B);
inline b4 operator!=(vec3 A, vec3 B);
inline b4 operator!=(vec4 A, vec4 B);
inline void print(vec2 v);
inline void print(vec3 v);
inline void print(vec4 v);

inline void print(vec2 v)
{
    std::cout << "(" << v.x << ", " << v.y << ")" << std::endl;
}
inline void print(vec3 v)
{
    std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
}
inline void print(vec4 v)
{
    std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.z << ")" << std::endl;
}

inline b4 operator!=(vec2 A, vec2 B)
{
    if (A.x != B.x) { return true; }
    if (A.y != B.y) { return true; }
    return false;
}
inline b4 operator!=(vec3 A, vec3 B)
{
    if (A.x != B.x) { return true; }
    if (A.y != B.y) { return true; }
    if (A.z != B.z) { return true; }
    return false;
}
inline b4 operator!=(vec4 A, vec4 B)
{
    if (A.x != B.x) { return true; }
    if (A.y != B.y) { return true; }
    if (A.z != B.z) { return true; }
    if (A.w != B.w) { return true; }
    return false;
}

/*
    subtract, add
*/
inline vec2 b_vec2subtract(vec2 A, vec2 B) {
    vec2 result;
    result.x = A.x - B.x;
    result.y = A.y - B.y;
    return result;
}
inline vec2 b_vec2add(vec2 A, vec2 B) {
    vec2 result;
    result.x = A.x + B.x;
    result.y = A.y + B.y;
    return result;
}
inline vec3 b_vec3subtract(vec3 A, vec3 B) {
    vec3 result;
    result.x = A.x - B.x;
    result.y = A.y - B.y;
    result.z = A.z - B.z;
    return result;
}
inline vec3 b_vec3add(vec3 A, vec3 B) {
    vec3 result;
    result.x = A.x + B.x;
    result.y = A.y + B.y;
    result.z = A.z + B.z;
    return result;
}
inline vec4 b_vec4subtract(vec4 A, vec4 B) {
    vec4 result;
    result.x = A.x - B.x;
    result.y = A.y - B.y;
    result.z = A.z - B.z;
    result.w = A.w - B.w;
    return result;
}
inline vec4 b_vec4add(vec4 A, vec4 B) {
    vec4 result;
    result.x = A.x + B.x;
    result.y = A.y + B.y;
    result.z = A.z + B.z;
    result.w = A.w + B.w;
    return result;
}
inline vec2 operator-(vec2 A, vec2 B) {    
    return b_vec2subtract(A,B);
}
inline vec2 operator+(vec2 A, vec2 B) {
    return b_vec2add(A,B);   
}
inline vec3 operator-(vec3 A, vec3 B) {    
    return b_vec3subtract(A,B);
}
inline vec3 operator+(vec3 A, vec3 B) {
    return b_vec3add(A,B);   
}
inline vec4 operator-(vec4 A, vec4 B) {    
    return b_vec4subtract(A,B);
}
inline vec4 operator+(vec4 A, vec4 B) {
    return b_vec4add(A,B);   
}

#endif