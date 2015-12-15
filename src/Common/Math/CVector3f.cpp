#include "CVector3f.h"
#include "CVector2f.h"
#include "CVector4f.h"
#include "CTransform4f.h"
#include <iomanip>
#include <float.h>

CVector3f::CVector3f()
{
    x = y = z = 0.f;
}

CVector3f::CVector3f(float xyz)
{
    x = y = z = xyz;
}

CVector3f::CVector3f(float _x, float _y, float _z)
{
    x = _x;
    y = _y;
    z = _z;
}

CVector3f::CVector3f(CInputStream& Input)
{
    x = Input.ReadFloat();
    y = Input.ReadFloat();
    z = Input.ReadFloat();
}

void CVector3f::Write(COutputStream &Output)
{
    Output.WriteFloat(x);
    Output.WriteFloat(y);
    Output.WriteFloat(z);
}

// ************ SWIZZLE ************
CVector2f CVector3f::xy()
{
    return CVector2f(x, y);
}

CVector2f CVector3f::xz()
{
    return CVector2f(x, z);
}

CVector2f CVector3f::yz()
{
    return CVector2f(y, z);
}

// ************ MATH ************
float CVector3f::Magnitude() const
{
    return sqrtf(SquaredMagnitude());
}

float CVector3f::SquaredMagnitude() const
{
    return Dot(*this);
}

CVector3f CVector3f::Normalized() const
{
    return *this / Magnitude();
}

float CVector3f::Dot(const CVector3f& other) const
{
    return (x * other.x) + (y * other.y) + (z * other.z);
}

CVector3f CVector3f::Cross(const CVector3f& other) const
{
    return CVector3f((y * other.z) - (z * other.y), (z * other.x) - (x * other.z), (x * other.y) - (y * other.x));
}

float CVector3f::Distance(const CVector3f& point) const
{
    return (*this - point).Magnitude();
}

float CVector3f::SquaredDistance(const CVector3f& point) const
{
    return (*this - point).SquaredMagnitude();
}

// ************ OPERATORS ************
// VECTOR/VECTOR
CVector3f CVector3f::operator+(const CVector3f& src) const
{
    CVector3f out;
    out.x = this->x + src.x;
    out.y = this->y + src.y;
    out.z = this->z + src.z;
    return out;
}

CVector3f CVector3f::operator-(const CVector3f& src) const
{
    CVector3f out;
    out.x = this->x - src.x;
    out.y = this->y - src.y;
    out.z = this->z - src.z;
    return out;
}

CVector3f CVector3f::operator*(const CVector3f& src) const
{
    CVector3f out;
    out.x = this->x * src.x;
    out.y = this->y * src.y;
    out.z = this->z * src.z;
    return out;
}

CVector3f CVector3f::operator/(const CVector3f& src) const
{
    CVector3f out;
    out.x = this->x / src.x;
    out.y = this->y / src.y;
    out.z = this->z / src.z;
    return out;
}

void CVector3f::operator+=(const CVector3f& other)
{
    *this = *this + other;
}

void CVector3f::operator-=(const CVector3f& other)
{
    *this = *this - other;
}

void CVector3f::operator*=(const CVector3f& other)
{
    *this = *this * other;
}

void CVector3f::operator/=(const CVector3f& other)
{
    *this = *this / other;
}

bool CVector3f::operator> (const CVector3f& other) const
{
    return ((x > other.x) && (y > other.y) && (z > other.z));
}

bool CVector3f::operator>=(const CVector3f& other) const
{
    return ((x >= other.x) && (y >= other.y) && (z >= other.z));
}

bool CVector3f::operator< (const CVector3f& other) const
{
    return ((x < other.x) && (y < other.y) && (z < other.z));
}

bool CVector3f::operator<=(const CVector3f& other) const
{
    return ((x <= other.x) && (y <= other.y) && (z <= other.z));
}

bool CVector3f::operator==(const CVector3f& other) const
{
    return ((x == other.x) && (y == other.y) && (z == other.z));
}

bool CVector3f::operator!=(const CVector3f& other) const
{
    return (!(*this == other));
}

