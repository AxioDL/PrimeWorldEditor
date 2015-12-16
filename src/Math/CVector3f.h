#ifndef CVECTOR3F_H
#define CVECTOR3F_H

#include <FileIO/CInputStream.h>
#include <FileIO/COutputStream.h>
#include <ostream>

class CMatrix4f;
class CVector2f;
class CVector4f;
class CTransform4f;

class CVector3f
{
public:
    float x, y, z;

    CVector3f();
    CVector3f(float xyz);
    CVector3f(float _x, float _y, float _z);
    CVector3f(CInputStream& Input);
    void Write(COutputStream& Output);

    // Swizzle
    CVector2f xy();
    CVector2f xz();
    CVector2f yz();

    // Math
    float Magnitude() const;
    float SquaredMagnitude() const;
    CVector3f Normalized() const;
    float Dot(const CVector3f& other) const;
    CVector3f Cross(const CVector3f& other) const;
    float Distance(const CVector3f& point) const;
    float SquaredDistance(const CVector3f& point) const;

    // Vector/Vector
    CVector3f operator+(const CVector3f& other) const;
    CVector3f operator-(const CVector3f& other) const;
    CVector3f operator*(const CVector3f& other) const;
    CVector3f operator/(const CVector3f& other) const;
    void operator+=(const CVector3f& other);
    void operator-=(const CVector3f& other);
    void operator*=(const CVector3f& other);
    void operator/=(const CVector3f& other);
    bool operator> (const CVector3f& other) const;
    bool operator>=(const CVector3f& other) const;
    bool operator< (const CVector3f& other) const;
    bool operator<=(const CVector3f& other) const;
    bool operator==(const CVector3f& other) const;
    bool operator!=(const CVector3f& other) const;

    // Vector/Float
    CVector3f operator+(const float other) const;
    CVector3f operator-(const float other) const;
    CVector3f operator*(const float other) const;
    CVector3f operator/(const float other) const;
    void operator+=(const float other);
    void operator-=(const float other);
    void operator*=(const float other);
    void operator/=(const float other);

    // Vector/Matrix
    CVector3f operator*(const CTransform4f& mtx) const;
    void operator*=(const CTransform4f& mtx);
    CVector3f operator*(const CMatrix4f& mtx) const;

    // Unary
    CVector3f operator-() const;
    float& operator[](long index);
    const float& operator[](long index) const;

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
    friend std::ostream& operator<<(std::ostream& o, const CVector3f& Vector);
};

#endif // CVECTOR3F_H
