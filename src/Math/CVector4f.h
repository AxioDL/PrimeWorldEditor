#ifndef CVECTOR4F
#define CVECTOR4F

#include <FileIO/IInputStream.h>
#include <FileIO/IOutputStream.h>

class CMatrix4f;
class CTransform4f;
class CVector2f;
class CVector3f;

class CVector4f
{
public:
    float x, y, z, w;

    CVector4f();
    CVector4f(float xyzw);
    CVector4f(float _x, float _y, float _z, float _w);
    CVector4f(const CVector2f& xy, float _z, float _w);
    CVector4f(const CVector3f& xyz);
    CVector4f(const CVector3f& xyz, float _w);
    CVector4f(IInputStream& Input);
    void Write(IOutputStream& Output);

    // Swizzle
    CVector3f xyz();
    CVector3f xzw();
    CVector3f yzw();
    CVector2f xy();
    CVector2f xz();
    CVector2f xw();
    CVector2f yz();
    CVector2f yw();
    CVector2f zw();

    // Vector/Vector
    CVector4f operator+(const CVector4f& other) const;
    CVector4f operator-(const CVector4f& other) const;
    CVector4f operator*(const CVector4f& other) const;
    CVector4f operator/(const CVector4f& other) const;
    void operator+=(const CVector4f& other);
    void operator-=(const CVector4f& other);
    void operator*=(const CVector4f& other);
    void operator/=(const CVector4f& other);
    bool operator==(const CVector4f& other) const;

    // Vector/Float
    CVector4f operator+(const float other) const;
    CVector4f operator-(const float other) const;
    CVector4f operator*(const float other) const;
    CVector4f operator/(const float other) const;
    void operator+=(const float other);
    void operator-=(const float other);
    void operator*=(const float other);
    void operator/=(const float other);

    // Vector/Matrix
    CVector4f operator*(const CTransform4f& mtx) const;
    void operator*=(const CTransform4f& mtx);
    CVector4f operator*(const CMatrix4f& mtx) const;
    void operator*=(const CMatrix4f& mtx);

    // Unary
    float& operator[](long index);
};

#endif // CVECTOR4F