// VECTOR/FLOAT
CVector3f CVector3f::operator+(const float src) const
{
    CVector3f out;
    out.x = this->x + src;
    out.y = this->y + src;
    out.z = this->z + src;
    return out;
}

CVector3f CVector3f::operator-(const float src) const
{
    CVector3f out;
    out.x = this->x - src;
    out.y = this->y - src;
    out.z = this->z - src;
    return out;
}

CVector3f CVector3f::operator*(const float src) const
{
    CVector3f out;
    out.x = this->x * src;
    out.y = this->y * src;
    out.z = this->z * src;
    return out;
}

CVector3f CVector3f::operator/(const float src) const
{
    CVector3f out;
    out.x = this->x / src;
    out.y = this->y / src;
    out.z = this->z / src;
    return out;
}

void CVector3f::operator+=(const float other)
{
    *this = *this + other;
}

void CVector3f::operator-=(const float other)
{
    *this = *this - other;
}

void CVector3f::operator*=(const float other)
{
    *this = *this * other;
}

void CVector3f::operator/=(const float other)
{
    *this = *this / other;
}

// VECTOR/MATRIX
CVector3f CVector3f::operator*(const CTransform4f& mtx) const
{
    CVector3f out;
    out.x = (x * mtx[0][0]) + (y * mtx[1][0]) + (z * mtx[2][0]);
    out.y = (x * mtx[0][1]) + (y * mtx[1][1]) + (z * mtx[2][1]);
    out.z = (x * mtx[0][2]) + (y * mtx[1][2]) + (z * mtx[2][2]);
    return out;
}

void CVector3f::operator*=(const CTransform4f& mtx)
{
    *this = *this * mtx;
}

CVector3f CVector3f::operator*(const CMatrix4f& mtx) const
{
    // To multiply by a Matrix4f, we consider the vector w component to be 1
    CVector3f out;
    float w = (x * mtx[0][3]) + (y * mtx[1][3]) + (z * mtx[2][3]) + mtx[3][3];
    out.x = ((x * mtx[0][0]) + (y * mtx[1][0]) + (z * mtx[2][0]) + mtx[3][0]) / w;
    out.y = ((x * mtx[0][1]) + (y * mtx[1][1]) + (z * mtx[2][1]) + mtx[3][1]) / w;
    out.z = ((x * mtx[0][2]) + (y * mtx[1][2]) + (z * mtx[2][2]) + mtx[3][2]) / w;
    return out;
}

// UNARY
CVector3f CVector3f::operator-() const
{
    return CVector3f(-x, -y, -z);
}

float& CVector3f::operator[](long index)
{
    return (&x)[index];
}

const float& CVector3f::operator[](long index) const
{
    return (&x)[index];
}

// ************ CONSTANTS ************
const CVector3f CVector3f::skZero     = CVector3f(0.f);
const CVector3f CVector3f::skOne      = CVector3f(1.f);
const CVector3f CVector3f::skInfinite = CVector3f(FLT_MAX);
const CVector3f CVector3f::skUnitX    = CVector3f(1.f, 0.f, 0.f);
const CVector3f CVector3f::skUnitY    = CVector3f(0.f, 1.f, 0.f);
const CVector3f CVector3f::skUnitZ    = CVector3f(0.f, 0.f, 1.f);
const CVector3f CVector3f::skRight    = CVector3f::skUnitX;
const CVector3f CVector3f::skLeft     = -CVector3f::skUnitX;
const CVector3f CVector3f::skForward  = CVector3f::skUnitY;
const CVector3f CVector3f::skBack     = -CVector3f::skUnitY;
const CVector3f CVector3f::skUp       = CVector3f::skUnitZ;
const CVector3f CVector3f::skDown     = -CVector3f::skUnitZ;

// ************ OTHER ************
std::ostream& operator<<(std::ostream& o, const CVector3f& Vector)
{
    o << std::setprecision(6)
      << std::fixed
      << "["
      << ((Vector.x >= 0) ? " " : "")
      << Vector.x
      << ", "
      << ((Vector.y >= 0) ? " " : "")
      << Vector.y
      << ", "
      << ((Vector.z >= 0) ? " " : "")
      << Vector.z
      << "]";

    return o;
}
