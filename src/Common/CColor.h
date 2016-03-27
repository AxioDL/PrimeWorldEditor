#ifndef CCOLOR_H
#define CCOLOR_H

#include <FileIO/IInputStream.h>
#include <FileIO/IOutputStream.h>
#include "types.h"

class CColor
{
public:
    float R, G, B, A;

    CColor();
    CColor(IInputStream& rInput, bool Integral = false);
    CColor(float RGBA);
    CColor(float _R, float _G, float _B, float A = 1.f);
    void SetIntegral(u8 RGBA);
    void SetIntegral(u8 _R, u8 _G, u8 _B, u8 _A = 255);
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
    CColor operator*(float Other) const;
    void operator*=(float Other);
    CColor operator/(const CColor& rkOther) const;
    void operator/=(const CColor& rkOther);

    // Static
    static CColor Integral(u8 RGBA);
    static CColor Integral(u8 _R, u8 _G, u8 _B, u8 _A = 255);
    static CColor RandomColor(bool Transparent);
    static CColor RandomLightColor(bool Transparent);
    static CColor RandomDarkColor(bool Transparent);

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
