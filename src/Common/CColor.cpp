#include "CColor.h"

CColor::CColor()
{
    r = g = b = a = 0;
}

CColor::CColor(CInputStream& src)
{
    src.ReadBytes(&r, 4);
}

CColor::CColor(u32 rgba)
{
    r = (rgba >> 24) & 0xFF;
    g = (rgba >> 16) & 0xFF;
    b = (rgba >>  8) & 0xFF;
    a = rgba & 0xFF;
}

CColor::CColor(u8 rgba)
{
    r = g = b = a = rgba;
}

CColor::CColor(u8 _r, u8 _g, u8 _b, u8 _a)
{
    r = _r;
    g = _g;
    b = _b;
    a = _a;
}

CColor::CColor(float rgba)
{
    r = g = b = a = u8(rgba * 255.f);
}

CColor::CColor(float _r, float _g, float _b, float _a)
{
    r = u8(_r * 255.f);
    g = u8(_g * 255.f);
    b = u8(_b * 255.f);
    a = u8(_a * 255.f);
}

void CColor::Write(COutputStream &Output)
{
    Output.WriteBytes(&r, 4);
}

long CColor::AsLongRGBA() const
{
    return (r << 24) | (g << 16) | (b << 8) | a;
}

long CColor::ToLongARGB() const
{
    return (a << 24) | (r << 16) | (g << 8) | b;
}

CVector4f CColor::ToVector4f() const
{
    return CVector4f(float(r) / 255.f,
                     float(g) / 255.f,
                     float(b) / 255.f,
                     float(a) / 255.f);
}

bool CColor::operator==(const CColor& other) const
{
    return ((r == other.r) &&
            (g == other.g) &&
            (b == other.b) &&
            (a == other.a));
}

bool CColor::operator!=(const CColor& other) const
{
    return (!(*this == other));
}

CColor CColor::operator+(const CColor& other) const
{
    u16 NewR = r + other.r;
    if (NewR > 255) NewR = 255;
    u16 NewG = g + other.g;
    if (NewG > 255) NewG = 255;
    u16 NewB = b + other.b;
    if (NewB > 255) NewB = 255;
    u16 NewA = a + other.a;
    if (NewA > 255) NewA = 255;
    return CColor((u8) NewR, (u8) NewG, (u8) NewB, (u8) NewA);
}

void CColor::operator+=(const CColor& other)
{
    *this = (*this + other);
}

CColor CColor::operator-(const CColor& other) const
{
    s16 NewR = r - other.r;
    if (NewR < 0) NewR = 0;
    s16 NewG = g - other.g;
    if (NewG < 0) NewG = 0;
    s16 NewB = b - other.b;
    if (NewB < 0) NewB = 0;
    s16 NewA = a - other.a;
    if (NewA < 0) NewA = 0;
    return CColor((u8) NewR, (u8) NewG, (u8) NewB, (u8) NewA);
}

void CColor::operator-=(const CColor& other)
{
    *this = (*this - other);
}

CColor CColor::operator*(const CColor& other) const
{
    CVector4f A = ToVector4f();
    CVector4f B = other.ToVector4f();

    float NewR = A.x * B.x;
    float NewG = A.y * B.y;
    float NewB = A.z * B.z;
    float NewA = A.w * B.w;

    return CColor(NewR, NewG, NewB, NewA);
}

void CColor::operator*=(const CColor& other)
{
    *this = (*this * other);
}

CColor CColor::operator*(const float other) const
{
    CVector4f Vec4f = ToVector4f() * other;
    return CColor(Vec4f.x, Vec4f.y, Vec4f.z, Vec4f.w);
}

void CColor::operator*=(const float other)
{
    *this = (*this * other);
}

CColor CColor::operator/(const CColor& other) const
{
    u16 NewR = (other.r == 0) ? 0 : r / other.r;
    u16 NewG = (other.g == 0) ? 0 : g / other.g;
    u16 NewB = (other.b == 0) ? 0 : b / other.b;
    u16 NewA = (other.a == 0) ? 0 : a / other.a;
    return CColor((u8) NewR, (u8) NewG, (u8) NewB, (u8) NewA);
}

void CColor::operator/=(const CColor& other)
{
    *this = (*this / other);
}

// ************ STATIC ************
CColor CColor::RandomColor(bool transparent)
{
    CColor out;
    out.r = rand() % 255;
    out.g = rand() % 255;
    out.b = rand() % 255;
    out.a = (transparent ? rand() % 255 : 0);
    return out;
}

CColor CColor::RandomLightColor(bool transparent)
{
    CColor out;
    out.r = 127 + (rand() % 128);
    out.g = 127 + (rand() % 128);
    out.b = 127 + (rand() % 128);
    out.a = (transparent ? 127 + (rand() % 128) : 0);
    return out;
}

CColor CColor::RandomDarkColor(bool transparent)
{
    CColor out;
    out.r = rand() % 128;
    out.g = rand() % 128;
    out.b = rand() % 128;
    out.a = (transparent ? rand() % 128 : 0);
    return out;
}

// defining predefined colors
const CColor CColor::skRed   (u32(0xFF0000FF));
const CColor CColor::skGreen (u32(0x00FF00FF));
const CColor CColor::skBlue  (u32(0x0000FFFF));
const CColor CColor::skYellow(u32(0xFFFF00FF));
const CColor CColor::skPurple(u32(0xFF00FFFF));
const CColor CColor::skCyan  (u32(0x00FFFFFF));
const CColor CColor::skWhite (u32(0xFFFFFFFF));
const CColor CColor::skBlack (u32(0x000000FF));
const CColor CColor::skGray  (u32(0x808080FF));
const CColor CColor::skTransparentRed   (u32(0xFF000000));
const CColor CColor::skTransparentGreen (u32(0x00FF0000));
const CColor CColor::skTransparentBlue  (u32(0x0000FF00));
const CColor CColor::skTransparentYellow(u32(0xFFFF0000));
const CColor CColor::skTransparentPurple(u32(0xFF00FF00));
const CColor CColor::skTransparentCyan  (u32(0x00FFFF00));
const CColor CColor::skTransparentWhite (u32(0xFFFFFF00));
const CColor CColor::skTransparentBlack (u32(0x00000000));
const CColor CColor::skTransparentGray  (u32(0x80808000));
