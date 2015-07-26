#ifndef CTRANSFORM4F_H
#define CTRANSFORM4F_H

#include <FileIO/FileIO.h>
#include "CMatrix4f.h"
#include <glm.hpp>

class CVector3f;
class CVector4f;
class CQuaternion;
class CMatrix4f;

class CTransform4f
{
    union
    {
        float m[3][4];
        float _m[12];
    };

public:
    CTransform4f();
    CTransform4f(CInputStream& input);
    CTransform4f(float v);
    CTransform4f(float m00, float m01, float m02, float m03,
                 float m10, float m11, float m12, float m13,
                 float m20, float m21, float m22, float m23);
    CTransform4f(CVector3f Position, CQuaternion Rotation, CVector3f Scale);
    CTransform4f(CVector3f Position, CVector3f Rotation, CVector3f Scale);

    // Math
    void Translate(CVector3f Translation);
    void Translate(float XTrans, float YTrans, float ZTrans);
    void Rotate(CQuaternion Rotation);
    void Rotate(CVector3f Rotation);
    void Rotate(float XRot, float YRot, float ZRot);
    void Scale(CVector3f Scale);
    void Scale(float XScale, float YScale, float ZScale);
    CTransform4f MultiplyIgnoreTranslation(const CTransform4f& mtx) const;
    CTransform4f Inverse() const;
    CTransform4f QuickInverse() const;
    CTransform4f NoTranslation() const;
    CTransform4f RotationOnly() const;

    // Conversion
    CMatrix4f ToMatrix4f() const;

    // Static
    static CTransform4f TranslationMatrix(CVector3f Translation);
    static CTransform4f RotationMatrix(CQuaternion Rotation);
    static CTransform4f ScaleMatrix(CVector3f Scale);
    static CTransform4f FromMatrix4f(const CMatrix4f& mtx);
    static CTransform4f FromGlmMat4(const glm::mat4& mtx);

    // Operators
    float* operator[](long index);
    const float* operator[](long index) const;
    CVector3f operator*(const CVector3f& vec) const;
    CVector4f operator*(const CVector4f& vec) const;
    CTransform4f operator*(const CTransform4f& mtx) const;
    void operator*=(const CTransform4f& mtx);
    bool operator==(const CTransform4f& mtx) const;
    bool operator!=(const CTransform4f& mtx) const;

    // Constant
    static const CTransform4f skIdentity;
    static const CTransform4f skZero;
};

#endif // CTRANSFORM4F_H
