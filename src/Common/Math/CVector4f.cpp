#include "CVector4f.h"
#include "CVector2f.h"
#include "CVector3f.h"
#include "CTransform4f.h"

CVector4f::CVector4f()
{
    x = y = z = w = 0.f;
}

CVector4f::CVector4f(float xyzw)
{
    x = y = z = w = xyzw;
}

CVector4f::CVector4f(float _x, float _y, float _z, float _w)
{
    x = _x;
    y = _y;
    z = _z;
    w = _w;
}

CVector4f::CVector4f(const CVector2f& xy, float _z, float _w)
{
    x = xy.x;
    y = xy.y;
    z = _z;
    w = _w;
}

CVector4f::CVector4f(const CVector3f& xyz)
{
    x = xyz.x;
    y = xyz.y;
    z = xyz.z;
    w = 1.f;
}

CVector4f::CVector4f(const CVector3f& xyz, float _w)
{
    x = xyz.x;
    y = xyz.y;
    z = xyz.z;
    w = _w;
}

CVector4f::CVector4f(CInputStream& Input)
{
    x = Input.ReadFloat();
    y = Input.ReadFloat();
    z = Input.ReadFloat();
    w = Input.ReadFloat();
}

void CVector4f::Write(COutputStream &Output)
{
    Output.WriteFloat(x);
    Output.WriteFloat(y);
    Output.WriteFloat(z);
    Output.WriteFloat(w);
}

// ************ SWIZZLE ************
CVector3f CVector4f::xyz()
{
    return CVector3f(x, y, z);
}

CVector3f CVector4f::xzw()
{
    return CVector3f(x, z, w);
}

CVector3f CVector4f::yzw()
{
    return CVector3f(y, z, w);
}

CVector2f CVector4f::xy()
{
    return CVector2f(x, y);
}

CVector2f CVector4f::xz()
{
    return CVector2f(x, z);
}

CVector2f CVector4f::xw()
{
    return CVector2f(x, w);
}

CVector2f CVector4f::yz()
{
    return CVector2f(y, z);
}

CVector2f CVector4f::yw()
{
    return CVector2f(y, w);
}

CVector2f CVector4f::zw()
{
    return CVector2f(z, w);
}

// ************ MATH ************
// ************ VECTOR/VECTOR ************
CVector4f CVector4f::operator+(const CVector4f& src) const
{
    CVector4f out;
    out.x = this->x + src.x;
    out.y = this->y + src.y;
    out.z = this->z + src.z;
    out.w = this->w + src.w;
    return out;
}

CVector4f CVector4f::operator-(const CVector4f& src) const
{
    CVector4f out;
    out.x = this->x - src.x;
    out.y = this->y - src.y;
    out.z = this->z - src.z;
    out.w = this->w - src.w;
    return out;
}

CVector4f CVector4f::operator*(const CVector4f& src) const
{
    CVector4f out;
    out.x = this->x * src.x;
    out.y = this->y * src.y;
    out.z = this->z * src.z;
    out.w = this->w * src.w;
    return out;
}

CVector4f CVector4f::operator/(const CVector4f& src) const
{
    CVector4f out;
    out.x = this->x / src.x;
    out.y = this->y / src.y;
    out.z = this->z / src.z;
    out.w = this->w / src.w;
    return out;
}

void CVector4f::operator+=(const CVector4f& other)
{
    *this = *this + other;
}

void CVector4f::operator-=(const CVector4f& other)
{
    *this = *this - other;
}

void CVector4f::operator*=(const CVector4f& other)
{
    *this = *this * other;
}

void CVector4f::operator/=(const CVector4f& other)
{
    *this = *this / other;
}

bool CVector4f::operator==(const CVector4f& other) const
{
    return ((x == other.x) && (y == other.y) && (z == other.z) && (w == other.w));
}

// ************ VECTOR/FLOAT ************
CVector4f CVector4f::operator+(const float src) const
{
    CVector4f out;
    out.x = this->x + src;
    out.y = this->y + src;
    out.z = this->z + src;
    out.w = this->w + src;
    return out;
}

CVector4f CVector4f::operator-(const float src) const
{
    CVector4f out;
    out.x = this->x - src;
    out.y = this->y - src;
    out.z = this->z - src;
    out.w = this->w - src;
    return out;
}

CVector4f CVector4f::operator*(const float src) const
{
    CVector4f out;
    out.x = this->x * src;
    out.y = this->y * src;
    out.z = this->z * src;
    out.w = this->w * src;
    return out;
}

CVector4f CVector4f::operator/(const float src) const
{
    CVector4f out;
    out.x = this->x / src;
    out.y = this->y / src;
    out.z = this->z / src;
    out.w = this->w / src;
    return out;
}

void CVector4f::operator+=(const float other)
{
    *this = *this + other;
}

void CVector4f::operator-=(const float other)
{
    *this = *this - other;
}

void CVector4f::operator*=(const float other)
{
    *this = *this * other;
}

void CVector4f::operator/=(const float other)
{
    *this = *this / other;
}

// ************ VECTOR/MATRIX ************
CVector4f CVector4f::operator*(const CTransform4f& mtx) const
{
    CVector4f out;
    out.x = (x * mtx[0][0]) + (y * mtx[1][0]) + (z * mtx[2][0]);
    out.y = (x * mtx[0][1]) + (y * mtx[1][1]) + (z * mtx[2][1]);
    out.z = (x * mtx[0][2]) + (y * mtx[1][2]) + (z * mtx[2][2]);
    out.w = (x * mtx[0][3]) + (y * mtx[1][3]) + (z * mtx[2][3]) + w;
    return out;
}

void CVector4f::operator*=(const CTransform4f& mtx)
{
    *this = *this * mtx;
}

CVector4f CVector4f::operator*(const CMatrix4f& mtx) const
{
    CVector4f out;
    out.x = (x * mtx[0][0]) + (y * mtx[1][0]) + (z * mtx[2][0]) + (w * mtx[3][0]);
    out.y = (x * mtx[0][1]) + (y * mtx[1][1]) + (z * mtx[2][1]) + (w * mtx[3][1]);
    out.z = (x * mtx[0][2]) + (y * mtx[1][2]) + (z * mtx[2][2]) + (w * mtx[3][2]);
    out.w = (x * mtx[0][3]) + (y * mtx[1][3]) + (z * mtx[2][3]) + (w * mtx[3][3]);
    return out;
}

void CVector4f::operator*=(const CMatrix4f& mtx)
{
    *this = *this * mtx;
}

// ************ UNARY ************
float& CVector4f::operator[](long index)
{
    return (&x)[index];
}
