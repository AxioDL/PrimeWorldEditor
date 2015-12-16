#ifndef CCOLOR_H
#define CCOLOR_H

#include <FileIO/IInputStream.h>
#include <FileIO/IOutputStream.h>
#include "types.h"

class CColor
{
public:
    float r, g, b, a;

    CColor();
    CColor(IInputStream& rSrc, bool Integral = false);
    CColor(float rgba);
    CColor(float _r, float _g, float _b, float _a = 1.f);
    void SetIntegral(u8 rgba);
    void SetIntegral(u8 _r, u8 _g, u8 _b, u8 _a = 255);
    void Write(IOutputStream& rOutput, bool Integral = false);

    long ToLongRGBA() const;
    long ToLongARGB() const;
    bool operator==(const CColor& rkOther) const;
    bool operator!=(const CColor& rkOther) const;
    CColor operator+(const CColor& rkOther) const;
    void operator+=(const CColor& rkOther);
    CColor operator-(const CColor& rkOther) const;
    void operator-=(const CColor& rkOther);
    CColor operator*(const CColor& rkOther) const;
    void operator*=(const CColor& rkOther);
    CColor operator*(float other) const;
    void operator*=(float other);
    CColor operator/(const CColor& rkOther) const;
    void operator/=(const CColor& rkOther);

    // Static
    static CColor Integral(u8 rgba);
    static CColor Integral(u8 _r, u8 _g, u8 _b, u8 _a = 255);
    static CColor RandomColor(bool transparent);
    static CColor RandomLightColor(bool transparent);
    static CColor RandomDarkColor(bool transparent);

    // some predefined colors below for ease of use
    static const CColor skRed;
    static const CColor skGreen;
    static const CColor skBlue;
    static const CColor skYellow;
    static const CColor skPurple;
    static const CColor skCyan;
    static const CColor skWhite;
    static const CColor skBlack;
    static const CColor skGray;
    static const CColor skTransparentRed;
    static const CColor skTransparentGreen;
    static const CColor skTransparentBlue;
    static const CColor skTransparentYellow;
    static const CColor skTransparentPurple;
    static const CColor skTransparentCyan;
    static const CColor skTransparentWhite;
    static const CColor skTransparentBlack;
    static const CColor skTransparentGray;
};

#endif // CCOLOR_H
