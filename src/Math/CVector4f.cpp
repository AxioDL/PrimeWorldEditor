#include "CVector4f.h"
#include "CVector2f.h"
#include "CVector3f.h"
#include "CTransform4f.h"

CVector4f::CVector4f()
{
    X = Y = Z = W = 0.f;
}

CVector4f::CVector4f(float XYZW)
{
    X = Y = Z = W = XYZW;
}

CVector4f::CVector4f(float _X, float _Y, float _Z, float _W)
{
    X = _X;
    Y = _Y;
    Z = _Z;
    W = _W;
}

CVector4f::CVector4f(const CVector2f& rkXY, float _Z, float _W)
{
    X = rkXY.X;
    Y = rkXY.Y;
    Z = _Z;
    W = _W;
}

CVector4f::CVector4f(const CVector3f& rkXYZ)
{
    X = rkXYZ.X;
    Y = rkXYZ.Y;
    Z = rkXYZ.Z;
    W = 1.f;
}

CVector4f::CVector4f(const CVector3f& rkXYZ, float _W)
{
    X = rkXYZ.X;
    Y = rkXYZ.Y;
    Z = rkXYZ.Z;
    W = _W;
}

CVector4f::CVector4f(IInputStream& rInput)
{
    X = rInput.ReadFloat();
    Y = rInput.ReadFloat();
    Z = rInput.ReadFloat();
    W = rInput.ReadFloat();
}

void CVector4f::Write(IOutputStream& rOutput)
{
    rOutput.WriteFloat(X);
    rOutput.WriteFloat(Y);
    rOutput.WriteFloat(Z);
    rOutput.WriteFloat(W);
}

// ************ SWIZZLE ************
CVector3f CVector4f::XYZ() const
{
    return CVector3f(X, Y, Z);
}

CVector3f CVector4f::XZW() const
{
    return CVector3f(X, Z, W);
}

CVector3f CVector4f::YZW() const
{
    return CVector3f(Y, Z, W);
}

CVector2f CVector4f::XY() const
{
    return CVector2f(X, Y);
}

CVector2f CVector4f::XZ() const
{
    return CVector2f(X, Z);
}

CVector2f CVector4f::XW() const
{
    return CVector2f(X, W);
}

CVector2f CVector4f::YZ() const
{
    return CVector2f(Y, Z);
}

CVector2f CVector4f::YW() const
{
    return CVector2f(Y, W);
}

CVector2f CVector4f::ZW() const
{
    return CVector2f(Z, W);
}

// ************ MATH ************
// ************ VECTOR/VECTOR ************
CVector4f CVector4f::operator+(const CVector4f& rkSrc) const
{
    CVector4f Out;
    Out.X = this->X + rkSrc.X;
    Out.Y = this->Y + rkSrc.Y;
    Out.Z = this->Z + rkSrc.Z;
    Out.W = this->W + rkSrc.W;
    return Out;
}

CVector4f CVector4f::operator-(const CVector4f& rkSrc) const
{
    CVector4f Out;
    Out.X = this->X - rkSrc.X;
    Out.Y = this->Y - rkSrc.Y;
    Out.Z = this->Z - rkSrc.Z;
    Out.W = this->W - rkSrc.W;
    return Out;
}

CVector4f CVector4f::operator*(const CVector4f& rkSrc) const
{
    CVector4f Out;
    Out.X = this->X * rkSrc.X;
    Out.Y = this->Y * rkSrc.Y;
    Out.Z = this->Z * rkSrc.Z;
    Out.W = this->W * rkSrc.W;
    return Out;
}

CVector4f CVector4f::operator/(const CVector4f& rkSrc) const
{
    CVector4f Out;
    Out.X = this->X / rkSrc.X;
    Out.Y = this->Y / rkSrc.Y;
    Out.Z = this->Z / rkSrc.Z;
    Out.W = this->W / rkSrc.W;
    return Out;
}

