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
    float X, Y, Z, W;

    CVector4f();
    CVector4f(float XYZW);
    CVector4f(float _X, float _Y, float _Z, float _W);
    CVector4f(const CVector2f& rkXY, float _Z, float _W);
    CVector4f(const CVector3f& rkXYZ);
    CVector4f(const CVector3f& rkXYZ, float _W);
    CVector4f(IInputStream& rInput);
    void Write(IOutputStream& rOutput);

    // Swizzle
    CVector3f XYZ() const;
    CVector3f XZW() const;
    CVector3f YZW() const;
    CVector2f XY() const;
    CVector2f XZ() const;
    CVector2f XW() const;
    CVector2f YZ() const;
    CVector2f YW() const;
    CVector2f ZW() const;

    // Vector/Vector
    CVector4f operator+(const CVector4f& rkOther) const;
    CVector4f operator-(const CVector4f& rkOther) const;
    CVector4f operator*(const CVector4f& rkOther) const;
    CVector4f operator/(const CVector4f& rkOther) const;
    void operator+=(const CVector4f& rkOther);
    void operator-=(const CVector4f& rkOther);
    void operator*=(const CVector4f& rkOther);
    void operator/=(const CVector4f& rkOther);
    bool operator==(const CVector4f& rkOther) const;

    // Vector/Float
    CVector4f operator+(const float Other) const;
    CVector4f operator-(const float Other) const;
    CVector4f operator*(const float Other) const;
    CVector4f operator/(const float Other) const;
    void operator+=(const float Other);
    void operator-=(const float Other);
    void operator*=(const float Other);
    void operator/=(const float Other);

    // Vector/Matrix
    CVector4f operator*(const CTransform4f& rkMtx) const;
    void operator*=(const CTransform4f& rkMtx);
    CVector4f operator*(const CMatrix4f& rkMtx) const;
    void operator*=(const CMatrix4f& rkMtx);

    // Unary
    float& operator[](long Index);
};

#endif // CVECTOR4F
