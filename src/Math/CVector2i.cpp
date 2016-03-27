#include "CVector2i.h"

CVector2i::CVector2i()
{
    X = Y = 0;
}

CVector2i::CVector2i(int XY)
{
    X = Y = XY;
}

CVector2i::CVector2i(int _X, int _Y)
{
    X = _X;
    Y = _Y;
}

CVector2i CVector2i::operator+(const CVector2i& rkOther) const
{
    CVector2i Out;
    Out.X = this->X + rkOther.X;
    Out.Y = this->Y + rkOther.Y;
    return Out;
}

CVector2i CVector2i::operator-(const CVector2i& rkOther) const
{
    CVector2i Out;
    Out.X = this->X - rkOther.X;
    Out.Y = this->Y - rkOther.Y;
    return Out;
}

CVector2i CVector2i::operator*(const CVector2i& rkOther) const
{
    CVector2i Out;
    Out.X = this->X * rkOther.X;
    Out.Y = this->Y * rkOther.Y;
    return Out;
}

CVector2i CVector2i::operator/(const CVector2i& rkOther) const
{
    CVector2i Out;
    Out.X = this->X / rkOther.X;
    Out.Y = this->Y / rkOther.Y;
    return Out;
}

void CVector2i::operator+=(const CVector2i& rkOther)
{
    *this = *this + rkOther;
}

void CVector2i::operator-=(const CVector2i& rkOther)
{
    *this = *this - rkOther;
}

void CVector2i::operator*=(const CVector2i& rkOther)
{
    *this = *this * rkOther;
}

void CVector2i::operator/=(const CVector2i& rkOther)
{
    *this = *this / rkOther;
}

CVector2i CVector2i::operator+(const int Other) const
{
    CVector2i Out;
    Out.X = this->X + Other;
    Out.Y = this->Y + Other;
    return Out;
}

CVector2i CVector2i::operator-(const int Other) const
{
    CVector2i Out;
    Out.X = this->X - Other;
    Out.Y = this->Y - Other;
    return Out;
}

CVector2i CVector2i::operator*(const int Other) const
{
    CVector2i Out;
    Out.X = this->X * Other;
    Out.Y = this->Y * Other;
    return Out;
}

CVector2i CVector2i::operator/(const int Other) const
{
    CVector2i Out;
    Out.X = this->X / Other;
    Out.Y = this->Y / Other;
    return Out;
}

void CVector2i::operator+=(const int Other)
{
    *this = *this + Other;
}

void CVector2i::operator-=(const int Other)
{
    *this = *this - Other;
}

void CVector2i::operator*=(const int Other)
{
    *this = *this * Other;
}

void CVector2i::operator/=(const int Other)
{
    *this = *this / Other;
}

bool CVector2i::operator==(const CVector2i& rkOther) const
{
    return ((this->X == rkOther.X) && (this->Y == rkOther.Y));
}

bool CVector2i::operator!=(const CVector2i& rkOther) const
{
    return (!(*this == rkOther));
}

int& CVector2i::operator[](int Index)
{
    return (&X)[Index];
}

// ************ STATIC MEMBER INTIALIZATION ************
const CVector2i CVector2i::skZero = CVector2i(0,0);
