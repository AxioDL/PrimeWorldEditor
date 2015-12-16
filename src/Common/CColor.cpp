#include "CColor.h"

CColor::CColor()
    : r(0.f), g(0.f), b(0.f), a(0.f)
{
}

CColor::CColor(IInputStream& rInput, bool Integral /*= false*/)
{
    if (Integral)
    {
        r = (u8) rInput.ReadByte() / 255.f;
        g = (u8) rInput.ReadByte() / 255.f;
        b = (u8) rInput.ReadByte() / 255.f;
        a = (u8) rInput.ReadByte() / 255.f;
    }
    else
    {
        r = rInput.ReadFloat();
        g = rInput.ReadFloat();
        b = rInput.ReadFloat();
        a = rInput.ReadFloat();
    }
}

CColor::CColor(float rgba)
    : r(rgba), g(rgba), b(rgba), a(rgba)
{
}

CColor::CColor(float _r, float _g, float _b, float _a /*= 1.f*/)
    : r(_r), g(_g), b(_b), a(_a)
{
}

void CColor::SetIntegral(u8 rgba)
{
    float f = rgba / 255.f;
    r = g = b = a = f;
}

void CColor::SetIntegral(u8 _r, u8 _g, u8 _b, u8 _a /*= 255*/)
{
    r = _r / 255.f;
    g = _g / 255.f;
    b = _b / 255.f;
    a = _a / 255.f;
}

void CColor::Write(IOutputStream &rOutput, bool Integral /*= false*/)
{
    if (Integral)
    {
        rOutput.WriteLong(ToLongRGBA());
    }

    else
    {
        rOutput.WriteFloat(r);
        rOutput.WriteFloat(g);
        rOutput.WriteFloat(b);
        rOutput.WriteFloat(a);
    }
}

long CColor::ToLongRGBA() const
{
    u8 _r = (u8) (r * 255);
    u8 _g = (u8) (g * 255);
    u8 _b = (u8) (b * 255);
    u8 _a = (u8) (a * 255);
    return (_r << 24) | (_g << 16) | (_b << 8) | _a;
}

long CColor::ToLongARGB() const
{
    u8 _r = (u8) (r * 255);
    u8 _g = (u8) (g * 255);
    u8 _b = (u8) (b * 255);
    u8 _a = (u8) (a * 255);
    return (_a << 24) | (_r << 16) | (_g << 8) | _b;
}

bool CColor::operator==(const CColor& rkOther) const
{
    return ((r == rkOther.r) &&
            (g == rkOther.g) &&
            (b == rkOther.b) &&
            (a == rkOther.a));
}

bool CColor::operator!=(const CColor& rkOther) const
{
    return (!(*this == rkOther));
}

CColor CColor::operator+(const CColor& rkOther) const
{
    float NewR = fmin(r + rkOther.r, 1.f);
    float NewG = fmin(g + rkOther.g, 1.f);
    float NewB = fmin(b + rkOther.b, 1.f);
    float NewA = fmin(a + rkOther.a, 1.f);
    return CColor(NewR, NewG, NewB, NewA);
}

void CColor::operator+=(const CColor& rkOther)
{
    *this = (*this + rkOther);
}

CColor CColor::operator-(const CColor& rkOther) const
{
    float NewR = fmax(r - rkOther.r, 0.f);
    float NewG = fmax(g - rkOther.g, 0.f);
    float NewB = fmax(b - rkOther.b, 0.f);
    float NewA = fmax(a - rkOther.a, 0.f);
    return CColor(NewR, NewG, NewB, NewA);
}

void CColor::operator-=(const CColor& other)
{
    *this = (*this - other);
}

CColor CColor::operator*(const CColor& rkOther) const
{
    float NewR = r * rkOther.r;
    float NewG = g * rkOther.g;
    float NewB = b * rkOther.b;
    float NewA = a * rkOther.a;
    return CColor(NewR, NewG, NewB, NewA);
}

void CColor::operator*=(const CColor& rkOther)
{
    *this = (*this * rkOther);
}

