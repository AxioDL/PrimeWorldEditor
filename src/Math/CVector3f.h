#ifndef CVECTOR3F_H
#define CVECTOR3F_H

#include <Common/FileIO/IInputStream.h>
#include <Common/FileIO/IOutputStream.h>
#include <Common/Serialization/IArchive.h>
#include <ostream>

class CMatrix4f;
class CVector2f;
class CVector4f;
class CTransform4f;

class CVector3f
{
public:
    float X, Y, Z;

    CVector3f();
    CVector3f(float XYZ);
    CVector3f(float _X, float _Y, float _Z);
    CVector3f(IInputStream& rInput);
    void Write(IOutputStream& rOutput) const;
    void Serialize(IArchive& rArc);
    TString ToString() const;

    // Swizzle
    CVector2f XY();
    CVector2f XZ();
    CVector2f YZ();

    // Math
    float Magnitude() const;
    float SquaredMagnitude() const;
    CVector3f Normalized() const;
    float Dot(const CVector3f& rkOther) const;
    CVector3f Cross(const CVector3f& rkOther) const;
    float Distance(const CVector3f& rkPoint) const;
    float SquaredDistance(const CVector3f& rkPoint) const;

    // Vector/Vector
    CVector3f operator+(const CVector3f& rkOther) const;
    CVector3f operator-(const CVector3f& rkOther) const;
    CVector3f operator*(const CVector3f& rkOther) const;
    CVector3f operator/(const CVector3f& rkOther) const;
    void operator+=(const CVector3f& rkOther);
    void operator-=(const CVector3f& rkOther);
    void operator*=(const CVector3f& rkOther);
    void operator/=(const CVector3f& rkOther);
    bool operator> (const CVector3f& rkOther) const;
    bool operator>=(const CVector3f& rkOther) const;
    bool operator< (const CVector3f& rkOther) const;
    bool operator<=(const CVector3f& rkOther) const;
    bool operator==(const CVector3f& rkOther) const;
    bool operator!=(const CVector3f& rkOther) const;

    // Vector/Float
    CVector3f operator+(const float Other) const;
    CVector3f operator-(const float Other) const;
    CVector3f operator*(const float Other) const;
    CVector3f operator/(const float Other) const;
    void operator+=(const float Other);
    void operator-=(const float Other);
    void operator*=(const float Other);
    void operator/=(const float Other);

    // Vector/Matrix
    CVector3f operator*(const CTransform4f& rkMtx) const;
    void operator*=(const CTransform4f& rkMtx);
    CVector3f operator*(const CMatrix4f& rkMtx) const;

    // Unary
    CVector3f operator-() const;
    float& operator[](long Index);
    const float& operator[](long Index) const;

    // Constants
    static const CVector3f skZero;
    static const CVector3f skOne;
    static const CVector3f skInfinite;
    static const CVector3f skUnitX;
    static const CVector3f skUnitY;
    static const CVector3f skUnitZ;
    static const CVector3f skRight;
    static const CVector3f skLeft;
    static const CVector3f skForward;
    static const CVector3f skBack;
    static const CVector3f skUp;
    static const CVector3f skDown;

    // Other
    friend std::ostream& operator<<(std::ostream& rOut, const CVector3f& rkVector);
};

#endif // CVECTOR3F_H