void CVector4f::operator+=(const CVector4f& rkOther)
{
    *this = *this + rkOther;
}

void CVector4f::operator-=(const CVector4f& rkOther)
{
    *this = *this - rkOther;
}

void CVector4f::operator*=(const CVector4f& rkOther)
{
    *this = *this * rkOther;
}

void CVector4f::operator/=(const CVector4f& rkOther)
{
    *this = *this / rkOther;
}

bool CVector4f::operator==(const CVector4f& rkOther) const
{
    return ((X == rkOther.X) && (Y == rkOther.Y) && (Z == rkOther.Z) && (W == rkOther.W));
}

// ************ VECTOR/FLOAT ************
CVector4f CVector4f::operator+(const float Src) const
{
    CVector4f Out;
    Out.X = this->X + Src;
    Out.Y = this->Y + Src;
    Out.Z = this->Z + Src;
    Out.W = this->W + Src;
    return Out;
}

CVector4f CVector4f::operator-(const float Src) const
{
    CVector4f Out;
    Out.X = this->X - Src;
    Out.Y = this->Y - Src;
    Out.Z = this->Z - Src;
    Out.W = this->W - Src;
    return Out;
}

CVector4f CVector4f::operator*(const float Src) const
{
    CVector4f Out;
    Out.X = this->X * Src;
    Out.Y = this->Y * Src;
    Out.Z = this->Z * Src;
    Out.W = this->W * Src;
    return Out;
}

CVector4f CVector4f::operator/(const float Src) const
{
    CVector4f Out;
    Out.X = this->X / Src;
    Out.Y = this->Y / Src;
    Out.Z = this->Z / Src;
    Out.W = this->W / Src;
    return Out;
}

void CVector4f::operator+=(const float Other)
{
    *this = *this + Other;
}

void CVector4f::operator-=(const float Other)
{
    *this = *this - Other;
}

void CVector4f::operator*=(const float Other)
{
    *this = *this * Other;
}

void CVector4f::operator/=(const float Other)
{
    *this = *this / Other;
}

// ************ VECTOR/MATRIX ************
CVector4f CVector4f::operator*(const CTransform4f& rkMtx) const
{
    CVector4f Out;
    Out.X = (X * rkMtx[0][0]) + (Y * rkMtx[1][0]) + (Z * rkMtx[2][0]);
    Out.Y = (X * rkMtx[0][1]) + (Y * rkMtx[1][1]) + (Z * rkMtx[2][1]);
    Out.Z = (X * rkMtx[0][2]) + (Y * rkMtx[1][2]) + (Z * rkMtx[2][2]);
    Out.W = (X * rkMtx[0][3]) + (Y * rkMtx[1][3]) + (Z * rkMtx[2][3]) + W;
    return Out;
}

void CVector4f::operator*=(const CTransform4f& rkMtx)
{
    *this = *this * rkMtx;
}

CVector4f CVector4f::operator*(const CMatrix4f& rkMtx) const
{
    CVector4f Out;
    Out.X = (X * rkMtx[0][0]) + (Y * rkMtx[1][0]) + (Z * rkMtx[2][0]) + (W * rkMtx[3][0]);
    Out.Y = (X * rkMtx[0][1]) + (Y * rkMtx[1][1]) + (Z * rkMtx[2][1]) + (W * rkMtx[3][1]);
    Out.Z = (X * rkMtx[0][2]) + (Y * rkMtx[1][2]) + (Z * rkMtx[2][2]) + (W * rkMtx[3][2]);
    Out.W = (X * rkMtx[0][3]) + (Y * rkMtx[1][3]) + (Z * rkMtx[2][3]) + (W * rkMtx[3][3]);
    return Out;
}

void CVector4f::operator*=(const CMatrix4f& rkMtx)
{
    *this = *this * rkMtx;
}

// ************ UNARY ************
float& CVector4f::operator[](long Index)
{
    return (&X)[Index];
}
