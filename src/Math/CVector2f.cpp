#include "CVector2f.h"

CVector2f::CVector2f()
{
    X = Y = 0.f;
}

CVector2f::CVector2f(float XY)
{
    X = Y = XY;
}

CVector2f::CVector2f(float _X, float _Y)
{
    X = _X;
    Y = _Y;
}

CVector2f::CVector2f(IInputStream& rInput)
{
    X = rInput.ReadFloat();
    Y = rInput.ReadFloat();
}

void CVector2f::Write(IOutputStream& rOutput)
{
    rOutput.WriteFloat(X);
    rOutput.WriteFloat(Y);
}

float CVector2f::Magnitude() const
{
    return sqrtf(SquaredMagnitude());
}

float CVector2f::SquaredMagnitude() const
{
    return Dot(*this);
}

CVector2f CVector2f::Normalized() const
{
    return *this / Magnitude();
}

float CVector2f::Dot(const CVector2f& rkOther) const
{
    return ((X * rkOther.X) + (Y * rkOther.Y));
}

CVector2f CVector2f::operator+(const CVector2f& rkSrc) const
{
    CVector2f Out;
    Out.X = this->X + rkSrc.X;
    Out.Y = this->Y + rkSrc.Y;
    return Out;
}

CVector2f CVector2f::operator-(const CVector2f& rkSrc) const
{
    CVector2f Out;
    Out.X = this->X - rkSrc.X;
    Out.Y = this->Y - rkSrc.Y;
    return Out;
}

CVector2f CVector2f::operator*(const CVector2f& rkSrc) const
{
    CVector2f Out;
    Out.X = this->X * rkSrc.X;
    Out.Y = this->Y * rkSrc.Y;
    return Out;
}

CVector2f CVector2f::operator/(const CVector2f& rkSrc) const
{
    CVector2f Out;
    Out.X = this->X / rkSrc.X;
    Out.Y = this->Y / rkSrc.Y;
    return Out;
}

void CVector2f::operator+=(const CVector2f& rkOther)
{
    *this = *this + rkOther;
}

void CVector2f::operator-=(const CVector2f& rkOther)
{
    *this = *this - rkOther;
}

void CVector2f::operator*=(const CVector2f& rkOther)
{
    *this = *this * rkOther;
}

void CVector2f::operator/=(const CVector2f& rkOther)
{
    *this = *this / rkOther;
}

CVector2f CVector2f::operator+(const float Src) const
{
    CVector2f Out;
    Out.X = this->X + Src;
    Out.Y = this->Y + Src;
    return Out;
}

CVector2f CVector2f::operator-(const float Src) const
{
    CVector2f Out;
    Out.X = this->X - Src;
    Out.Y = this->Y - Src;
    return Out;
}

CVector2f CVector2f::operator*(const float Src) const
{
    CVector2f Out;
    Out.X = this->X * Src;
    Out.Y = this->Y * Src;
    return Out;
}

CVector2f CVector2f::operator/(const float Src) const
{
    CVector2f Out;
    Out.X = this->X / Src;
    Out.Y = this->Y / Src;
    return Out;
}

void CVector2f::operator+=(const float Other)
{
    *this = *this + Other;
}

void CVector2f::operator-=(const float Other)
{
    *this = *this - Other;
}

void CVector2f::operator*=(const float Other)
{
    *this = *this * Other;
}

void CVector2f::operator/=(const float Other)
{
    *this = *this / Other;
}

bool CVector2f::operator==(const CVector2f& rkOther) const
{
    return ((X == rkOther.X) && (Y == rkOther.Y));
}

bool CVector2f::operator!=(const CVector2f& rkOther) const
{
    return (!(*this == rkOther));
}

CVector2f CVector2f::operator-() const
{
    return CVector2f(-X, -Y);
}

float& CVector2f::operator[](long Index)
{
    return (&X)[Index];
}

const float& CVector2f::operator[](long Index) const
{
    return (&X)[Index];
}

// ************ STATIC MEMBER INITIALIZATION ************
const CVector2f CVector2f::skZero  = CVector2f(0, 0);
const CVector2f CVector2f::skOne   = CVector2f(1, 1);
const CVector2f CVector2f::skUp    = CVector2f(0, 1);
const CVector2f CVector2f::skRight = CVector2f(1, 0);
const CVector2f CVector2f::skDown  = CVector2f(0,-1);
const CVector2f CVector2f::skLeft  = CVector2f(-1,0);