CColor CColor::operator*(float other) const
{
    float NewR = fmin( fmax(r * other, 0.f), 1.f);
    float NewG = fmin( fmax(g * other, 0.f), 1.f);
    float NewB = fmin( fmax(b * other, 0.f), 1.f);
    float NewA = fmin( fmax(a * other, 0.f), 1.f);
    return CColor(NewR, NewG, NewB, NewA);
}

void CColor::operator*=(float other)
{
    *this = (*this * other);
}

CColor CColor::operator/(const CColor& rkOther) const
{
    float NewR = (rkOther.r == 0.f) ? 0.f : r / rkOther.r;
    float NewG = (rkOther.g == 0.f) ? 0.f : g / rkOther.g;
    float NewB = (rkOther.b == 0.f) ? 0.f : b / rkOther.b;
    float NewA = (rkOther.a == 0.f) ? 0.f : a / rkOther.a;
    return CColor(NewR, NewG, NewB, NewA);
}

void CColor::operator/=(const CColor& rkOther)
{
    *this = (*this / rkOther);
}

// ************ STATIC ************
CColor CColor::Integral(u8 rgba)
{
    CColor out;
    out.SetIntegral(rgba);
    return out;
}

CColor CColor::Integral(u8 _r, u8 _g, u8 _b, u8 _a /*= 255*/)
{
    CColor out;
    out.SetIntegral(_r, _g, _b, _a);
    return out;
}

CColor CColor::RandomColor(bool transparent)
{
    float _r = (rand() % 255) / 255.f;
    float _g = (rand() % 255) / 255.f;
    float _b = (rand() % 255) / 255.f;
    float _a = (transparent ? (rand() % 255) / 255.f : 0);
    return CColor(_r, _g, _b, _a);
}

CColor CColor::RandomLightColor(bool transparent)
{
    float _r = 0.5f + (rand() % 128) / 255.f;
    float _g = 0.5f + (rand() % 128) / 255.f;
    float _b = 0.5f + (rand() % 128) / 255.f;
    float _a = (transparent ? 0.5f + ((rand() % 128) / 255.f) : 0);
    return CColor(_r, _g, _b, _a);
}

CColor CColor::RandomDarkColor(bool transparent)
{
    float _r = (rand() % 128) / 255.f;
    float _g = (rand() % 128) / 255.f;
    float _b = (rand() % 128) / 255.f;
    float _a = (transparent ? (rand() % 128) / 255.f : 0);
    return CColor(_r, _g, _b, _a);
}

// defining predefined colors
const CColor CColor::skRed   (1.0f, 0.0f, 0.0f);
const CColor CColor::skGreen (0.0f, 1.0f, 0.0f);
const CColor CColor::skBlue  (0.0f, 0.0f, 1.0f);
const CColor CColor::skYellow(1.0f, 1.0f, 0.0f);
const CColor CColor::skPurple(1.0f, 0.0f, 1.0f);
const CColor CColor::skCyan  (0.0f, 1.0f, 1.0f);
const CColor CColor::skWhite (1.0f, 1.0f, 1.0f);
const CColor CColor::skBlack (0.0f, 0.0f, 0.0f);
const CColor CColor::skGray  (0.5f, 0.5f, 0.5f);
const CColor CColor::skTransparentRed   (1.0f, 0.0f, 0.0f, 0.0f);
const CColor CColor::skTransparentGreen (0.0f, 1.0f, 0.0f, 0.0f);
const CColor CColor::skTransparentBlue  (0.0f, 0.0f, 1.0f, 0.0f);
const CColor CColor::skTransparentYellow(1.0f, 1.0f, 0.0f, 0.0f);
const CColor CColor::skTransparentPurple(1.0f, 0.0f, 1.0f, 0.0f);
const CColor CColor::skTransparentCyan  (0.0f, 1.0f, 1.0f, 0.0f);
const CColor CColor::skTransparentWhite (1.0f, 1.0f, 1.0f, 0.0f);
const CColor CColor::skTransparentBlack (0.0f, 0.0f, 0.0f, 0.0f);
const CColor CColor::skTransparentGray  (0.5f, 0.5f, 0.5f, 0.0f);
