#ifndef CVECTOR2F
#define CVECTOR2F

#include <FileIO/IInputStream.h>
#include <FileIO/IOutputStream.h>

class CVector2f
{
public:
    float X, Y;
    CVector2f();
    CVector2f(float XY);
    CVector2f(float _X, float _Y);
    CVector2f(IInputStream& rInput);
    void Write(IOutputStream& rOutput);

    float Magnitude() const;
    float SquaredMagnitude() const;
    CVector2f Normalized() const;
    float Dot(const CVector2f& rkOther) const;

    CVector2f operator+(const CVector2f& rkOther) const;
    CVector2f operator-(const CVector2f& rkOther) const;
    CVector2f operator*(const CVector2f& rkOther) const;
    CVector2f operator/(const CVector2f& rkOther) const;
    void operator+=(const CVector2f& rkOther);
    void operator-=(const CVector2f& rkOther);
    void operator*=(const CVector2f& rkOther);
    void operator/=(const CVector2f& rkOther);
    CVector2f operator+(const float Other) const;
    CVector2f operator-(const float Other) const;
    CVector2f operator*(const float Other) const;
    CVector2f operator/(const float Other) const;
    void operator+=(const float Other);
    void operator-=(const float Other);
    void operator*=(const float Other);
    void operator/=(const float Other);
    bool operator==(const CVector2f& rkOther) const;
    bool operator!=(const CVector2f& rkOther) const;
    CVector2f operator-() const;
    float& operator[](long Index);
    const float& operator[](long Index) const;

    // Static Members
    static const CVector2f skZero;
    static const CVector2f skOne;
    static const CVector2f skUp;
    static const CVector2f skRight;
    static const CVector2f skDown;
    static const CVector2f skLeft;
};

#endif // CVECTOR2F
