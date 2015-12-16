#include "CVector2i.h"

CVector2i::CVector2i()
{
    x = y = 0;
}

CVector2i::CVector2i(int xy)
{
    x = y = xy;
}

CVector2i::CVector2i(int _x, int _y)
{
    x = _x;
    y = _y;
}

CVector2i CVector2i::operator+(const CVector2i& other) const
{
    CVector2i out;
    out.x = this->x + other.x;
    out.y = this->y + other.y;
    return out;
}

CVector2i CVector2i::operator-(const CVector2i& other) const
{
    CVector2i out;
    out.x = this->x - other.x;
    out.y = this->y - other.y;
    return out;
}

CVector2i CVector2i::operator*(const CVector2i& other) const
{
    CVector2i out;
    out.x = this->x * other.x;
    out.y = this->y * other.y;
    return out;
}

CVector2i CVector2i::operator/(const CVector2i& other) const
{
    CVector2i out;
    out.x = this->x / other.x;
    out.y = this->y / other.y;
    return out;
}

void CVector2i::operator+=(const CVector2i& other)
{
    *this = *this + other;
}

void CVector2i::operator-=(const CVector2i& other)
{
    *this = *this - other;
}

void CVector2i::operator*=(const CVector2i& other)
{
    *this = *this * other;
}

void CVector2i::operator/=(const CVector2i& other)
{
    *this = *this / other;
}

CVector2i CVector2i::operator+(const int other) const
{
    CVector2i out;
    out.x = this->x + other;
    out.y = this->y + other;
    return out;
}

CVector2i CVector2i::operator-(const int other) const
{
    CVector2i out;
    out.x = this->x - other;
    out.y = this->y - other;
    return out;
}

CVector2i CVector2i::operator*(const int other) const
{
    CVector2i out;
    out.x = this->x * other;
    out.y = this->y * other;
    return out;
}

CVector2i CVector2i::operator/(const int other) const
{
    CVector2i out;
    out.x = this->x / other;
    out.y = this->y / other;
    return out;
}

void CVector2i::operator+=(const int other)
{
    *this = *this + other;
}

void CVector2i::operator-=(const int other)
{
    *this = *this - other;
}

void CVector2i::operator*=(const int other)
{
    *this = *this * other;
}

void CVector2i::operator/=(const int other)
{
    *this = *this / other;
}

bool CVector2i::operator==(const CVector2i& other) const
{
    return ((this->x == other.x) && (this->y == other.y));
}

bool CVector2i::operator!=(const CVector2i& other) const
{
    return (!(*this == other));
}

int& CVector2i::operator[](int index)
{
    return (&x)[index];
}

// ************ STATIC MEMBER INTIALIZATION ************
const CVector2i CVector2i::skZero = CVector2i(0,0);
