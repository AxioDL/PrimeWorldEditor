#ifndef CMATRIX4_H
#define CMATRIX4_H

#include <glm.hpp>

class CVector3f;
class CVector4f;
class CTransform4f;

class CMatrix4f
{
    union
    {
        float m[4][4];
        float _m[16];
    };

public:
    CMatrix4f();
    CMatrix4f(float v);
    CMatrix4f(float m00, float m01, float m02, float m03,
              float m10, float m11, float m12, float m13,
              float m20, float m21, float m22, float m23,
              float m30, float m31, float m32, float m33);

    // Math
    CMatrix4f Transpose() const;
    CMatrix4f Inverse() const;
    float Determinant() const;

    // Conversion
    glm::mat4 ToGlmMat4() const;

    // Static
    static CMatrix4f FromGlmMat4(glm::mat4 src);

    // Operators
    inline float* operator[](long index);
    inline const float* operator[](long index) const;
    CVector3f operator*(const CVector3f& vec) const;
    CVector4f operator*(const CVector4f& vec) const;
    CMatrix4f operator*(const CTransform4f& mtx) const;
    CMatrix4f operator*(const CMatrix4f& mtx) const;

    // Constants
    static const CMatrix4f skZero;
    static const CMatrix4f skIdentity;
};

#endif // CMATRIX4_H
