#include "CVector3f.h"
#include "CVector2f.h"
#include "CVector4f.h"
#include "CTransform4f.h"
#include <iomanip>
#include <float.h>

CVector3f::CVector3f()
{
    X = Y = Z = 0.f;
}

CVector3f::CVector3f(float XYZ)
{
    X = Y = Z = XYZ;
}

CVector3f::CVector3f(float _X, float _Y, float _Z)
{
    X = _X;
    Y = _Y;
    Z = _Z;
}

CVector3f::CVector3f(IInputStream& rInput)
{
    X = rInput.ReadFloat();
    Y = rInput.ReadFloat();
    Z = rInput.ReadFloat();
}

void CVector3f::Write(IOutputStream& rOutput) const
{
    rOutput.WriteFloat(X);
    rOutput.WriteFloat(Y);
    rOutput.WriteFloat(Z);
}

void CVector3f::Serialize(IArchive& rArc)
{
    rArc << SerialParameter("X", X)
         << SerialParameter("Y", Y)
         << SerialParameter("Z", Z);
}

TString CVector3f::ToString() const
{
    return TString::Format("%s, %s, %s",
                           *TString::FromFloat(X),
                           *TString::FromFloat(Y),
                           *TString::FromFloat(Z));
}

// ************ SWIZZLE ************
CVector2f CVector3f::XY()
{
    return CVector2f(X, Y);
}

CVector2f CVector3f::XZ()
{
    return CVector2f(X, Z);
}

CVector2f CVector3f::YZ()
{
    return CVector2f(Y, Z);
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

float CVector3f::Dot(const CVector3f& rkOther) const
{
    return (X * rkOther.X) + (Y * rkOther.Y) + (Z * rkOther.Z);
}

CVector3f CVector3f::Cross(const CVector3f& rkOther) const
{
    return CVector3f((Y * rkOther.Z) - (Z * rkOther.Y), (Z * rkOther.X) - (X * rkOther.Z), (X * rkOther.Y) - (Y * rkOther.X));
}

float CVector3f::Distance(const CVector3f& rkPoint) const
{
    return (*this - rkPoint).Magnitude();
}

float CVector3f::SquaredDistance(const CVector3f& rkPoint) const
{
    return (*this - rkPoint).SquaredMagnitude();
}

// ************ OPERATORS ************
// VECTOR/VECTOR
CVector3f CVector3f::operator+(const CVector3f& rkSrc) const
{
    CVector3f Out;
    Out.X = this->X + rkSrc.X;
    Out.Y = this->Y + rkSrc.Y;
    Out.Z = this->Z + rkSrc.Z;
    return Out;
}

CVector3f CVector3f::operator-(const CVector3f& rkSrc) const
{
    CVector3f Out;
    Out.X = this->X - rkSrc.X;
    Out.Y = this->Y - rkSrc.Y;
    Out.Z = this->Z - rkSrc.Z;
    return Out;
}

CVector3f CVector3f::operator*(const CVector3f& rkSrc) const
{
    CVector3f Out;
    Out.X = this->X * rkSrc.X;
    Out.Y = this->Y * rkSrc.Y;
    Out.Z = this->Z * rkSrc.Z;
    return Out;
}

CVector3f CVector3f::operator/(const CVector3f& rkSrc) const
{
    CVector3f Out;
    Out.X = this->X / rkSrc.X;
    Out.Y = this->Y / rkSrc.Y;
    Out.Z = this->Z / rkSrc.Z;
    return Out;
}

void CVector3f::operator+=(const CVector3f& rkOther)
{
    *this = *this + rkOther;
}

void CVector3f::operator-=(const CVector3f& rkOther)
{
    *this = *this - rkOther;
}

void CVector3f::operator*=(const CVector3f& rkOther)
{
    *this = *this * rkOther;
}

void CVector3f::operator/=(const CVector3f& rkOther)
{
    *this = *this / rkOther;
}

bool CVector3f::operator> (const CVector3f& rkOther) const
{
    return ((X > rkOther.X) && (Y > rkOther.Y) && (Z > rkOther.Z));
}

bool CVector3f::operator>=(const CVector3f& rkOther) const
{
    return ((X >= rkOther.X) && (Y >= rkOther.Y) && (Z >= rkOther.Z));
}

bool CVector3f::operator< (const CVector3f& rkOther) const
{
    return ((X < rkOther.X) && (Y < rkOther.Y) && (Z < rkOther.Z));
}

bool CVector3f::operator<=(const CVector3f& rkOther) const
{
    return ((X <= rkOther.X) && (Y <= rkOther.Y) && (Z <= rkOther.Z));
}

bool CVector3f::operator==(const CVector3f& rkOther) const
{
    return ((X == rkOther.X) && (Y == rkOther.Y) && (Z == rkOther.Z));
}

bool CVector3f::operator!=(const CVector3f& rkOther) const
{
    return (!(*this == rkOther));
}

// VECTOR/FLOAT
CVector3f CVector3f::operator+(const float Src) const
{
    CVector3f Out;
    Out.X = this->X + Src;
    Out.Y = this->Y + Src;
    Out.Z = this->Z + Src;
    return Out;
}

CVector3f CVector3f::operator-(const float Src) const
{
    CVector3f Out;
    Out.X = this->X - Src;
    Out.Y = this->Y - Src;
    Out.Z = this->Z - Src;
    return Out;
}

CVector3f CVector3f::operator*(const float Src) const
{
    CVector3f Out;
    Out.X = this->X * Src;
    Out.Y = this->Y * Src;
    Out.Z = this->Z * Src;
    return Out;
}

CVector3f CVector3f::operator/(const float Src) const
{
    CVector3f Out;
    Out.X = this->X / Src;
    Out.Y = this->Y / Src;
    Out.Z = this->Z / Src;
    return Out;
}

void CVector3f::operator+=(const float Other)
{
    *this = *this + Other;
}

void CVector3f::operator-=(const float Other)
{
    *this = *this - Other;
}

void CVector3f::operator*=(const float Other)
{
    *this = *this * Other;
}

void CVector3f::operator/=(const float Other)
{
    *this = *this / Other;
}

// VECTOR/MATRIX
CVector3f CVector3f::operator*(const CTransform4f& rkMtx) const
{
    CVector3f Out;
    Out.X = (X * rkMtx[0][0]) + (Y * rkMtx[1][0]) + (Z * rkMtx[2][0]);
    Out.Y = (X * rkMtx[0][1]) + (Y * rkMtx[1][1]) + (Z * rkMtx[2][1]);
    Out.Z = (X * rkMtx[0][2]) + (Y * rkMtx[1][2]) + (Z * rkMtx[2][2]);
    return Out;
}

void CVector3f::operator*=(const CTransform4f& rkMtx)
{
    *this = *this * rkMtx;
}

CVector3f CVector3f::operator*(const CMatrix4f& rkMtx) const
{
    // To multiply by a Matrix4f, we consider the vector w component to be 1
    CVector3f Out;
    float W = (X * rkMtx[0][3]) + (Y * rkMtx[1][3]) + (Z * rkMtx[2][3]) + rkMtx[3][3];
    Out.X = ((X * rkMtx[0][0]) + (Y * rkMtx[1][0]) + (Z * rkMtx[2][0]) + rkMtx[3][0]) / W;
    Out.Y = ((X * rkMtx[0][1]) + (Y * rkMtx[1][1]) + (Z * rkMtx[2][1]) + rkMtx[3][1]) / W;
    Out.Z = ((X * rkMtx[0][2]) + (Y * rkMtx[1][2]) + (Z * rkMtx[2][2]) + rkMtx[3][2]) / W;
    return Out;
}

// UNARY
CVector3f CVector3f::operator-() const
{
    return CVector3f(-X, -Y, -Z);
}

float& CVector3f::operator[](long Index)
{
    return (&X)[Index];
}

const float& CVector3f::operator[](long Index) const
{
    return (&X)[Index];
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
std::ostream& operator<<(std::ostream& rOut, const CVector3f& rkVector)
{
    rOut << std::setprecision(6)
      << std::fixed
      << "["
      << ((rkVector.X >= 0) ? " " : "")
      << rkVector.X
      << ", "
      << ((rkVector.Y >= 0) ? " " : "")
      << rkVector.Y
      << ", "
      << ((rkVector.Z >= 0) ? " " : "")
      << rkVector.Z
      << "]";

    return rOut;
}
